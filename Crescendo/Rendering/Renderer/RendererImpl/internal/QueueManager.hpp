#pragma once

#include "VkBootstrap/VkBootstrap.h"

namespace Crescendo::internal
{
	class QueueManager
	{
	public:
		/*
		 *	Universal is capable of all operations
		 *	Transfer is transfer dedicated
		 *	Compute is compute dedicated
		 */
		VkQueue universal, transfer, compute;
		uint32_t universalFamily, transferFamily, computeFamily;

		inline void GetQueues(const vkb::Device& device)
		{
			this->universal = device.get_queue(vkb::QueueType::graphics).value();
			this->transfer = device.get_queue(vkb::QueueType::transfer).value();
			this->compute = device.get_queue(vkb::QueueType::compute).value();

			this->universalFamily = device.get_queue_index(vkb::QueueType::graphics).value();
			this->transferFamily = device.get_queue_index(vkb::QueueType::transfer).value();
			this->computeFamily = device.get_queue_index(vkb::QueueType::compute).value();
		}

		QueueManager() = default;
		inline QueueManager(const vkb::Device& device) { this->GetQueues(device); }
	};
}