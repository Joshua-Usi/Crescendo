#include "Crescendo.h"
using namespace Crescendo::Engine;

#include <tuple>

enum class Type
{
	float3,
	float4,
	vec3,
	vec4,
};

class Sandbox : public Application
{
	void OnStartup()
	{
		Console::BeginFileLog("Sandbox");
	}
	void OnUpdate()
	{

	}
	void OnExit()
	{
		Console::EndFileLog();
	}
};

Application* Crescendo::Engine::CreateApplication() {
	return new Sandbox();
}