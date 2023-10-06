#pragma once
/*
 * Special thanks to:
 *		ppy - For inspiring me to write amazing, clean code
 *		The Cherno - For helping with amazing video tutorials on game engine development
 *		The Isetta Team - For blogging on their creation of a game engine
 *		Epic Games - For maintaining a cutting-edge engine that started my fascination with engine development in UE5
 * 
 *		Bailey (mungo_) and mart (martplus) - For the idea of a game engine competition
 */

// I have no idea why, but it will only compile if it is in this header, not in the Core.hpp header
#include "entrypoint.hpp"

#include "Engine/Engine.hpp"
#include "Graphics/Graphics.hpp"
#include "IO/IO.hpp"
#include "Libraries/Libraries.hpp"
#include "Rendering/Rendering.hpp"

#include "cs_std/file.hpp"
#include "cs_std/console.hpp"

#define CrescendoRegisterApp(app) unique<Crescendo::Engine::Application> Crescendo::Engine::CreateApplication() { return std::make_unique<app>(); }