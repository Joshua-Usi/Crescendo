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
	:: move back out to third party directory
	cd ../
) else (
	echo simdjson is already installed.
)
:: finally exit back out to main directory
cd ../../