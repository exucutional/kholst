# GitHub Copilot Instructions for KHOLST

## Project Overview
KHOLST is a modern C++20 graphics rendering project using Vulkan, with Slang for shader compilation and LightweightVK as the rendering backend.

## C++ Code Conventions

### Naming Conventions

- **Classes/Structs**: PascalCase
  ```cpp
  class SlangCompiler { };
  struct VertexData { };
  ```

- **Functions/Methods**: camelCase
  ```cpp
  bool initialize();
  void compileToSPIRV();
  ```

- **Member Variables**: camelCase (no prefix)
  ```cpp
  class MyClass {
      bool initialized;
      std::string lastDiagnostics;
  };
  ```

- **Local Constants**: kPascalCase
  ```cpp
  constexpr int kMaxBufferSize = 1024;
  const float kDefaultFov = 45.0f;
  ```

- **Global Constants**: UPPERCASE with underscores
  ```cpp
  constexpr int MAX_BUFFER_SIZE = 1024;
  const float DEFAULT_FOV = 45.0f;
  ```

- **Global Static Variables**: lowercase with underscores
  ```cpp
  static int frame_counter = 0;
  static std::vector<uint32_t> spirv_cache;
  ```

- **Namespaces**: lowercase with underscores if needed
  ```cpp
  namespace kholst {
  namespace render {
  namespace shader {
  ```

- **Template Parameters**: PascalCase
  ```cpp
  template<typename DataType, typename AllocatorType>
  ```

### File Organization

- **Header Guards**: Use `#pragma once` exclusively
  ```cpp
  #pragma once
  
  // header content
  ```

- **Brace Style**: Allman style - opening braces on new line for functions, classes, and control structures
  ```cpp
  // Functions
  bool initialize()
  {
      return true;
  }
  
  // Classes
  class MyClass
  {
  public:
      void doSomething();
  };
  
  // Control structures with multiple statements
  if (condition)
  {
      doWork();
      logResult();
  }
  else
  {
      doOtherWork();
  }
  
  // One-liners: braces are optional, omit for single statements
  if (condition)
      doWork();
  else
      doOtherWork();
  
  // Loops
  for (int i = 0; i < count; i++)
  {
      process(i);
  }
  
  while (running)
  {
      update();
  }
  
  // One-liner loops
  for (int i = 0; i < count; i++)
      process(i);
  ```

- **Include Order**:
  1. Corresponding header (for .cpp files)
  2. C++ standard library headers
  3. Third-party library headers (Slang, Vulkan, GLM, etc.)
  4. Project headers
  ```cpp
  #include "myclass.h"          // Corresponding header
  
  #include <vector>             // C++ standard library
  #include <memory>
  #include <string>
  
  #include <slang.h>            // Third-party
  #include <vulkan/vulkan.h>
  
  #include "utils.h"            // Project headers
  ```

- **Header Self-Sufficiency**: All headers must include their dependencies and be compilable on their own

### C++ Standard and Language Features

- **Standard**: C++20
  - Use C++20 features where appropriate (concepts, ranges, etc.)
  - Maintain compatibility with major compilers (MSVC, GCC, Clang)

- **Auto Keyword**: Use sparingly
  - ✅ Acceptable: Iterator types, complex template types, obvious contexts
    ```cpp
    for (auto& item : collection) { }
    auto it = map.find(key);
    auto lambda = [](int x) { return x * 2; };
    ```
  - ❌ Avoid: When type clarity is important
    ```cpp
    // Prefer explicit types
    std::vector<uint32_t> spirvCode;  // Good
    auto spirvCode = std::vector<uint32_t>();  // Avoid
    ```

- **Memory Management**:
  - **Prefer smart pointers** for ownership
    ```cpp
    std::unique_ptr<Renderer> renderer;
    std::shared_ptr<Texture> texture;
    ```
  - Use raw pointers only for non-owning references
  - Prefer `std::unique_ptr` over `std::shared_ptr` unless shared ownership is needed
  - Use `Slang::ComPtr` for COM-style interfaces (Slang API)

- **Error Handling**:
  - **Do NOT use exceptions**
  - Return error codes, bool, or std::optional/std::expected
  - Provide detailed error messages via out parameters
    ```cpp
    bool compileShader(const std::string& path, 
                      std::vector<uint32_t>& outSpirv,
                      std::string& outErrorMsg);
    ```

### Code Structure

- **Composition over Inheritance**
  - Favor composition and interfaces over deep inheritance hierarchies
  - Use inheritance sparingly and primarily for interfaces/abstract base classes

