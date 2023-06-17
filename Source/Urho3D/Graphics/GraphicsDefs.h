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

/// \file

#pragma once

#include "../Container/FlagSet.h"
#include "../Container/Hash.h"
#include "../Math/StringHash.h"
#include "../RenderAPI/RenderAPIDefs.h"

namespace Urho3D
{

class ConstantBuffer;
class Vector3;

// Graphics capability support level. Web platform (Emscripten) also uses OpenGL ES, but is considered a desktop platform capability-wise.
// TODO(diligent): Revisit this distinction
//#if defined(IOS) || defined(TVOS) || defined(__ANDROID__) || defined(__arm__) || (defined(__aarch64__) && !defined(MACOS))
//#define MOBILE_GRAPHICS
//#else
#define DESKTOP_GRAPHICS
//#endif

/// %Geometry type for vertex shader geometry variations.
enum GeometryType
{
    GEOM_STATIC = 0,
    GEOM_SKINNED = 1,
    GEOM_INSTANCED = 2,
    GEOM_BILLBOARD = 3,
    GEOM_DIRBILLBOARD = 4,
    GEOM_TRAIL_FACE_CAMERA = 5,
    GEOM_TRAIL_BONE = 6,
    MAX_GEOMETRYTYPES = 7,
    // This is not a real geometry type for VS, but used to mark objects that do not desire to be instanced
    GEOM_STATIC_NOINSTANCING = 7,
};

/// Vertex/index buffer lock state.
enum LockState
{
    LOCK_NONE = 0,
    LOCK_HARDWARE,
    LOCK_SHADOW,
    LOCK_SCRATCH
};

/// Hardcoded legacy vertex elements.
enum LegacyVertexElement
{
    ELEMENT_POSITION = 0,
    ELEMENT_NORMAL,
    ELEMENT_COLOR,
    ELEMENT_TEXCOORD1,
    ELEMENT_TEXCOORD2,
    ELEMENT_CUBETEXCOORD1,
    ELEMENT_CUBETEXCOORD2,
    ELEMENT_TANGENT,
    ELEMENT_BLENDWEIGHTS,
    ELEMENT_BLENDINDICES,
    ELEMENT_INSTANCEMATRIX1,
    ELEMENT_INSTANCEMATRIX2,
    ELEMENT_INSTANCEMATRIX3,
    // Custom 32-bit integer object index. Due to API limitations, not supported on D3D9
    ELEMENT_OBJECTINDEX,
    MAX_LEGACY_VERTEX_ELEMENTS
};

/// Vertex element description for arbitrary vertex declarations.
struct URHO3D_API VertexElement
{
    /// Default-construct.
    VertexElement() noexcept :
        type_(TYPE_VECTOR3),
        semantic_(SEM_POSITION),
        index_(0),
        perInstance_(false),
        offset_(0)
    {
    }

    /// Construct with type, semantic, index and whether is per-instance data.
    VertexElement(VertexElementType type, VertexElementSemantic semantic, unsigned char index = 0, bool perInstance = false) noexcept :
        type_(type),
        semantic_(semantic),
        index_(index),
        perInstance_(perInstance),
        offset_(0)
    {
    }

    /// Test for equality with another vertex element. Offset is intentionally not compared, as it's relevant only when an element exists within a vertex buffer.
    bool operator ==(const VertexElement& rhs) const { return type_ == rhs.type_ && semantic_ == rhs.semantic_ && index_ == rhs.index_ && perInstance_ == rhs.perInstance_; }

    /// Test for inequality with another vertex element.
    bool operator !=(const VertexElement& rhs) const { return !(*this == rhs); }

    /// Return hash value of the vertex element.
    unsigned ToHash() const
    {
        unsigned hash = 0;
        CombineHash(hash, type_);
        CombineHash(hash, semantic_);
        CombineHash(hash, index_);
        CombineHash(hash, perInstance_);
        CombineHash(hash, offset_);
        return hash;
    }

