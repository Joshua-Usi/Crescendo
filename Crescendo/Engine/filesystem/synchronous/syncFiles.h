#pragma once

#include <string>
#include <fstream>
#include <filesystem>

#include "core/core.h"
#include "filesystem/filesystem.h"
// Synchronous file access
// NOTE ALL OPERATIONS ARE NONREVERSIBLE
namespace Crescendo::Engine::FileSystem {
	// Opens a file for read-write IO
	void CS_API Open(std::fstream* file, std::string fileName);
	// Determines if the end of a file has been reached
	bool CS_API IsEOF(std::fstream* file);
	/// <summary>
	/// Determines the size of a file in bytes
	/// </summary>
	/// <param name="file">File reference</param>
	/// <returns>Size of file in bytes</returns>
	cs::int64 CS_API Size(std::fstream* file);

	// Read Methods
	// Read the next character in the file
	void CS_API ReadNextCharacter(std::fstream* file);
	// Read the next whole line in the file, returns 
	void CS_API ReadNextLine(std::fstream* file);
	// Reads the entire file
	void CS_API Read(std::fstream* file);
	// Read a specific character from the file
	void CS_API ReadCharacter(std::fstream*, cs::uint64 character);
	// Read a specific set of characters from the file
	void CS_API ReadCharacters(std::fstream*, cs::uint64 begin, cs::uint64 end);

	// Write Methods
	// Appends and writes data to a file
	void CS_API Write(std::fstream* file, std::string data);
	// Appends and writes data to a file, then adds a line break character
	void CS_API WriteLine(std::fstream* file, std::string data);
	// Inserts a string of characters after a given character
	void CS_API Insert(std::fstream* file, cs::uint64 character, std::string data);
	// Inserts a line after a given line in the file
	void CS_API InsertLine(std::fstream* file, cs::uint64 line, std::string data);
	// writes over the data of the file
	void CS_API Overwrite(std::fstream* file, std::string data);
	// writes over a specific line in the file
	void CS_API OverwriteLine(std::fstream* file, cs::uint64 line, std::string data);
	// closes a file, recommended once you are done with it
	void CS_API Close(std::fstream* file);
}