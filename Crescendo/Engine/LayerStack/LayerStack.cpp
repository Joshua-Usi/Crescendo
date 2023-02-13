#include <vector>

#include "LayerStack.h"
namespace Crescendo::Engine
{
	LayerStack::LayerStack()
	{

	}
	LayerStack::~LayerStack() {
		for (uint64_t i = 0, size = layers.size(); i < size; i++)
		{
			layers[i]->OnDetach();
		}
	}
	uint64_t LayerStack::Count()
	{
		return layers.size();
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
		for (i = 0, size = layers.size(); i < size; i++)
		{
			if (layers[i]->priority > layer->priority)
			{
				break;
			}
		}
		layers.emplace(layers.begin() + i, layer);
		layer->OnAttach();
	}
	void LayerStack::Insert(uint64_t index, Layer* layer)
	{
		// Find the average priority of the 2 adjacent layers
		uint64_t lower = layers[index - 1]->priority;
		uint64_t higher = layers[index]->priority;
		uint64_t avg = (lower + higher) / 2;
		// Set priority to the average
		layer->priority = avg;
		// Insert in place
		layers.emplace(layers.begin() + index, layer);
		layer->OnAttach();
	}
	Layer* LayerStack::Replace(uint64_t index, Layer* layer)
	{
		Layer* replaced = layers[index].get();
		layer->priority = replaced->priority;
		layers[index].reset(layer);

		layer->OnAttach();
		replaced->OnDetach();

		return replaced;
	}
	Layer* LayerStack::Detach(uint64_t index)
	{
		Layer* l = layers[index].get();
		l->OnDetach();
		layers.erase(layers.begin() + index);
		return l;
	}
	void LayerStack::Init(double time)
	{
		for (uint64_t i = 0, size = layers.size(); i < size; i++)
		{
			layers[i]->OnInit();
			layers[i]->lastCalled = time;
		}
	}
	void LayerStack::QueryForUpdate(double time)
	{
		for (uint64_t i = 0, size = layers.size(); i < size; i++)
		{
			if (layers[i]->ShouldRun(time)) {
				layerQueue.push_back(layers[i].get());
			}
		}
	}
	void LayerStack::Update(double time)
	{
		for (uint64_t i = 0, size = layerQueue.size(); i < size; i++)
		{
			double dt = time - layerQueue[i]->lastCalled;
			double callDt = layerQueue[i]->updateRate;
			// Prevents blowup of layer calls when:
			// - Execution is paused
			// - Windows PC enters hiberation (Nullpomino suffers from this problem)
			// - Program execution paused via waiting for user input
			if (dt > 4 * callDt)
			{
				callDt = dt;
			}
			layerQueue[i]->OnUpdate(dt);
			layerQueue[i]->lastCalled += callDt;
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
			Layer* a = layers[first].get();
			Layer* b = layers[second].get();
			// Store their priorities
			uint64_t aPrio = a->priority;
			uint64_t bPrio = b->priority;
			// Swap their priorities
			b->priority = aPrio;
			a->priority = bPrio;
			// Swap their indexes 
			layers[second].reset(a);
			layers[first].reset(b);
		}
	}
	Layer* LayerStack::Get(uint64_t index)
	{
		return layers[index].get();
	}
}