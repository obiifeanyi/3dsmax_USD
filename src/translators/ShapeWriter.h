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

#include <MaxUsd/Translators/primWriter.h>
#include <MaxUsd/Translators/writeJobContext.h>

#include <pxr/pxr.h>

PXR_NAMESPACE_OPEN_SCOPE

class MaxUsdShapeWriter : public MaxUsdPrimWriter
{
public:
    static ContextSupport CanExport(INode* node, const MaxUsd::USDSceneBuilderOptions& exportArgs);

    MaxUsdShapeWriter(const MaxUsdWriteJobContext& jobCtx, INode* node);

    bool
    Write(UsdPrim& targetPrim, bool applyOffsetTransform, const MaxUsd::ExportTime& time) override;

    MaxUsd::XformSplitRequirement RequiresXformPrim() override;

    TfToken GetPrimType() override { return MaxUsdPrimTypeTokens->BasisCurves; };

    TfToken GetObjectPrimSuffix() override { return TfToken("Shape"); };

    WStr GetWriterName() override { return L"Shape writer"; };
};

PXR_NAMESPACE_CLOSE_SCOPE
