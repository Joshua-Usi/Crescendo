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

#include "entrypoint.hpp"

#include "Engine/Engine.hpp"
#include "Assets/Assets.hpp"
#include "Libraries/Libraries.hpp"

#include "cs_std/file.hpp"


#define CrescendoRegisterApp(app) std::unique_ptr<CrescendoEngine::Application> CrescendoEngine::CreateApplication(const ApplicationCommandLineArgs& args) { return std::make_unique<app>(args); }