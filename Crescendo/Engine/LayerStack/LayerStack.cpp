#include <vector>

#include "LayerStack.h"
namespace Crescendo::Engine
{
	LayerStack::LayerStack()
	{

	}
	LayerStack::~LayerStack() {
		for (gt::Int64 i = 0, size = layers.size(); i < size; i++)
		{
			layers[i]->OnDetach();
		}
	}
	gt::Int64 LayerStack::Count()
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
		gt::Int64 i, size;
		for (i = 0, size = layers.size(); i < size; i++)
		{
			if (layers[i]->priority > layer->priority)
			{
				break;
			}
		}
		layers.insert(layers.begin() + i, layer);
		layer->OnAttach();
	}
	void LayerStack::Insert(int index, Layer* layer)
	{
		// Find the average priority of the 2 adjacent layers
		gt::Uint64 lower = layers[index - 1]->priority;
		gt::Uint64 higher = layers[index]->priority;
		gt::Uint64 avg = (lower + higher) / 2;
		// Set priority to the average
		layer->priority = avg;
		// Insert in place
		layers.insert(layers.begin() + index, layer);

		layer->OnAttach();
	}
	Layer* LayerStack::Replace(int index, Layer* layer)
	{
		Layer* replaced = layers[index];
		layer->priority = replaced->priority;
		layers[index] = layer;

		layer->OnAttach();
		replaced->OnDetach();

		return replaced;
	}
	Layer* LayerStack::Detach(int index)
	{
		Layer* l = layers[index];
		l->OnDetach();
		layers.erase(layers.begin() + index);
		return l;
	}
	void LayerStack::Init(double time)
	{
		for (gt::Int64 i = 0, size = layers.size(); i < size; i++)
		{
			layers[i]->OnInit();
			layers[i]->lastCalled = time;
		}
	}
	void LayerStack::QueryForUpdate(double time)
	{
		for (gt::Int64 i = 0, size = layers.size(); i < size; i++)
		{
			if (layers[i]->ShouldRun(time)) {
				layerQueue.push_back(layers[i]);
			}
		}
	}
	void LayerStack::Update(double time)
	{
		for (gt::Int64 i = 0, size = layerQueue.size(); i < size; i++)
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
	void LayerStack::Swap(int first, int second)
	{
		// Ignore swap query if same
		if (first != second)
		{
			// Store references to the layers
			Layer* a = layers[first];
			Layer* b = layers[second];
			// Store their priorities
			gt::Uint64 aPrio = a->priority;
			gt::Uint64 bPrio = b->priority;
			// Swap their priorities
			b->priority = aPrio;
			a->priority = bPrio;
			// Swap their indexes 
			layers[second] = a;
			layers[first] = b;
		}
	}
	Layer* LayerStack::Get(int index)
	{
		return layers[index];
	}
}