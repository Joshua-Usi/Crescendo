@echo off

if not exist Crescendo\thirdparty mkdir Crescendo\thirdparty
cd Crescendo\thirdparty

:: Install simdjson
if not exist simdjson (
	echo Installing simdjson...
	mkdir simdjson
	cd simdjson
	
	curl -L --output simdjson.h --url https://github.com/simdjson/simdjson/releases/download/v3.10.1/simdjson.h
	curl -L --output simdjson.cpp --url https://github.com/simdjson/simdjson/releases/download/v3.10.1/simdjson.cpp
	
	echo simdjson has been installed.
	cd ../
) else (
	echo simdjson is already installed.
)

:: Install entt
if not exist entt (
	echo Installing entt...
	mkdir entt
	cd entt

	curl -L --output entt.zip --url https://github.com/skypjack/entt/archive/refs/tags/v3.14.0.zip
    tar -xf entt.zip --strip-components 3 -C . "entt-3.14.0/single_include/entt/entt.hpp"
    del -q entt.zip

    echo entt has been installed.
    cd ../
) else (
	echo entt is already installed.
)

:: install glfw
if not exist glfw (
	echo Installing glfw...
	mkdir glfw
	cd glfw

	:: Install includes
	curl -L --output glfw.zip --url https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.WIN64.zip
	tar -xf glfw.zip --strip-components 3 -C . "glfw-3.4.bin.WIN64/include/GLFW"

	:: Install libraries
	mkdir libs
	cd libs
	tar -xf ../glfw.zip --strip-components 2 -C . "glfw-3.4.bin.WIN64/lib-vc2022"

	cd../

	del -q glfw.zip

	echo glfw has been installed.
	cd ../
) else (
	echo glfw is already installed.
)


:: finally exit back out to main directory
cd ../../