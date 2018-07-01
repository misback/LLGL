/*
 * Utility.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef LLGL_ENABLE_UTILITY

#include <LLGL/Utility.h>
#include <LLGL/Buffer.h>
#include <LLGL/Texture.h>
#include <LLGL/Sampler.h>
#include <cstring>


namespace LLGL
{


/* ----- TextureDescriptor utility functions ----- */

LLGL_EXPORT TextureDescriptor Texture1DDesc(Format format, std::uint32_t width, long flags)
{
    TextureDescriptor desc;
    {
        desc.type   = TextureType::Texture1D;
        desc.format = format;
        desc.flags  = flags;
        desc.width  = width;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DDesc(Format format, std::uint32_t width, std::uint32_t height, long flags)
{
    TextureDescriptor desc;
    {
        desc.type   = TextureType::Texture2D;
        desc.format = format;
        desc.flags  = flags;
        desc.width  = width;
        desc.height = height;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture3DDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t depth, long flags)
{
    TextureDescriptor desc;
    {
        desc.type   = TextureType::Texture3D;
        desc.format = format;
        desc.flags  = flags;
        desc.width  = width;
        desc.height = height;
        desc.depth  = depth;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor TextureCubeDesc(Format format, std::uint32_t width, std::uint32_t height, long flags)
{
    TextureDescriptor desc;
    {
        desc.type   = TextureType::TextureCube;
        desc.format = format;
        desc.flags  = flags;
        desc.width  = width;
        desc.height = height;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture1DArrayDesc(Format format, std::uint32_t width, std::uint32_t layers, long flags)
{
    TextureDescriptor desc;
    {
        desc.type   = TextureType::Texture1DArray;
        desc.format = format;
        desc.flags  = flags;
        desc.width  = width;
        desc.layers = layers;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t layers, long flags)
{
    TextureDescriptor desc;
    {
        desc.type   = TextureType::Texture2DArray;
        desc.format = format;
        desc.flags  = flags;
        desc.width  = width;
        desc.height = height;
        desc.layers = layers;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor TextureCubeArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t layers, long flags)
{
    TextureDescriptor desc;
    {
        desc.type   = TextureType::TextureCubeArray;
        desc.format = format;
        desc.flags  = flags;
        desc.width  = width;
        desc.height = height;
        desc.layers = layers;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DMSDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t samples, long flags)
{
    TextureDescriptor desc;
    {
        desc.type       = TextureType::Texture2DMS;
        desc.format     = format;
        desc.flags      = flags;
        desc.width      = width;
        desc.height     = height;
        desc.samples    = samples;
    }
    return desc;
}

LLGL_EXPORT TextureDescriptor Texture2DMSArrayDesc(Format format, std::uint32_t width, std::uint32_t height, std::uint32_t layers, std::uint32_t samples, long flags)
{
    TextureDescriptor desc;
    {
        desc.type       = TextureType::Texture2DMSArray;
        desc.format     = format;
        desc.flags      = flags;
        desc.width      = width;
        desc.height     = height;
        desc.layers     = layers;
        desc.samples    = samples;
    }
    return desc;
}

/* ----- BufferDescriptor utility functions ----- */

LLGL_EXPORT BufferDescriptor VertexBufferDesc(std::uint64_t size, const VertexFormat& vertexFormat, long flags)
{
    BufferDescriptor desc;
    {
        desc.type                   = BufferType::Vertex;
        desc.size                   = size;
        desc.flags                  = flags;
        desc.vertexBuffer.format    = vertexFormat;
    }
    return desc;
}

LLGL_EXPORT BufferDescriptor IndexBufferDesc(std::uint64_t size, const IndexFormat& indexFormat, long flags)
{
    BufferDescriptor desc;
    {
        desc.type               = BufferType::Index;
        desc.size               = size;
        desc.flags              = flags;
        desc.indexBuffer.format = indexFormat;
    }
    return desc;
}

LLGL_EXPORT BufferDescriptor ConstantBufferDesc(std::uint64_t size, long flags)
{
    BufferDescriptor desc;
    {
        desc.type   = BufferType::Constant;
        desc.size   = size;
        desc.flags  = flags;
    }
    return desc;
}

LLGL_EXPORT BufferDescriptor StorageBufferDesc(std::uint64_t size, const StorageBufferType storageType, std::uint32_t stride, long flags)
{
    BufferDescriptor desc;
    {
        desc.type                       = BufferType::Storage;
        desc.size                       = size;
        desc.flags                      = flags;
        desc.storageBuffer.storageType  = storageType;
        desc.storageBuffer.stride       = stride;
    }
    return desc;
}

/* ----- ShaderDescriptor utility functions ----- */

LLGL_EXPORT ShaderDescriptor ShaderDescFromFile(const ShaderType type, const char* filename, const char* entryPoint, const char* profile, long flags)
{
    ShaderDescriptor desc;

    if (filename != nullptr)
    {
        if (auto fileExt = std::strrchr(filename, '.'))
        {
            /* Check if filename refers to a text-based source file */
            bool isTextFile = false;

            for (auto ext : { "hlsl", "fx", "glsl", "vert", "tesc", "tese", "geom", "comp" })
            {
                if (std::strcmp(fileExt + 1, ext) == 0)
                {
                    isTextFile = true;
                    break;
                }
            }

            /* Initialize descriptor */
            desc.type       = type;
            desc.source     = filename;
            desc.sourceSize = 0;
            desc.sourceType = (isTextFile ? ShaderSourceType::CodeFile : ShaderSourceType::BinaryFile);
            desc.entryPoint = entryPoint;
            desc.profile    = profile;
            desc.flags      = flags;
        }
    }

    return desc;
}


} // /namespace LLGL

#endif



// ================================================================================
