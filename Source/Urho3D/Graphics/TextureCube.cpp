//
// Copyright (c) 2008-2022 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../Precompiled.h"

#include "../Core/Context.h"
#include "../Core/Profiler.h"
#include "../Graphics/Graphics.h"
#include "../Graphics/GraphicsEvents.h"
#include "../Graphics/GraphicsImpl.h"
#include "../Graphics/Renderer.h"
#include "../Graphics/TextureCube.h"
#include "../IO/FileSystem.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/XMLFile.h"

#include "../DebugNew.h"

#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

namespace Urho3D
{

static const char* cubeMapLayoutNames[] = {
    "horizontal",
    "horizontalnvidia",
    "horizontalcross",
    "verticalcross",
    "blender",
    nullptr
};

static SharedPtr<Image> GetTileImage(Image* src, int tileX, int tileY, int tileWidth, int tileHeight)
{
    return SharedPtr<Image>(
        src->GetSubimage(IntRect(tileX * tileWidth, tileY * tileHeight, (tileX + 1) * tileWidth, (tileY + 1) * tileHeight)));
}

TextureCube::TextureCube(Context* context)
    : Texture(context)
{
    SetSamplerStateDesc(SamplerStateDesc::Bilinear(ADDRESS_CLAMP));
}

TextureCube::~TextureCube()
{
}

void TextureCube::RegisterObject(Context* context)
{
    context->AddFactoryReflection<TextureCube>();
}

bool TextureCube::BeginLoad(Deserializer& source)
{
    auto* cache = GetSubsystem<ResourceCache>();

    // In headless mode, do not actually load the texture, just return success
    if (!graphics_)
        return true;

    cache->ResetDependencies(this);

    // Load resource
    loadImageCube_ = cache->GetTempResource<ImageCube>(GetName());
    if (!loadImageCube_)
        return false;

    // Update dependencies
    for (Image* image : loadImageCube_->GetImages())
    {
        if (image && !image->GetName().empty())
            cache->StoreResourceDependency(this, image->GetName());
    }

    return true;
}

bool TextureCube::EndLoad()
{
    // In headless mode, do not actually load the texture, just return success
    if (!graphics_ || graphics_->IsDeviceLost())
        return true;

    // If over the texture budget, see if materials can be freed to allow textures to be freed
    CheckTextureBudget(GetTypeStatic());

    SetParameters(loadImageCube_->GetParametersXML());

    const auto& images = loadImageCube_->GetImages();
    for (unsigned i = 0; i < images.size() && i < MAX_CUBEMAP_FACES; ++i)
        SetData((CubeMapFace)i, images[i]);

    loadImageCube_ = nullptr;

    return true;
}

bool TextureCube::SetSize(int size, TextureFormat format, TextureFlags flags, int multiSample)
{
    RawTextureParams params;
    params.type_ = TextureType::TextureCube;
    params.format_ = format;
    params.size_ = {size, size, 1};
    params.numLevels_ = requestedLevels_;
    params.flags_ = flags;
    params.multiSample_ = multiSample;

    return Create(params);
}

SharedPtr<Image> TextureCube::GetImage(CubeMapFace face) const
{
#if 0
    if (format_ != Graphics::GetRGBAFormat() && format_ != Graphics::GetRGBFormat())
    {
        URHO3D_LOGERROR("Unsupported texture format, can not convert to Image");
        return SharedPtr<Image>();
    }

    auto rawImage = MakeShared<Image>(context_);
    if (format_ == Graphics::GetRGBAFormat())
        rawImage->SetSize(width_, height_, 4);
    else if (format_ == Graphics::GetRGBFormat())
        rawImage->SetSize(width_, height_, 3);
    else
        assert(false);

    GetData(face, 0, rawImage->GetData());
    return SharedPtr<Image>(rawImage);
#endif
    return nullptr;
}

}
