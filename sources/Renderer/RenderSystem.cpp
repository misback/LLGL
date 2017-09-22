/*
 * RenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../Platform/Module.h"
#include "../Core/Helper.h"
#include <LLGL/Platform/Platform.h>
#include <LLGL/Log.h>
#include "BuildID.h"

#include <LLGL/RenderSystem.h>
#include <array>
#include <map>

#ifdef LLGL_ENABLE_DEBUG_LAYER
#   include "DebugLayer/DbgRenderSystem.h"
#endif

#ifdef LLGL_BUILD_STATIC_LIB
#   include "ModuleInterface.h"
#endif


namespace LLGL
{


/* ----- Render system ----- */

static std::map<RenderSystem*, std::unique_ptr<Module>> g_renderSystemModules;

RenderSystem::~RenderSystem()
{
}

std::vector<std::string> RenderSystem::FindModules()
{
    /* Iterate over all known modules and return those that are available on the current platform */
    const std::vector<std::string> knownModules
    {
        #if defined(LLGL_OS_IOS) || defined(LLGL_OS_ANDROID)
        "OpenGLES2",
        #else
        "OpenGL",
        #endif
        
        #if defined(LLGL_OS_MACOS) || defined(LLGL_OS_IOS)
        "Metal",
        #else
        "Vulkan",
        #endif

        #ifdef LLGL_OS_WIN32
        "Direct3D11",
        "Direct3D12",
        #endif
    };
    
    std::vector<std::string> modules;
    
    for (const auto& m : knownModules)
    {
        if (Module::IsAvailable(Module::GetModuleFilename(m)))
            modules.push_back(m);
    }
    
    return modules;
}

#ifndef LLGL_BUILD_STATIC_LIB

static bool LoadRenderSystemBuildID(Module& module, const std::string& moduleFilename)
{
    /* Load "LLGL_RenderSystem_BuildID" procedure */
    LLGL_PROC_INTERFACE(int, PFN_RENDERSYSTEM_BUILDID, (void));

    auto RenderSystem_BuildID = reinterpret_cast<PFN_RENDERSYSTEM_BUILDID>(module.LoadProcedure("LLGL_RenderSystem_BuildID"));
    if (!RenderSystem_BuildID)
        throw std::runtime_error("failed to load \"LLGL_RenderSystem_BuildID\" procedure from module \"" + moduleFilename + "\"");

    return (RenderSystem_BuildID() == LLGL_BUILD_ID);
}

static int LoadRenderSystemRendererID(Module& module)
{
    /* Load "LLGL_RenderSystem_RendererID" procedure */
    LLGL_PROC_INTERFACE(int, PFN_RENDERSYSTEM_RENDERERID, (void));

    auto RenderSystem_RendererID = reinterpret_cast<PFN_RENDERSYSTEM_RENDERERID>(module.LoadProcedure("LLGL_RenderSystem_RendererID"));
    if (RenderSystem_RendererID)
        return RenderSystem_RendererID();

    return RendererID::Undefined;
}

static std::string LoadRenderSystemName(Module& module)
{
    /* Load "LLGL_RenderSystem_Name" procedure */
    LLGL_PROC_INTERFACE(const char*, PFN_RENDERSYSTEM_NAME, (void));

    auto RenderSystem_Name = reinterpret_cast<PFN_RENDERSYSTEM_NAME>(module.LoadProcedure("LLGL_RenderSystem_Name"));
    if (RenderSystem_Name)
        return std::string(RenderSystem_Name());

    return "";
}

static RenderSystem* LoadRenderSystem(Module& module, const std::string& moduleFilename)
{
    /* Load "LLGL_RenderSystem_Alloc" procedure */
    LLGL_PROC_INTERFACE(void*, PFN_RENDERSYSTEM_ALLOC, (void));

    auto RenderSystem_Alloc = reinterpret_cast<PFN_RENDERSYSTEM_ALLOC>(module.LoadProcedure("LLGL_RenderSystem_Alloc"));
    if (!RenderSystem_Alloc)
        throw std::runtime_error("failed to load \"LLGL_RenderSystem_Alloc\" procedure from module \"" + moduleFilename + "\"");

    return reinterpret_cast<RenderSystem*>(RenderSystem_Alloc());
}

#endif

