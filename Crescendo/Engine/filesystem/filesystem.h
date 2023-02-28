#pragma once

#include "core/core.h"

namespace Crescendo::Engine::FileSystem
{
	/*class File
	{
	private:
		std::fstream file;
	public:
		File(const char* file);
		~File();

		void Append();
		void AppendLine();

		uint64_t Size();

		std::string Read();

		void Clear();
	};*/
	/// <summary>
	/// Determines if a given file exists
	/// </summary>
	/// <param name="fileName">File string or path name to the file</param>
	/// <returns>Boolean value where true means the file exists</returns>
	bool Exists(const char* fileName);
	/// <summary>
	/// Queries the space left on the disk in bytes
	/// </summary>
	/// <returns>Space left on the disk in bytes</returns>
	uint64_t QuerySpaceLeft();
	/// <summary>
	/// Queries the total space of the disk in bytes
	/// </summary>
	/// <returns>Total space of the disk in bytes</returns>
	uint64_t QueryDiskSize();
	/// <summary>
	/// Creates a directory / folder
	/// </summary>
	/// <param name="path">Name of the directory or path to directory</param>
	void CreateDirectory(const char* path);
	/// <summary>
	/// Deletes a directory / folder and all contents in it. Note this action is irreversible!
	/// </summary>
	/// <param name="path">Path or name to the directorys</param>
	void DeleteDirectory(const char* path);
	/// <summary>
	/// Creates a new file
	/// </summary>
	/// <param name="fileName">Name of the file</param>
	void Create(const char* fileName);
	/// <summary>
	/// Deletes the file and all contents. Note this action is irreversible!
	/// </summary>
	/// <param name="fileName">name of the file to delete</param>
	void Delete(const char* fileName);
	/// <summary>
	/// Copies a file from one place to another
	/// </summary>
	/// <param name="from">The current residing directory of the file</param>
	/// <param name="to">The new directory to copy the file to</param>
	void Copy(const char* from, const char* to);
	/// <summary>
	/// Renames a file to a new name
	/// </summary>
	/// <param name="fileName">The original name of the file</param>
	/// <param name="newFileName">The new name of the file</param>
	void Rename(const char* fileName, const char* newFileName);
	/// <summary>
	/// Moves a file from one place to another
	/// </summary>
	/// <param name="fileName">The original residing directory / name of the file</param>
	/// <param name="path">The path to the new directory to move to</param>
	/// <returns></returns>
	void Move(const char* fileName, const char* path);
}