    /// Data type of element.
    VertexElementType type_;
    /// Semantic of element.
    VertexElementSemantic semantic_;
    /// Semantic index of element, for example multi-texcoords.
    unsigned char index_;
    /// Per-instance flag.
    bool perInstance_;
    /// Offset of element from vertex start. Filled by VertexBuffer once the vertex declaration is built.
    unsigned offset_;
};

/// Sizes of vertex element types.
extern URHO3D_API const unsigned ELEMENT_TYPESIZES[];

/// Vertex element definitions for the legacy elements.
extern URHO3D_API const VertexElement LEGACY_VERTEXELEMENTS[];

static const char* textureFilterModeNames[] = {
    "NEAREST",
    "BILINEAR",
    "TRILINEAR",
    "ANISOTROPIC",
    "NEAREST_ANISOTROPIC",
    "DEFAULT",
    nullptr,
};

/// Cube map faces.
enum CubeMapFace
{
    FACE_POSITIVE_X = 0,
    FACE_NEGATIVE_X,
    FACE_POSITIVE_Y,
    FACE_NEGATIVE_Y,
    FACE_POSITIVE_Z,
    FACE_NEGATIVE_Z,
    MAX_CUBEMAP_FACES
};

/// Cubemap single image layout modes.
enum CubeMapLayout
{
    CML_HORIZONTAL = 0,
    CML_HORIZONTALNVIDIA,
    CML_HORIZONTALCROSS,
    CML_VERTICALCROSS,
    CML_BLENDER
};

/// Update mode for render surface viewports.
enum RenderSurfaceUpdateMode
{
    SURFACE_MANUALUPDATE = 0,
    SURFACE_UPDATEVISIBLE,
    SURFACE_UPDATEALWAYS
};

/// Shader parameter groups for determining need to update. On APIs that support constant buffers, these correspond to different constant buffers.
static const char* shaderParameterGroupNames[] = {
    "Frame",
    "Camera",
    "Zone",
    "Light",
    "Material",
    "Object",
    "Custom",
    nullptr,
};

//TODO(diligent): Refactor ShaderVariant to use this
static const char* elementSemanticNames[] = {
    "POSITION",
    "NORMAL",
    "BINORMAL",
    "TANGENT",
    "TEXCOORD",
    "COLOR",
    "BLENDWEIGHT",
    "BLENDINDICES",
    "OBJECTINDEX",
    nullptr
};
static ea::unordered_map<ea::string, ShaderParameterGroup> constantBuffersNamesLookup = {
    { "Frame", ShaderParameterGroup::SP_FRAME },
    { "Camera", ShaderParameterGroup::SP_CAMERA },
    { "Zone", ShaderParameterGroup::SP_ZONE },
    { "Light", ShaderParameterGroup::SP_LIGHT },
    { "Material", ShaderParameterGroup::SP_MATERIAL },
    { "Object", ShaderParameterGroup::SP_OBJECT },
    { "Custom", ShaderParameterGroup::SP_CUSTOM }
};

/// Texture units.
/// @manualbind
enum TextureUnit
{
    TU_DIFFUSE = 0,
    TU_ALBEDOBUFFER = 0,
    TU_NORMAL = 1,
    TU_NORMALBUFFER = 1,
    TU_SPECULAR = 2,
    TU_EMISSIVE = 3,
    TU_ENVIRONMENT = 4,
#ifdef DESKTOP_GRAPHICS
    TU_VOLUMEMAP = 5,
    TU_CUSTOM1 = 6,
    TU_CUSTOM2 = 7,
    TU_LIGHTRAMP = 8,
    TU_LIGHTSHAPE = 9,
    TU_SHADOWMAP = 10,
    TU_FACESELECT = 11,
    TU_INDIRECTION = 12,
    TU_DEPTHBUFFER = 13,
    TU_LIGHTBUFFER = 14,
    TU_ZONE = 15,
    MAX_MATERIAL_TEXTURE_UNITS = 8,
    MAX_TEXTURE_UNITS = 16
#else
    TU_LIGHTRAMP = 5,
    TU_LIGHTSHAPE = 6,
    TU_SHADOWMAP = 7,
    MAX_MATERIAL_TEXTURE_UNITS = 5,
    MAX_TEXTURE_UNITS = 8
#endif
};

/// Billboard camera facing modes.
enum FaceCameraMode
{
    FC_NONE = 0,
    FC_ROTATE_XYZ,
    FC_ROTATE_Y,
    FC_LOOKAT_XYZ,
    FC_LOOKAT_Y,
    FC_LOOKAT_MIXED,
    FC_DIRECTION,
    FC_AXIS_ANGLE,
};

/// Shadow type.
enum ShadowQuality
{
    SHADOWQUALITY_SIMPLE_16BIT = 0,
    SHADOWQUALITY_SIMPLE_24BIT,
    SHADOWQUALITY_PCF_16BIT,
    SHADOWQUALITY_PCF_24BIT,
    SHADOWQUALITY_VSM,
    SHADOWQUALITY_BLUR_VSM
};

/// Range of constant buffer to bind.
struct ConstantBufferRange
{
    /// Constant buffer.
    ConstantBuffer* constantBuffer_{};
    /// Offset in buffer. Shall be multiply of constant buffer offset alignment.
    unsigned offset_{};
    /// Size of region.
    unsigned size_{};

