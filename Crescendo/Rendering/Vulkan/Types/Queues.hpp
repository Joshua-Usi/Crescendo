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
			this->universal.queue = device.get_queue(vkb::QueueType::graphics).value();
			this->transfer.queue = device.get_queue(vkb::QueueType::transfer).value();
			this->compute.queue = device.get_queue(vkb::QueueType::compute).value();

			this->universal.family = device.get_queue_index(vkb::QueueType::graphics).value();
			this->transfer.family = device.get_queue_index(vkb::QueueType::transfer).value();
			this->compute.family = device.get_queue_index(vkb::QueueType::compute).value();
		}
	};
}