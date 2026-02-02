#include "TextureLoader.h"
#include "Entity.h"
#include "DirectXMath.h"
#include "DXRenderSystem.h"
#include "WICTextureLoader.h"
#include <fileSystem>
TextureLoader::TextureLoader()
{
}

TextureLoader::~TextureLoader()
{
}

std::unique_ptr<TextureLoader> TextureLoader::instance = nullptr;

ComPtr<ID3D12Resource> TextureLoader::LoadTexture(const std::string filePath)
{
	auto index = filePath.find_last_of("/\\");

	std::string fileName = filePath.substr(index, filePath.length());

	if (textures.find(fileName) != textures.end())
	{
		return textures[fileName];
	}

	ComPtr<ID3D12Resource> resource;

	int len = MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, NULL, 0);
	std::wstring ws(len, L'\0');

	MultiByteToWideChar(CP_UTF8, 0, filePath.c_str(), -1, &ws[0], len);

	ws.pop_back();

	DirectX::ResourceUploadBatch batch(DXRenderSystem::GetInstance().GetDevice().Get());

	batch.Begin();

	std::filesystem::path path(ws);

	std::wstring ext = path.extension().wstring();
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	HRESULT hr;

	if (ext == L".dds")
	{
		hr = DirectX::CreateDDSTextureFromFile(DXRenderSystem::GetInstance().GetDevice().Get(), batch, ws.c_str(), resource.GetAddressOf(), false);
	}
	else if (ext == L".png" || ext == L".jpg" || ext == L".jpeg" || ext == L".bmp" || ext == L".tif")
	{
		hr = DirectX::CreateWICTextureFromFile(DXRenderSystem::GetInstance().GetDevice().Get(), batch, ws.c_str(), resource.GetAddressOf(), false);
		
	}

	std::wcout << L"path = " << ws << std::endl;

	if (FAILED(hr))
	{
		std::cerr << "テクスチャファイル読み込みエラー" << std::endl;

		hr = DirectX::CreateDDSTextureFromFile(DXRenderSystem::GetInstance().GetDevice().Get(), batch, L"Textures/error.dds", resource.GetAddressOf(), false);
	}

	auto load = batch.End(DXRenderSystem::GetInstance().GetCommandQueue().Get());

	load.wait();

	textures[fileName] = resource;

	std::cout << resource->GetDesc().MipLevels << std::endl;

	return textures[fileName];
}

TextureLoader& TextureLoader::GetInstance()
{
	if (!instance) {
		instance.reset(new TextureLoader());
	}
	return *instance;
}