std::unique_ptr<RenderSystem> RenderSystem::Load(
    const std::string& moduleName, RenderingProfiler* profiler, RenderingDebugger* debugger)
{
    #ifdef LLGL_BUILD_STATIC_LIB

    /*
    Verify build ID from render system module to detect a module,
    that has compiled with a different compiler (type, version, debug/release mode etc.)
    */
    if (LLGL_RenderSystem_BuildID() != LLGL_BUILD_ID)
        throw std::runtime_error("build ID mismatch in render system module");

    /* Allocate render system */
    auto renderSystem   = std::unique_ptr<RenderSystem>(reinterpret_cast<RenderSystem*>(LLGL_RenderSystem_Alloc()));

    if (profiler != nullptr || debugger != nullptr)
    {
        #ifdef LLGL_ENABLE_DEBUG_LAYER

        /* Create debug layer render system */
        renderSystem = MakeUnique<DbgRenderSystem>(std::move(renderSystem), profiler, debugger);

        #else

        Log::StdErr() << "LLGL was not compiled with debug layer support" << std::endl;

        #endif
    }

    renderSystem->name_         = LLGL_RenderSystem_Name();
    renderSystem->rendererID_   = LLGL_RenderSystem_RendererID();

    /* Return new render system and unique pointer */
    return renderSystem;

    #else

    /* Load render system module */
    auto moduleFilename = Module::GetModuleFilename(moduleName);
    auto module         = Module::Load(moduleFilename);

    /*
    Verify build ID from render system module to detect a module,
    that has compiled with a different compiler (type, version, debug/release mode etc.)
    */
    if (!LoadRenderSystemBuildID(*module, moduleFilename))
        throw std::runtime_error("build ID mismatch in render system module");

    try
    {
        /* Allocate render system */
        auto renderSystem   = std::unique_ptr<RenderSystem>(LoadRenderSystem(*module, moduleFilename));

        if (profiler != nullptr || debugger != nullptr)
        {
            #ifdef LLGL_ENABLE_DEBUG_LAYER

            /* Create debug layer render system */
            renderSystem = MakeUnique<DbgRenderSystem>(std::move(renderSystem), profiler, debugger);

            #else

            Log::StdErr() << "LLGL was not compiled with debug layer support" << std::endl;

            #endif
        }

        renderSystem->name_         = LoadRenderSystemName(*module);
        renderSystem->rendererID_   = LoadRenderSystemRendererID(*module);

        /* Store new module inside internal map */
        g_renderSystemModules[renderSystem.get()] = std::move(module);

        /* Return new render system and unique pointer */
        return renderSystem;
    }
    catch (const std::exception&)
    {
        /* Keep module, otherwise the exception 's vtable might be corrupted because it's part of the module */
        g_renderSystemModules[nullptr] = std::move(module);
        throw;
    }

    #endif
}

void RenderSystem::Unload(std::unique_ptr<RenderSystem>&& renderSystem)
{
    auto it = g_renderSystemModules.find(renderSystem.get());
    if (it != g_renderSystemModules.end())
    {
        renderSystem.release();
        g_renderSystemModules.erase(it);
    }
}

void RenderSystem::SetConfiguration(const RenderSystemConfiguration& config)
{
    config_ = config;
}


/*
 * ======= Protected: =======
 */

void RenderSystem::SetRendererInfo(const RendererInfo& info)
{
    info_ = info;
}

void RenderSystem::SetRenderingCaps(const RenderingCaps& caps)
{
    caps_ = caps;
}

std::vector<ColorRGBAub> RenderSystem::GetDefaultTextureImageRGBAub(int numPixels) const
{
    return std::vector<ColorRGBAub>(static_cast<size_t>(numPixels), GetConfiguration().imageInitialization.color);
}

void RenderSystem::AssertCreateBuffer(const BufferDescriptor& desc)
{
    if (desc.type < BufferType::Vertex || desc.type > BufferType::StreamOutput)
        throw std::invalid_argument("cannot create buffer of unknown type (0x" + ToHex(static_cast<unsigned char>(desc.type)) + ")");
}

static void AssertCreateResourceArrayCommon(unsigned int numResources, void* const * resourceArray, const std::string& resourceName)
{
    /* Validate number of buffers */
    if (numResources == 0)
        throw std::invalid_argument("cannot create " + resourceName + " array with zero " + resourceName + "s");

    /* Validate array pointer */
    if (resourceArray == nullptr)
        throw std::invalid_argument("cannot create " + resourceName + " array with invalid array pointer");
    
    /* Validate pointers in array */
    for (unsigned int i = 0; i < numResources; ++i)
    {
        if (resourceArray[i] == nullptr)
            throw std::invalid_argument("cannot create " + resourceName + " array with invalid pointer in array");
    }
}

void RenderSystem::AssertCreateBufferArray(unsigned int numBuffers, Buffer* const * bufferArray)
{
    /* Validate common resource array parameters */
    AssertCreateResourceArrayCommon(numBuffers, reinterpret_cast<void* const*>(bufferArray), "buffer");
    
    /* Validate buffer types */
    auto refType = bufferArray[0]->GetType();
    for (unsigned int i = 1; i < numBuffers; ++i)
    {
        if (bufferArray[i]->GetType() != refType)
            throw std::invalid_argument("cannot create buffer array with type mismatch");
    }

    /* Validate buffer array type */
    if ( refType != BufferType::Vertex      &&
         refType != BufferType::Constant    &&
         refType != BufferType::Storage     &&
         refType != BufferType::StreamOutput )
    {
        throw std::invalid_argument("invalid buffer type for buffer array");
    }
}

void RenderSystem::AssertCreateTextureArray(unsigned int numTextures, Texture* const * textureArray)
{
    /* Validate common resource array parameters */
    AssertCreateResourceArrayCommon(numTextures, reinterpret_cast<void* const*>(textureArray), "texture");
}

void RenderSystem::AssertCreateSamplerArray(unsigned int numSamplers, Sampler* const * samplerArray)
{
    /* Validate common resource array parameters */
    AssertCreateResourceArrayCommon(numSamplers, reinterpret_cast<void* const*>(samplerArray), "sampler");
}


} // /namespace LLGL



// ================================================================================
