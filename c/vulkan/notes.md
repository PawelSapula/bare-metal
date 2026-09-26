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

# Graphical programming
    - Mipmap: Scaled image that for example is far away of us, reduced resolution because it's going to be small either way.

    Graphics pipeline: Sequence of operations that are going to create something that can be displayed with a GPU.
        Input: Vertex/index buffer
        - Input assembler (P)       -- Collects raw vertex data from buffers and can store be stored in index buffers for reuse.
        - Vertex shader             -- Applies transformations to every vertex. Passes down vertex data.
        - Tessellation              -- Allows to subdivide geometry to increase mesh quality. (ex. close brick walls, stairs)
        - Geometry shader           -- Runs every primitive (triangle, line, point), Can discard or output more that came in.
        - Rasterization (P)         -- Primitives -> Fragments (Pixel elements). Discards everything outside the screen.
                                       Discards usually by depth too.
        - Fragment shader           -- Determines which framebuffers the fragments are written to, with which color and depth vales.
        - Color blending  (P)       -- Operations to mix different fragments that map to the same pixel in the framebuffer.
        Output: Framebuffer 

        P -> Stages that are pre-defined but can be tweaked on with parameters.
        Rest of the stages are programmable you can upload own code to apply exactly the operations that you want.
        Each pipeline has to be recreated for scrach for minor changes due to Vulkan providing an immutable state. Despite this, being known in advance the driver can optimize it much better.

    - Shader code has to be written in a bytecode format SPIR-V (Opposed to human-readable languages like GLSL and HLSL)
        - VulkanSDK comes tho with a compiled translator for GLSL that fullfills all the standards so the code works on all non-trivial GPU's.

    - Clip coorinates: A 4D vector turned into normalized screen coordinates ([-1, -1] [1,1]) by dividing the vector by its last component w.
                       Z -> [0,1]

