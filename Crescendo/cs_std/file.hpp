#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

namespace cs_std
{
	typedef uint8_t byte;
	class file
	{
	protected:
		std::filesystem::path file_path;
		std::fstream stream;
		file(const std::filesystem::path& filePath) { this->file_path = filePath; }
	public:
		file() = delete;
		virtual ~file() { this->close(); }
		void close() { if (this->is_open()) this->stream.close(); }
		bool exists() const { return std::filesystem::exists(this->file_path); }
		bool is_open() const { return this->stream.is_open(); }
		void create() const
		{
			std::fstream stream;
			stream.open(this->file_path, std::ios_base::out);
			stream.close();
		}
		void remove()
		{
			// Close file stream first
			this->close();
			std::filesystem::remove(this->file_path);
		}
		void rename(const std::string& name)
		{
			// Get path
			std::filesystem::path parentDir = file_path.parent_path();
			// Create new path with new name
			std::filesystem::path newPath = parentDir / name;
			std::filesystem::rename(file_path, newPath);
			this->file_path = newPath;
		}
		void clear() const
		{
			std::fstream file(this->file_path, std::ios::out | std::ios::trunc);
			file.close();
		}
		size_t size() const { return std::filesystem::file_size(this->file_path); }
		std::string name() const { return this->file_path.filename().string(); }
		std::string extension() const { return this->file_path.extension().string(); }
		std::filesystem::path path() const { return this->file_path; }
	};
	class binary_file : public file
	{
	public:
		binary_file(const std::filesystem::path& filePath) : file(filePath) {}
		binary_file& open()
		{
			this->stream = std::fstream(this->file_path, std::ios::in | std::ios::out | std::ios::binary);
			return *this;
		}
		std::vector<byte> read(size_t start, size_t count = 1)
		{
			this->stream.seekg(start, std::ios::beg);
			// Generate buffer to store data
			std::vector<byte> buffer(count);
			// Read file data to buffer
			this->stream.read(reinterpret_cast<char*>(buffer.data()), count);
			return buffer;
		}
		std::vector<byte> read() { return this->read(0, this->size()); }
		std::vector<byte> read_if_exists()
		{
			if (this->exists()) return this->read();
			return std::vector<byte>();
		}
		binary_file& append(const std::vector<byte>& data)
		{
			// Seek to end and append
			this->stream.seekp(0, std::ios::end);
			this->stream.write(reinterpret_cast<const char*>(data.data()), data.size());
			return *this;
		}
	};
	class text_file : public file
	{
	public:
		text_file(const std::filesystem::path& filePath) : file(filePath) {}
		text_file& open()
		{
			this->stream = std::fstream(this->file_path, std::ios::in | std::ios::out);
			return *this;
		}
		std::string read(size_t start, size_t count = 1)
		{
			this->stream.seekg(start, std::ios::beg);
			std::string buffer;
			buffer.resize(count);
			this->stream.read(buffer.data(), count);
			return buffer;
		}
		std::string read() { return this->read(0, this->size()); }
		std::string read_if_exists()
		{
			if (this->exists()) return this->read();
			return std::string();
		}
		text_file& append(const std::string& data)
		{
			// Seek to end and append
			this->stream.seekp(0, std::ios::end);
			this->stream.write(reinterpret_cast<const char*>(data.data()), data.size());
			return *this;
		}
		text_file& append_line(const std::string& data)
		{
			this->append(data);
			this->append("\n");
			return *this;
		}
	};
}