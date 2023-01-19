#pragma once
/*
 * Special thanks to:
 *		ppy - For inspiring me to write amazing, clean code
 *		The Cherno - For helping with amazing video tutorials on game engine development
 *		The Isetta Team - For blogging on their creation of a game engine
 *		Epic Games - For maintaining a cutting-edge engine that started my fascination with engine development
 */ 

#define CS_MAJOR_VERSION 0
#define CS_MINOR_VERSION 0
#define CS_PATCH_VERSION 1
#define CS_TAG_VERSION "a1"

#define CS_DISCORD_APPLICATION_ID 1011601725414715432

#include "Application/Application.h"

#include "core/entrypoint.h"

#include "console/console.h"

#include "filesystem/filesystem.h"
#include "filesystem/synchronous/syncFiles.h"
#include "filesystem/asynchronous/asyncFiles.h"

#include "interfaces/Input.h"
#include "interfaces/Window.h"