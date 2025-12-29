#include <lvk/LVK.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "utils.h"

static constexpr uint32_t WIDTH = 1680;
static constexpr uint32_t HEIGHT = 720;

static constexpr uint32_t CUBE_TRIANGLES = 36;

static const char* LOG_FILE_PATH = ".log.last.txt"; 

static const char* VERT_PATH = "src/shaders/cube.vert";
static const char* FRAG_PATH = "src/shaders/cube.frag";

class WindowApp final
{
public:
    WindowApp(const char *name, int width, int height)
    : window(lvk::initWindow(name, width, height), glfwDestroyWindow)
    {
        ctx = lvk::createVulkanContextWithSwapchain(window.get(), width, height, {});
        initRender();
    }
    
    void initRender()
    {
        vert = load_shader_module(ctx, VERT_PATH);
        frag = load_shader_module(ctx, FRAG_PATH);
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
