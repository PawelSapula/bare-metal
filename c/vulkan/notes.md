# Drawing a triangle step-by-step
    1) VkInstance creation
    2) VkPhysicalDevice - Select a supported GPU
    3) VkDevice (logical device) and VkQueue creation for drawing and presentation
    4) Window creation, window surface & swapchain
    5) Wrap swap chain images into VkImageView
    6) Render pass creation - Specifies render targets and usage
    7) Framebuffers creation for ^^^
    8) Set up graphics pipeline
    9) Allocate & Record a command buffer with commands for every possible swap chain image
    10) Draw frames by aquiring images, submitting the right draw command buffer and returning the images back to the swap chain

# Vulkan SDK
    - Function prefix: `vk`
    - Structs and enums prefix: `Vk´
    - Enumeration values: `VK_`

    - Functions that create an object can have a custom allocator for driver memory (VkAllocationCallbacks)
    - Almost all functions return VkResult.

    - Vulkan´s principle: High performance with low driver overhead.
        -> Limited error checking and capabilities by default
        -> Often causes driver crashes or limited GPU support.
        -> Can develop using validation layers to enable extensive checks, applied between the driver and API communication.

    - Every vulkan object we create has to be released.

    - Structs in vulkan
        - Often has variable "sType" - Struct type that has to be filled out.
        - Also "pType" that can point to extension information in the future.
