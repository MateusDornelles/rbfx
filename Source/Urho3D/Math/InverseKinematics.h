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

#pragma once

#include "../Math/Vector3.h"
#include "../Math/Quaternion.h"

#include <EASTL/span.h>

namespace Urho3D
{

struct IKNodeSegment;

/// Aggregated settings of the IK solver.
struct URHO3D_API IKSettings
{
    unsigned maxIterations_{10};
    float tolerance_{0.001f};
    /// Whether to consider node rotations from the previous frame when solving.
    /// Results in smoother motion, but may cause rotation bleeding over time.
    bool continuousRotations_{};
};

/// Singular node of the IK chain.
/// Node should be used only once within a chain.
/// However, node may belong to multiple independet chains.
struct URHO3D_API IKNode
{
    Vector3 originalPosition_;
    Quaternion originalRotation_;

    Vector3 previousPosition_;
    Quaternion previousRotation_;

    Vector3 position_;
    Quaternion rotation_;

    bool positionDirty_{};
    bool rotationDirty_{};

    IKNode() = default;
    IKNode(const Vector3& position, const Quaternion& rotation) { InitializeTransform(position, rotation); }

    void InitializeTransform(const Vector3& position, const Quaternion& rotation);
    void StorePreviousTransform();
    void MarkPositionDirty() { positionDirty_ = true; }
    void MarkRotationDirty() { rotationDirty_ = true; }
};

/// Fixed-length segment that consists of two nodes.
struct URHO3D_API IKNodeSegment
{
    IKNode* beginNode_{};
    IKNode* endNode_{};
    float length_{};

    /// Calculate rotation delta from positions, using previous positions as reference.
    Quaternion CalculateRotationDeltaFromPrevious() const;
    /// Calculate rotation delta from positions, using original positions as reference.
    Quaternion CalculateRotationDeltaFromOriginal() const;
    /// Calculate current rotation.
    Quaternion CalculateRotation(const IKSettings& settings) const;
    /// Calculate current direction.
    Vector3 CalculateDirection() const;

    /// Update cached length.
    void UpdateLength();
    /// Update current rotation for nodes.
    void UpdateRotationInNodes(const IKSettings& settings, bool isLastSegment);
};

/// Trigonometric two-segment IK chain.
class URHO3D_API IKTrigonometricChain
{
public:
    void Initialize(IKNode* node1, IKNode* node2, IKNode* node3);
    void UpdateLengths();
    void Solve(const Vector3& target, const Vector3& bendNormal,
        float minAngle, float maxAngle, const IKSettings& settings);

    /// Return positions of second and third bones.
    static ea::pair<Vector3, Vector3> Solve(const Vector3& pos0, float len01, float len12,
        const Vector3& target, const Vector3& bendNormal, float minAngle, float maxAngle);

    IKNode* GetBeginNode() const { return segments_[0].beginNode_; }
    IKNode* GetMiddleNode() const { return segments_[1].beginNode_; }
    IKNode* GetEndNode() const { return segments_[1].endNode_; }
    float GetFirstLength() const { return segments_[0].length_; }
    float GetSecondLength() const { return segments_[1].length_; }

private:
    IKNodeSegment segments_[2];
};

/// Generic unconstrained FABRIK chain.
class URHO3D_API IKFabrikChain
{
public:
    void Clear();
    void AddNode(IKNode* node);

    void UpdateLengths();

    void Solve(const Vector3& target, const IKSettings& settings);

    // TODO: Make private.
    ea::vector<IKNodeSegment> segments_;

private:
    void StorePreviousTransforms();
    void RestorePreviousTransforms();
    Vector3 GetDeadlockRotationAxis(const Vector3& target) const;
    void RotateChain(const Quaternion& rotation);
    void RotateChainNode(IKNode& node, const Quaternion& rotation) const;
    void SolveIteration(const Vector3& target, const IKSettings& settings, bool backward);
    bool TrySolve(const Vector3& target, const IKSettings& settings);
};

}
