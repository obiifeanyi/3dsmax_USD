//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// © 2023 Autodesk, Inc. All rights reserved.
//
#include "HdMaxInstancer.h"

#include <RenderDelegate/Sampler.h>

#include <MaxUsd/Utilities/MaxSupportUtils.h>

#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/quaternion.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/imaging/hd/sceneDelegate.h>

PXR_NAMESPACE_OPEN_SCOPE

// in 2025, we moved to Pixar 23.11 and the tokens changed in that release:
// https://forum.aousd.org/t/hydra-synthesized-instancer-primvars-renamed-in-release-23-11/797
#ifdef IS_MAX2025_OR_GREATER
const auto instanceTransformsToken = HdInstancerTokens->instanceTransforms;
const auto instanceRotationsToken = HdInstancerTokens->instanceRotations;
const auto instanceScalesToken = HdInstancerTokens->instanceScales;
const auto instanceTranslationsToken = HdInstancerTokens->instanceTranslations;
#else
const auto instanceTransformsToken = HdInstancerTokens->instanceTransform;
const auto instanceRotationsToken = HdInstancerTokens->rotate;
const auto instanceScalesToken = HdInstancerTokens->scale;
const auto instanceTranslationsToken = HdInstancerTokens->translate;
#endif

/*! \brief  Constructor.

    \param delegate     The scene delegate backing this instancer's data.
    \param id           The unique id of this instancer.
    \param parentId     The unique id of the parent instancer,
                        or an empty id if not applicable.
*/
HdMaxInstancer::HdMaxInstancer(HdSceneDelegate* delegate, SdfPath const& id)
    : HdInstancer(delegate, id)
{
}

/*! \brief  Destructor.
 */
HdMaxInstancer::~HdMaxInstancer()
{
    TF_FOR_ALL(it, _primvarMap) { delete it->second; }
    _primvarMap.clear();
}

/*! \brief  Checks the change tracker to determine whether instance primvars are
            dirty, and if so pulls them.

    Since primvars can only be pulled once, and are cached, this function is not
    re-entrant. However, this function is called by ComputeInstanceTransforms,
    which is called by HdMaxMesh::Sync(), which is dispatched in parallel, so it needs
    to be guarded by _instanceLock.
*/
void HdMaxInstancer::_SyncPrimvars()
{
    HD_TRACE_FUNCTION();
    HF_MALLOC_TAG_FUNCTION();

    HdChangeTracker& changeTracker = GetDelegate()->GetRenderIndex().GetChangeTracker();
    SdfPath const&   id = GetId();

    // Use the double-checked locking pattern to check if this instancer's
    // primvars are dirty.
    HdDirtyBits dirtyBits = changeTracker.GetInstancerDirtyBits(id);
    if (HdChangeTracker::IsAnyPrimvarDirty(dirtyBits, id)
        || HdChangeTracker::IsInstancerDirty(dirtyBits, id)
        || HdChangeTracker::IsInstanceIndexDirty(dirtyBits, id)) {
        std::lock_guard<std::mutex> lock(_instanceLock);

        // If not dirty, then another thread did the job
        dirtyBits = changeTracker.GetInstancerDirtyBits(id);

#if defined(HD_API_VERSION) && HD_API_VERSION >= 36
        _UpdateInstancer(GetDelegate(), &dirtyBits);
#endif

        if (HdChangeTracker::IsAnyPrimvarDirty(dirtyBits, id)) {

            // If this instancer has dirty primvars, get the list of
            // primvar names and then cache each one.

            TfTokenVector             primvarNames;
            HdPrimvarDescriptorVector primvars
                = GetDelegate()->GetPrimvarDescriptors(id, HdInterpolationInstance);

            for (HdPrimvarDescriptor const& pv : primvars) {
                if (HdChangeTracker::IsPrimvarDirty(dirtyBits, id, pv.name)) {
                    VtValue value = GetDelegate()->Get(id, pv.name);
                    if (!value.IsEmpty()) {
                        if (_primvarMap.count(pv.name) > 0) {
                            delete _primvarMap[pv.name];
                        }
                        _primvarMap[pv.name] = new HdVtBufferSource(pv.name, value);
                    }
                }
            }
        }

        // Mark the instancer as clean
        changeTracker.MarkInstancerClean(id);
    }
}

