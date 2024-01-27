#pragma once
/*
 * Unlike the default vulkan.hpp raii classes, these classes can take ownership of raw handles
 * However they are not capable (yet) of custom allocators
 */
#include "raii/Allocator.hpp"
#include "raii/Buffer.hpp"
#include "raii/DescriptorSetLayout.hpp"
#include "raii/Device.hpp"
#include "raii/Fence.hpp"
#include "raii/Framebuffer.hpp"
#include "raii/Image.hpp"
#include "raii/ImageView.hpp"
#include "raii/Instance.hpp"
#include "raii/PhysicalDevice.hpp"
#include "raii/PipelineLayout.hpp"
#include "raii/RenderPass.hpp"
#include "raii/Semaphore.hpp"
#include "raii/ShaderModule.hpp"
#include "raii/Surface.hpp"
#include "raii/Swapchain.hpp"