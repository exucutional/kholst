#pragma once

#include <slang.h>
#include <slang-com-ptr.h>
#include <string>
#include <vector>


namespace kholst
{
namespace render
{
namespace shader
{

class SlangCompiler
{
public:
    SlangCompiler();
    ~SlangCompiler();

    // Initialize the compiler with target settings
    bool initialize(SlangCompileTarget target = SLANG_SPIRV);

    // Compile a Slang shader file to SPIRV
    bool compileToSPIRV(
        const std::string& shaderPath,
        const std::string& entryPoint,
        SlangStage stage,
        std::vector<uint32_t>& outSpirv,
        std::string& outErrorMsg
    );

    // Compile vertex and fragment shaders from a single Slang file
    bool compileVertexFragment(
        const std::string& shaderPath,
        const std::string& vertexEntry,
        const std::string& fragmentEntry,
        std::vector<uint32_t>& outVertexSpirv,
        std::vector<uint32_t>& outFragmentSpirv,
        std::string& outErrorMsg
    );

    // Get diagnostics from the last compilation
    std::string getLastDiagnostics() const;

    // Check if the compiler is initialized
    bool isInitialized() const { return initialized; }

private:
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    Slang::ComPtr<slang::ISession> session;
    bool initialized = false;
    std::string lastDiagnostics;

    // Helper to create a compilation request
    bool compileEntryPoint(
        const std::string& shaderPath,
        const std::string& entryPoint,
        SlangStage stage,
        std::vector<uint32_t>& outSpirv,
        std::string& outErrorMsg
    );
};

} // namespace shader
} // namespace render
} // namespace kholst

