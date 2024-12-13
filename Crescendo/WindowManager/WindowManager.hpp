#pragma once
#include "Interfaces/Module.hpp"
#include "Window.hpp"
using namespace CrescendoEngine;

class WindowManagerInterface
{
public:
	virtual Window CreateWindow(int width, int height, const char* title) = 0;
	virtual void DestroyWindow(Window window) = 0;
};

class WindowManager : public Module, public WindowManagerInterface
{
private:
	size_t s_windowCounts = 0;
public:
	void OnLoad() override;
	void OnUnload() override;
	void OnUpdate(double dt) override;
	static ModuleMetadata GetMetadata();
	Window CreateWindow(int width, int height, const char* title) override;
	void DestroyWindow(Window window) override;
};