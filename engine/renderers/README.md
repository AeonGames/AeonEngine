# Renderer Plugin Development Guide

Welcome to the AeonEngine renderer plugin development guide! This document will help you understand how to port existing renderers or create new ones for the AeonEngine.

## Architecture Overview

The AeonEngine renderer system is designed around a **minimum common denominator** approach. This means that the API and feature set are intentionally limited to what can be consistently implemented across all supported graphics APIs (OpenGL, Vulkan, DirectX, etc.).

### Why Minimum Common Denominator?

This design philosophy ensures:

- **Consistency**: Shaders and rendering code work identically across all platforms
- **Portability**: Games can target multiple graphics APIs without API-specific code paths
- **Maintainability**: Single shader authoring workflow for all platforms
- **Predictability**: Developers know exactly what features are available

## Key Design Principles

### Uniform Variables Structure

The engine enforces a **structured uniform approach** rather than allowing free-floating uniform variables, even though some APIs (like OpenGL) support them natively.

#### Why No Free Uniforms in OpenGL?

Even though OpenGL supports individual uniform variables, the engine requires all uniforms to be organized into **Uniform Buffer Objects (UBOs)** to maintain compatibility with:

- **Vulkan**: Which primarily uses descriptor sets and uniform buffers
- **DirectX**: Which uses constant buffers
- **Metal**: Which uses buffer-based uniforms

This ensures that shaders written for one API work identically on all others.

### Reserved Variable Names

Certain variable names carry special meaning within the engine and are automatically handled by the renderer system:

#### Transform Matrices

- **`ModelMatrix`**: Object-to-world transformation matrix
- **`ViewMatrix`**: World-to-view (camera) transformation matrix  
- **`ProjectionMatrix`**: View-to-clip space projection matrix

#### Material System

- **`Material`**: Uniform block containing material properties
- **`Skeleton`**: Uniform block for skeletal animation bone matrices

#### Usage Example

```glsl
// Standard matrices uniform block
layout(binding = 0, std140) uniform Matrices {
    mat4 ModelMatrix;
    mat4 ViewMatrix; 
    mat4 ProjectionMatrix;
};

// Material properties uniform block
layout(binding = 1, std140) uniform Material {
    vec4 diffuseColor;
    float roughness;
    float metallic;
    // ... other material properties
};

// Skeletal animation (when needed)
layout(binding = 3, std140) uniform Skeleton {
    mat4 skeleton[256];
};
```

## Implementing a New Renderer

### 1. Core Classes to Implement

#### Renderer Class

```cpp
class MyRenderer : public Renderer {
public:
    // Initialize graphics context and resources
    MyRenderer(/* parameters */);
    
    // Core rendering methods
    void BeginRender() override;
    void EndRender() override;
    void Render(const Mesh& mesh, const Pipeline& pipeline, const Transform& transform) override;
    
    // Resource management
    std::unique_ptr<Pipeline> CreatePipeline(const PipelineDescriptor& descriptor) override;
    std::unique_ptr<Buffer> CreateBuffer(const BufferDescriptor& descriptor) override;
    std::unique_ptr<Texture> CreateTexture(const TextureDescriptor& descriptor) override;
};
```

#### Pipeline Class

```cpp
class MyPipeline : public Pipeline {
public:
    MyPipeline(const MyRenderer& renderer, const PipelineDescriptor& descriptor);
    
    // Shader reflection and uniform extraction
    void ReflectUniforms(/* shader module */);
    void ReflectAttributes(/* shader module */);
    
    // Uniform block and sampler access
    const UniformBlock* GetUniformBlock(uint32_t nameHash) const override;
    uint32_t GetSamplerBinding(uint32_t nameHash) const override;
};
```

### 2. Shader Compilation Pipeline

Each renderer must handle shader compilation from a common source format to the target API:

```cpp
void CompileShaders(const PipelineDescriptor& descriptor) {
    // 1. Parse common shader source
    // 2. Convert to API-specific format (SPIR-V, HLSL, GLSL)
    // 3. Compile for target platform
    // 4. Extract reflection data (uniforms, attributes, samplers)
    // 5. Create native pipeline objects
}
```

### 3. Uniform Buffer Management

Implement consistent uniform buffer handling:

```cpp
// Map engine uniform blocks to API-specific resources
void BindUniformBlock(const std::string& blockName, const Buffer& buffer) {
    uint32_t nameHash = crc32(blockName);
    
    if (nameHash == crc32("Matrices")) {
        // Bind to matrices descriptor set/slot
    } else if (nameHash == crc32("Material")) {
        // Bind to material descriptor set/slot  
    } else if (nameHash == crc32("Skeleton")) {
        // Bind to skeleton descriptor set/slot
    }
}
```

## Porting Existing Renderers

### From OpenGL

1. **Replace free uniforms**: Convert individual `glUniform*` calls to UBO-based approach
2. **Structured binding**: Organize uniforms into logical blocks (Matrices, Material, etc.)
3. **Consistent reflection**: Extract uniform information using a common interface

### From DirectX

1. **Constant buffer mapping**: Map DirectX constant buffers to engine uniform blocks
2. **Resource binding**: Adapt DirectX resource slots to engine binding model
3. **Shader reflection**: Use DirectX reflection APIs to extract uniform metadata

### From Vulkan

1. **Descriptor set organization**: Map Vulkan descriptor sets to engine uniform blocks
2. **Pipeline layout**: Ensure pipeline layouts match engine expectations
3. **SPIR-V reflection**: Use SPIR-V Reflect library for uniform extraction

## Best Practices

### Naming Conventions

- Use CRC32 hashing for uniform and attribute name lookups
- Maintain sorted containers for efficient searching
- Follow consistent binding slot assignments across all renderers

### Resource Management

```cpp
// Use RAII for resource cleanup
class MyBuffer : public Buffer {
    ~MyBuffer() {
        // Clean up API-specific resources
        if (mNativeBuffer) {
            api_delete_buffer(mNativeBuffer);
        }
    }
};
```

### Error Handling

```cpp
// Provide detailed error messages for debugging
if (result != SUCCESS) {
    std::ostringstream stream;
    stream << "Pipeline creation failed: " << GetErrorString(result);
    throw std::runtime_error(stream.str());
}
```

## Testing Your Renderer

### Validation Steps

1. **Uniform reflection**: Verify all expected uniforms are detected and accessible
2. **Cross-platform consistency**: Ensure identical rendering results across APIs
3. **Performance testing**: Validate that performance characteristics are reasonable
4. **Memory management**: Check for leaks and proper resource cleanup

### Debug Output

Enable verbose logging to track:

- Shader compilation results
- Uniform block detection
- Resource binding operations
- Pipeline state changes

## Example Renderers

Study the existing renderer implementations:

- **OpenGL Renderer**: `engine/renderers/opengl/`
- **Vulkan Renderer**: `engine/renderers/vulkan/`

These provide complete examples of the architectural patterns and implementation strategies described in this guide.

## Contributing

When submitting a new renderer:

1. Ensure full compliance with the minimum common denominator design
2. Include comprehensive tests covering all major features
3. Document any platform-specific limitations or optimizations
4. Follow the existing code style and patterns

---

Happy rendering! The minimum common denominator approach might seem restrictive at first, but it enables the engine to provide consistent, predictable behavior across all platforms while maintaining excellent performance characteristics.
