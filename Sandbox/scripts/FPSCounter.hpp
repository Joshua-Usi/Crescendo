#pragma once

#include "Crescendo.hpp"
using namespace CrescendoEngine;

class FPSCounter : public Behaviour
{
public:
	Text* text;

	std::deque<double> frameTimes;
	virtual void OnAttach(Entity e) override
	{
		text = &e.GetComponent<Text>();
	}
	virtual void OnUpdate(double dt) override
	{
		frameTimes.push_back(Application::Get()->GetTime<double>());
		while (frameTimes[0] < Application::Get()->GetTime<double>() - 2.0)
			frameTimes.pop_front();

		uint32_t fps = static_cast<uint32_t>(frameTimes.size() / 2);
		text->text = std::to_string(fps) + "fps";
	}
};