#version 450

// Depth pre-pass mark stage (Phase R2). Marks the cluster each rasterized
// fragment falls into as active, so the light-cull stage can skip clusters that
// contain no geometry. The colour attachment is written with a sentinel value
// and discarded; the main colour pass re-clears it.
#ifdef VULKAN
layout(set = 0, binding = 0, std140)
#else
layout(binding = 0, std140)
#endif
uniform Matrices
{
#ifndef VULKAN
      mat4 ModelMatrix;
#endif
      mat4 ProjectionMatrix;
      mat4 ViewMatrix;
};

// Clustered-shading parameters, shared with the lighting compute pipeline.
#ifdef VULKAN
layout(set = 1, binding = 0, std140)
#else
layout(binding = 1, std140)
#endif
uniform ClusterParams
{
      mat4  inverse_projection;
      uvec4 cluster_grid;   // x,y,z cluster counts; w = total cluster count
      vec4  cluster_screen; // viewport width, height, heatmap-toggle, active-cull-enable
};

// Per-cluster active flag. Set to 1 for every cluster a fragment lands in.
#ifdef VULKAN
layout(set = 2, binding = 0, std430)
#else
layout(binding = 0, std430)
#endif
buffer ClusterActive
{
      uint cluster_active[];
};

layout(location = 0) in vec3 eyeCoords;
layout(location = 0) out vec4 FragColor;

// Map a view-space fragment to its cluster index, matching the tiling used by
// the cluster-build compute stage and diffuse_map_phong.frag exactly.
uint fragment_cluster_index()
{
      vec4 clip = ProjectionMatrix * vec4 ( eyeCoords, 1.0 );
      vec2 ndc = clip.xy / clip.w;

      uint gx = cluster_grid.x;
      uint gy = cluster_grid.y;
      uint gz = cluster_grid.z;
      uint ix = uint ( clamp ( ( ndc.x * 0.5 + 0.5 ) * float ( gx ), 0.0, float ( gx - 1u ) ) );
      uint iy = uint ( clamp ( ( ndc.y * 0.5 + 0.5 ) * float ( gy ), 0.0, float ( gy - 1u ) ) );

      vec4 n = inverse_projection * vec4 ( 0.0, 0.0, -1.0, 1.0 );
      vec4 f = inverse_projection * vec4 ( 0.0, 0.0,  1.0, 1.0 );
      float near_depth = n.y / n.w;
      float far_depth  = f.y / f.w;
      float depth = max ( eyeCoords.y, near_depth );

      float slice = log ( depth / near_depth ) / log ( far_depth / near_depth );
      uint iz = uint ( clamp ( slice * float ( gz ), 0.0, float ( gz - 1u ) ) );

      return ix + iy * gx + iz * gx * gy;
}

void main()
{
      cluster_active[fragment_cluster_index()] = 1u;
      FragColor = vec4 ( 0.0 );
}
