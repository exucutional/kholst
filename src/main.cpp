#include <lvk/LVK.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "utils.h"
#include "render/shader/compiler/compiler.h"

static constexpr uint32_t WIDTH = 1680;
static constexpr uint32_t HEIGHT = 720;

static constexpr uint32_t CUBE_TRIANGLES = 36;

static const char* LOG_FILE_PATH = ".log.last.txt"; 

static const char* SLANG_CUBE_PATH = "src/shaders/cube.slang";

class WindowApp final
{
public:
    WindowApp(const char *name, int width, int height)
    : window(lvk::initWindow(name, width, height), glfwDestroyWindow)
    {
        ctx = lvk::createVulkanContextWithSwapchain(window.get(), width, height, {});
        
        // Initialize Slang compiler
        if (!compiler.initialize(SLANG_SPIRV))
        {
            LLOGW("Failed to initialize Slang compiler: %s\n", compiler.getLastDiagnostics().c_str());
            return;
        }
        
        initRender();
    }
    
    void initRender()
    {
        // Compile vertex and fragment shaders from Slang
        std::vector<uint32_t> vertSpirv, fragSpirv;
        std::string errorMsg;
        
        if (!compiler.compileVertexFragment(
            SLANG_CUBE_PATH,
            "cubeVertex",
            "cubeFragment",
            vertSpirv,
            fragSpirv,
            errorMsg))
        {
            LLOGW("Failed to compile shaders: %s\n", errorMsg.c_str());
            LLOGW("Diagnostics: %s\n", compiler.getLastDiagnostics().c_str());
            return;
        }
        
        // Create shader modules from compiled SPIRV
        vert = ctx->createShaderModule({
            vertSpirv.data(),
            vertSpirv.size() * sizeof(uint32_t),
            lvk::Stage_Vert,
            "Shader Module: cube.slang (vert)"
        });
        
        frag = ctx->createShaderModule({
            fragSpirv.data(),
            fragSpirv.size() * sizeof(uint32_t),
            lvk::Stage_Frag,
            "Shader Module: cube.slang (frag)"
        });
        
        pipeline = ctx->createRenderPipeline({
            .smVert = vert,
            .smFrag = frag,
            .color  = { { .format = ctx->getSwapchainFormat() } },
            .cullMode = lvk::CullMode_Back,
        });
        
        wireframePipeline = ctx->createRenderPipeline({
            .smVert = vert,
            .smFrag = frag,
            .specInfo = { .entries = { { .constantId = 0, .size = sizeof(isWireframe) } }, .data = &isWireframe, .dataSize = sizeof(isWireframe) },
            .color  = { { .format = ctx->getSwapchainFormat() } },
            .cullMode = lvk::CullMode_Back,
            .polygonMode = lvk::PolygonMode_Line,
        });
    }

    void run()
    {
        while (!glfwWindowShouldClose(window.get()))
        {
            glfwPollEvents();

            int width = 0;
            int height = 0;
            glfwGetFramebufferSize(window.get(), &width, &height);

            if (!width || !height)
                continue;

            const float ratio = width / (float)height;

            const glm::mat4 m = glm::rotate(
                glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.5f)),
                (float)glfwGetTime(),
                glm::vec3(1.0f, 1.0f, 1.0f));
            const glm::mat4 p = glm::perspective(45.0f, ratio, 0.1f, 1000.0f);

            lvk::ICommandBuffer& buf = ctx->acquireCommandBuffer();

            buf.cmdBeginRendering(
                { .color = { { .loadOp = lvk::LoadOp_Clear, .clearColor = { 1.0f, 1.0f, 1.0f, 1.0f } } } },
                { .color = { { .texture = ctx->getCurrentSwapchainTexture() } } }
            );

            {
                buf.cmdPushDebugGroupLabel("Render cube", 0xff0000ff);
                buf.cmdBindRenderPipeline(pipeline);
                buf.cmdPushConstants(p * m);
                buf.cmdDraw(CUBE_TRIANGLES);
                buf.cmdPopDebugGroupLabel();
            }

            {
                buf.cmdPushDebugGroupLabel("Render wireframe cube", 0xff0000ff);
                buf.cmdBindRenderPipeline(wireframePipeline);
                buf.cmdPushConstants(p * m);
                buf.cmdDraw(CUBE_TRIANGLES);
                buf.cmdPopDebugGroupLabel();
            }

            buf.cmdEndRendering();

            ctx->submit(buf, ctx->getCurrentSwapchainTexture());
        }
    }

    ~WindowApp()
    {
        window.reset();
        vert.reset();
        frag.reset();
        pipeline.reset();
        wireframePipeline.reset();
        ctx.reset();
        glfwTerminate();
    }
private:
    VkBool32 isWireframe = false;

    kholst::render::shader::SlangCompiler compiler;
    lvk::Holder<lvk::ShaderModuleHandle> vert;
    lvk::Holder<lvk::ShaderModuleHandle> frag;
    lvk::Holder<lvk::RenderPipelineHandle> pipeline;
    lvk::Holder<lvk::RenderPipelineHandle> wireframePipeline;

    std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window;
    std::unique_ptr<lvk::IContext> ctx;
};

int main()
{
    minilog::initialize(LOG_FILE_PATH, { .threadNames = false });
    WindowApp app("Kholst", WIDTH, HEIGHT);
    app.run();
    return 0;
}
