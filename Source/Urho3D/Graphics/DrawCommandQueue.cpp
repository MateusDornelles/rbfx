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
#include "../Graphics/DrawCommandQueue.h"
#include "../Graphics/RenderSurface.h"

#include "../DebugNew.h"

namespace Urho3D
{

DrawCommandQueue::DrawCommandQueue(Graphics* graphics)
    : graphics_(graphics)
    , currentCBufferTicket_(nullptr)
{
    srbCache_ = MakeShared<ShaderResourceBindingCache>(graphics->GetContext());
    cbufferManager_ = graphics->GetSubsystem<ConstantBufferManager>();
}

void DrawCommandQueue::Reset()
{
    // Reset state accumulators
    currentDrawCommand_ = {};
    currentShaderResourceGroup_ = {};

    // Clear shadep parameters
    constantBuffers_.collection_.ClearAndInitialize(graphics_->GetCaps().constantBufferOffsetAlignment_);
    constantBuffers_.currentLayout_ = nullptr;
    constantBuffers_.currentData_ = nullptr;
    constantBuffers_.currentHashes_.fill(0);

    currentDrawCommand_.constantBuffers_.fill({});

    currentDrawCommand_.cbufferTicketIds_.fill(M_MAX_UNSIGNED);

    // Clear arrays and draw commands
    shaderResources_.clear();
    drawCommands_.clear();
    scissorRects_.clear();
    scissorRects_.push_back(IntRect::ZERO);
}

void DrawCommandQueue::Execute()
{
    if (drawCommands_.empty())
        return;

    // Constant buffers to store all shader parameters for queue
    ea::vector<SharedPtr<ConstantBuffer>> constantBuffers;

    // Cached current state
    PipelineState* currentPipelineState = nullptr;
    ShaderProgramLayout* currentShaderReflection = nullptr;
    IndexBuffer* currentIndexBuffer = nullptr;
    ea::array<VertexBuffer*, MAX_VERTEX_STREAMS> currentVertexBuffers{};
    ShaderResourceRange currentShaderResources;
    PrimitiveType currentPrimitiveType{};
    unsigned currentScissorRect = M_MAX_UNSIGNED;

    cbufferManager_->PrepareBuffers();

    // Temporary collections
    ea::vector<VertexBuffer*> tempVertexBuffers;
    ea::array<ConstantBufferRange, MAX_SHADER_PARAMETER_GROUPS> constantBufferRanges{};

    for (const DrawCommandDescription& cmd : drawCommands_)
    {
        // Set pipeline state
        if (cmd.pipelineState_ != currentPipelineState)
        {
            // Skip this pipeline if something goes wrong.
            if (!cmd.pipelineState_->Apply(graphics_))
                continue;
            currentPipelineState = cmd.pipelineState_;
            currentShaderReflection = cmd.pipelineState_->GetReflection();
            currentPrimitiveType = currentPipelineState->GetDesc().primitiveType_;
            // Reset current shader resources because of HasTextureUnit check below
            currentShaderResources = {};
        }

        // Set scissor
        if (cmd.scissorRect_ != currentScissorRect)
        {
            const bool scissorEnabled = currentPipelineState->GetDesc().scissorTestEnabled_;
            graphics_->SetScissorTest(scissorEnabled, scissorRects_[cmd.scissorRect_]);
            currentScissorRect = cmd.scissorRect_;
        }

        // Set index buffer
        if (cmd.inputBuffers_.indexBuffer_ != currentIndexBuffer)
        {
            graphics_->SetIndexBuffer(cmd.inputBuffers_.indexBuffer_);
            currentIndexBuffer = cmd.inputBuffers_.indexBuffer_;
        }

        // Set vertex buffers
        if (cmd.inputBuffers_.vertexBuffers_ != currentVertexBuffers || cmd.instanceCount_ != 0)
        {
            tempVertexBuffers.clear();
            tempVertexBuffers.assign(cmd.inputBuffers_.vertexBuffers_.begin(), cmd.inputBuffers_.vertexBuffers_.end());
            // If something goes wrong here, skip current command
            if (!graphics_->SetVertexBuffers(tempVertexBuffers, cmd.instanceStart_))
                continue;
            currentVertexBuffers = cmd.inputBuffers_.vertexBuffers_;
        }

        // TODO(diligent): Revisit
        ShaderResourceBindingCacheCreateInfo ci;
        ci.pipeline_ = cmd.pipelineState_;
        /*bool stopDebugger = false;
        if (cmd.pipelineState_->GetDesc().debugName_ == ea::string("DrawablePipeline - Materials/StoneTiled.xml"))
            stopDebugger = true;*/
        // On Diligent backend, we need to create Shader Resource Binding and attach on device context.
        {
            //if (cmd.pipelineState_ != srbCacheCI.pipeline_) {
            //    dirty = true;
            //    srbCacheCI.pipeline_ = cmd.pipelineState_;
            //}
            //// Set shader resources
            //if (cmd.shaderResources_ != currentShaderResources)
            //{
            //    dirty = true;
            //    srbCacheCI.ResetTextures();
            //    for (unsigned i = cmd.shaderResources_.first; i < cmd.shaderResources_.second; ++i)
            //    {
            //        const auto& unitAndResource = shaderResources_[i];
            //        srbCacheCI.textures_[unitAndResource.unit_] = unitAndResource.texture_;
            //    }
            //    currentShaderResources = cmd.shaderResources_;
            //}

            //// Update used ranges for each group
            //for (unsigned i = 0; i < MAX_SHADER_PARAMETER_GROUPS; ++i)
            //{
            //    // Check for constant buffer bindings
            //    if (cmd.constantBuffers_[i].size_ == 0 && srbCacheCI.constantBuffers_[i]) {
            //        dirty = true;
            //        srbCacheCI.constantBuffers_[i] = nullptr;
            //        continue;
            //    }

            //    ConstantBuffer* cbuffer = constantBuffers[cmd.constantBuffers_[i].index_];
            //    if (cbuffer != srbCacheCI.constantBuffers_[i]) {
            //        dirty = true;
            //        srbCacheCI.constantBuffers_[i] = cbuffer;
            //    }
            //}
            for (unsigned i = cmd.shaderResources_.first; i < cmd.shaderResources_.second; ++i) {
                const auto& nameAndResource = shaderResources_[i];
                Texture* texture = nameAndResource.texture_;
                if (texture && currentShaderReflection->GetShaderResource(nameAndResource.name_))
                {
                    RenderSurface* currRT = graphics_->GetRenderTarget(0);
                    if (currRT && currRT->GetParentTexture() == texture)
                        texture = texture->GetBackupTexture();
                    if (texture->GetLevelsDirty())
                        texture->RegenerateLevels();
                    if (texture->GetParametersDirty())
                        texture->UpdateParameters();
                    ci.textures_.emplace_back(nameAndResource.name_, texture);
                }
            }

            for (unsigned i = 0; i < MAX_SHADER_PARAMETER_GROUPS; ++i) {
                WeakPtr<ConstantBuffer> cbuffer = cbufferManager_->GetCBuffer((ShaderParameterGroup)i);
                if (cbuffer != nullptr)
                    ci.constantBuffers_[i] = cbuffer;
                cbufferManager_->Dispatch((ShaderParameterGroup)i, cmd.cbufferTicketIds_[i]);
            }

            ShaderResourceBinding* srb = srbCache_->GetOrCreateSRB(ci);
            assert(srb);
            graphics_->CommitSRB(srb);
        }

        // Invoke appropriate draw command
        const unsigned vertexStart = 0;
        const unsigned vertexCount = 0;
        if (cmd.instanceCount_ != 0)
        {
            if (cmd.baseVertexIndex_ == 0)
            {
                graphics_->DrawInstanced(currentPrimitiveType, cmd.indexStart_, cmd.indexCount_,
                    vertexStart, vertexCount, cmd.instanceCount_);
            }
            else
            {
                graphics_->DrawInstanced(currentPrimitiveType, cmd.indexStart_, cmd.indexCount_,
                    cmd.baseVertexIndex_, vertexStart, vertexCount, cmd.instanceCount_);
            }
        }
        else
        {
            if (!currentIndexBuffer)
            {
                graphics_->Draw(currentPrimitiveType, cmd.indexStart_, cmd.indexCount_);
            }
            else if (cmd.baseVertexIndex_ == 0)
            {
                graphics_->Draw(currentPrimitiveType, cmd.indexStart_, cmd.indexCount_,
                    vertexStart, vertexCount);
            }
            else
            {
                graphics_->Draw(currentPrimitiveType, cmd.indexStart_, cmd.indexCount_,
                    cmd.baseVertexIndex_, vertexStart, vertexCount);
            }
        }
    }

    cbufferManager_->Finalize();
}

}
