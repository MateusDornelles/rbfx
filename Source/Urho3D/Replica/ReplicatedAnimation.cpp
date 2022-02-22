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

#include "../Core/Context.h"
#include "../Graphics/AnimationController.h"
#include "../Network/NetworkEvents.h"
#include "../Replica/ReplicatedAnimation.h"

namespace Urho3D
{

ReplicatedAnimation::ReplicatedAnimation(Context* context)
    : NetworkBehavior(context, CallbackMask)
{
}

ReplicatedAnimation::~ReplicatedAnimation()
{
}

void ReplicatedAnimation::RegisterObject(Context* context)
{
    context->RegisterFactory<ReplicatedAnimation>();

    URHO3D_COPY_BASE_ATTRIBUTES(NetworkBehavior);

    URHO3D_ATTRIBUTE("Num Upload Attempts", unsigned, numUploadAttempts_, DefaultNumUploadAttempts, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Replicate Owner", bool, replicateOwner_, false, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Smoothing Time", float, smoothingTime_, DefaultSmoothingTime, AM_DEFAULT);
}

void ReplicatedAnimation::InitializeStandalone()
{
    InitializeCommon();
}

void ReplicatedAnimation::InitializeOnServer()
{
    InitializeCommon();
    if (!animationController_)
        return;

    server_.latestRevision_ = animationController_->GetRevision();

    SubscribeToEvent(E_ENDSERVERNETWORKFRAME,
        [this](StringHash, VariantMap& eventData)
    {
        using namespace BeginServerNetworkFrame;
        const auto serverFrame = static_cast<NetworkFrame>(eventData[P_FRAME].GetInt64());
        OnServerFrameEnd(serverFrame);
    });
}

void ReplicatedAnimation::InitializeFromSnapshot(NetworkFrame frame, Deserializer& src, bool isOwned)
{
    InitializeCommon();
    if (!animationController_)
        return;

    const auto replicationManager = GetNetworkObject()->GetReplicationManager();
    const unsigned traceDuration = replicationManager->GetTraceDurationInFrames();

    client_.networkStepTime_ = 1.0f / replicationManager->GetUpdateFrequency();
    client_.animationTrace_.Resize(traceDuration);
    client_.latestAppliedFrame_ = ea::nullopt;
}

void ReplicatedAnimation::InitializeCommon()
{
    animationController_ = GetComponent<AnimationController>();
    if (!animationController_)
        return;

    // Update controller manually
    animationController_->SetEnabled(false);
}

void ReplicatedAnimation::OnServerFrameEnd(NetworkFrame frame)
{
    if (!animationController_)
        return;

    if (server_.pendingUploadAttempts_ > 0)
        --server_.pendingUploadAttempts_;

    const unsigned revision = animationController_->GetRevision();
    if (server_.latestRevision_ != revision)
    {
        server_.latestRevision_ = revision;
        server_.pendingUploadAttempts_ = numUploadAttempts_;
    }
}

void ReplicatedAnimation::WriteSnapshot(Serializer& dest) const
{
    // TODO(animation): Refactor me
    VariantVector buf = animationController_->GetAnimationsAttr();
    dest.WriteVariantVector(buf);
}

ReplicatedAnimation::AnimationSnapshot ReplicatedAnimation::ReadSnapshot(Deserializer& src) const
{
    // TODO(animation): Refactor me
    return src.ReadVariantVector();
}

void ReplicatedAnimation::DecodeSnapshot(const AnimationSnapshot& snapshot, ea::vector<AnimationParameters>& result) const
{
    // TODO(animation): Refactor me
    result.clear();
    result.resize(snapshot[0].GetUInt());
    for (unsigned i = 0; i < result.size(); ++i)
        result[i] = AnimationParameters::FromVariantSpan(context_, {&snapshot[i * AnimationParameters::NumVariants + 1], AnimationParameters::NumVariants});
}

bool ReplicatedAnimation::PrepareUnreliableDelta(NetworkFrame frame)
{
    return animationController_ && (server_.pendingUploadAttempts_ > 0 || numUploadAttempts_ == 0);
}

void ReplicatedAnimation::WriteUnreliableDelta(NetworkFrame frame, Serializer& dest)
{
    WriteSnapshot(dest);
}

void ReplicatedAnimation::ReadUnreliableDelta(NetworkFrame frame, Deserializer& src)
{
    const auto snapshot = ReadSnapshot(src);
    client_.animationTrace_.Set(frame, snapshot);
}

void ReplicatedAnimation::InterpolateState(float timeStep, const NetworkTime& replicaTime, const NetworkTime& inputTime)
{
    if (!animationController_ || !GetNetworkObject()->IsReplicatedClient())
        return;

    // Find extrapolation point
    const auto closestPriorFrame = client_.animationTrace_.FindClosestAllocatedFrame(replicaTime.Frame(), true, false);
    if (!closestPriorFrame || *closestPriorFrame == client_.latestAppliedFrame_)
        return;

    client_.latestAppliedFrame_ = *closestPriorFrame;
    const AnimationSnapshot& snapshot = client_.animationTrace_.GetRawUnchecked(*closestPriorFrame);
    DecodeSnapshot(snapshot, client_.snapshotAnimations_);

    // TODO: Should subtract timeStep here as well, but it works better this way this way for some reason
    const float delay = (replicaTime - NetworkTime{*closestPriorFrame}) * client_.networkStepTime_;
    animationController_->ReplaceAnimations(client_.snapshotAnimations_, delay, smoothingTime_);
}

void ReplicatedAnimation::Update(float replicaTimeStep, float inputTimeStep)
{
    if (!animationController_)
        return;

    NetworkObject* networkObject = GetNetworkObject();
    if (networkObject->IsOwnedByThisClient() && !replicateOwner_)
        animationController_->Update(inputTimeStep);
    else
        animationController_->Update(replicaTimeStep);
}

}
