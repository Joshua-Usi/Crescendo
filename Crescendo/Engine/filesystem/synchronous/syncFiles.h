#pragma once

#include <fstream>
#include <filesystem>

#include "core/core.h"
#include "filesystem/filesystem.h"
// Synchronous file access
// NOTE ALL OPERATIONS ARE NONREVERSIBLE
namespace Crescendo::Engine::FileSystem {
	// Opens a file for read-write IO
	void Open(std::fstream* file, const char* fileName);
	// Determines if the end of a file has been reached
	bool IsEOF(std::fstream* file);
	/// <summary>
	/// Determines the size of a file in bytes
	/// </summary>
	/// <param name="file">File reference</param>
	/// <returns>Size of file in bytes</returns>
	int64_t Size(std::fstream* file);

	// Read Methods
	// Read the next character in the file
	void ReadNextCharacter(std::fstream* file);
	// Read the next whole line in the file, returns 
	void ReadNextLine(std::fstream* file);
	// Reads the entire file
	void Read(std::fstream* file, std::string* data);
	// Read a specific character from the file
	void ReadCharacter(std::fstream*, uint64_t character);
	// Read a specific set of characters from the file
	void ReadCharacters(std::fstream*, uint64_t begin, uint64_t end);

	// Write Methods
	// Appends and writes data to a file
	void Write(std::fstream* file, const char* data);
	// Appends and writes data to a file, then adds a line break character
	void WriteLine(std::fstream* file, const char* data);
	// Inserts a string of characters after a given character
	void Insert(std::fstream* file, uint64_t character, const char* data);
	// Inserts a line after a given line in the file
	void InsertLine(std::fstream* file, uint64_t line, const char* data);
	// closes a file, recommended once you are done with it
	void Close(std::fstream* file);
}