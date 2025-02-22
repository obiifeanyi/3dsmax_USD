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

#include <MaxUsd/Builders/MaxSceneBuilderOptions.h>

/**
 * \brief Abstract class describing the interface for a USD Import View.
 */
class IUSDImportView
{
public:
    /**
     * \brief Destructor.
     */
    virtual ~IUSDImportView() = 0 { }

    /**
     * \brief Return the build options for the 3ds Max scene to translate from the USD Stage.
     * \return The build options for the 3ds Max scene to translate from the USD Stage.
     */
    virtual MaxUsd::MaxSceneBuilderOptions GetBuildOptions() const = 0;

    /**
     * \brief Display the View.
     * \return A flag indicating whether the User chose to import the USD file.
     */
    virtual bool Execute() = 0;
};
