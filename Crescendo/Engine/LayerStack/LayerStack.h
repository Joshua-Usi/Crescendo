#pragma once

#include "core/core.h"
#include "Layer/Layer.h"

#include <vector>
#include <memory>

namespace Crescendo::Engine
{
	// TODO CRESCENDO Convert to job system / multithread
	class LayerStack
	{
	private:
		// updates in order
		std::vector<std::unique_ptr<Layer>> layers;
		// Holds a list of layers to be updated this frame
		std::vector<Layer*> layerQueue;
	public:
		LayerStack();
		~LayerStack();
		/// <summary>
		/// Get the number of layers in the stack
		/// </summary>
		uint64_t Count();
		/// <summary>
		/// Append a new layer to the end of the stack. This layer will run last
		/// </summary>
		/// <param name="layer">Reference to the layer to attach</param>
		void Attach(Layer* layer);
		 /// <summary>
		 /// Insert a layer at the index (places before), note this method automatically changes the priority field to suit the layer sorting engine
		 /// </summary>
		void Insert(uint64_t index, Layer* layer);
		/// <summary>
		/// Replaces a layer with another, sets the layer priority field to the layer it replaces, returns the layer removed
		/// </summary>
		/// <param name="index">the index to replace the layer with</param>
		/// <param name="layer">The layer to be replaced</param>
		/// <returns>The replaced and removed layer</returns>
		Layer* Replace(uint64_t index, Layer* layer);
		/// <summary>
		/// Removes a layer from the stack
		/// </summary>
		/// <param name="index">The index of the to be removed layer</param>
		/// <returns>The removed layer</returns>
		Layer* Detach(uint64_t index);
		/// <summary>
		/// Initialise the layers and the layer manager. Usually only called once
		/// </summary>
		/// <param name="time">The time of initialisation</param>
		void Init(double time);
		/// <summary>
		/// Query layers to determine if they should be running or not
		/// </summary>
		/// <param name="time">The current time</param>
		void QueryForUpdate(double time);
		/// <summary>
		/// After QueryForUpdate, update the layers in the order they were added and only update the layers who expect updates
		/// </summary>
		/// <param name="time">The currentTime</param>
		void Update(double time);
		/// <summary>
		/// Swaps 2 layers order, changes the priority field to suit
		/// </summary>
		/// <param name="first">The index of the first layer</param>
		/// <param name="second">The index of the second layer</param>
		void Swap(uint64_t first, uint64_t second);
		/// <summary>
		/// Get a specific layer via index
		/// </summary>
		/// <param name="index">Index to get layer</param>
		/// <returns>Layer at index</returns>
		Layer* Get(uint64_t index);
	};
}