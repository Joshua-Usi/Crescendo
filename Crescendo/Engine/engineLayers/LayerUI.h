#pragma once

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "ImGui/imgui.h"
#include "ImGui/backends/imgui_impl_opengl3.h"
#include "ImGui/backends/imgui_impl_glfw.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "core/core.h"
#include "Layer/Layer.h"
#include "console/console.h"
#include "Application/Application.h"
#include "filesystem/filesystem.h"

namespace Crescendo::Engine
{
	// Handles UI
	class LayerUI : public Layer
	{
	public:
		using Layer::Layer;

		Application* app;
		bool wind = true;

		void OnAttach()
		{
			app = Application::Get();

			ImGui::CreateContext();

			ImGuiIO& io = ImGui::GetIO();
			io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
			io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
			// Use Roboto-Medium.ttf if available, otherwise fallback to default ImGui font
			std::string fontFile = "data/fonts/Roboto-Medium.ttf";
			if (FileSystem::Exists(fontFile))
			{
				io.Fonts->AddFontFromFileTTF(fontFile.c_str(), 18);
			}
			else
			{
				Console::EngineWarn("Cannot open file {}, falling back to ImGui default", fontFile);
			}

			ImGui_ImplGlfw_InitForOpenGL(&CastVoid(GLFWwindow, app->GetWindow()->GetNative()), true);
			ImGui_ImplOpenGL3_Init("#version 460");

			// Setup Dear ImGui style
			ImGui::StyleColorsDark();

			ImGuiStyle& style = ImGui::GetStyle();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				style.WindowRounding = 0.0f;
				style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			}
		}
		void OnDetach()
		{
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		}
		void OnInit()
		{
		}
		void OnUpdate(double dt)
		{
			if (app->GetWindow()->IsOpen())
			{
				ImGui::GetIO().DisplaySize = ImVec2(app->GetWindow()->GetWidth(), app->GetWindow()->GetHeight());

				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();

				if (wind)
				{
					// Create a window called "My First Tool", with a menu bar.
					ImGui::Begin("My First Tool", &wind, ImGuiWindowFlags_MenuBar);
					if (ImGui::BeginMenuBar())
					{
						if (ImGui::BeginMenu("File"))
						{
							if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
							if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */ }
							if (ImGui::MenuItem("Close", "Ctrl+W")) { wind = false; }
							ImGui::EndMenu();
						}
						ImGui::EndMenuBar();
					}
					// Display contents in a scrolling region
					ImGui::TextColored(ImVec4(1, 1, 0, 1), "Important Stuff");
					ImGui::BeginChild("Scrolling");
					ImGui::Text("n: Some text");
					ImGui::EndChild();
					ImGui::End();
				}

				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

				ImGuiIO io = ImGui::GetIO();
				if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				{
					// Some platforms may change the context
					GLFWwindow* currentContext = glfwGetCurrentContext();
					ImGui::UpdatePlatformWindows();
					ImGui::RenderPlatformWindowsDefault();
					if (currentContext)
					{
						glfwMakeContextCurrent(currentContext);
					}
				}

				glfwSwapBuffers(&CastVoid(GLFWwindow, app->GetWindow()->GetNative()));
			}
		}
	};
}
