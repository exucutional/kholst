#include "compiler.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace kholst
{
namespace render
{
namespace shader
{

SlangCompiler::SlangCompiler() = default;

SlangCompiler::~SlangCompiler() = default;

bool SlangCompiler::initialize(SlangCompileTarget target)
{
    if (initialized)
        return true;

    // Create global session
    if (SLANG_FAILED(slang::createGlobalSession(globalSession.writeRef())))
    {
        lastDiagnostics = "Failed to create Slang global session";
        return false;
    }

    // Configure session description
    slang::TargetDesc targetDesc = {
        .format = target,
        .profile = globalSession->findProfile("spirv_1_5"),
    };

    slang::SessionDesc sessionDesc = {
        .targets = &targetDesc,
        .targetCount = 1,
    };
    
    // Add common search paths if needed
    // sessionDesc.searchPaths = searchPaths;
    // sessionDesc.searchPathCount = searchPathCount;

    // Create session
    if (SLANG_FAILED(globalSession->createSession(sessionDesc, session.writeRef())))
    {
        lastDiagnostics = "Failed to create Slang session";
        return false;
    }

    initialized = true;
    return true;
}

bool SlangCompiler::compileToSPIRV(
    const std::string& shaderPath,
    const std::string& entryPoint,
    SlangStage stage,
    std::vector<uint32_t>& outSpirv,
    std::string& outErrorMsg
)
{
    if (!initialized)
    {
        outErrorMsg = "Compiler not initialized";
        return false;
    }

    return compileEntryPoint(shaderPath, entryPoint, stage, outSpirv, outErrorMsg);
}

bool SlangCompiler::compileVertexFragment(
    const std::string& shaderPath,
    const std::string& vertexEntry,
    const std::string& fragmentEntry,
    std::vector<uint32_t>& outVertexSpirv,
    std::vector<uint32_t>& outFragmentSpirv,
    std::string& outErrorMsg
)
{
    if (!initialized)
    {
        outErrorMsg = "Compiler not initialized";
        return false;
    }

    // Compile vertex shader
    if (!compileEntryPoint(shaderPath, vertexEntry, SLANG_STAGE_VERTEX, outVertexSpirv, outErrorMsg))
        return false;

    // Compile fragment shader
    if (!compileEntryPoint(shaderPath, fragmentEntry, SLANG_STAGE_FRAGMENT, outFragmentSpirv, outErrorMsg))
        return false;

    return true;
}

bool SlangCompiler::compileEntryPoint(
    const std::string& shaderPath,
    const std::string& entryPoint,
    SlangStage stage,
    std::vector<uint32_t>& outSpirv,
    std::string& outErrorMsg
)
{
    lastDiagnostics.clear();

    // Read shader file
    std::ifstream file(shaderPath, std::ios::binary);
    if (!file.is_open())
    {
        outErrorMsg = "Failed to open shader file: " + shaderPath;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string shaderCode = buffer.str();

    // Load the module
    Slang::ComPtr<slang::IBlob> diagnosticsBlob;
    slang::IModule* module = session->loadModule(
        shaderPath.c_str(),
        diagnosticsBlob.writeRef()
    );

    if (diagnosticsBlob)
    {
        lastDiagnostics = std::string(
            (const char*)diagnosticsBlob->getBufferPointer(),
            diagnosticsBlob->getBufferSize()
        );
    }

    if (!module)
    {
        outErrorMsg = "Failed to load module: " + shaderPath + "\n" + lastDiagnostics;
        return false;
    }

    // Find entry point
    Slang::ComPtr<slang::IEntryPoint> entryPointObj;
    module->findAndCheckEntryPoint(entryPoint.c_str(), stage, entryPointObj.writeRef(),
        diagnosticsBlob.writeRef());

    if (diagnosticsBlob)
    {
        lastDiagnostics = std::string(
            (const char*)diagnosticsBlob->getBufferPointer(),
            diagnosticsBlob->getBufferSize()
        );
    }

    if (!entryPointObj)
    {
        outErrorMsg = "Failed to find entry point: " + entryPoint + " in " + shaderPath + "\n" + lastDiagnostics;
        return false;
    }

    // Create component type list for linking
    std::vector<slang::IComponentType*> componentTypes;
    componentTypes.push_back(module);
    componentTypes.push_back(entryPointObj);

    // Create composite component type (linked program)
    Slang::ComPtr<slang::IComponentType> composedProgram;
    {
        Slang::ComPtr<slang::IBlob> composeDiagnostics;
        SlangResult result = session->createCompositeComponentType(
            componentTypes.data(),
            componentTypes.size(),
            composedProgram.writeRef(),
            composeDiagnostics.writeRef()
        );

        if (composeDiagnostics)
        {
            std::string diag(
                (const char*)composeDiagnostics->getBufferPointer(),
                composeDiagnostics->getBufferSize()
            );
            lastDiagnostics += diag;
        }

        if (SLANG_FAILED(result))
        {
            outErrorMsg = "Failed to compose program\n" + lastDiagnostics;
            return false;
        }
    }

    // Link the program
    Slang::ComPtr<slang::IComponentType> linkedProgram;
    {
        Slang::ComPtr<slang::IBlob> linkDiagnostics;
        SlangResult result = composedProgram->link(linkedProgram.writeRef(), linkDiagnostics.writeRef());

        if (linkDiagnostics)
        {
            std::string diag(
                (const char*)linkDiagnostics->getBufferPointer(),
                linkDiagnostics->getBufferSize()
            );
            lastDiagnostics += diag;
        }

        if (SLANG_FAILED(result))
        {
            outErrorMsg = "Failed to link program\n" + lastDiagnostics;
            return false;
        }
    }

    // Print reflection information
    {
        slang::ProgramLayout* layout = linkedProgram->getLayout();
        if (layout)
        {
            std::cout << "=== Shader Reflection: " << entryPoint << " ===" << std::endl;
            
            // Print entry point info
            unsigned int entryPointCount = layout->getEntryPointCount();
            std::cout << "Entry Points: " << entryPointCount << std::endl;
            
            for (unsigned int i = 0; i < entryPointCount; i++)
            {
                slang::EntryPointLayout* ep = layout->getEntryPointByIndex(i);
                const char* epName = ep->getName();
                SlangStage epStage = ep->getStage();
                
                const char* stageName = "unknown";
                switch (epStage)
                {
                    case SLANG_STAGE_VERTEX: stageName = "vertex"; break;
                    case SLANG_STAGE_FRAGMENT: stageName = "fragment"; break;
                    case SLANG_STAGE_COMPUTE: stageName = "compute"; break;
                    case SLANG_STAGE_GEOMETRY: stageName = "geometry"; break;
                    case SLANG_STAGE_HULL: stageName = "hull"; break;
                    case SLANG_STAGE_DOMAIN: stageName = "domain"; break;
                    default: break;
                }
                
                std::cout << "  [" << i << "] " << epName << " (" << stageName << ")" << std::endl;
            }
            
            // Print global parameters
            unsigned int globalParamCount = layout->getParameterCount();
            if (globalParamCount > 0)
            {
                std::cout << "Global Parameters: " << globalParamCount << std::endl;
                
                for (unsigned int i = 0; i < globalParamCount; i++)
                {
                    slang::VariableLayoutReflection* param = layout->getParameterByIndex(i);
                    const char* paramName = param->getName();
                    slang::TypeReflection* paramType = param->getType();
                    const char* typeName = paramType ? paramType->getName() : "unknown";
                    
                    std::cout << "  [" << i << "] " << paramName << " : " << typeName << std::endl;
                }
            }
            
            std::cout << "================================" << std::endl;
        }
    }

    // Get the compiled code
    Slang::ComPtr<slang::IBlob> spirvCode;
    {
        Slang::ComPtr<slang::IBlob> getDiagnostics;
        SlangResult result = linkedProgram->getEntryPointCode(
            0, // Entry point index (we only have one in the linked program)
            0, // Target index
            spirvCode.writeRef(),
            getDiagnostics.writeRef()
        );

        if (getDiagnostics)
        {
            std::string diag(
                (const char*)getDiagnostics->getBufferPointer(),
                getDiagnostics->getBufferSize()
            );
            lastDiagnostics += diag;
        }

        if (SLANG_FAILED(result) || !spirvCode)
        {
            outErrorMsg = "Failed to get compiled code\n" + lastDiagnostics;
            return false;
        }
    }

    // Copy SPIRV data to output vector
    const uint32_t* spirvData = (const uint32_t*)spirvCode->getBufferPointer();
    size_t spirvSize = spirvCode->getBufferSize();
    
    if (spirvSize % 4 != 0)
    {
        outErrorMsg = "Invalid SPIRV size (not multiple of 4 bytes)";
        return false;
    }

    outSpirv.resize(spirvSize / 4);
    std::memcpy(outSpirv.data(), spirvData, spirvSize);

    return true;
}

std::string SlangCompiler::getLastDiagnostics() const
{
    return lastDiagnostics;
}

} // namespace shader
} // namespace render
} // namespace kholst
