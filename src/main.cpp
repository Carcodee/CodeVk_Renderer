﻿//

// Created by carlo on 2024-09-21.
//



#include "WindowAPI/WindowInclude.hpp"
#include "Engine/EngineInclude.hpp"
#include "Rendering/RenderingInclude.hpp"

#define ENGINE_ENABLE_DEBUGGING

#define GLM_FORCE_RADIANS
#define GLM_DEPTH_ZERO_TO_ONE
//#define GLM_FORCE_RIGHT_HANDED
#define GLM_ENABLE_EXPERIMENTAL

CONST int WINDOWS_WIDTH = 1024;
CONST int WINDOWS_HEIGHT = 1024;


void run(WindowProvider* windowProvider)
{
    int imageCount = 3;
    
    ENGINE::WindowDesc windowDesc = {};
    windowDesc.hInstance = GetModuleHandle(NULL);
    windowDesc.hWnd = glfwGetWin32Window(windowProvider->window);

    bool enableDebugging = false;
#if defined ENGINE_ENABLE_DEBUGGING
    enableDebugging = true;
#endif

    const char* glfwExtensions[] = {"VK_KHR_surface", "VK_KHR_win32_surface"};
    uint32_t glfwExtensionCount = sizeof(glfwExtensions) / sizeof(glfwExtensions[0]);

    std::unique_ptr<ENGINE::Core> core = std::make_unique<ENGINE::Core>(
        glfwExtensions, glfwExtensionCount, &windowDesc, enableDebugging);
    
    std::unique_ptr<ENGINE::RenderGraph> renderGraph = core->CreateRenderGraph();
    
    std::unique_ptr<ENGINE::InFlightQueue> inFlightQueue = std::make_unique<ENGINE::InFlightQueue>(
        core.get(),renderGraph.get(), windowDesc, imageCount, vk::PresentModeKHR::eMailbox,
        windowProvider->GetWindowSize());
    
    std::unique_ptr<ENGINE::DescriptorAllocator> descriptorAllocator = std::make_unique<ENGINE::DescriptorAllocator>();
   
    std::vector<ENGINE::DescriptorAllocator::PoolSizeRatio> poolSizeRatios ={
        {vk::DescriptorType::eSampler, 1.5f},
        {vk::DescriptorType::eStorageBuffer, 1.5f},
        {vk::DescriptorType::eUniformBuffer, 1.5f},
        {vk::DescriptorType::eStorageImage, 1.5f},
    };
    descriptorAllocator->BeginPool(core->logicalDevice.get(), 10, poolSizeRatios);


    std::unique_ptr<Rendering::ForwardRenderer> fRenderer = std::make_unique<Rendering::ForwardRenderer>(core.get(), windowProvider, descriptorAllocator.get());
    fRenderer->SetRenderOperation(inFlightQueue.get());
    
    std::unique_ptr<Rendering::ImguiRenderer> imguiRenderer = std::make_unique<Rendering::ImguiRenderer>(
        core.get(), windowProvider);
     
    while (!windowProvider->WindowShouldClose())
    {
        windowProvider->PollEvents();
        {
            glm::uvec2 windowSize = windowProvider->GetWindowSize();
            if (core->resizeRequested || windowProvider->framebufferResized)
            {
                std::cout << "recreated swapchain\n";
                core->WaitIdle();
                inFlightQueue.reset();
                inFlightQueue = std::make_unique<ENGINE::InFlightQueue>(
                    core.get(), renderGraph.get(),windowDesc, imageCount, vk::PresentModeKHR::eMailbox,
                    windowSize);
                windowProvider->framebufferResized = false;
                core->resizeRequested = false;
                renderGraph->RecreateFrameResources();
                fRenderer->RecreateSwapChainResources();
                fRenderer->SetRenderOperation(inFlightQueue.get());
            }
            try
            {

                if (glfwGetKey(windowProvider->window, GLFW_KEY_W))
                {
                   fRenderer->ReloadShaders(); 
                }
                
                inFlightQueue->BeginFrame();

                auto& currFrame = inFlightQueue->frameResources[inFlightQueue->frameIndex];

                core->renderGraphRef->ExecuteAll(&currFrame);
              
                imguiRenderer->RenderFrame(currFrame.commandBuffer.get(),
                                           inFlightQueue->currentSwapchainImageView->imageView.get());
                
                inFlightQueue->EndFrame();
            }
            catch (vk::OutOfDateKHRError err)
            {
                core->resizeRequested = true;
            }
        }
        core->WaitIdle();
    }
    imguiRenderer->Destroy();
    windowProvider->DestroyWindow();
}

int main()
{
    std::unique_ptr<WindowProvider> windowProvider= std::make_unique<WindowProvider>(WINDOWS_WIDTH, WINDOWS_HEIGHT, "Vulkan Engine Template");
    windowProvider->InitGlfw();
    
    run(windowProvider.get());

    windowProvider->Terminate();

    return 0;
}
