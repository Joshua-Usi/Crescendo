#pragma once

#include "common.hpp"

#include "Input.hpp"

CS_NAMESPACE_BEGIN
{
	class Window
	{
	public:
		struct Specification
		{
			std::string title;
			uint32_t width, height, positionX, positionY;
			Specification() : title(""), width(0), height(0), positionX(0), positionY(0) {}
			Specification(const std::string& windowTitle, uint32_t width = 1280, uint32_t height = 720, uint32_t positionX = 320, uint32_t positionY = 180)
				: title(windowTitle), width(width), height(height), positionX(positionX), positionY(positionY) {}
		};
	protected:
		std::unique_ptr<Input> input;
		struct Data
		{
			std::string title = "";
			Window* windowPointer = nullptr;
			uint32_t width = 0, height = 0, windowedWidth = 0, windowedHeight = 0;
			bool vSync = false, isCursorLocked = false, isOpen = false, isFullScreen = false;
		} data;
	public:
		static std::unique_ptr<Window> Create(const Specification& spec = Specification());
		virtual ~Window() = default;
		virtual void Close() = 0;
		virtual void OnUpdate() = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual float GetAspectRatio() const = 0;
		virtual void* GetNative() const = 0;
		virtual int32_t GetRefreshRate() const = 0;
		virtual std::string GetName() const = 0;
		Input* GetInput() const { return this->input.get(); }
		virtual void SetSize(uint32_t width, uint32_t height) = 0;
		virtual void SetVSync(bool isEnabled) = 0;
		virtual void SetCursorLock(bool isEnabled) = 0;
		virtual void SetName(const std::string& name) = 0;
		virtual void SetFullScreen(bool isFullScreen) = 0;
		virtual bool IsVSynced() const = 0;
		virtual bool IsCursorLocked() const = 0;
		virtual bool IsOpen() const = 0;
		virtual bool IsFullScreen() const = 0;
	};
}