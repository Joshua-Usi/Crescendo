#pragma once

#include <filesystem>
#include <fstream>

#include "core/core.h"

#define CS_FILE_DOES_NOT_EXIST "does not exist"

namespace Crescendo::Engine::FileSystem {
	/// <summary>
	/// Determines if a given file exists
	/// </summary>
	/// <param name="fileName">File string or path name to the file</param>
	/// <returns>Boolean value where true means the file exists</returns>
	bool CS_API Exists(std::string fileName);
	/// <summary>
	/// Queries the space left on the disk in bytes
	/// </summary>
	/// <returns>Space left on the disk in bytes</returns>
	gt::Uint64 CS_API QuerySpaceLeft();
	/// <summary>
	/// Queries the total space of the disk in bytes
	/// </summary>
	/// <returns>Total space of the disk in bytes</returns>
	gt::Uint64 CS_API QueryDiskSize();
	/// <summary>
	/// Creates a directory / folder
	/// </summary>
	/// <param name="path">Name of the directory or path to directory</param>
	void CS_API CreateDirectory(std::string path);
	/// <summary>
	/// Deletes a directory / folder and all contents in it. Note this action is irreversible!
	/// </summary>
	/// <param name="path">Path or name to the directorys</param>
	void CS_API DeleteDirectory(std::string path);
	/// <summary>
	/// Creates a new file
	/// </summary>
	/// <param name="fileName">Name of the file</param>
	void CS_API Create(std::string fileName);
	/// <summary>
	/// Deletes the file and all contents. Note this action is irreversible!
	/// </summary>
	/// <param name="fileName">name of the file to delete</param>
	void CS_API Delete(std::string fileName);
	/// <summary>
	/// Deletes all data from a file
	/// </summary>
	/// <param name="fileName">Name of the file to clear</param>
	void CS_API Clear(std::string fileName);
	/// <summary>
	/// Copies a file from one place to another
	/// </summary>
	/// <param name="from">The current residing directory of the file</param>
	/// <param name="to">The new directory to copy the file to</param>
	void CS_API Copy(std::string from, std::string to);
	/// <summary>
	/// Renames a file to a new name
	/// </summary>
	/// <param name="fileName">The original name of the file</param>
	/// <param name="newFileName">The new name of the file</param>
	void CS_API Rename(std::string fileName, std::string newFileName);
	/// <summary>
	/// Moves a file from one place to another
	/// </summary>
	/// <param name="fileName">The original residing directory / name of the file</param>
	/// <param name="path">The path to the new directory to move to</param>
	/// <returns></returns>
	void CS_API Move(std::string fileName, std::string path);
}