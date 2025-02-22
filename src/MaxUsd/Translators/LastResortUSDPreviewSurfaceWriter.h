//
// Copyright 2023 Autodesk
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
#pragma once

#include "ShaderWriter.h"
#include "ShaderWriterRegistry.h"
#include "WriteJobContext.h"

#include <pxr/pxr.h>
#include <pxr/usd/sdf/path.h>

PXR_NAMESPACE_OPEN_SCOPE

class LastResortUSDPreviewSurfaceWriter : public MaxUsdShaderWriter
{
public:
    LastResortUSDPreviewSurfaceWriter(
        Mtl*                   material,
        const SdfPath&         usdPath,
        MaxUsdWriteJobContext& jobCtx);

    static ContextSupport CanExport(const MaxUsd::USDSceneBuilderOptions&);

    void Write() override;
};

PXR_NAMESPACE_CLOSE_SCOPE
