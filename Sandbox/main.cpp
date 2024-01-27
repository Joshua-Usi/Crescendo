#define CS_SHOW_TIMINGS
#include "Crescendo.hpp"
using namespace CrescendoEngine;

#include "cs_std/math/math.hpp"
#include "cs_std/graphics/algorithms.hpp"
#include "cs_std/xml/xml.hpp"
namespace math = cs_std::math;

#include "scripts/CameraController.hpp"

#include "cs_std/packed_vector.hpp"

#include "Rendering/Vulkan2/Instance.hpp"

class Sandbox : public Application
{
private:
	cs_std::packed_vector<Entity> entities;
	uint32_t skyboxEntityIdx, activeCameraIdx;
	std::vector<uint32_t> transformSSBOIdx;
	std::vector<uint32_t> directionalLightSSBOIdx, pointLightSSBOIdx, spotLightSSBOIdx;

	Vulkan::Instance instance;

	int frame = 0;
	double lastTime = 0.0;
public:

	void OnStartup()
	{
		this->GetWindow()->SetCursorLock(true);

		Vulkan::InstanceSpecification spec
		{
			true,
			"Sandbox", "Crescendo",
			{ this->GetWindow()->GetNative() },
		};
		this->instance = Vulkan::Instance(spec);
	}
	void OnUpdate(double dt)
	{
		// Frame counter
		frame++;
		if (this->GetTime() - this->lastTime >= 1.0)
		{
			this->lastTime += 1.0;
			this->GetWindow()->SetName("Crescendo | FPS: " + std::to_string(this->frame));
			this->frame = 0;
		}

		if (Input::GetKeyDown(Key::F11)) this->GetWindow()->SetFullScreen(!this->GetWindow()->IsFullScreen());
		if (Input::GetMouseButtonDown(MouseButton::Left)) this->GetWindow()->SetCursorLock(true);
		if (Input::GetKeyDown(Key::Escape)) this->GetWindow()->IsCursorLocked() ? this->GetWindow()->SetCursorLock(false) : this->Exit();
		if (Input::GetKeyPressed(Key::ControlLeft) && Input::GetKeyPressed(Key::F5)) this->Restart();
	}
	void OnExit()
	{
		cs_std::console::log("Exiting...");
	}
};

CrescendoRegisterApp(Sandbox);