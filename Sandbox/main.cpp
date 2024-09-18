#include "Crescendo.hpp"
using namespace CrescendoEngine;

#include "cs_std/xml/xml.hpp"

// Scripts
#include "scripts/CameraController.hpp"
#include "scripts/Campfire.hpp"
#include "scripts/FPSCounter.hpp"
#include "scripts/Flashlight.hpp"

class Sandbox : public Application
{
public:
	using Application::Application;
	void OnStartup()
	{
		ScriptStorage::RegisterScript("CameraController", []() { return new CameraController(); });
		ScriptStorage::RegisterScript("Campfire", []() { return new Campfire(); });
		ScriptStorage::RegisterScript("FPSCounter", []() { return new FPSCounter(); });
		ScriptStorage::RegisterScript("Flashlight", []() { return new Flashlight(); });

		Scene& currentScene = GetActiveScene();

		cs_std::image fireParticle = LoadImage("./assets/fire-particle.png");
		Vulkan::TextureHandle fireParticleHandle = resourceManager.UploadTexture(fireParticle);

		cs_std::image skybox = LoadImage("./assets/skybox-night.png");
		currentScene.skybox = resourceManager.UploadTexture(skybox);

		Entity sun = currentScene.entityManager.CreateEntity();
		sun.EmplaceComponent<Transform>(math::vec3(2.0f, 10.0f, 0.0f));
		sun.GetComponent<Transform>().LookAt(math::vec3(0.0f));
		sun.EmplaceComponent<DirectionalLight>(math::vec3(1.0f, 1.0f, 1.0f), 0.1f, false);
		currentScene.entities.insert(sun);

		Entity cameraEntity = currentScene.entityManager.CreateEntity();
		cameraEntity.EmplaceComponent<Transform>(math::vec3(0.0f, 0.0f, 0.0f));
		cameraEntity.EmplaceComponent<PerspectiveCamera>(70.0f, 0.1f, 1000.0f);
		cameraEntity.EmplaceComponent<SpotLight>(math::vec3(1.0f, 1.0f, 1.0f), 10.0f, math::radians(1.0f), math::radians(25.0f), false);
		cameraEntity.AddBehaviour("CameraController");
		cameraEntity.AddBehaviour("Flashlight");
		currentScene.activeCamera = cameraEntity;
		currentScene.entities.insert(cameraEntity);

		Entity updatingText = currentScene.entityManager.CreateEntity();
		updatingText.EmplaceComponent<Transform>(math::vec3(0.0f, 0.0f, 0.0));
		updatingText.EmplaceComponent<Text>(
			"0fps",
			"Inter", Color(255), 32.0f, 1.0f, Text::Alignment::Left);
		updatingText.EmplaceComponent<TextRenderer>(TextRenderer::RenderMode::ScreenSpace);
		updatingText.AddBehaviour("FPSCounter");
		currentScene.entities.insert(updatingText);

		Entity infoText = currentScene.entityManager.CreateEntity();
		infoText.EmplaceComponent<Transform>(math::vec3(0.0f, -32.0f, 0.0));
		infoText.EmplaceComponent<Text>(
			"'W', 'A', 'S', 'D' to move\n'Space' to fly up\n'Shift' to fly down\n'R' to move faster\n'F11' to toggle fullscreen\n'Esc' to unlock cursor\n'Esc' again to exit\n'Ctrl' + 'F5' to restart\nClick on the window to lock cursor\n'F' to toggle flashlight",
			"Inter", Color(255), 24.0f, 1.0f, Text::Alignment::Left);
		infoText.EmplaceComponent<TextRenderer>(TextRenderer::RenderMode::ScreenSpace);
		currentScene.entities.insert(infoText);

		Entity particleEmitter = currentScene.entityManager.CreateEntity();
		particleEmitter.EmplaceComponent<Transform>(math::vec3(0.0f, 45.0f, 0.0f));
		particleEmitter.EmplaceComponent<ParticleEmitter>(12000, 0.05f, 100,
			[](float currentTime, float dt, ParticleEmitter::Particle& p) {
				// air friction
				p.velocity *= math::pow(0.1f, dt);
				p.velocity.y += 5.0f * dt;
				p.position += p.velocity * dt;
			},
			[](float currentTime) {
				ParticleEmitter::Particle p;

				float radius = math::random(80.0f, 100.0f);

				float u = math::random<float>(0.0f, 1.0f);
				float v = math::random<float>(0.0f, 1.0f);

				float theta = 2.0f * math::pi<float>() * u; // azimuthal angle
				float phi = math::acos(2.0 * v - 1.0); // polar angle

				float x = radius * sinf(phi) * cosf(theta);
				float y = radius * sinf(phi) * sinf(theta);
				float z = radius * cosf(phi);

				p.position = math::vec3(0.0f);
				p.velocity = math::vec3(x, y, z);
				p.deathTime = currentTime + math::random<float>(3.0f, 5.0f);
				return p;
			}
		);
		particleEmitter.EmplaceComponent<ParticleRenderer>(fireParticleHandle);
		// Create point light for fire
		particleEmitter.EmplaceComponent<PointLight>(glm::vec3(1.0f, 0.55f, 0.0f), 10.0f, true);
		particleEmitter.AddBehaviour("Campfire");
		currentScene.entities.insert(particleEmitter);

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
			pointLight.EmplaceComponent<Transform>(pointLights[i]);
			pointLight.EmplaceComponent<PointLight>(glm::vec3(1.0f, 0.654f, 0.341f), 1.0f, true);
			currentScene.entities.insert(pointLight);
		}

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
	}
};

CrescendoRegisterApp(Sandbox);