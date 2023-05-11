//
// Copyright (c) 2017-2020 the rbfx project.
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

#include "../Graphics/Graphics.h"
#include "../Graphics/Renderer.h"
#include "../IO/Log.h"
#include "../RenderPipeline/CameraProcessor.h"
#include "../RenderPipeline/ShaderConsts.h"
#include "../RenderPipeline/InstancingBuffer.h"
#include "../RenderPipeline/LightProcessor.h"
#include "../RenderPipeline/PipelineStateBuilder.h"
#include "../RenderPipeline/SceneProcessor.h"
#include "../RenderPipeline/ShadowMapAllocator.h"

#include <EASTL/span.h>

#include "../DebugNew.h"

namespace Urho3D
{

namespace
{

CullMode GetEffectiveCullMode(CullMode mode, bool isCameraReversed)
{
    if (mode == CULL_NONE || !isCameraReversed)
        return mode;

    return mode == CULL_CW ? CULL_CCW : CULL_CW;
}

CullMode GetEffectiveCullMode(CullMode passCullMode, CullMode materialCullMode, bool isCameraReversed)
{
    const CullMode cullMode = passCullMode != MAX_CULLMODES ? passCullMode : materialCullMode;
    return GetEffectiveCullMode(cullMode, isCameraReversed);
}

}

PipelineStateBuilder::PipelineStateBuilder(Context* context,
    const SceneProcessor* sceneProcessor, const CameraProcessor* cameraProcessor,
    const ShadowMapAllocator* shadowMapAllocator, const InstancingBuffer* instancingBuffer)
    : Object(context)
    , sceneProcessor_(sceneProcessor)
    , cameraProcessor_(cameraProcessor)
    , shadowMapAllocator_(shadowMapAllocator)
    , instancingBuffer_(instancingBuffer)
    , graphics_(GetSubsystem<Graphics>())
    , renderer_(GetSubsystem<Renderer>())
    , compositor_(MakeShared<ShaderProgramCompositor>(context_))
{
}

void PipelineStateBuilder::SetSettings(const ShaderProgramCompositorSettings& settings)
{
    compositor_->SetSettings(settings);
}

void PipelineStateBuilder::UpdateFrameSettings()
{
    compositor_->SetFrameSettings(cameraProcessor_);
}

SharedPtr<PipelineState> PipelineStateBuilder::CreateBatchPipelineState(
    const BatchStateCreateKey& key, const BatchStateCreateContext& ctx)
{
    Light* light = key.pixelLight_ ? key.pixelLight_->GetLight() : nullptr;
    const bool hasShadow = key.pixelLight_ && key.pixelLight_->HasShadow();

    BatchCompositorPass* batchCompositorPass = sceneProcessor_->GetUserPass(ctx.pass_);
    const bool isShadowPass = batchCompositorPass == nullptr && ctx.subpassIndex_ == BatchCompositor::ShadowSubpass;
    const bool isLightVolumePass = batchCompositorPass == nullptr && ctx.subpassIndex_ == BatchCompositor::LitVolumeSubpass;

    ClearState();

    if (isShadowPass)
    {
        compositor_->ProcessShadowBatch(shaderProgramDesc_,
            key.geometry_, key.geometryType_, key.material_, key.pass_, light);
        SetupShadowPassState(ctx.shadowSplitIndex_, key.pixelLight_, key.material_, key.pass_);

        SetupSamplersForUserOrShadowPass(key.material_, nullptr, false, false);
        SetupInputLayoutAndPrimitiveType(pipelineStateDesc_, shaderProgramDesc_, key.geometry_);
        SetupShaders(pipelineStateDesc_, shaderProgramDesc_);
    }
    else if (isLightVolumePass)
    {
        compositor_->ProcessLightVolumeBatch(shaderProgramDesc_,
            key.geometry_, key.geometryType_, key.pass_, light, hasShadow);
        SetupLightVolumePassState(key.pixelLight_);

        // TODO(diligent): Setup samplers for light volume pass
        SetupInputLayoutAndPrimitiveType(pipelineStateDesc_, shaderProgramDesc_, key.geometry_);
        SetupShaders(pipelineStateDesc_, shaderProgramDesc_);
    }
    else if (batchCompositorPass && batchCompositorPass->IsFlagSet(DrawableProcessorPassFlag::PipelineStateCallback))
    {
        batchCompositorPass->CreatePipelineState(pipelineStateDesc_, this, key, ctx);
    }
    else if (batchCompositorPass)
    {
        const auto subpass = static_cast<BatchCompositorSubpass>(ctx.subpassIndex_);
        const bool lightMaskToStencil = subpass == BatchCompositorSubpass::Deferred
            && batchCompositorPass->GetFlags().Test(DrawableProcessorPassFlag::DeferredLightMaskToStencil);
        const bool hasAmbient = batchCompositorPass->GetFlags().Test(DrawableProcessorPassFlag::HasAmbientLighting);
        const bool hasLightmap = key.drawable_->GetGlobalIlluminationType() == GlobalIlluminationType::UseLightMap;

        compositor_->ProcessUserBatch(shaderProgramDesc_, batchCompositorPass->GetFlags(),
            key.drawable_, key.geometry_, key.geometryType_, key.material_, key.pass_, light, hasShadow, subpass);
#ifdef  URHO3D_DEBUG
        pipelineStateDesc_.debugName_ = Format("DrawablePipeline({})",key.material_->GetName());
#endif
        SetupUserPassState(key.drawable_, key.material_, key.pass_, lightMaskToStencil);

        // Support negative lights
        if (light && light->IsNegative())
        {
            assert(subpass == BatchCompositorSubpass::Light);
            if (pipelineStateDesc_.blendMode_ == BLEND_ADD)
                pipelineStateDesc_.blendMode_ = BLEND_SUBTRACT;
            else if (pipelineStateDesc_.blendMode_ == BLEND_ADDALPHA)
                pipelineStateDesc_.blendMode_ = BLEND_SUBTRACTALPHA;
        }

        SetupSamplersForUserOrShadowPass(key.material_, key.pixelLight_, hasLightmap, hasAmbient);
        SetupInputLayoutAndPrimitiveType(pipelineStateDesc_, shaderProgramDesc_, key.geometry_);
        SetupShaders(pipelineStateDesc_, shaderProgramDesc_);
    }


    return renderer_->GetOrCreatePipelineState(pipelineStateDesc_);
}

void PipelineStateBuilder::ClearState()
{
    pipelineStateDesc_ = {};
    shaderProgramDesc_.Clear();
}

void PipelineStateBuilder::SetupShadowPassState(unsigned splitIndex, const LightProcessor* lightProcessor,
        const Material* material, const Pass* pass)
{
    const CookedLightParams& lightParams = lightProcessor->GetParams();
    const float biasMultiplier = lightParams.shadowDepthBiasMultiplier_[splitIndex];
    const BiasParameters& biasParameters = lightProcessor->GetLight()->GetShadowBias();

#ifdef URHO3D_DEBUG
    pipelineStateDesc_.debugName_ = Format("ShadowPass({})|Split: {}", pass->GetName(), splitIndex);
#endif
    if (shadowMapAllocator_->GetSettings().enableVarianceShadowMaps_)
    {
        pipelineStateDesc_.colorWriteEnabled_ = true;
        pipelineStateDesc_.constantDepthBias_ = 0.0f;
        pipelineStateDesc_.slopeScaledDepthBias_ = 0.0f;
        pipelineStateDesc_.depthStencilFormat_ = Graphics::GetRGFloat32Format();
    }
    else
    {
        pipelineStateDesc_.colorWriteEnabled_ = false;
        pipelineStateDesc_.constantDepthBias_ = biasMultiplier * biasParameters.constantBias_;
        pipelineStateDesc_.slopeScaledDepthBias_ = biasMultiplier * biasParameters.slopeScaledBias_;

#ifdef GL_ES_VERSION_2_0
        const float multiplier = renderer_->GetMobileShadowBiasMul();
        const float addition = renderer_->GetMobileShadowBiasAdd();
        desc.constantDepthBias_ = desc.constantDepthBias_ * multiplier + addition;
        desc.slopeScaledDepthBias_ *= multiplier;
#endif
        pipelineStateDesc_.depthStencilFormat_ = shadowMapAllocator_->GetSettings().use16bitShadowMaps_
            ? graphics_->GetShadowMapFormat()
            : graphics_->GetHiresShadowMapFormat();
    }

    pipelineStateDesc_.depthWriteEnabled_ = pass->GetDepthWrite();
    pipelineStateDesc_.depthCompareFunction_ = pass->GetDepthTestMode();

    pipelineStateDesc_.cullMode_ = GetEffectiveCullMode(pass->GetCullMode(), material->GetShadowCullMode(), false);
}

void PipelineStateBuilder::SetupLightVolumePassState(const LightProcessor* lightProcessor)
{
    const Light* light = lightProcessor->GetLight();
#if defined URHO3D_DEBUG
    pipelineStateDesc_.debugName_ = "LightVolumePass";
#endif
    pipelineStateDesc_.colorWriteEnabled_ = true;
    pipelineStateDesc_.blendMode_ = light->IsNegative() ? BLEND_SUBTRACT : BLEND_ADD;

    if (light->GetLightType() != LIGHT_DIRECTIONAL)
    {
        if (lightProcessor->DoesOverlapCamera())
        {
            pipelineStateDesc_.cullMode_ = GetEffectiveCullMode(CULL_CW, cameraProcessor_->IsCameraReversed());
            pipelineStateDesc_.depthCompareFunction_ = CMP_GREATER;
        }
        else
        {
            pipelineStateDesc_.cullMode_ = GetEffectiveCullMode(CULL_CCW, cameraProcessor_->IsCameraReversed());
            pipelineStateDesc_.depthCompareFunction_ = CMP_LESSEQUAL;
        }
    }
    else
    {
        pipelineStateDesc_.cullMode_ = CULL_NONE;
        pipelineStateDesc_.depthCompareFunction_ = CMP_ALWAYS;
    }

    pipelineStateDesc_.stencilTestEnabled_ = true;
    pipelineStateDesc_.stencilCompareFunction_ = CMP_NOTEQUAL;
    pipelineStateDesc_.stencilCompareMask_ = light->GetLightMaskEffective() & PORTABLE_LIGHTMASK;
    pipelineStateDesc_.stencilReferenceValue_ = 0;
}

void PipelineStateBuilder::SetupUserPassState(const Drawable* drawable,
    const Material* material, const Pass* pass, bool lightMaskToStencil)
{
    // TODO(refactor): Change this or refactor code in a deal way
    pipelineStateDesc_.renderTargetsFormats_.resize(1);
    pipelineStateDesc_.renderTargetsFormats_[0] = graphics_->GetRGBAFormat();
    pipelineStateDesc_.depthStencilFormat_ = graphics_->GetSwapChainDepthFormat();

    pipelineStateDesc_.depthWriteEnabled_ = pass->GetDepthWrite();
    pipelineStateDesc_.depthCompareFunction_ = pass->GetDepthTestMode();

    pipelineStateDesc_.colorWriteEnabled_ = pass->GetColorWrite();
    pipelineStateDesc_.blendMode_ = pass->GetBlendMode();
    pipelineStateDesc_.alphaToCoverageEnabled_ = pass->GetAlphaToCoverage() || material->GetAlphaToCoverage();
    pipelineStateDesc_.constantDepthBias_ = material->GetDepthBias().constantBias_;
    pipelineStateDesc_.slopeScaledDepthBias_ = material->GetDepthBias().slopeScaledBias_;

    pipelineStateDesc_.fillMode_ = ea::max(cameraProcessor_->GetCameraFillMode(), material->GetFillMode());
    pipelineStateDesc_.cullMode_ = GetEffectiveCullMode(pass->GetCullMode(),
        material->GetCullMode(), cameraProcessor_->IsCameraReversed());

    if (lightMaskToStencil)
    {
        pipelineStateDesc_.stencilTestEnabled_ = true;
        pipelineStateDesc_.stencilOperationOnPassed_ = OP_REF;
        pipelineStateDesc_.stencilWriteMask_ = PORTABLE_LIGHTMASK;
        pipelineStateDesc_.stencilReferenceValue_ = drawable->GetLightMaskInZone() & PORTABLE_LIGHTMASK;
    }
}

void PipelineStateBuilder::SetupInputLayoutAndPrimitiveType(
    PipelineStateDesc& pipelineStateDesc, const ShaderProgramDesc& shaderProgramDesc, const Geometry* geometry) const
{
    if (shaderProgramDesc.isInstancingUsed_)
        pipelineStateDesc.InitializeInputLayoutAndPrimitiveType(geometry, instancingBuffer_->GetVertexBuffer());
    else
        pipelineStateDesc.InitializeInputLayoutAndPrimitiveType(geometry);
}

void PipelineStateBuilder::SetupShaders(PipelineStateDesc& pipelineStateDesc, ShaderProgramDesc& shaderProgramDesc) const
{
    for (ea::string& shaderDefines : shaderProgramDesc.shaderDefines_)
        shaderDefines += shaderProgramDesc.commonShaderDefines_;

    pipelineStateDesc.vertexShader_ = graphics_->GetShader(
        VS, shaderProgramDesc.shaderName_[VS], shaderProgramDesc.shaderDefines_[VS]);
    pipelineStateDesc.pixelShader_ = graphics_->GetShader(
        PS, shaderProgramDesc.shaderName_[PS], shaderProgramDesc.shaderDefines_[PS]);
}

void PipelineStateBuilder::SetupSamplersForUserOrShadowPass(
    const Material* material, const LightProcessor* lightProcessor, bool hasLightmap, bool hasAmbient)
{
    // TODO(diligent): Make configurable
    static const SamplerStateDesc lightMapSampler{};
    static const SamplerStateDesc reflectionMapSampler{};

    const Light* light = lightProcessor ? lightProcessor->GetLight() : nullptr;
    bool materialHasEnvironmentMap = false;
    for (const auto& [unit, texture] : material->GetTextures())
    {
        if (texture)
        {
            const StringHash textureName = Material::TextureUnitToShaderResource(unit);
            if (textureName == ShaderResources::EmissiveMap && hasLightmap)
                continue;
            if (textureName == ShaderResources::EnvMap)
                materialHasEnvironmentMap = true;
            pipelineStateDesc_.AddSampler(textureName, texture->GetSamplerStateDesc());
        }
    }

    if (hasLightmap)
        pipelineStateDesc_.AddSampler(ShaderResources::EmissiveMap, lightMapSampler);
    if (light)
    {
        if (Texture* rampTexture = light->GetRampTexture())
            pipelineStateDesc_.AddSampler(ShaderResources::LightRampMap, rampTexture->GetSamplerStateDesc());
        if (Texture* shapeTexture = light->GetShapeTexture())
            pipelineStateDesc_.AddSampler(ShaderResources::LightRampMap, shapeTexture->GetSamplerStateDesc());
    }
    if (lightProcessor && lightProcessor->HasShadow())
        pipelineStateDesc_.AddSampler(ShaderResources::ShadowMap, shadowMapAllocator_->GetSamplerStateDesc());

    if (hasAmbient)
    {
        pipelineStateDesc_.AddSampler(ShaderResources::EnvMap, reflectionMapSampler);
        pipelineStateDesc_.AddSampler(ShaderResources::ZoneCubeMap, reflectionMapSampler);
    }
}

}
