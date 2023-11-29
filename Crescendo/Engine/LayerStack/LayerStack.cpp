#include "LayerStack.hpp"

CS_NAMESPACE_BEGIN
{
	LayerStack::~LayerStack()
	{
		for (auto& layer : this->layers) layer->OnDetach();
	}
	size_t LayerStack::Count() const
	{
		return this->layers.size();
	}
	size_t LayerStack::Attach(Layer* layer)
	{
		size_t index = this->layers.size();
		this->layers.push_back(std::unique_ptr<Layer>(layer));
		layer->OnAttach();
		return index;
	}
	Layer* LayerStack::Detach(uint64_t index)
	{
		Layer* l = this->layers[index].get();
		l->OnDetach();
		this->layers.erase(this->layers.begin() + index);
		return l;
	}
	Layer* LayerStack::Replace(size_t index, Layer* layer)
	{
		Layer* replaced = this->layers[index].get();
		this->layers[index].reset(layer);

		layer->OnAttach();
		replaced->OnDetach();

		return replaced;
	}
	void LayerStack::Init(double time)
	{
		for (auto& layer : this->layers)
		{
			layer->OnInit();
			layer->lastCalled = time;
		}
	}
	void LayerStack::QueryForUpdate(double time)
	{
		for (auto& layer : this->layers)
		{
			if (layer->ShouldRun(time)) this->layersToUpdate.push_back(layer.get());
		}
	}
	void LayerStack::Update(double time)
	{
		constexpr double DELTA_TIME_CAP_MULTIPLIER = 4.0;
		for (auto& layer : this->layersToUpdate)
		{
			double dt = time - layer->lastCalled;
			// Prevents death spirals
			double callDt = (dt > DELTA_TIME_CAP_MULTIPLIER * callDt) ? dt : layer->updateRate;
			layer->OnUpdate(dt);
			layer->lastCalled += callDt;
		}
		layersToUpdate.clear();
	}
	Layer& LayerStack::Get(uint64_t index) const
	{
		return *layers[index];
	}
}
