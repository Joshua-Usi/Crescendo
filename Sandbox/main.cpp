#define CS_SHOW_TIMINGS
#include "Crescendo.hpp"
using namespace CrescendoEngine;

#include "cs_std/math/math.hpp"
#include "cs_std/graphics/algorithms.hpp"
#include "cs_std/xml/xml.hpp"
namespace math = cs_std::math;

#include "scripts/CameraController.hpp"

struct MainPassParams
{
	Vulkan::BindlessDescriptorManager::BufferHandle directionalLights;
	Vulkan::BindlessDescriptorManager::BufferHandle pointLights;
	Vulkan::BindlessDescriptorManager::BufferHandle spotLights;
	uint32_t pad0;
};

struct SkyboxParams
{
	Vulkan::BindlessDescriptorManager::BufferHandle camera;
	Vulkan::BindlessDescriptorManager::ImageHandle skyboxTexture;
	uint32_t pad0;
	uint32_t pad1;
};

class Sandbox : public Application
{
private:
	// fps timers
	double lastTime = 0.0;
	uint32_t frames = 0;
public:
	void OnStartup()
	{
		Scene& currentScene = GetActiveScene();

		Entity cameraEntity = currentScene.entityManager.CreateEntity();
		cameraEntity.EmplaceComponent<Name>("Main Camera");
		cameraEntity.EmplaceComponent<Transform>(math::vec3(0.0f, 0.0f, 0.0f));
		cameraEntity.EmplaceComponent<PerspectiveCamera>(70.0f, this->GetWindow()->GetAspectRatio(), 0.1f, 1000.0f);
		cameraEntity.EmplaceComponent<Behaviours>(std::make_shared<CameraController>());
		// cameraEntity.EmplaceComponent<SpotLight>(glm::vec3(1.0f, 1.0f, 1.0f), 25.0f, math::radians(1.0f), math::radians(30.0f), true);
		currentScene.activeCamera = cameraEntity;
		currentScene.entities.insert(cameraEntity);
	
		// Default sky light
		/*Entity skyLight = currentScene.entityManager.CreateEntity();
		skyLight.EmplaceComponent<Name>("Default Sunlight");
		skyLight.EmplaceComponent<Transform>(math::vec3(0.0f, 75.0f, 0.0f)).LookAt(math::vec3(0.0f, 0.0f, 0.0f));
		skyLight.EmplaceComponent<DirectionalLight>(glm::vec3(0.992f, 0.984f, 0.827f), 5.0f, true);
		currentScene.entities.insert(skyLight);*/

		// Each of the different lights in the default sponza scene
		std::vector<math::vec3> pointLights =
		{
			math::vec3(15.5f, 3.25f, -0.9f), math::vec3(-13.5f, 4.0f, 0.0f),

			math::vec3(10.25f, 4.0f, 4.75f), math::vec3(6.25f, 4.0f, 4.75f),
			math::vec3(2.25f, 4.0f, 4.75f), math::vec3(-1.75f, 4.0f, 4.75f),
			math::vec3(-5.75f, 4.0f, 4.75f), math::vec3(-9.75f, 4.0f, 4.75f),

			math::vec3(10.25f, 4.0f, -4.75f), math::vec3(6.25f, 4.0f, -4.75f),
			math::vec3(2.25f, 4.0f, -4.75f), math::vec3(-1.75f, 4.0f, -4.75f),
			math::vec3(-5.75f, 4.0f, -4.75f), math::vec3(-9.75f, 4.0f, -4.75f),

			math::vec3(10.2f, 9.25f, 4.75f), math::vec3(1.8f, 9.25f, 4.75f),
			math::vec3(-6.0f, 9.25f, 4.75f), math::vec3(-9.6f, 9.25f, 4.75f),

			math::vec3(10.2f, 9.25f, -4.75f), math::vec3(1.8f, 9.25f, -4.75f),
			math::vec3(-6.0f, 9.25f, -4.75f), math::vec3(-9.6f, 9.25f, -4.75f)
		};
		for (uint32_t i = 0; i < pointLights.size(); i++)
		{
			Entity pointLight = currentScene.entityManager.CreateEntity();
			pointLight.EmplaceComponent<Name>("Default Pointlight " + std::to_string(i));
			pointLight.EmplaceComponent<Transform>(pointLights[i]);
			pointLight.EmplaceComponent<PointLight>(glm::vec3(1.0f, 0.654f, 0.341f), 5.0f, true);
			currentScene.entities.insert(pointLight);
		}
		
		// Camera following spotlight
		/*Entity spotLight = currentScene.entityManager.CreateEntity();
		spotLight.EmplaceComponent<Name>("Default Spotlight");
		spotLight.EmplaceComponent<Transform>(math::vec3(15.0f, 1.0f, 0.0f));
		spotLight.GetComponent<Transform>().LookAt(math::vec3(0.0f, 1.0f, 0.0f));
		spotLight.EmplaceComponent<SpotLight>(glm::vec3(1.0f, 1.0f, 1.0f), 30.0f, math::radians(1.0f), math::radians(12.5f), true);
		currentScene.entities.insert(spotLight);*/

		std::string assetPath = CVar::Get<std::string>("pc_assetpath");
		cs_std::xml::document modelsXML(cs_std::text_file("./configs/models.xml").open().read());
		std::vector<cs_std::graphics::model> models;
		for (const auto& model : modelsXML)
		{
			if (model->tag == "gltf") models.push_back(LoadGLTF(assetPath + model->innerText));
			else if (model->tag == "obj") models.push_back(LoadOBJ(assetPath + model->innerText));
		}

		currentScene.LoadModels(models);

		currentScene.entityManager.ForEach<Name>([&](Name& n) {
			cs_std::console::log(n.name);
		});
	}
	void OnUpdate(double dt)
	{
		this->PostUpdateInputs();
	}
	void OnExit()
	{
		cs_std::console::log("Exiting...");
	}
	void PostUpdateInputs()
	{

		Window* window = this->GetWindow();
		Input* input = window->GetInput();
		if (input->GetKeyDown(Key::F11)) window->SetFullScreen(!window->IsFullScreen());
		if (input->GetMouseDown(MouseButton::Left)) window->SetCursorLock(true);
		if (input->GetKeyPressed(Key::ControlLeft) && input->GetKeyPressed(Key::F5)) this->Restart();
		if (input->GetKeyDown(Key::Escape)) (window->IsCursorLocked()) ? window->SetCursorLock(false) : this->Exit();

		double currentTime = this->GetTime();
		frames++;
		if (currentTime - lastTime >= 1.0)
		{
			window->SetName("Crescendo Sandbox | " + std::to_string(frames) + " fps");
			frames = 0;
			lastTime += 1.0;
		}
	}
};

CrescendoRegisterApp(Sandbox);