#pragma once

#include "common.hpp"

#include "Layer.hpp"

CS_NAMESPACE_BEGIN
{
	class LayerStack
	{
	private:
		std::vector<std::unique_ptr<Layer>> layers;
		std::vector<Layer*> layersToUpdate;
	public:
		~LayerStack();
		size_t Count() const;
		size_t Attach(Layer* layer);
		// Returns the detached layer
		Layer* Detach(size_t index);
		// Returns the replaced layer
		Layer* Replace(size_t index, Layer* layer);
		void Init(double time);
		void QueryForUpdate(double time);
		void Update(double time);
		Layer& Get(size_t index) const;
	};
}