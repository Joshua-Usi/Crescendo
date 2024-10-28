#include "Crescendo.hpp"
using namespace CrescendoEngine;

#include "cs_std/xml/xml.hpp"

// Scripts
#include "scripts/CameraController.hpp"
#include "scripts/Campfire.hpp"
#include "scripts/FPSCounter.hpp"
#include "scripts/Flashlight.hpp"
#include "scripts/FlashingCube.hpp"

class Sandbox : public Application
{
public:
	using Application::Application;
	void OnStartup()
	{
		ScriptStorage::RegisterScript("CameraController", [](Entity e) { return new CameraController(e); });
		ScriptStorage::RegisterScript("Campfire", [](Entity e) { return new Campfire(e); });
		ScriptStorage::RegisterScript("FPSCounter", [](Entity e) { return new FPSCounter(e); });
		ScriptStorage::RegisterScript("Flashlight", [](Entity e) { return new Flashlight(e); });
		ScriptStorage::RegisterScript("FlashingCube", [](Entity e) { return new FlashingCube(e); });


		Scene& currentScene = GetActiveScene();

		cs_std::image fireParticle = LoadImage("./assets/fire-particle.png");
		Vulkan::TextureHandle fireParticleHandle = renderer.resourceManager.UploadTexture(fireParticle);

		cs_std::image skybox = LoadImage("./assets/skybox-night.png");
		currentScene.SetSkybox(renderer.resourceManager.UploadTexture(skybox));

		Construct::Mesh cube = Construct::Cube();

		// convert to cs_std::mesh
		cs_std::graphics::mesh<uint32_t> csMesh;
		csMesh.add_attribute(cs_std::graphics::Attribute::POSITION, cube.vertices);
		csMesh.add_attribute(cs_std::graphics::Attribute::NORMAL, cube.normals);
		csMesh.add_attribute(cs_std::graphics::Attribute::TEXCOORD_0, cube.textureUVs);
		csMesh.indices = cube.indices;

		// Generate tangents
		cs_std::graphics::generate_tangents(csMesh);

		Vulkan::Mesh cubeMesh;

		cubeMesh.vertexAttributes.emplace_back(renderer.resourceManager.CreateGPUBuffer(cube.vertices.data(), sizeof(float) * cube.vertices.size(), RenderResourceManager::GPUBufferUsage::VertexBuffer), cs_std::graphics::Attribute::POSITION);
		cubeMesh.vertexAttributes.emplace_back(renderer.resourceManager.CreateGPUBuffer(cube.normals.data(), sizeof(float) * cube.normals.size(), RenderResourceManager::GPUBufferUsage::VertexBuffer), cs_std::graphics::Attribute::NORMAL);
		cubeMesh.vertexAttributes.emplace_back(renderer.resourceManager.CreateGPUBuffer(cube.textureUVs.data(), sizeof(float) * cube.textureUVs.size(), RenderResourceManager::GPUBufferUsage::VertexBuffer), cs_std::graphics::Attribute::TEXCOORD_0);
		cubeMesh.vertexAttributes.emplace_back(renderer.resourceManager.CreateGPUBuffer(csMesh.get_attribute(cs_std::graphics::Attribute::TANGENT).data.data(), sizeof(float) * csMesh.get_attribute(cs_std::graphics::Attribute::TANGENT).data.size(), RenderResourceManager::GPUBufferUsage::VertexBuffer), cs_std::graphics::Attribute::TANGENT);
		cubeMesh.indexBuffer = renderer.resourceManager.CreateGPUBuffer(cube.indices.data(), sizeof(uint32_t) * cube.indices.size(), RenderResourceManager::GPUBufferUsage::IndexBuffer);
		cubeMesh.indexCount = cube.indices.size();
		cubeMesh.indexType = VK_INDEX_TYPE_UINT32;

		for (int32_t i = -5; i < 5; i++)
		{
			for (int32_t j = -5; j < 5; j++)
			{
				Entity emissiveCube = currentScene.CreateEntity();
				emissiveCube.EmplaceComponent<Transform>(math::vec3(i * 2.0f, 5.0f, j * 2.0f), math::vec3(0.5f));
				emissiveCube.EmplaceComponent<MeshData>(cs_std::graphics::bounding_aabb(cube.vertices), cubeMesh);
				Material& emissiveMaterial = emissiveCube.EmplaceComponent<Material>();
				emissiveMaterial.albedo = Color(math::random<uint8_t>(0, 255), math::random<uint8_t>(0, 255), math::random<uint8_t>(0, 255));
				emissiveMaterial.emissive = math::vec3(0.0f, 0.0f, 0.0f);
				emissiveCube.AddBehaviour("FlashingCube");
			}
		}


		Entity sun = currentScene.CreateEntity();
		sun.EmplaceComponent<Transform>(math::vec3(2.0f, 10.0f, 0.0f)).LookAt(math::vec3(0.0f));
		sun.EmplaceComponent<DirectionalLight>(math::vec3(1.0f, 1.0f, 1.0f), 0.1f, false);

		Entity cameraEntity = currentScene.CreateEntity();
		cameraEntity.EmplaceComponent<Transform>(math::vec3(0.0f, 0.0f, 0.0f));
		cameraEntity.EmplaceComponent<PerspectiveCamera>(70.0f, 0.1f, 1000.0f);
		cameraEntity.AddBehaviour("CameraController");
		cameraEntity.AddBehaviour("Flashlight");
		currentScene.SetActiveCamera(cameraEntity);

		Entity updatingText = currentScene.CreateEntity();
		updatingText.EmplaceComponent<Transform>(math::vec3(1200.0f, 0.0f, 0.0));
		updatingText.EmplaceComponent<Text>(
			"0fps",
			"Inter", Color(255), 32.0f, 1.0f, Text::Alignment::Left);
		updatingText.EmplaceComponent<TextRenderer>(TextRenderer::RenderMode::ScreenSpace);
		updatingText.AddBehaviour("FPSCounter");

		Entity infoText = currentScene.CreateEntity();
		infoText.EmplaceComponent<Transform>(math::vec3(0.0f, 0.0f, 0.0));
		infoText.EmplaceComponent<Text>(
			"'W', 'A', 'S', 'D' to move\n'Space' to fly up\n'Shift' to fly down\n'R' to move faster\n'F11' to toggle fullscreen\n'Esc' to unlock cursor\n'Esc' again to exit\n'Ctrl' + 'F5' to restart\nClick on the window to lock cursor\n'F' to toggle flashlight",
			"Inter", Color(255), 24.0f, 1.0f, Text::Alignment::Left);
		infoText.EmplaceComponent<TextRenderer>(TextRenderer::RenderMode::ScreenSpace);

		Entity particleEmitter = currentScene.CreateEntity();
		particleEmitter.EmplaceComponent<Transform>(math::vec3(0.0f, 45.0f, 0.0f));
		particleEmitter.EmplaceComponent<ParticleEmitter>(12000, 0.05f, 100,
			[](float currentTime, float dt, ParticleEmitter::Particle& p) {
				// air friction
				p.velocity *= math::pow(0.1f, dt);
				p.velocity.y -= 0.5f * dt;
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

				p.position = math::vec3(0.0, 10.0f, 0.0f);
				p.velocity = math::vec3(x, y, z);
				p.deathTime = currentTime + math::random<float>(3.0f, 5.0f);
				return p;
			}
		);
		particleEmitter.EmplaceComponent<ParticleRenderer>(fireParticleHandle);
		// Create point light for fire
		particleEmitter.EmplaceComponent<PointLight>(glm::vec3(1.0f, 0.55f, 0.0f), 100.0f, true);
		particleEmitter.AddBehaviour("Campfire");

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
			Entity pointLight = currentScene.CreateEntity();
			pointLight.EmplaceComponent<Transform>(pointLights[i]);
			pointLight.EmplaceComponent<PointLight>(glm::vec3(1.0f, 0.654f, 0.341f), 10.0f, true);
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