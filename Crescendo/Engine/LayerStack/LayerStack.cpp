#include <vector>

#include "LayerStack.h"
namespace Crescendo::Engine
{
	LayerStack::LayerStack()
	{

	}
	LayerStack::~LayerStack()
	{
		for (uint64_t i = 0, size = this->layers.size(); i < size; i++)
		{
			this->layers[i]->OnDetach();
		}
	}
	uint64_t LayerStack::Count()
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
		// O(n) insertion, is fine considering layers aren't attached very often
		uint64_t i, size;
		for (i = 0, size = this->layers.size(); i < size; i++)
		{
			if (this->layers[i]->priority > layer->priority)
			{
				break;
			}
		}
		this->layers.emplace(this->layers.begin() + i, layer);
		layer->OnAttach();
	}
	void LayerStack::Insert(uint64_t index, Layer* layer)
	{
		// Find the average priority of the 2 adjacent layers
		uint64_t lower = this->layers[index - 1]->priority;
		uint64_t higher = this->layers[index]->priority;
		uint64_t avg = (lower + higher) / 2;
		// Set priority to the average
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
			// Prevents blowup of layer calls when:
			// - Execution is paused
			// - Windows PC enters hiberation (Nullpomino suffers from this problem)
			// - Program execution paused via waiting for user input
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
		if (first != second)
		{
			// Store references to the layers
			Layer* a = this->layers[first].get();
			Layer* b = this->layers[second].get();
			// Store their priorities
			uint64_t aPrio = a->priority;
			uint64_t bPrio = b->priority;
			// Swap their priorities
			b->priority = aPrio;
			a->priority = bPrio;
			// Swap their indexes 
			this->layers[second].reset(a);
			this->layers[first].reset(b);
		}
	}
	Layer* LayerStack::Get(uint64_t index)
	{
		return layers[index].get();
	}
}