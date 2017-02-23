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

#include "VulkanTexture.h"
#include "aeongames/Image.h"
#include "aeongames/Utilities.h"

namespace AeonGames
{
    VulkanTexture::VulkanTexture ( const std::shared_ptr<Image> aImage ) :
        mImage ( aImage ), mTexture ( 0 )
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

    VulkanTexture::~VulkanTexture()
    {
        Finalize();
    }

    void VulkanTexture::Initialize()
    {
    }

    void VulkanTexture::Finalize()
    {
        mTexture = 0;
    }

    const uint32_t VulkanTexture::GetTexture() const
    {
        return mTexture;
    }
}
