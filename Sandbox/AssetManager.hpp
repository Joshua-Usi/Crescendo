#pragma once

#include "common.hpp"

namespace CrescendoEngine
{
	class AssetFactory
	{
	public:
		virtual std::string LoadFromPath(const std::string& assetPath) = 0;
		virtual std::string LoadFromData(void* assetData) = 0;
	};


	struct AssetHandle {};

	class AssetManager
	{
	private:

	private:
		void QueryAssetsToBeUnloaded();
	public:
		AssetManager();
		~AssetManager();
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;
		AssetManager(AssetManager&& other) noexcept;
		AssetManager& operator=(AssetManager&& other) noexcept;
	public:
		AssetHandle LoadAsset();
		// Tell an asset to be immediately unloaded when it's ref count goes to 0, The asset can no longer be issued, only existing issued assets can be used
		void UnloadAssetOnDisuse(AssetHandle asset);
		void ForceUnloadAsset(AssetHandle asset);
		AssetHandle AddAsset();
		void UnloadUnusedAssets();
	};
}