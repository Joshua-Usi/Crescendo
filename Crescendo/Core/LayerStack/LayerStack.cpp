#include "LayerStack.hpp"

namespace Crescendo
{
	LayerStack::~LayerStack()
	{
		for (auto& layer : this->layers)
		{
			layer->OnDetach();
		}
	}
	uint64_t LayerStack::Count() const
	{
		return this->layers.size();
	}
	// Insertion rules:
	// - A layer is inserted at the place where:
	//		- The previous element is of a lower or equal priority
	//		- The next element is of a higher priority
	// - A layer inserted with the same priority is inserted after all layers with the same priority
	void LayerStack::Attach(Layer* layer)
	{
		// O(n) insertion, is fine considering layers usually aren't attached very often
		uint64_t i = 0;
		for (uint64_t size = this->layers.size(); i < size; i++)
		{
			if (this->layers[i]->priority > layer->priority) break;
		}
		this->layers.emplace(this->layers.begin() + i, layer);
		layer->OnAttach();
	}
	// Theres an edge case when index is 0, but I don't care enough yet to fix it
	void LayerStack::Insert(uint64_t index, Layer* layer)
	{
		// Find the average priority of the 2 adjacent layers
		uint64_t lower = this->layers[index - 1]->priority,
				 higher = this->layers[index]->priority,
				 avg = (lower + higher) / 2;
		layer->priority = avg;

		// Insert in place
		this->layers.emplace(layers.begin() + index, layer);
		layer->OnAttach();
	}
	Layer* LayerStack::Replace(uint64_t index, Layer* layer)
	{
		// Replace the priorities
		Layer* replaced = this->layers[index].get();
		layer->priority = replaced->priority;
		this->layers[index].reset(layer);

		// Detach the layer
		layer->OnAttach();
		replaced->OnDetach();

		return replaced;
	}
	Layer* LayerStack::Detach(uint64_t index)
	{
		Layer* l = this->layers[index].get();
		l->OnDetach();
		this->layers.erase(this->layers.begin() + index);
		return l;
	}
	void LayerStack::Init(double time)
	{
		for (const auto& layer : this->layers)
		{
			layer->OnInit();
			layer->lastCalled = time;
		}
	}
	void LayerStack::QueryForUpdate(double time)
	{
		for (const auto& layer : this->layers)
		{
			if (layer->ShouldRun(time))
			{
				layerQueue.push_back(layer.get());
			}
		}
	}
	void LayerStack::Update(double time)
	{
		for (const auto& layer : this->layerQueue)
		{
			double dt = time - layer->lastCalled;
			double callDt = layer->updateRate;
			/*
			 *	Prevents blowup of layer calls when:
			 *	- Execution is paused
			 *	- Windows PC enters hiberation (Nullpomino suffers from this problem)
			 *	- Program execution paused via waiting for user input
			 */
			if (dt > 4 * callDt) callDt = dt;
			layer->OnUpdate(dt);
			layer->lastCalled += callDt;
		}
		// Clear the queue afterwards
		layerQueue.clear();
	}
	void LayerStack::Swap(uint64_t first, uint64_t second)
	{
		// Ignore swap query if same
		if (first == second) return;

		std::swap(this->layers[first]->priority, this->layers[second]->priority);
		std::swap(this->layers[first], this->layers[second]);
	}
	Layer& LayerStack::Get(uint64_t index) const { return *layers[index]; }
}