    /// Compare equal.
    bool operator ==(const ConstantBufferRange& rhs) const
    {
        return constantBuffer_ == rhs.constantBuffer_
            && offset_ == rhs.offset_
            && size_ == rhs.size_;
    }

    /// Compare not equal.
    bool operator !=(const ConstantBufferRange& rhs) const { return !(*this == rhs); }
};

// Inbuilt shader parameters.
extern URHO3D_API const StringHash VSP_AMBIENTSTARTCOLOR;
extern URHO3D_API const StringHash VSP_AMBIENTENDCOLOR;
extern URHO3D_API const StringHash VSP_BILLBOARDROT;
extern URHO3D_API const StringHash VSP_CAMERAPOS;
extern URHO3D_API const StringHash VSP_CLIPPLANE;
extern URHO3D_API const StringHash VSP_NEARCLIP;
extern URHO3D_API const StringHash VSP_FARCLIP;
extern URHO3D_API const StringHash VSP_DEPTHMODE;
extern URHO3D_API const StringHash VSP_DELTATIME;
extern URHO3D_API const StringHash VSP_ELAPSEDTIME;
extern URHO3D_API const StringHash VSP_FRUSTUMSIZE;
extern URHO3D_API const StringHash VSP_GBUFFEROFFSETS;
extern URHO3D_API const StringHash VSP_LIGHTDIR;
extern URHO3D_API const StringHash VSP_LIGHTPOS;
extern URHO3D_API const StringHash VSP_NORMALOFFSETSCALE;
extern URHO3D_API const StringHash VSP_MODEL;
extern URHO3D_API const StringHash VSP_VIEW;
extern URHO3D_API const StringHash VSP_VIEWINV;
extern URHO3D_API const StringHash VSP_VIEWPROJ;
extern URHO3D_API const StringHash VSP_UOFFSET;
extern URHO3D_API const StringHash VSP_VOFFSET;
extern URHO3D_API const StringHash VSP_ZONE;
extern URHO3D_API const StringHash VSP_LIGHTMATRICES;
extern URHO3D_API const StringHash VSP_SKINMATRICES;
extern URHO3D_API const StringHash VSP_VERTEXLIGHTS;
extern URHO3D_API const StringHash VSP_LMOFFSET;
extern URHO3D_API const StringHash VSP_SHAR;
extern URHO3D_API const StringHash VSP_SHAG;
extern URHO3D_API const StringHash VSP_SHAB;
extern URHO3D_API const StringHash VSP_SHBR;
extern URHO3D_API const StringHash VSP_SHBG;
extern URHO3D_API const StringHash VSP_SHBB;
extern URHO3D_API const StringHash VSP_SHC;
extern URHO3D_API const StringHash VSP_AMBIENT;
extern URHO3D_API const StringHash PSP_AMBIENTCOLOR;
extern URHO3D_API const StringHash PSP_CAMERAPOS;
extern URHO3D_API const StringHash PSP_DELTATIME;
extern URHO3D_API const StringHash PSP_DEPTHRECONSTRUCT;
extern URHO3D_API const StringHash PSP_ELAPSEDTIME;
extern URHO3D_API const StringHash PSP_FOGCOLOR;
extern URHO3D_API const StringHash PSP_FOGPARAMS;
extern URHO3D_API const StringHash PSP_GBUFFERINVSIZE;
extern URHO3D_API const StringHash PSP_LIGHTCOLOR;
extern URHO3D_API const StringHash PSP_LIGHTDIR;
extern URHO3D_API const StringHash PSP_LIGHTPOS;
extern URHO3D_API const StringHash PSP_NORMALOFFSETSCALE;
extern URHO3D_API const StringHash PSP_MATDIFFCOLOR;
extern URHO3D_API const StringHash PSP_MATEMISSIVECOLOR;
extern URHO3D_API const StringHash PSP_MATENVMAPCOLOR;
extern URHO3D_API const StringHash PSP_MATSPECCOLOR;
extern URHO3D_API const StringHash PSP_NEARCLIP;
extern URHO3D_API const StringHash PSP_FARCLIP;
extern URHO3D_API const StringHash PSP_SHADOWCUBEADJUST;
extern URHO3D_API const StringHash PSP_SHADOWCUBEUVBIAS;
extern URHO3D_API const StringHash PSP_SHADOWDEPTHFADE;
extern URHO3D_API const StringHash PSP_SHADOWINTENSITY;
extern URHO3D_API const StringHash PSP_SHADOWMAPINVSIZE;
extern URHO3D_API const StringHash PSP_SHADOWSPLITS;
extern URHO3D_API const StringHash PSP_LIGHTMATRICES;
extern URHO3D_API const StringHash PSP_VSMSHADOWPARAMS;
extern URHO3D_API const StringHash PSP_ROUGHNESS;
extern URHO3D_API const StringHash PSP_METALLIC;
extern URHO3D_API const StringHash PSP_LIGHTRAD;
extern URHO3D_API const StringHash PSP_LIGHTLENGTH;
extern URHO3D_API const StringHash PSP_ZONEMIN;
extern URHO3D_API const StringHash PSP_ZONEMAX;

// Scale calculation from bounding box diagonal.
extern URHO3D_API const Vector3 DOT_SCALE;

enum MaterialQuality : unsigned
{
    QUALITY_LOW = 0,
    QUALITY_MEDIUM = 1,
    QUALITY_HIGH = 2,
    QUALITY_MAX = 15,
};

enum ClearTarget : unsigned
{
    CLEAR_NONE = 0x0,
    CLEAR_COLOR = 0x1,
    CLEAR_DEPTH = 0x2,
    CLEAR_STENCIL = 0x4,
};
URHO3D_FLAGSET(ClearTarget, ClearTargetFlags);

// Legacy vertex element bitmasks.
enum VertexMask : unsigned
{
    MASK_NONE = 0x0,
    MASK_POSITION = 0x1,
    MASK_NORMAL = 0x2,
    MASK_COLOR = 0x4,
    MASK_TEXCOORD1 = 0x8,
    MASK_TEXCOORD2 = 0x10,
    MASK_CUBETEXCOORD1 = 0x20,
    MASK_CUBETEXCOORD2 = 0x40,
    MASK_TANGENT = 0x80,
    MASK_BLENDWEIGHTS = 0x100,
    MASK_BLENDINDICES = 0x200,
    MASK_INSTANCEMATRIX1 = 0x400,
    MASK_INSTANCEMATRIX2 = 0x800,
    MASK_INSTANCEMATRIX3 = 0x1000,
    MASK_OBJECTINDEX = 0x2000,
};
URHO3D_FLAGSET(VertexMask, VertexMaskFlags);

static const int MAX_RENDERTARGETS = (int)MaxRenderTargets;
static const int MAX_VERTEX_STREAMS = (int)MaxVertexStreams;

/// Shader translation policy.
enum class ShaderTranslationPolicy
{
    /// Do not translate shaders, use universal GLSL shaders directly.
    /// This mode is only supported for OpenGL and OpenGL ES backends (GLSL-based backends).
    Verbatim,
    /// Preprocess and translate shader to the target language through SPIR-V without any optimization.
    /// This results in slower shader compilation, especially in Debug builds.
    /// This mode may help to work around OpenGL driver bugs if used for GLSL-based backends.
    Translate,
    /// Fully process and optimize shader via SPIR-V Tools.
    /// This results in even slower shader compilation, especially in Debug builds.
    /// This mode may improve realtime performance of the shaders, especially on mobile platforms.
    Optimize
};

/// TODO(diligent): revisit
#ifdef URHO3D_DEBUG
static ea::string ConstantBufferDebugNames[] = {
    "FrameCB",
    "CameraCB",
    "ZoneCB",
    "LightCB",
    "MaterialCB",
    "ObjectCB",
    "CustomCB"
};
#endif

}
