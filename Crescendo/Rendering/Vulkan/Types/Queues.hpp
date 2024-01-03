#pragma once

#include "common.hpp"

#include "VkBootstrap/VkBootstrap.h"

CS_NAMESPACE_BEGIN::Vulkan
{
	class Queues
	{
	public:
		struct Queue { VkQueue queue;  uint32_t family; };
		/*
		 *	Universal is capable of all operations
		 *	Transfer is transfer dedicated
		 *	Compute is compute dedicated
		 */
		Queue universal, transfer, compute;

		Queues() = default;
		Queues(const vkb::Device& device)
		{
			auto universal = device.get_queue(vkb::QueueType::graphics);
			auto transfer = device.get_queue(vkb::QueueType::transfer);
			auto compute = device.get_queue(vkb::QueueType::compute);

			if (!universal) cs_std::console::fatal("Failed to get universal queue");
			if (!transfer) cs_std::console::warn("Failed to get transfer queue:", transfer.error().message(), ". Falling back to universal"); // Fallback to universal
			if (!compute) cs_std::console::warn("Failed to get compute queue:", compute.error().message(), ". Falling back to universal"); // Fallback to universal

			this->universal.queue = universal.value();
			this->transfer.queue = (transfer) ? transfer.value() : this->universal.queue; // Fallback to universal
			this->compute.queue = (compute) ? compute.value() : this->universal.queue; // Fallback to universal

			this->universal.family = device.get_queue_index(vkb::QueueType::graphics).value();
			this->transfer.family = device.get_queue_index((transfer) ? vkb::QueueType::transfer : vkb::QueueType::graphics).value(); // Fallback to universal
			this->compute.family = device.get_queue_index((compute) ? vkb::QueueType::compute : vkb::QueueType::graphics).value(); // Fallback to universal
		}
	};
}