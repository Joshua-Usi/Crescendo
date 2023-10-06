#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

namespace cs_std
{
	typedef uint8_t byte;
	/// <summary>
	/// Provides a class wrapper around the File stream
	/// </summary>
	class file
	{
	protected:
		std::filesystem::path file_path;
		std::fstream stream;
		inline file(const std::filesystem::path& filePath) { this->file_path = filePath; }
	public:
		file() = delete;
		inline virtual ~file() { this->close(); }
		/// <summary>
		/// Close the file stream
		/// </summary>
		inline void close() { if (this->is_open()) this->stream.close(); }

		/// <summary>
		/// Determines if a given file exists
		/// </summary>
		/// <returns>True if it exists</returns>
		inline bool exists() const { return std::filesystem::exists(this->file_path); }
		/// <summary>
		/// Determines if the file stream is open
		/// </summary>
		/// <returns>True if open</returns>
		inline bool is_open() const { return this->stream.is_open(); }

		/// <summary>
		/// Creates a file
		/// </summary>
		inline void create() const
		{
			std::fstream stream;
			stream.open(this->file_path, std::ios_base::out);
			stream.close();
		}
		/// <summary>
		/// Removes / Deletes the file
		/// </summary>
		inline void remove()
		{
			// Close file stream first
			this->close();
			std::filesystem::remove(this->file_path);
		}
		/// <summary>
		/// Renamed a file to a new name
		/// </summary>
		/// <param name="name">The new name of the file</param>
		inline void rename(const std::string& name)
		{
			// Get path
			std::filesystem::path parentDir = file_path.parent_path();
			// Create new path with new name
			std::filesystem::path newPath = parentDir / name;
			std::filesystem::rename(file_path, newPath);
			this->file_path = newPath;
		}
		/// <summary>
		/// Deletes all content from a file, THIS CANNOT BE UNDONE
		/// </summary>
		inline void clear() const
		{
			std::fstream file(this->file_path, std::ios::out | std::ios::trunc);
			file.close();
		}

		/// <summary>
		/// Get the size of the file in bytes
		/// </summary>
		/// <returns>Size of file in bytes</returns>
		inline size_t size() const { return std::filesystem::file_size(this->file_path); }
		/// <summary>
		/// Returns the name of the file
		/// </summary>
		/// <returns>Name of file</returns>
		inline std::string name() const { return this->file_path.filename().string(); }
		/// <summary>
		/// Returns the extension of the file, includes the .
		/// </summary>
		/// <returns>File extension (including .)</returns>
		inline std::string extension() const { return this->file_path.extension().string(); }
		/// <summary>
		/// Return the path of the file
		/// </summary>
		/// <returns></returns>
		inline std::filesystem::path path() const { return this->file_path; }
	};
	/// <summary>
	/// Opens files in binary mode
	/// </summary>
	class binary_file : public file
	{
	public:
		/// <summary>
		/// Creates a file stream associated with a file
		/// </summary>
		/// <param name="filePath">File path</param>
		inline binary_file(const std::filesystem::path& filePath) : file(filePath) {}
		/// <summary>
		/// Opens the file stream, In this class, opens in Binary mode
		/// </summary>
		inline binary_file& open()
		{
			this->stream = std::fstream(this->file_path, std::ios::in | std::ios::out | std::ios::binary);
			return *this;
		}
		/// <summary>
		/// Reads from a given start position and count amount of data
		/// </summary>
		/// <param name="start">Start position of data to read</param>
		/// <param name="count">Number of bytes to read</param>
		/// <returns>Vector array of bytes</returns>
		inline std::vector<byte> read(size_t start, size_t count = 1)
		{
			this->stream.seekg(start, std::ios::beg);
			// Generate buffer to store data
			std::vector<byte> buffer(count);
			// Read file data to buffer
			this->stream.read(reinterpret_cast<char*>(buffer.data()), count);
			return buffer;
		}
		/// <summary>
		/// Reads the entire file
		/// </summary>
		/// <returns>Vector array of bytes</returns>
		inline std::vector<byte> read() { return this->read(0, this->size()); }
		/// <summary>
		/// Appends binary data to the end of the file
		/// </summary>
		/// <param name="data">Binary data in the form of a Vector array of bytes</param>
		inline binary_file& append(const std::vector<byte>& data)
		{
			// Seek to end and append
			this->stream.seekp(0, std::ios::end);
			this->stream.write(reinterpret_cast<const char*>(data.data()), data.size());
			return *this;
		}
	};
	/// <summary>
	/// Opens files in text mode
	/// </summary>
	class text_file : public file
	{
	public:
		/// <summary>
		/// Creates a file stream associated with a file
		/// </summary>
		/// <param name="filePath">File path</param>
		inline text_file(const std::filesystem::path& filePath) : file(filePath) {}
		/// <summary>
		/// Opens the file stream, In this class, opens in Text mode
		/// </summary>
		inline text_file& open()
		{
			this->stream = std::fstream(this->file_path, std::ios::in | std::ios::out);
			return *this;
		}
		/// <summary>
		/// Reads from a given start position and count amount of data
		/// </summary>
		/// <param name="start">Start position of data to read</param>
		/// <param name="count">Number of characters to read</param>
		/// <returns>String of characters</returns>
		inline std::string read(size_t start, size_t count = 1)
		{
			this->stream.seekg(start, std::ios::beg);
			std::string buffer;
			buffer.resize(count);
			this->stream.read(buffer.data(), count);
			return buffer;
		}
		/// <summary>
		/// Reads the entire file
		/// </summary>
		/// <returns>String of characters</returns>
		inline std::string read() { return this->read(0, this->size()); }
		/// <summary>
		/// Appends a string to the end of a file
		/// </summary>
		/// <param name="data">String to append</param>
		inline text_file& append(const std::string& data)
		{
			// Seek to end and append
			this->stream.seekp(0, std::ios::end);
			this->stream.write(reinterpret_cast<const char*>(data.data()), data.size());
			return *this;
		}
		/// <summary>
		/// Appends a string to the end of a file and adds a new line
		/// </summary>
		/// <param name="data">String to append</param>
		inline text_file& append_line(const std::string& data)
		{
			this->append(data);
			this->append("\n");
			return *this;
		}
	};
}