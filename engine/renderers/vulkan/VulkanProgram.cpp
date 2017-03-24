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
#include <sstream>
#include <vulkan/vulkan.h>
#include "aeongames/ResourceCache.h"
#include "aeongames/Program.h"
#include "VulkanProgram.h"
#include "VulkanTexture.h"
#include "VulkanRenderer.h"
#include "VulkanUtilities.h"
#include "SPIR-V/CompilerLinker.h"

namespace AeonGames
{
    VulkanProgram::VulkanProgram ( const std::shared_ptr<Program> aProgram, const VulkanRenderer* aVulkanRenderer ) :
        mProgram ( aProgram ),
        mVulkanRenderer ( aVulkanRenderer )
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
        if ( !mVulkanRenderer )
        {
            throw std::runtime_error ( "Pointer to Vulkan Renderer is nullptr." );
        }
        CompilerLinker compiler_linker;
        compiler_linker.AddShaderSource ( EShLanguage::EShLangVertex, mProgram->GetVertexShaderSource().c_str() );
        compiler_linker.AddShaderSource ( EShLanguage::EShLangFragment, mProgram->GetFragmentShaderSource().c_str() );
        if ( CompilerLinker::FailCode result = compiler_linker.CompileAndLink() )
        {
            throw std::runtime_error ( compiler_linker.GetLog().c_str() );
        }
        {
            VkShaderModuleCreateInfo shader_module_create_info{};
            shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shader_module_create_info.codeSize = compiler_linker.GetSpirV ( EShLanguage::EShLangVertex ).size() * sizeof ( uint32_t );
            shader_module_create_info.pCode = compiler_linker.GetSpirV ( EShLanguage::EShLangVertex ).data();
            if ( VkResult result = vkCreateShaderModule ( mVulkanRenderer->GetDevice(), &shader_module_create_info, nullptr, &mVkVertexShaderModule ) )
            {
                std::ostringstream stream;
                stream << "Shader module creation failed: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
        {
            VkShaderModuleCreateInfo shader_module_create_info{};
            shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shader_module_create_info.codeSize = compiler_linker.GetSpirV ( EShLanguage::EShLangFragment ).size() * sizeof ( uint32_t );
            shader_module_create_info.pCode = compiler_linker.GetSpirV ( EShLanguage::EShLangFragment ).data();
            if ( VkResult result = vkCreateShaderModule ( mVulkanRenderer->GetDevice(), &shader_module_create_info, nullptr, &mVkFragmentShaderModule ) )
            {
                std::ostringstream stream;
                stream << "Shader module creation failed: ( " << GetVulkanResultString ( result ) << " )";
                throw std::runtime_error ( stream.str().c_str() );
            }
        }
    }

    void VulkanProgram::Finalize()
    {
        if ( mVkVertexShaderModule != VK_NULL_HANDLE )
        {
            vkDestroyShaderModule ( mVulkanRenderer->GetDevice(), mVkVertexShaderModule, nullptr );
            mVkVertexShaderModule = VK_NULL_HANDLE;
        }
        if ( mVkFragmentShaderModule != VK_NULL_HANDLE )
        {
            vkDestroyShaderModule ( mVulkanRenderer->GetDevice(), mVkFragmentShaderModule, nullptr );
            mVkFragmentShaderModule = VK_NULL_HANDLE;
        }
    }
}
