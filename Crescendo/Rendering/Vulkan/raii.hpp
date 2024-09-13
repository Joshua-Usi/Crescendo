#pragma once
/*
 * Unlike the default vulkan.hpp raii classes, these classes can take ownership of raw handles
 * However they are not capable (yet) of custom allocators
 */
#include "raii/Buffer.hpp"
#include "raii/Device.hpp"
#include "raii/Fence.hpp"
#include "raii/Framebuffer.hpp"
#include "raii/Image.hpp"
#include "raii/Instance.hpp"
#include "raii/PhysicalDevice.hpp"
#include "raii/Pipeline.hpp"
#include "raii/PipelineVariants.hpp"
#include "raii/RenderPass.hpp"
#include "raii/Sampler.hpp"
#include "raii/Semaphore.hpp"
#include "raii/ShaderModule.hpp"
#include "raii/ShaderReflection.hpp"
#include "raii/Surface.hpp"
#include "raii/Swapchain.hpp"