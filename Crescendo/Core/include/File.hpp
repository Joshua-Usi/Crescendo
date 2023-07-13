#pragma once

#include "Core/common.hpp"

#include <vector>
#include <fstream>
#include <fileSystem>

namespace Crescendo::Core
{
	typedef uint8_t byte;
	/// <summary>
	/// Provides a class wrapper around the File stream
	/// </summary>
	class File
	{
	protected:
		std::filesystem::path path;
		std::fstream file;
		inline File(std::filesystem::path filePath) { this->path = filePath; }
	public:
		inline virtual ~File() { this->Close(); }
		/// <summary>
		/// Close the file stream
		/// </summary>
		inline void Close() { if (this->IsOpen()) this->file.close(); }

		/// <summary>
		/// Determines if a given file exists
		/// </summary>
		/// <returns>True if it exists</returns>
		inline bool Exists() const { return std::filesystem::exists(this->path); }
		/// <summary>
		/// Determines if the file stream is open
		/// </summary>
		/// <returns>True if open</returns>
		inline bool IsOpen() const { return this->file.is_open(); }

		/// <summary>
		/// Creates a file
		/// </summary>
		inline void Create() const
		{
			std::fstream file;
			file.open(this->path, std::ios_base::out);
			file.close();
		}
		/// <summary>
		/// Removes / Deletes the file
		/// </summary>
		inline void Remove()
		{
			// Close file stream first
			this->Close();
			std::filesystem::remove(this->path);
		}
		/// <summary>
		/// Renamed a file to a new name
		/// </summary>
		/// <param name="name">The new name of the file</param>
		inline void Rename(const std::string& name)
		{
			// Get path
			std::filesystem::path parentDir = path.parent_path();
			// Create new path with new name
			std::filesystem::path newPath = parentDir / name;
			std::filesystem::rename(path, newPath);
			path = newPath;
		}
		/// <summary>
		/// Deletes all content from a file, THIS CANNOT BE UNDONE
		/// </summary>
		inline void Clear() const
		{
			std::fstream file(path, std::ios::out | std::ios::trunc);
			file.close();
		}
		
		/// <summary>
		/// Get the size of the file in bytes
		/// </summary>
		/// <returns>Size of file in bytes</returns>
		inline size_t GetSize() const { return std::filesystem::file_size(this->path); }
		/// <summary>
		/// Returns the name of the file
		/// </summary>
		/// <returns>Name of file</returns>
		inline std::string GetName() const { return this->path.filename().string(); }
		/// <summary>
		/// Returns the extension of the file, includes the .
		/// </summary>
		/// <returns>File extension (including .)</returns>
		inline std::string GetExtension() const { return this->path.extension().string(); }
		/// <summary>
		/// Return the path of the file
		/// </summary>
		/// <returns></returns>
		inline std::filesystem::path GetPath() const { return this->path; }
	};
	/// <summary>
	/// Opens files in binary mode
	/// </summary>
	class BinaryFile : public File
	{
	public:
		/// <summary>
		/// Creates a file stream associated with a file
		/// </summary>
		/// <param name="filePath">File path</param>
		inline BinaryFile(std::filesystem::path& filePath) : File(filePath) {}
		/// <summary>
		/// Opens the file stream, In this class, opens in Binary mode
		/// </summary>
		inline BinaryFile& Open()
		{
			this->file = std::fstream(this->path, std::ios::in | std::ios::out | std::ios::binary);
			return *this;
		}
		/// <summary>
		/// Reads from a given start position and count amount of data
		/// </summary>
		/// <param name="start">Start position of data to read</param>
		/// <param name="count">Number of bytes to read</param>
		/// <returns>Vector array of bytes</returns>
		inline std::vector<byte> Read(size_t start, size_t count = 1)
		{
			this->file.seekg(start, std::ios::beg);
			// Generate buffer to store data
			std::vector<byte> buffer(count, 0);
			// Read file data to buffer
			file.read(reinterpret_cast<char*>(buffer.data()), count);
			return buffer;
		}
		/// <summary>
		/// Reads the entire file
		/// </summary>
		/// <returns>Vector array of bytes</returns>
		inline std::vector<byte> Read() { return this->Read(0, this->GetSize()); }
		/// <summary>
		/// Appends binary data to the end of the file
		/// </summary>
		/// <param name="data">Binary data in the form of a Vector array of bytes</param>
		inline BinaryFile& Append(const std::vector<byte>& data)
		{
			// Seek to end and append
			this->file.seekp(0, std::ios::end);
			this->file.write(reinterpret_cast<const char*>(data.data()), data.size());
			return *this;
		}
	};
	/// <summary>
	/// Opens files in text mode
	/// </summary>
	class TextFile : public File
	{
	public:
		/// <summary>
		/// Creates a file stream associated with a file
		/// </summary>
		/// <param name="filePath">File path</param>
		inline TextFile(std::filesystem::path filePath) : File(filePath) {}
		/// <summary>
		/// Opens the file stream, In this class, opens in Text mode
		/// </summary>
		inline TextFile& Open()
		{
			this->file = std::fstream(this->path, std::ios::in | std::ios::out);
			return *this;
		}
		/// <summary>
		/// Reads from a given start position and count amount of data
		/// </summary>
		/// <param name="start">Start position of data to read</param>
		/// <param name="count">Number of characters to read</param>
		/// <returns>String of characters</returns>
		inline std::string Read(size_t start, size_t count = 1)
		{
			this->file.seekg(start, std::ios::beg);
			std::string buffer;
			buffer.resize(count);
			file.read(&buffer[0], count);
			return buffer;
		}
		/// <summary>
		/// Reads the entire file
		/// </summary>
		/// <returns>String of characters</returns>
		inline std::string Read() { return this->Read(0, this->GetSize()); }
		/// <summary>
		/// Appends a string to the end of a file
		/// </summary>
		/// <param name="data">String to append</param>
		inline TextFile& Append(const std::string& data)
		{
			// Seek to end and append
			this->file.seekp(0, std::ios::end);
			this->file.write(reinterpret_cast<const char*>(data.data()), data.size());
			return *this;
		}
		/// <summary>
		/// Appends a string to the end of a file and adds a new line
		/// </summary>
		/// <param name="data">String to append</param>
		inline TextFile& AppendLine(const std::string& data)
		{
			this->Append(data);
			this->Append("\n");
			return *this;
		}
	};
}