/*! \brief  Computes all instance transforms for the provided prototype id.

    Taking into account the scene delegate's instancerTransform and the
    instance primvars "instanceTransform", "translate", "rotate", "scale".
    Computes and flattens nested transforms, if necessary.

    \param prototypeId The prototype to compute transforms for.

    \return One transform per instance, to apply when drawing.
*/
VtMatrix4dArray HdMaxInstancer::ComputeInstanceTransforms(SdfPath const& prototypeId)
{
    HD_TRACE_FUNCTION();
    HF_MALLOC_TAG_FUNCTION();

    _SyncPrimvars();

    // The transforms for this level of instancer are computed by:
    // foreach(index : indices) {
    //     instancerTransform * translate(index) * rotate(index) *
    //     scale(index) * instanceTransform(index)
    // }
    // If any transform isn't provided, it's assumed to be the identity.

    GfMatrix4d instancerTransform = GetDelegate()->GetInstancerTransform(GetId());
    VtIntArray instanceIndices = GetDelegate()->GetInstanceIndices(GetId(), prototypeId);

    VtMatrix4dArray transforms(instanceIndices.size());
    for (size_t i = 0; i < instanceIndices.size(); ++i) {
        transforms[i] = instancerTransform;
    }

    // "instanceTranslationsToken" holds a translation vector for each index.
    if (_primvarMap.count(instanceTranslationsToken) > 0) {
        HdMaxBufferSampler sampler(*_primvarMap[instanceTranslationsToken]);
        for (size_t i = 0; i < instanceIndices.size(); ++i) {
            GfVec3f translate;
            if (sampler.Sample(instanceIndices[i], &translate)) {
                GfMatrix4d translateMat(1);
                translateMat.SetTranslate(GfVec3d(translate));
                transforms[i] = translateMat * transforms[i];
            }
        }
    }

    // "instanceRotationsToken" holds a quaternion in <real, i, j, k> format for each index.
    if (_primvarMap.count(instanceRotationsToken) > 0) {
        HdMaxBufferSampler sampler(*_primvarMap[instanceRotationsToken]);
        for (size_t i = 0; i < instanceIndices.size(); ++i) {
            GfQuath quath;
            if (sampler.Sample(instanceIndices[i], &quath)) {
                GfMatrix4d rotateMat(1);
                rotateMat.SetRotate(quath);
                transforms[i] = rotateMat * transforms[i];
            } else {
                GfVec4f quat;
                if (sampler.Sample(instanceIndices[i], &quat)) {
                    GfMatrix4d rotateMat(1);
                    rotateMat.SetRotate(GfQuatd(quat[0], quat[1], quat[2], quat[3]));
                    transforms[i] = rotateMat * transforms[i];
                }
            }
        }
    }

    // "instanceScalesToken" holds an axis-aligned scale vector for each index.
    if (_primvarMap.count(instanceScalesToken) > 0) {
        HdMaxBufferSampler sampler(*_primvarMap[instanceScalesToken]);
        for (size_t i = 0; i < instanceIndices.size(); ++i) {
            GfVec3f scale;
            if (sampler.Sample(instanceIndices[i], &scale)) {
                GfMatrix4d scaleMat(1);
                scaleMat.SetScale(GfVec3d(scale));
                transforms[i] = scaleMat * transforms[i];
            }
        }
    }

    // "instanceTransformsToken" holds a 4x4 transform matrix for each index.
    if (_primvarMap.count(instanceTransformsToken) > 0) {
        HdMaxBufferSampler sampler(*_primvarMap[instanceTransformsToken]);
        for (size_t i = 0; i < instanceIndices.size(); ++i) {
            GfMatrix4d instanceTransform;
            if (sampler.Sample(instanceIndices[i], &instanceTransform)) {
                transforms[i] = instanceTransform * transforms[i];
            }
        }
    }

    if (GetParentId().IsEmpty()) {
        return transforms;
    }

    HdInstancer* parentInstancer = GetDelegate()->GetRenderIndex().GetInstancer(GetParentId());
    if (!TF_VERIFY(parentInstancer)) {
        return transforms;
    }

    // The transforms taking nesting into account are computed by:
    // parentTransforms = parentInstancer->ComputeInstanceTransforms(GetId())
    // foreach (parentXf : parentTransforms, xf : transforms) {
    //     parentXf * xf
    // }
    VtMatrix4dArray parentTransforms
        = static_cast<HdMaxInstancer*>(parentInstancer)->ComputeInstanceTransforms(GetId());

    VtMatrix4dArray final(parentTransforms.size() * transforms.size());
    for (size_t i = 0; i < parentTransforms.size(); ++i) {
        for (size_t j = 0; j < transforms.size(); ++j) {
            final[i * transforms.size() + j] = transforms[j] * parentTransforms[i];
        }
    }
    return final;
}

HdDirtyBits HdMaxInstancer::GetInitialDirtyBitsMask() const
{
    return HdChangeTracker::DirtyPrimvar | HdChangeTracker::DirtyTransform
        | HdChangeTracker::DirtyInstanceIndex | HdChangeTracker::DirtyInstancer;
}

PXR_NAMESPACE_CLOSE_SCOPE
