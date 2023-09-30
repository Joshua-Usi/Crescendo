#pragma once

#include "VkBootstrap/VkBootstrap.h"

namespace Crescendo::internal
{
	class QueueManager
	{
	public:
		struct Queue { VkQueue queue;  uint32_t family; };
		/*
		 *	Universal is capable of all operations
		 *	Transfer is transfer dedicated
		 *	Compute is compute dedicated
		 */
		Queue universal, transfer, compute, present;

		inline void GetQueues(const vkb::Device& device)
		{
			this->universal.queue = device.get_queue(vkb::QueueType::graphics).value();
			this->transfer.queue = device.get_queue(vkb::QueueType::transfer).value();
			this->compute.queue = device.get_queue(vkb::QueueType::compute).value();

			this->universal.family = device.get_queue_index(vkb::QueueType::graphics).value();
			this->transfer.family = device.get_queue_index(vkb::QueueType::transfer).value();
			this->compute.family = device.get_queue_index(vkb::QueueType::compute).value();
		}

		QueueManager() = default;
		inline QueueManager(const vkb::Device& device) { this->GetQueues(device); }
	};
}