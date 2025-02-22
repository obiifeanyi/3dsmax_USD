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

#include <pxr/base/tf/hashmap.h>
#include <pxr/base/tf/token.h>
#include <pxr/imaging/hd/instancer.h>
#include <pxr/imaging/hd/vtBufferSource.h>
#include <pxr/pxr.h>

#include <mutex>

PXR_NAMESPACE_OPEN_SCOPE

/*! \brief  3dsMax/Nitrous instancing of prototype geometry with varying transforms
    Nested instancing can be handled by recursion, and by taking the
    cartesian product of the transform arrays at each nesting level, to
    create a flattened transform array.
*/
class HdMaxInstancer final : public HdInstancer
{
public:
    HdMaxInstancer(HdSceneDelegate* delegate, SdfPath const& id);
    ~HdMaxInstancer();

    VtMatrix4dArray ComputeInstanceTransforms(SdfPath const& prototypeId);

    HdDirtyBits GetInitialDirtyBitsMask() const override;

private:
    void _SyncPrimvars();

    //! Mutex guard for _SyncPrimvars().
    std::mutex _instanceLock;

    /*! Map of the latest primvar data for this instancer, keyed by
        primvar name. Primvar values are VtValue, an any-type; they are
        interpreted at consumption time (here, in ComputeInstanceTransforms).
    */
    TfHashMap<TfToken, HdVtBufferSource*, TfToken::HashFunctor> _primvarMap;
};

PXR_NAMESPACE_CLOSE_SCOPE
