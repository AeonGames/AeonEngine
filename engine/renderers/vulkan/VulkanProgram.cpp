/*
Copyright (C) 2017 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <cassert>
#include <vector>
#include "aeongames/ResourceCache.h"
#include "aeongames/Program.h"
#include "VulkanProgram.h"
#include "VulkanTexture.h"

namespace AeonGames
{
    VulkanProgram::VulkanProgram ( const std::shared_ptr<Program> aProgram ) :
        mProgram ( aProgram ),
        mProgramId ( 0 )
    {
        try
        {
            Initialize();
        }
        catch ( ... )
        {
            Finalize();
            throw;
        }
    }

    VulkanProgram::~VulkanProgram()
    {
        Finalize();
    }

    void VulkanProgram::Use() const
    {
    }

    void VulkanProgram::Initialize()
    {
    }

    void VulkanProgram::Finalize()
    {
    }
}
