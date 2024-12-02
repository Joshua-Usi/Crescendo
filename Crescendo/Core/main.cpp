#include "Core.hpp"

int main(int argc, char* argv[])
{
	CrescendoEngine::Core core;
	core.Run((argc > 1) ? argv[1] : "./crescendo_config.json");
	return 0;
}