#pragma once

#include <fstream>
#include <filesystem>

#include "core/core.h"
#include "filesystem/filesystem.h"
// Synchronous file access
// NOTE ALL OPERATIONS ARE NONREVERSIBLE
namespace Crescendo::Engine::FileSystem
{
	// Opens a file for read-write IO
	void Open(std::fstream& file, const char* fileName);
	/// <summary>
	/// Determines the size of a file in bytes
	/// </summary>
	/// <param name="file">File reference</param>
	/// <returns>Size of file in bytes</returns>
	int64_t Size(std::fstream& file);

	// Read Methods
	// Reads the entire file
	void Read(std::fstream& file, std::string& data);
	// Reads the entire file no fuss without exposing a file stream
	void Read(const char* fileName, std::string& data);

	// Write Methods
	// Appends and writes data to a file
	void Write(std::fstream& file, const char* data);
	// Appends and writes data to a file, then adds a line break character
	void WriteLine(std::fstream& file, const char* data);

	// closes a file, recommended once you are done with it
	void Close(std::fstream& file);
}