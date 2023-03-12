#include "syncFiles.h"

namespace Crescendo::Engine::FileSystem
{
	namespace
	{
		void SeekToEnd(std::fstream& file)
		{
			// get size of the file
			file.seekg(0, file.end);
			// supports up to 9.2 Exabyte files, more than enough for a long time
			uint64_t size = file.tellg();
			// set write to last character, makes it append mode
			file.seekg(size);
		}
	}
	// TODO CRESCENDO Finish Sync file handling
	void Open(std::fstream& file, const char* fileName)
	{
		file = std::fstream(fileName);
	}

	int64_t Size(std::fstream& file)
	{
		return file.tellg();
	}

	void Read(std::fstream& file, std::string& data)
	{
		std::stringstream buffer;
		buffer << file.rdbuf();
		data = buffer.str();
	}

	void Read(const char* fileName, std::string& data)
	{
		std::fstream file;
		FileSystem::Open(file, fileName);
		FileSystem::Read(file, data);
		FileSystem::Close(file);
	}

	void Write(std::fstream& file, const char* data)
	{
		SeekToEnd(file);
		file.write(data, strlen(data));
	}
	void WriteLine(std::fstream& file, const char* data)
	{
		SeekToEnd(file);
		if (Size(file) != 0)
		{
			file.write("\n", 1);
		}
		file.write(data, strlen(data));
	}

	void Close(std::fstream& file)
{
		file.close();
	}
}