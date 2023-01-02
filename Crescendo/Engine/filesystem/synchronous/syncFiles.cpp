#include "syncFiles.h"

namespace Crescendo::Engine::FileSystem {
	// TODO CRESCENDO Finish Sync file handling
	void Open(std::fstream* file, std::string fileName) {
		*file = std::fstream(fileName);
	}
	bool IsEOF(std::fstream* file) {
		return false;
	}

	cs::int64 CS_API Size(std::fstream* file)
	{
		return file->tellg();
	}

	void ReadNextCharacter(std::fstream* file) {

	}
	void ReadNextLine(std::fstream* file) {

	}
	void Read(std::fstream* file) {

	}
	void ReadCharacter(std::fstream* file, cs::uint64 character) {

	}
	void ReadCharacters(std::fstream* file, cs::uint64 begin, cs::uint64 end) {

	}

	void Write(std::fstream* file, std::string data) {
		// get size of the file
		file->seekg(0, file->end);
		// supports up to 9.2 Exabyte files, more than enough for a long time
		cs::uint64 size = file->tellg();
		// set write to last character, makes it append mode
		file->seekg(size);
		file->write(data.c_str(), data.length());
	}
	void WriteLine(std::fstream* file, std::string data) {

	}
	void Insert(std::fstream* file, cs::uint64 character, std::string data) {

	}
	void InsertLine(std::fstream* file, cs::uint64 line, std::string data) {

	}
	void Overwrite(std::fstream* file, std::string data) {

	}
	void OverwriteLine(std::fstream* file, cs::uint64 line, std::string data) {

	}
	void Close(std::fstream* file) {
		file->close();
	}
}