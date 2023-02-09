#pragma once

#include <vector>

#include "core/core.h"

#include "XML/XML.h"

#include "UICommandQueue.h";

namespace Crescendo::Editor::Slick
{
	class Window
	{
	private:
		bool isOpen;
		UICommandQueue commandQueue;
		Crescendo::Tools::XML::Document document;
	public:
		Window(gt::string xmlDocPath)
		{
			isOpen = false;
			Crescendo::Tools::XML::ParseFromFile(&document, xmlDocPath);

			// Construct UI command queue as a stack, in Depth-first fashion

		}
	};
}