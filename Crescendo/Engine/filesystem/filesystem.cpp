#include "filesystem.h"

#include <filesystem>
#include <fstream>

#include "console/console.h"

namespace Crescendo::Engine::FileSystem {
	bool Exists(const char* fileName) {
		return std::filesystem::exists(fileName);
	}
	uint64_t QuerySpaceLeft() {
		std::filesystem::path root = std::filesystem::current_path().root_path();
		std::filesystem::space_info info = std::filesystem::space(root);
		return info.available;
	}
	uint64_t QueryDiskSize() {
		std::filesystem::path root = std::filesystem::current_path().root_path();
		std::filesystem::space_info info = std::filesystem::space(root);
		return info.capacity;
	}
	void CreateDirectory(const char* path) {
		std::filesystem::create_directory(path);
	}
	void DeleteDirectory(const char* path) {
		std::filesystem::remove_all(path);
	}
	void Create(const char* fileName) {
		std::fstream file;
		file.open(fileName, std::ios_base::out);
		file.close();
	}
	void Delete(const char* fileName) {
		std::filesystem::remove(fileName);
	}
	void Clear(const char* fileName) {
		std::ofstream file(fileName);
		file.close();
	}
	void Copy(const char* from, const char* to) {
		if (!FileSystem::Exists(from)) {
			Console::EngineWarn("Could not move file \"{}\" to \"{}\" due to reason: {}", from, to, CS_FILE_DOES_NOT_EXIST);
			return;
		};
		std::filesystem::copy(from, to);
	}
	void Rename(const char* fileName, const char* newFileName) {
		if (!FileSystem::Exists(fileName)) {
			Console::EngineWarn("Could not rename file \"{}\" to \"{}\" due to reason: {}", fileName, newFileName, CS_FILE_DOES_NOT_EXIST);
			return;
		};
		std::filesystem::rename(fileName, newFileName);
	}
	void Move(const char* fileName, const char* path) {
		if (!FileSystem::Exists(fileName)) {
			Console::EngineWarn("Could not move file \"{}\" to \"{}\" due to reason: {}", fileName, path, CS_FILE_DOES_NOT_EXIST);
			return;
		};
		std::filesystem::rename(fileName, path);
	}
}