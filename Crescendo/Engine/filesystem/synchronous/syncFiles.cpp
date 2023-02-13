#include "syncFiles.h"

namespace Crescendo::Engine::FileSystem {
	// TODO CRESCENDO Finish Sync file handling
	void Open(std::fstream* file, const char* fileName) {
		*file = std::fstream(fileName);
	}
	bool IsEOF(std::fstream* file) {
		return false;
	}

	int64_t Size(std::fstream* file)
	{
		return file->tellg();
	}

	void ReadNextCharacter(std::fstream* file) {

	}
	void ReadNextLine(std::fstream* file) {

	}
	void Read(std::fstream* file, std::string* data) {
		std::stringstream buffer;
		buffer << file->rdbuf();
		*data = buffer.str();
	}
	void ReadCharacter(std::fstream* file, uint64_t character) {

	}
	void ReadCharacters(std::fstream* file, uint64_t begin, uint64_t end) {

	}

	void Write(std::fstream* file, const char* data) {
		// get size of the file
		file->seekg(0, file->end);
		// supports up to 9.2 Exabyte files, more than enough for a long time
		uint64_t size = file->tellg();
		// set write to last character, makes it append mode
		file->seekg(size);
		file->write(data, strlen(data));
	}
	void WriteLine(std::fstream* file, const char* data) {

	}
	void Insert(std::fstream* file, uint64_t character, const char* data) {

	}
	void InsertLine(std::fstream* file, uint64_t line, const char* data) {

	}
	void Overwrite(std::fstream* file, const char* data) {

	}
	void OverwriteLine(std::fstream* file, uint64_t line, const char* data) {

	}
	void Close(std::fstream* file) {
		file->close();
	}
}