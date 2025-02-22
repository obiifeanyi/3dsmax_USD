//
// Copyright 2024 Autodesk
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

#include <MaxUsd/MaxUSDAPI.h>

#include <pxr/usd/usdGeom/basisCurves.h>

#include <MaxUsd.h>
#include <splshape.h>

namespace MAXUSD_NS_DEF {

class MaxUSDAPI CurveConverter
{
public:
    /**
     * \brief Converts a USD BasisCurves to a 3ds Max SplineShape.
     * \param curve The USD BasisCurves to convert.
     * \param maxSpline The 3ds Max SplineShape to convert to.
     * \param timeCode The USD timeCode at which the conversion happens.
     * \param cleanMesh Flag to remove vertices that aren't being used from the converted mesh.
     * \return The number of splines created.
     */
    static size_t ConvertToSplineShape(
        const pxr::UsdGeomBasisCurves& curve,
        SplineShape&                   maxSpline,
        pxr::UsdTimeCode               timeCode = pxr::UsdTimeCode::Default());
};

} // namespace MAXUSD_NS_DEF