- **Classes vs Structs**
  - **Default to `class`** for types with behavior
  - Use `struct` only for plain data types (POD-like) with public members
    ```cpp
    struct VertexPosition
    {
        float x, y, z;
    };
    
    class Renderer
    {
    public:
        void render();
    private:
        bool initialized;
    };
    ```

- **Function Length**
  - Keep functions focused and reasonably sized
  - Extract complex logic into helper functions
  - Aim for functions under 50 lines when possible

- **Access Specifiers Order**:
  ```cpp
  class MyClass
  {
  public:
      // Public interface first
  
  private:
      // Private implementation last
  };
  ```

### Documentation

- **Style**: Doxygen format
  ```cpp
  /**
   * @brief Brief description of the function
   * 
   * Detailed description if needed, explaining the purpose,
   * algorithm, or important implementation details.
   * 
   * @param input Description of input parameter
   * @param output Description of output parameter
   * @return Description of return value
   */
  bool processData(const Data& input, Result& output);
  ```

- **Documentation Requirements**:
  - **Always document**:
    - All public APIs (classes, functions, methods)
    - Complex algorithms or non-obvious logic
    - Thread-safety considerations
    - Ownership semantics
  - **May skip documentation**:
    - Trivial getters/setters that are self-explanatory
    - Private implementation details that are obvious from code

- **Comment Style**:
  - Use `//` for inline comments
  - Use `/* */` for block comments only when necessary
  - Doxygen `/** */` for API documentation

### Graphics/Rendering Specific

- **Vulkan Conventions**:
  - Follow Vulkan naming conventions for structures that map to Vulkan
  - Use consistent resource naming (buffers, textures, pipelines)
  - Clearly document synchronization requirements

- **Shader Compilation**:
  - Prefer Slang shader compilation at runtime for flexibility
  - Cache compiled SPIRV when appropriate
  - Provide clear error messages for shader compilation failures

- **Resource Management**:
  - Use RAII for all graphics resources
  - Clearly document resource lifetime and ownership
  - Prefer wrapper classes over raw Vulkan handles

- **Coordinates and Math**:
  - Use GLM for math operations
  - Document coordinate system conventions (right-handed, Y-up, etc.)
  - Use explicit types for vectors and matrices (glm::vec3, glm::mat4)

### Code Quality

- **const Correctness**: Use `const` wherever applicable
  ```cpp
  void processData(const std::vector<int>& data) const;
  ```

- **Initialization**: Prefer member initializer lists
  ```cpp
  MyClass::MyClass() 
      : initialized(false)
      , bufferSize(0)
  {
  }
  ```

- **Aggregate Initialization**: Use C++20 designated initializers for structs
  ```cpp
  // Prefer designated initializers
  TargetDesc targetDesc = {
      .format = SLANG_SPIRV,
      .profile = profile,
      .flags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY,
  };
  
  // Avoid field-by-field assignment
  // TargetDesc targetDesc = {};
  // targetDesc.format = SLANG_SPIRV;  // Don't do this
  // targetDesc.profile = profile;
  ```

- **Modern C++ Features**:
  - Use `nullptr` instead of `NULL`
  - Use `override` for virtual function overrides
  - Use `= default` and `= delete` for special member functions
  - Use range-based for loops where appropriate

- **Performance**:
  - Pass large objects by const reference
  - Use move semantics for resource transfer
  - Reserve container capacity when size is known

### Platform Considerations

- **Windows**: Primary development platform
  - Use `#ifdef _WIN32` or `#ifdef _MSC_VER` for Windows-specific code
  - Disable common MSVC warnings appropriately

- **Cross-platform**: Code should be portable where possible
  - Use standard C++ facilities over platform-specific APIs
  - Isolate platform-specific code into separate modules

## Project Structure

```
kholst/
├── src/               # Source files
│   ├── main.cpp
│   ├── render/       # Rendering subsystem
│   │   └── shader/   # Shader compilation and management
│   └── shaders/      # Slang shader source files
├── external/         # External dependencies
└── build/           # Build output (generated)
```

## Build System

- **CMake** is the build system
- Dependencies are managed via Python bootstrap script
- Use CMake's target-based approach for dependencies

## Additional Guidelines

- **Error Messages**: Provide context-rich error messages for debugging
- **Logging**: Use appropriate logging levels (error, warning, info, debug)
- **Thread Safety**: Document thread-safety guarantees explicitly
- **Testing**: Write testable code; prefer dependency injection for flexibility
