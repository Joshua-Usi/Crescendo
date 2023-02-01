#include "filesystem.h"

#include "console/console.h"

namespace Crescendo::Engine::FileSystem {
	bool Exists(std::string fileName) {
		return std::filesystem::exists(fileName);
	}
	gt::Uint64 QuerySpaceLeft() {
		std::filesystem::path root = std::filesystem::current_path().root_path();
		std::filesystem::space_info info = std::filesystem::space(root);
		return info.available;
	}
	gt::Uint64 CS_API QueryDiskSize() {
		std::filesystem::path root = std::filesystem::current_path().root_path();
		std::filesystem::space_info info = std::filesystem::space(root);
		return info.capacity;
	}
	void CreateDirectory(std::string path) {
		std::filesystem::create_directory(path);
	}
	void DeleteDirectory(std::string path) {
		std::filesystem::remove_all(path);
	}
	void Create(std::string fileName) {
		std::fstream file;
		file.open(fileName, std::ios_base::out);
		file.close();
	}
	void Delete(std::string fileName) {
		std::filesystem::remove(fileName);
	}
	void Clear(std::string fileName) {
		std::ofstream file(fileName);
		file.close();
	}
	void Copy(std::string from, std::string to) {
		if (!FileSystem::Exists(from)) {
			Console::EngineWarn("Could not move file \"{}\" to \"{}\" due to reason: {}", from, to, CS_FILE_DOES_NOT_EXIST);
			return;
		};
		std::filesystem::copy(from, to);
	}
	void Rename(std::string fileName, std::string newFileName) {
		if (!FileSystem::Exists(fileName)) {
			Console::EngineWarn("Could not rename file \"{}\" to \"{}\" due to reason: {}", fileName, newFileName, CS_FILE_DOES_NOT_EXIST);
			return;
		};
		std::filesystem::rename(fileName, newFileName);
	}
	void Move(std::string fileName, std::string path) {
		if (!FileSystem::Exists(fileName)) {
			Console::EngineWarn("Could not move file \"{}\" to \"{}\" due to reason: {}", fileName, path, CS_FILE_DOES_NOT_EXIST);
			return;
		};
		std::filesystem::rename(fileName, path);
	}
}