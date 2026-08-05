/*
Copyright (C) 2026 Rodrigo Jose Hernandez Cordoba

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
#import <Metal/Metal.h>

#include <cstring>
#include <stdexcept>
#include <vector>
#include "aeongames/Mesh.hpp"
#include "MetalMesh.h"

namespace AeonGames
{
    class MetalMesh::Impl
    {
    public:
        Impl ( id<MTLDevice> aDevice, const Mesh& aMesh ) : mMesh{&aMesh}
        {
            if ( !aMesh.GetVertexBuffer().empty() )
            {
                mVertexBuffer = [aDevice newBufferWithBytes:aMesh.GetVertexBuffer().data()
                                 length:aMesh.GetVertexBuffer().size()
                                 options:MTLResourceStorageModeShared];
                if ( mVertexBuffer == nil )
                {
                    throw std::runtime_error ( "Metal mesh vertex-buffer allocation failed" );
                }
            }
            mIndexCount = aMesh.GetIndexCount();
            if ( mIndexCount == 0 )
            {
                return;
            }
            if ( aMesh.GetIndexSize() == 4 )
            {
                mIndex32 = true;
                mIndexBuffer = [aDevice newBufferWithBytes:aMesh.GetIndexBuffer().data()
                                length:aMesh.GetIndexBuffer().size()
                                options:MTLResourceStorageModeShared];
            }
            else
            {
                std::vector<uint16_t> indices ( mIndexCount );
                if ( aMesh.GetIndexSize() == 2 )
                {
                    std::memcpy ( indices.data(), aMesh.GetIndexBuffer().data(), indices.size() * sizeof ( uint16_t ) );
                }
                else if ( aMesh.GetIndexSize() == 1 )
                {
                    for ( uint32_t i = 0; i < mIndexCount; ++i )
                    {
                        indices[i] = aMesh.GetIndexBuffer() [i];
                    }
                }
                else
                {
                    throw std::runtime_error ( "Metal mesh has an unsupported index width" );
                }
                mIndexBuffer = [aDevice newBufferWithBytes:indices.data()
                                length:indices.size() * sizeof ( uint16_t )
                                options:MTLResourceStorageModeShared];
            }
            if ( mIndexBuffer == nil )
            {
                throw std::runtime_error ( "Metal mesh index-buffer allocation failed" );
            }
        }

        const Mesh* mMesh{nullptr};
        id<MTLBuffer> mVertexBuffer{nil};
        id<MTLBuffer> mIndexBuffer{nil};
        uint32_t mIndexCount{0};
        bool mIndex32{false};
    };

    MetalMesh::MetalMesh ( void* aDevice, const Mesh& aMesh ) :
        mImpl{std::make_unique<Impl> ( ( __bridge id<MTLDevice> ) aDevice, aMesh ) }
    {
    }

    MetalMesh::~MetalMesh() = default;
    MetalMesh::MetalMesh ( MetalMesh&& ) noexcept = default;
    MetalMesh& MetalMesh::operator= ( MetalMesh&& ) noexcept = default;

    const Mesh& MetalMesh::GetMesh() const
    {
        return *mImpl->mMesh;
    }
    void* MetalMesh::GetVertexBuffer() const
    {
        return ( __bridge void* ) mImpl->mVertexBuffer;
    }
    void* MetalMesh::GetIndexBuffer() const
    {
        return ( __bridge void* ) mImpl->mIndexBuffer;
    }
    size_t MetalMesh::GetVertexBufferSize() const
    {
        return mImpl->mMesh->GetVertexBuffer().size();
    }
    uint32_t MetalMesh::GetIndexCount() const
    {
        return mImpl->mIndexCount;
    }
    bool MetalMesh::Has32BitIndices() const
    {
        return mImpl->mIndex32;
    }
}