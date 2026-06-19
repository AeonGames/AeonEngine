/*
Copyright (C) 2014-2019,2021,2025,2026 Rodrigo Jose Hernandez Cordoba

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
#include "aeongames/AeonEngine.hpp"
#include "aeongames/Scene.hpp"
#include "aeongames/Node.hpp"
#include "aeongames/LogLevel.hpp"
#include "aeongames/Renderer.hpp"
#include "aeongames/Pipeline.hpp"
#include "aeongames/Mesh.hpp"
#include "aeongames/Frustum.hpp"
#include "aeongames/AABB.hpp"
#include "aeongames/GpuLight.hpp"
#include "aeongames/Transform.hpp"
#include "aeongames/Quaternion.hpp"
#include "aeongames/Vector3.hpp"
#include "aeongames/GpuShadowParams.hpp"
#include <array>
#include <algorithm>
#include <limits>
#include <cmath>
#include "aeongames/ProtoBufHelpers.hpp"
#include "aeongames/ProtoBufUtils.hpp"
#include "aeongames/CRC.hpp"
#include <cstring>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <span>
#include <unordered_map>
#include <variant>

#include "aeongames/ProtoBufClasses.hpp"
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : PROTOBUF_WARNINGS )
#endif
#include <google/protobuf/text_format.h>
#include "scene.pb.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include "aeongames/Resource.hpp"

namespace AeonGames
{
    /**@todo implement Linear Octrees:
    https://geidav.wordpress.com/2014/08/18/advanced-octrees-2-node-representations/
    */
    Scene::Scene() :
        mName{ "Scene" },
        mNodes{}
    {
    }

    Scene::~Scene()
    {
        for ( auto & mRootNode : mNodes )
        {
            Remove ( mRootNode.get() );
        }
    }

    void Scene::Load ( const std::string& aFilename )
    {
        Load ( crc32i ( aFilename.c_str(), aFilename.size() ) );
    }
    void Scene::Load ( uint32_t aId )
    {
        std::vector<uint8_t> buffer ( GetResourceSize ( aId ), 0 );
        LoadResource ( aId, buffer.data(), buffer.size() );
        try
        {
            Load ( buffer.data(), buffer.size() );
        }
        catch ( ... )
        {
            throw;
        }
    }
    void Scene::Load ( const void* aBuffer, size_t aBufferSize )
    {
        Deserialize ( {reinterpret_cast<const char*> ( aBuffer ), aBufferSize} );
    }

    void Scene::SetName ( const char* aName )
    {
        mName = aName;
    }

    const char* const Scene::GetName() const
    {
        return mName.c_str();
    }

    void Scene::SetCamera ( Node* aNode )
    {
        mCamera = aNode;
    }

    void Scene::SetCamera ( uint32_t aNodeId )
    {
        SetCamera ( Find ( [aNodeId] ( const Node & aNode ) -> bool { return aNode.GetId() == aNodeId; } ) );
    }

    void Scene::SetCamera ( const std::string& aNodeName )
    {
        SetCamera ( crc32i ( aNodeName.c_str(), aNodeName.size() ) );
    }

    const Node* Scene::GetCamera() const
    {
        return mCamera;
    }

    void Scene::SetInputSystem ( InputSystem* aInputSystem )
    {
        mInputSystem = aInputSystem;
    }

    InputSystem* Scene::GetInputSystem() const
    {
        return mInputSystem;
    }

    void Scene::AddLight ( const GpuLight& aLight )
    {
        mFrameLights.Add ( aLight );
    }

    std::span<const GpuLight> Scene::GetFrameLights() const
    {
        return mFrameLights.Get();
    }

    void Scene::SetLightingPipeline ( const ResourceId& aResourceId )
    {
        mLightingPipeline = aResourceId;
    }

    const Pipeline* Scene::GetLightingPipeline() const
    {
        if ( mLightingPipeline.GetPath() == 0 )
        {
            return nullptr;
        }
        return mLightingPipeline.Get<Pipeline>();
    }

    bool Scene::GetDirectionalShadowMatrix ( Matrix4x4& aLightViewProjection,
            const Matrix4x4& aCameraProjection ) const
    {
        // Pick the first directional light submitted this frame as the caster.
        const GpuLight* caster = nullptr;
        for ( const GpuLight& light : mFrameLights.Get() )
        {
            if ( light.type == static_cast<uint32_t> ( LightType::Directional ) )
            {
                caster = &light;
                break;
            }
        }
        if ( caster == nullptr )
        {
            return false;
        }
        // The scene's world-space extent comes from the octree root cell, which
        // BuildRenderQueue has already refreshed for this frame. It bounds the
        // shadow depth range so casters behind the camera (between the sun and
        // the visible region) still write into the map.
        if ( mSpatialIndexDirty )
        {
            BuildSpatialIndex();
        }
        if ( mSpatialIndex.GetNodeCount() == 0 )
        {
            return false;
        }
        const AABB& bounds = mSpatialIndex.GetRootBounds();
        const Vector3 scene_center = bounds.GetCenter();
        const Vector3 scene_radii = bounds.GetRadii();
        const float scene_radius = scene_radii.GetLength();
        if ( scene_radius <= 0.0f )
        {
            return false;
        }
        // direction_cosOuter.xyz points TOWARD the light (= -lightDir), so the
        // light travels along its negation.
        Vector3 light_dir
        {
            -caster->direction_cosOuter.GetX(),
                                       -caster->direction_cosOuter.GetY(),
                                       -caster->direction_cosOuter.GetZ()
        };
        if ( light_dir.GetLength() <= 0.0f )
        {
            return false;
        }
        light_dir = Normalize ( light_dir );

        // Fit the shadow map to the camera's view frustum, truncated to the
        // shadow coverage distance, instead of the whole scene: only geometry
        // the camera can see needs shadows, so a tighter fit both culls casters
        // from the shadow pass (recovering frame time) and spends the fixed
        // shadow resolution on the visible region (sharper shadows).
        //
        // Reconstruct the sub-frustum's eight world-space corners by inverting a
        // camera view-projection rebuilt in engine convention. The aspect ratio
        // is the only camera parameter the scene does not already hold; recover
        // it from the uploaded projection (|m9/m0|, invariant to the backend
        // depth flip) so this stays backend-agnostic.
        if ( std::fabs ( aCameraProjection[0] ) < 1e-6f )
        {
            return false;
        }
        const float aspect = std::fabs ( aCameraProjection[9] / aCameraProjection[0] );
        if ( aspect <= 0.0f )
        {
            return false;
        }
        const float shadow_far = mNear +
                                 std::min ( mFar - mNear, scene_radius * SHADOW_COVERAGE_FRACTION );
        Matrix4x4 camera_projection;
        camera_projection.Perspective ( mFieldOfView, aspect, mNear, shadow_far );
        Matrix4x4 camera_view_projection = camera_projection * mViewMatrix;
        const Matrix4x4 inverse_view_projection = camera_view_projection.GetInvertedMatrix4x4();
        // Engine NDC cube is z in [-1,1] (Matrix4x4::Perspective is GL-style);
        // transform each cube corner as a homogeneous point and divide by w.
        std::array<Vector3, 8> corners{};
        size_t corner_index = 0;
        for ( float z = -1.0f; z <= 1.0f; z += 2.0f )
        {
            for ( float y = -1.0f; y <= 1.0f; y += 2.0f )
            {
                for ( float x = -1.0f; x <= 1.0f; x += 2.0f )
                {
                    const float cx = x * inverse_view_projection[0] + y * inverse_view_projection[4] + z * inverse_view_projection[8]  + inverse_view_projection[12];
                    const float cy = x * inverse_view_projection[1] + y * inverse_view_projection[5] + z * inverse_view_projection[9]  + inverse_view_projection[13];
                    const float cz = x * inverse_view_projection[2] + y * inverse_view_projection[6] + z * inverse_view_projection[10] + inverse_view_projection[14];
                    const float cw = x * inverse_view_projection[3] + y * inverse_view_projection[7] + z * inverse_view_projection[11] + inverse_view_projection[15];
                    const float inv_w = ( std::fabs ( cw ) > 1e-6f ) ? ( 1.0f / cw ) : 1.0f;
                    corners[corner_index++] = Vector3{ cx * inv_w, cy * inv_w, cz * inv_w };
                }
            }
        }
        // Bounding sphere of the sub-frustum (centroid + farthest corner): its
        // size is rotation-invariant, so the shadow extent does not pulse as the
        // camera turns. Clamp the radius to the scene so a camera looking out
        // past the world never grows the fit beyond the whole scene.
        Vector3 frustum_center{ 0.0f, 0.0f, 0.0f };
        for ( const Vector3& c : corners )
        {
            frustum_center += c;
        }
        frustum_center *= ( 1.0f / 8.0f );
        float radius = 0.0f;
        for ( const Vector3& c : corners )
        {
            radius = std::max ( radius, ( c - frustum_center ).GetLength() );
        }
        radius = std::min ( radius, scene_radius );
        if ( radius <= 0.0f )
        {
            return false;
        }

        // Orient a virtual camera so its forward axis (engine +Y) looks along the
        // light direction. The shortest-arc rotation maps +Y onto light_dir,
        // with the antiparallel case handled explicitly.
        const Vector3 forward { 0.0f, 1.0f, 0.0f };
        const float d = Dot ( forward, light_dir );
        Quaternion orientation{ 1.0f, 0.0f, 0.0f, 0.0f };
        if ( d < -0.9999f )
        {
            // light_dir == -forward: rotate 180 degrees about any orthogonal axis.
            orientation = Quaternion::GetFromAxisAngle ( 180.0f, 0.0f, 0.0f, 1.0f );
        }
        else if ( d < 0.9999f )
        {
            const Vector3 axis = Normalize ( Cross ( forward, light_dir ) );
            const float angle = std::acos ( d ) * ( 180.0f / 3.14159265358979323846f );
            orientation = Quaternion::GetFromAxisAngle ( angle, axis.GetX(), axis.GetY(), axis.GetZ() );
        }
        Transform light_transform;
        light_transform.SetRotation ( orientation );
        light_transform.SetTranslation ( frustum_center );
        Matrix4x4 light_view = light_transform.GetInvertedMatrix();
        // Stabilize the fit against sub-texel crawl: snap the sphere centre to
        // shadow-texel increments along the light's two lateral axes. Those axes
        // are the columns of the light rotation that map a world vector onto
        // light-space X and Z, read directly out of the world->light matrix.
        const float texel = ( 2.0f * radius ) / static_cast<float> ( SHADOW_MAP_RESOLUTION );
        const Vector3 light_right { light_view[0], light_view[4], light_view[8] };
        const Vector3 light_up    { light_view[2], light_view[6], light_view[10] };
        const float proj_right = Dot ( frustum_center, light_right );
        const float proj_up    = Dot ( frustum_center, light_up );
        const float snapped_right = std::round ( proj_right / texel ) * texel;
        const float snapped_up    = std::round ( proj_up / texel ) * texel;
        const Vector3 snapped_center = frustum_center +
                                       light_right * ( snapped_right - proj_right ) +
                                       light_up * ( snapped_up - proj_up );
        light_transform.SetTranslation ( snapped_center );
        light_view = light_transform.GetInvertedMatrix();

        // Depth range (light-space forward axis = engine +Y) spanning the whole
        // scene so every potential caster between the sun and the visible region
        // is rendered. Transform the scene AABB's eight corners into light space
        // and take the forward extent, padded slightly so casters exactly on the
        // boundary are not clipped.
        float depth_min = std::numeric_limits<float>::max();
        float depth_max = std::numeric_limits<float>::lowest();
        for ( int i = 0; i < 8; ++i )
        {
            const Vector3 corner
            {
                scene_center.GetX() + ( ( i & 1 ) ? scene_radii.GetX() : -scene_radii.GetX() ),
                            scene_center.GetY() + ( ( i & 2 ) ? scene_radii.GetY() : -scene_radii.GetY() ),
                            scene_center.GetZ() + ( ( i & 4 ) ? scene_radii.GetZ() : -scene_radii.GetZ() )
            };
            const Vector3 light_space = light_view * corner;
            depth_min = std::min ( depth_min, light_space.GetY() );
            depth_max = std::max ( depth_max, light_space.GetY() );
        }
        const float depth_pad = ( depth_max - depth_min ) * 0.01f + 1.0f;
        depth_min -= depth_pad;
        depth_max += depth_pad;

        Matrix4x4 light_projection;
        light_projection.Ortho ( -radius, radius, -radius, radius, depth_min, depth_max );
        // Engine convention (column-major, clip = Proj * View * Model * v). The
        // per-backend depth flip is applied inside the shadow shaders.
        aLightViewProjection = light_projection * light_view;
        return true;
    }

    uint32_t Scene::GetSpotShadowCasters ( GpuSpotShadowParams& aSpotShadowParams ) const
    {
        // Start from the "no casters" state: zero matrices, zero positions, and
        // refill the filtering params below. Slots left zero never match a real
        // light's position, so the shader treats them as non-casters.
        aSpotShadowParams = GpuSpotShadowParams{};
        uint32_t count = 0;
        for ( const GpuLight& light : mFrameLights.Get() )
        {
            if ( count >= MAX_SPOT_SHADOW_CASTERS )
            {
                break;
            }
            if ( light.type != static_cast<uint32_t> ( LightType::Spot ) )
            {
                continue;
            }
            // The cone axis (direction the light travels) is the negation of
            // direction_cosOuter.xyz, which stores -lightDir.
            Vector3 cone_axis
            {
                -light.direction_cosOuter.GetX(),
                                         -light.direction_cosOuter.GetY(),
                                         -light.direction_cosOuter.GetZ()
            };
            if ( cone_axis.GetLength() <= 0.0f )
            {
                continue;
            }
            cone_axis = Normalize ( cone_axis );
            // The light's radius is the perspective far plane; without it there
            // is no finite frustum to render.
            const float radius = light.position_radius.GetW();
            if ( radius <= 0.0f )
            {
                continue;
            }
            // Outer cone half-angle -> full vertical field of view. Clamp below
            // 180 degrees so a very wide cone cannot form a degenerate frustum.
            const float cos_outer = std::clamp ( light.direction_cosOuter.GetW(), -1.0f, 1.0f );
            const float half_angle = std::acos ( cos_outer );
            const float fov_degrees =
                std::min ( 2.0f * half_angle * ( 180.0f / 3.14159265358979323846f ), 170.0f );
            if ( fov_degrees <= 0.0f )
            {
                continue;
            }
            const Vector3 eye
            {
                light.position_radius.GetX(),
                                     light.position_radius.GetY(),
                                     light.position_radius.GetZ()
            };
            // Orient a virtual camera so its forward axis (engine +Y) looks along
            // the cone axis (same shortest-arc construction as the directional
            // caster, with the antiparallel case handled explicitly).
            const Vector3 forward { 0.0f, 1.0f, 0.0f };
            const float d = Dot ( forward, cone_axis );
            Quaternion orientation{ 1.0f, 0.0f, 0.0f, 0.0f };
            if ( d < -0.9999f )
            {
                orientation = Quaternion::GetFromAxisAngle ( 180.0f, 0.0f, 0.0f, 1.0f );
            }
            else if ( d < 0.9999f )
            {
                const Vector3 axis = Normalize ( Cross ( forward, cone_axis ) );
                const float angle = std::acos ( d ) * ( 180.0f / 3.14159265358979323846f );
                orientation = Quaternion::GetFromAxisAngle ( angle, axis.GetX(), axis.GetY(), axis.GetZ() );
            }
            Transform light_transform;
            light_transform.SetRotation ( orientation );
            light_transform.SetTranslation ( eye );
            const Matrix4x4 light_view = light_transform.GetInvertedMatrix();
            // Near plane a small fraction of the range, floored so it is never 0.
            const float near_plane = std::max ( radius * 0.02f, 0.05f );
            Matrix4x4 light_projection;
            light_projection.Perspective ( fov_degrees, 1.0f, near_plane, radius );
            // Engine convention (clip = Proj * View * Model * v); the per-backend
            // depth flip is applied inside the shadow shaders.
            aSpotShadowParams.spot_light_view_projection[count] = light_projection * light_view;
            aSpotShadowParams.caster_position[count] =
                Vector4 { eye.GetX(), eye.GetY(), eye.GetZ(), 0.0f };
            ++count;
        }
        aSpotShadowParams.params[0] = 1.0f / static_cast<float> ( SPOT_SHADOW_MAP_RESOLUTION );
        aSpotShadowParams.params[1] = 0.0015f;
        aSpotShadowParams.params[2] = 1.0f;
        aSpotShadowParams.params[3] = static_cast<float> ( count );
        return count;
    }

    uint32_t Scene::GetPointShadowCasters ( GpuPointShadowParams& aPointShadowParams ) const
    {
        aPointShadowParams = GpuPointShadowParams{};
        uint32_t count = 0;
        // Per-face world->view rotation rows (view X, view Y = forward,
        // view Z), one entry per cube face in the hardware layer order
        // +X,-X,+Y,-Y,+Z,-Z (layer = caster*6 + face). They are derived so the
        // engine perspective's NDC (ndc.x = vx/vy, ndc.y = -vz/vy) reproduces
        // the GPU cube-map face and (s,t) selection convention for a world-space
        // sampling direction, letting the shading pass sample the cube by raw
        // direction. The bases are reflective (determinant -1) to match the
        // left-handed cube convention, so with back-face culling the pass
        // records back-face depth; that is correct (and reduces acne) for the
        // solid shadow casters here.
        struct FaceBasis
        {
            Vector3 r0, r1, r2;
        };
        static const FaceBasis face_basis[POINT_SHADOW_FACES] =
        {
            { { 0.0f, 0.0f, -1.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f,  0.0f } }, // +X
            { { 0.0f, 0.0f,  1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, 1.0f,  0.0f } }, // -X
            { { 1.0f, 0.0f,  0.0f }, {  0.0f, 1.0f,  0.0f }, { 0.0f, 0.0f, -1.0f } }, // +Y
            { { 1.0f, 0.0f,  0.0f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f,  1.0f } }, // -Y
            { { 1.0f, 0.0f,  0.0f }, {  0.0f, 0.0f,  1.0f }, { 0.0f, 1.0f,  0.0f } }, // +Z
            { {-1.0f, 0.0f,  0.0f }, {  0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f,  0.0f } }, // -Z
        };
        for ( const GpuLight& light : mFrameLights.Get() )
        {
            if ( count >= MAX_POINT_SHADOW_CASTERS )
            {
                break;
            }
            if ( light.type != static_cast<uint32_t> ( LightType::Point ) )
            {
                continue;
            }
            const float radius = light.position_radius.GetW();
            if ( radius <= 0.0f )
            {
                continue;
            }
            const Vector3 eye
            {
                light.position_radius.GetX(),
                                     light.position_radius.GetY(),
                                     light.position_radius.GetZ()
            };
            const float near_plane = std::max ( radius * 0.02f, 0.05f );
            // 90-degree field of view so the six faces tile the full sphere.
            Matrix4x4 light_projection;
            light_projection.Perspective ( 90.0f, 1.0f, near_plane, radius );
            for ( uint32_t face = 0; face < POINT_SHADOW_FACES; ++face )
            {
                const FaceBasis& b = face_basis[face];
                // view = R * (world - eye): rotation rows are the view axes in
                // world space, translation is -R*eye.
                const float tx = - ( b.r0.GetX() * eye.GetX() + b.r0.GetY() * eye.GetY() + b.r0.GetZ() * eye.GetZ() );
                const float ty = - ( b.r1.GetX() * eye.GetX() + b.r1.GetY() * eye.GetY() + b.r1.GetZ() * eye.GetZ() );
                const float tz = - ( b.r2.GetX() * eye.GetX() + b.r2.GetY() * eye.GetY() + b.r2.GetZ() * eye.GetZ() );
                const Matrix4x4 light_view
                {
                    b.r0.GetX(), b.r1.GetX(), b.r2.GetX(), 0.0f,
                        b.r0.GetY(), b.r1.GetY(), b.r2.GetY(), 0.0f,
                        b.r0.GetZ(), b.r1.GetZ(), b.r2.GetZ(), 0.0f,
                        tx,          ty,          tz,          1.0f
                };
                aPointShadowParams.point_light_view_projection[count * POINT_SHADOW_FACES + face] =
                    light_projection * light_view;
            }
            aPointShadowParams.caster_position_radius[count] =
                Vector4 { eye.GetX(), eye.GetY(), eye.GetZ(), radius };
            ++count;
        }
        aPointShadowParams.params[0] = 1.0f / static_cast<float> ( POINT_SHADOW_MAP_RESOLUTION );
        // Depth bias is in normalized radial-distance units (the depth pass
        // stores length(frag-light)/radius), so it is larger than the NDC bias
        // the directional/spot maps use.
        aPointShadowParams.params[1] = 0.004f;
        aPointShadowParams.params[2] = 1.0f;
        aPointShadowParams.params[3] = static_cast<float> ( count );
        return count;
    }

    void Scene::SetFieldOfView ( float aFieldOfView )
    {
        mFieldOfView = aFieldOfView;
    }

    void Scene::SetNear ( float aNear )
    {
        mNear = aNear;
    }

    void Scene::SetFar ( float aFar )
    {
        mFar = aFar;
    }

    float Scene::GetFieldOfView() const
    {
        return mFieldOfView;
    }
    float Scene::GetNear() const
    {
        return mNear;
    }
    float Scene::GetFar() const
    {
        return mFar;
    }

    void Scene::SetViewMatrix ( const Matrix4x4& aMatrix )
    {
        mViewMatrix = aMatrix;
    }

    const Matrix4x4& Scene::GetViewMatrix() const
    {
        return mViewMatrix;
    }

    size_t Scene::GetChildrenCount() const
    {
        return mNodes.size();
    }

    Node* Scene::GetChild ( size_t aIndex ) const
    {
        return mNodes.at ( aIndex ).get();
    }

    size_t Scene::GetChildIndex ( const Node* aNode ) const
    {
        auto index = std::find_if ( mNodes.begin(), mNodes.end(),
                                    [aNode] ( const std::unique_ptr<Node>& node )
        {
            return node.get() == aNode;
        } );
        if ( index != mNodes.end() )
        {
            return index - mNodes.begin();
        }
        throw std::runtime_error ( "Node is not a child of this object." );
    }

    const Node& Scene::operator[] ( const std::size_t index ) const
    {
        return * ( mNodes[index] );
    }

    Node& Scene::operator[] ( const std::size_t index )
    {
        return const_cast<Node&> ( static_cast<const Scene&> ( *this ) [index] );
    }

    void Scene::Update ( const double delta )
    {
        mFrameLights.Reset();
        // Recompute the shadow-geometry signature in the same traversal that
        // updates the nodes (avoids a second full-scene walk): FNV-1a over each
        // geometry node's world pose and size. Nodes without geometry (camera,
        // bare lights) have a degenerate AABB and are skipped, so moving the
        // camera leaves the signature unchanged; only a moved, resized, or
        // added/removed shadow caster changes it. Each node's world transform is
        // final when it is visited (parents update first in pre-order).
        uint64_t hash = 1469598103934665603ull;
        auto fold = [&hash] ( float aValue )
        {
            uint32_t bits;
            std::memcpy ( &bits, &aValue, sizeof ( bits ) );
            hash = ( hash ^ bits ) * 1099511628211ull;
        };
        LoopTraverseDFSPreOrder ( [delta, &fold] ( Node & aNode )
        {
            aNode.Update ( delta );
            const Vector3& radii = aNode.GetAABB().GetRadii();
            if ( radii.GetX() <= 0.0f && radii.GetY() <= 0.0f && radii.GetZ() <= 0.0f )
            {
                return;
            }
            const Matrix4x4 world { aNode.GetGlobalTransform() };
            const float* m = world.GetMatrix4x4();
            for ( int i = 0; i < 16; ++i )
            {
                fold ( m[i] );
            }
            fold ( radii.GetX() );
            fold ( radii.GetY() );
            fold ( radii.GetZ() );
        } );
        mShadowGeometrySignature = hash;
    }

    void Scene::InvalidateSpatialIndex()
    {
        mSpatialIndexDirty = true;
    }

    void Scene::BuildSpatialIndex() const
    {
        // Root bounds are the union of every node's world-space AABB; the depth is
        // a fixed cap (deeper trees buy finer culling at the cost of more cells).
        constexpr uint32_t kSceneOctreeMaxDepth = 8;
        bool any = false;
        AABB bounds{};
        LoopTraverseDFSPreOrder ( [&any, &bounds] ( const Node & aNode )
        {
            const AABB world = aNode.GetGlobalTransform() * aNode.GetAABB();
            if ( !any )
            {
                bounds = world;
                any = true;
            }
            else
            {
                bounds += world;
            }
        } );
        mSpatialIndex = any ? Octree{ bounds, kSceneOctreeMaxDepth } :
                        Octree{};
        if ( any )
        {
            LoopTraverseDFSPreOrder ( [this] ( const Node & aNode )
            {
                mSpatialIndex.AddNode ( &aNode );
            } );
        }
        mSpatialIndexDirty = false;
    }

    void Scene::CullVisible ( const Frustum& aFrustum, const std::function<void ( const Node& ) >& aCallback ) const
    {
        if ( mSpatialIndexDirty )
        {
            BuildSpatialIndex();
        }
        if ( mSpatialIndex.GetNodeCount() != 0 )
        {
            mSpatialIndex.QueryFrustum ( aFrustum, [&aFrustum, &aCallback] ( const Node * aNode )
            {
                const AABB world = aNode->GetGlobalTransform() * aNode->GetAABB();
                if ( aFrustum.Intersects ( world ) )
                {
                    aCallback ( *aNode );
                }
            } );
            return;
        }
        // Fallback: empty/degenerate index (e.g. a scene with no nodes). A plain
        // traversal keeps CullVisible's result identical to a brute-force cull.
        LoopTraverseDFSPreOrder ( [&aFrustum, &aCallback] ( const Node & aNode )
        {
            const AABB world = aNode.GetGlobalTransform() * aNode.GetAABB();
            if ( aFrustum.Intersects ( world ) )
            {
                aCallback ( aNode );
            }
        } );
    }

    void Scene::QueryAABB ( const AABB& aBox, const std::function<void ( const Node& ) >& aCallback ) const
    {
        if ( mSpatialIndexDirty )
        {
            BuildSpatialIndex();
        }
        if ( mSpatialIndex.GetNodeCount() != 0 )
        {
            mSpatialIndex.QueryAABB ( aBox, [&aBox, &aCallback] ( const Node * aNode )
            {
                const AABB world = aNode->GetGlobalTransform() * aNode->GetAABB();
                if ( aBox.Overlaps ( world ) )
                {
                    aCallback ( *aNode );
                }
            } );
            return;
        }
        // Fallback: empty/degenerate index. A plain traversal keeps QueryAABB's
        // result identical to a brute-force overlap scan.
        LoopTraverseDFSPreOrder ( [&aBox, &aCallback] ( const Node & aNode )
        {
            const AABB world = aNode.GetGlobalTransform() * aNode.GetAABB();
            if ( aBox.Overlaps ( world ) )
            {
                aCallback ( aNode );
            }
        } );
    }

    void Scene::ForEachOctreeCell ( const std::function<void ( const AABB&, uint32_t ) >& aCallback ) const
    {
        if ( mSpatialIndexDirty )
        {
            BuildSpatialIndex();
        }
        mSpatialIndex.ForEachCell ( aCallback );
    }

    void Scene::ForEachOctreeCell ( const Frustum& aFrustum, const std::function<void ( const AABB&, uint32_t ) >& aCallback ) const
    {
        if ( mSpatialIndexDirty )
        {
            BuildSpatialIndex();
        }
        mSpatialIndex.ForEachCell ( aFrustum, aCallback );
    }

    uint64_t Scene::GetShadowGeometrySignature() const
    {
        return mShadowGeometrySignature;
    }

    void Scene::BuildRenderQueue ( const Frustum& aFrustum ) const
    {
        // Per-node frustum culling is inherited from CullVisible; each visible
        // node appends the draws its components contribute. clear() keeps the
        // buffer capacity so steady-state frames perform no heap allocation.
        mRenderQueue.clear();
        CullVisible ( aFrustum, [this] ( const Node & aNode )
        {
            aNode.Collect ( mRenderQueue );
        } );
        // Sort so items sharing pipeline, material and mesh become adjacent,
        // letting ForEachRenderBatch merge them into one instanced draw. Skinned
        // items carry a distinct skinned-vertex pointer and sort apart, so they
        // never merge with each other or with non-skinned items. std::sort is in
        // place, keeping this routine allocation-free.
        std::sort ( mRenderQueue.begin(), mRenderQueue.end(),
                    [] ( const RenderItem & aLhs, const RenderItem & aRhs )
        {
            if ( aLhs.mPipeline != aRhs.mPipeline )
            {
                return aLhs.mPipeline < aRhs.mPipeline;
            }
            if ( aLhs.mMaterial != aRhs.mMaterial )
            {
                return aLhs.mMaterial < aRhs.mMaterial;
            }
            if ( aLhs.mMesh != aRhs.mMesh )
            {
                return aLhs.mMesh < aRhs.mMesh;
            }
            return aLhs.mSkinnedVertices < aRhs.mSkinnedVertices;
        } );
    }

    const std::vector<RenderItem>& Scene::GetRenderQueue() const
    {
        return mRenderQueue;
    }

    void Scene::ForEachRenderBatch ( const std::function<void ( std::span<const RenderItem> ) >& aCallback ) const
    {
        const size_t count = mRenderQueue.size();
        const RenderItem* const data = mRenderQueue.data();
        size_t i = 0;
        while ( i < count )
        {
            const RenderItem& head = data[i];
            size_t j = i + 1;
            // Skinned items are posed per node and must draw individually, so
            // only non-skinned items extend a batch; the run ends as soon as a
            // skinned item or a different pipeline/material/mesh is reached.
            if ( head.mSkinnedVertices == nullptr )
            {
                while ( j < count &&
                        data[j].mSkinnedVertices == nullptr &&
                        data[j].mPipeline == head.mPipeline &&
                        data[j].mMaterial == head.mMaterial &&
                        data[j].mMesh == head.mMesh )
                {
                    ++j;
                }
            }
            aCallback ( std::span<const RenderItem> ( data + i, j - i ) );
            i = j;
        }
    }

    void Scene::SubmitRenderQueue ( Renderer& aRenderer, void* aWindowId, RenderPass aRenderPass ) const
    {
        ForEachRenderBatch ( [this, &aRenderer, aWindowId, aRenderPass] ( std::span<const RenderItem> aBatch )
        {
            const RenderItem& head = aBatch.front();
            if ( aBatch.size() == 1 )
            {
                aRenderer.Render (
                    aWindowId,
                    head.mTransform,
                    *head.mMesh,
                    *head.mPipeline,
                    head.mMaterial,
                    Topology::TRIANGLE_LIST,
                    0,
                    0xffffffff,
                    1,
                    0,
                    head.mSkinnedVertices,
                    aRenderPass );
                return;
            }
            // Gather the batch's transforms contiguously for one instanced draw.
            // mInstanceTransforms is reused so this only allocates when a batch
            // grows beyond any previously seen size.
            mInstanceTransforms.clear();
            for ( const RenderItem& item : aBatch )
            {
                mInstanceTransforms.push_back ( item.mTransform );
            }
            aRenderer.RenderInstanced (
                aWindowId,
                mInstanceTransforms,
                *head.mMesh,
                *head.mPipeline,
                head.mMaterial,
                Topology::TRIANGLE_LIST,
                0,
                0xffffffff,
                aRenderPass );
        } );
    }

    void Scene::BroadcastMessage ( uint32_t aMessageType, const void* aMessageData )
    {
        for ( auto & mRootNode : mNodes )
        {
            mRootNode->LoopTraverseDFSPreOrder ( [aMessageType, aMessageData] ( Node & node )
            {
                if ( node.mFlags[Node::Enabled] )
                {
                    node.ProcessMessage ( aMessageType, aMessageData );
                }
            } );
        }
    }

    void Scene::LoopTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & mRootNode : mNodes )
        {
            mRootNode->LoopTraverseDFSPreOrder ( aAction );
        }
    }

    void Scene::LoopTraverseDFSPreOrder (
        const std::function<void ( Node& ) >& aPreamble,
        const std::function<void ( Node& ) >& aPostamble )
    {
        for ( auto & mRootNode : mNodes )
        {
            mRootNode->LoopTraverseDFSPreOrder ( aPreamble, aPostamble );
        }
    }

    void Scene::LoopTraverseDFSPreOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        for ( const auto& mRootNode : mNodes )
        {
            static_cast<const Node*> ( mRootNode.get() )->LoopTraverseDFSPreOrder ( aAction );
        }
    }

    void Scene::LoopTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & mRootNode : mNodes )
        {
            mRootNode->LoopTraverseDFSPostOrder ( aAction );
        }
    }

    void Scene::LoopTraverseDFSPostOrder ( const std::function<void ( const Node& ) >& aAction ) const
    {
        for ( const auto& mRootNode : mNodes )
        {
            static_cast<const Node*> ( mRootNode.get() )->LoopTraverseDFSPostOrder ( aAction );
        }
    }

    Node* Scene::Find ( const std::function<bool ( const Node& ) >& aUnaryPredicate ) const
    {
        for ( const auto& mRootNode : mNodes )
        {
            if ( Node * node = static_cast<Node * > ( mRootNode.get() )->Find ( aUnaryPredicate ) )
            {
                return node;
            }
        }
        return nullptr;
    }

    void Scene::RecursiveTraverseDFSPreOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & mRootNode : mNodes )
        {
            mRootNode->RecursiveTraverseDFSPreOrder ( aAction );
        }
    }

    void Scene::RecursiveTraverseDFSPostOrder ( const std::function<void ( Node& ) >& aAction )
    {
        for ( auto & mRootNode : mNodes )
        {
            mRootNode->RecursiveTraverseDFSPostOrder ( aAction );
        }
    }

    Node* Scene::Insert ( size_t aIndex, std::unique_ptr<Node> aNode )
    {
        // Never append null pointers.
        if ( aNode == nullptr )
        {
            return nullptr;
        }
        std::visit ( [&aNode] ( auto&& parent )
        {
            if ( parent != nullptr )
            {
                if ( !parent->Remove ( aNode.get() ) )
                {
                    std::cout << LogLevel::Warning << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
        },
        aNode->mParent );
        aNode->mParent = this;
        std::vector<std::unique_ptr<Node >>::iterator inserted_node;
        if ( aIndex < mNodes.size() )
        {
            inserted_node = mNodes.insert ( mNodes.begin() + aIndex, std::move ( aNode ) );
        }
        else
        {
            inserted_node = mNodes.insert ( mNodes.end(), std::move ( aNode ) );
        }
        // Force a recalculation of the LOCAL transform
        // by setting the GLOBAL transform to itself.
        ( *inserted_node )->SetGlobalTransform ( ( *inserted_node )->mGlobalTransform );
        mSpatialIndexDirty = true;
        return ( *inserted_node ).get();
    }

    Node* Scene::Add ( std::unique_ptr<Node> aNode )
    {
        // Never append null pointers.
        if ( aNode == nullptr )
        {
            return nullptr;
        }
        std::visit ( [&aNode] ( auto&& parent )
        {
            if ( parent != nullptr )
            {
                if ( !parent->Remove ( aNode.get() ) )
                {
                    std::cout << LogLevel::Warning << "Parent for node " << aNode->GetName() << " did not have it as a child.";
                }
            }
        },
        aNode->mParent );
        aNode->mParent = this;
        mNodes.emplace_back ( std::move ( aNode ) );
        // Force a recalculation of the LOCAL transform
        // by setting the GLOBAL transform to itself.
        mNodes.back()->SetGlobalTransform ( mNodes.back()->mGlobalTransform );
        mSpatialIndexDirty = true;
        return mNodes.back().get();
    }

    std::unique_ptr<Node> Scene::Remove ( Node* aNode )
    {
        if ( aNode == nullptr )
        {
            return nullptr;
        }
        // If passed a null or this pointer find SHOULD not find it on release builds.
        auto it = std::find_if ( mNodes.begin(), mNodes.end(),
                                 [aNode] ( const std::unique_ptr<Node>& node )
        {
            return aNode == node.get();
        } );
        if ( it != mNodes.end() )
        {
            // Force recalculation of transforms.
            aNode->mParent = static_cast<Node*> ( nullptr );
            aNode->SetLocalTransform ( aNode->mGlobalTransform );
            std::unique_ptr<Node> removed_node{std::move ( * ( it ) ) };
            mNodes.erase ( it );
            mSpatialIndexDirty = true;
            return removed_node;
        }
        return nullptr;
    }

    std::unique_ptr<Node> Scene::RemoveByIndex ( size_t aIndex )
    {
        if ( aIndex >= mNodes.size() )
        {
            return nullptr;
        }
        mNodes[aIndex]->mParent = static_cast<Node*> ( nullptr );
        mNodes[aIndex]->SetLocalTransform ( mNodes[aIndex]->mGlobalTransform );
        auto it = mNodes.begin() + aIndex;
        std::unique_ptr<Node> removed_node{std::move ( * ( it ) ) };
        mNodes.erase ( it );
        mSpatialIndexDirty = true;
        return removed_node;
    }

    std::string Scene::Serialize ( bool aAsBinary ) const
    {
        static SceneMsg scene_buffer;
        *scene_buffer.mutable_name() = mName;
        if ( mCamera )
        {
            *scene_buffer.mutable_camera()->mutable_node() = mCamera->GetName();
            scene_buffer.mutable_camera()->set_field_of_view ( mFieldOfView );
            scene_buffer.mutable_camera()->set_near_plane ( mNear );
            scene_buffer.mutable_camera()->set_far_plane ( mFar );
        }
        if ( mLightingPipeline.GetPath() != 0 )
        {
            std::string path = mLightingPipeline.GetPathString();
            if ( !path.empty() )
            {
                *scene_buffer.mutable_lighting_pipeline()->mutable_path() = path;
            }
            else
            {
                scene_buffer.mutable_lighting_pipeline()->set_id ( mLightingPipeline.GetPath() );
            }
        }
        std::unordered_map<const Node*, NodeMsg*> node_map;
        LoopTraverseDFSPreOrder (
            [&node_map] ( const Node & node )
        {
            NodeMsg* node_buffer;
            auto parent = node_map.find ( GetNodePtr ( node.GetParent() ) );
            if ( parent != node_map.end() )
            {
                node_buffer = ( *parent ).second->add_node();
            }
            else
            {
                node_buffer = scene_buffer.add_node();
            }
            node.Serialize ( *node_buffer );
            node_map.emplace ( std::make_pair ( &node, node_buffer ) );
        } );
        std::stringstream serialization;
        if ( aAsBinary )
        {
            serialization << "AEONSCN" << '\0';
            if ( !scene_buffer.SerializeToOstream ( &serialization ) )
            {
                std::cerr << LogLevel::Error << "Failed to serialize scene to binary format.";
                throw std::runtime_error ( "Failed to serialize scene to binary format." );
            }
        }
        else
        {
            std::string text;
            serialization << "AEONSCN\n";
            google::protobuf::TextFormat::Printer printer;
            if ( !printer.PrintToString ( scene_buffer, &text ) )
            {
                std::cerr << LogLevel::Error << "Failed to serialize scene to text format.";
                throw std::runtime_error ( "Failed to serialize scene to text format." );
            }
            serialization << text;
        }
        scene_buffer.Clear();
        return serialization.str();
    }
    void Scene::Deserialize ( const std::string& aSerializedScene )
    {
        static std::mutex m{};
        static SceneMsg scene_buffer{};
        std::lock_guard<std::mutex> hold ( m );
        LoadProtoBufObject ( scene_buffer, aSerializedScene.data(), aSerializedScene.size(), "AEONSCN"_mgk );
        mName = scene_buffer.name();

        std::unordered_map<const NodeMsg*, std::tuple<const NodeMsg*, int, Node* >> node_map;
        for ( auto &i : scene_buffer.node() )
        {
            const NodeMsg* node = &i;
            node_map[node] = std::tuple<const NodeMsg*, int, Node*> {nullptr, 0, Add ( std::make_unique<Node>() ) };
            std::get<2> ( node_map[node] )->Deserialize ( *node );
            while ( node )
            {
                if ( std::get<1> ( node_map[node] ) < node->node().size() )
                {
                    const NodeMsg* prev = node;
                    node = &node->node ( std::get<1> ( node_map[node] ) );
                    node_map[node] = std::tuple<const NodeMsg*, int, Node*> {prev, 0, std::get<2> ( node_map[prev] )->Add ( std::make_unique<Node>() ) };
                    std::get<2> ( node_map[node] )->Deserialize ( *node );
                    ++std::get<1> ( node_map[prev] );
                }
                else
                {
                    std::get<1> ( node_map[node] ) = 0;
                    node = std::get<0> ( node_map[node] );
                }
            }
        }
        SetCamera ( scene_buffer.camera().node() );
        mFieldOfView = scene_buffer.camera().field_of_view();
        mFieldOfView = ( mFieldOfView == 0.0f ) ? 60.0f : mFieldOfView;
        mNear = scene_buffer.camera().near_plane();
        mNear = ( mNear == 0.0f ) ? 1.0f : mNear;
        mFar = scene_buffer.camera().far_plane();
        mFar = ( mFar == 0.0f ) ? 1600.0f : mFar;
        if ( scene_buffer.has_lighting_pipeline() )
        {
            mLightingPipeline = ResourceId{ "Pipeline"_crc32, GetReferenceMsgId ( scene_buffer.lighting_pipeline() ) };
        }
        else
        {
            mLightingPipeline = ResourceId{};
        }
        if ( mCamera )
        {
            mViewMatrix = mCamera->GetGlobalTransform().GetInvertedMatrix();
        }
        scene_buffer.Clear();
    }
}
