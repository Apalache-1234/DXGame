#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include "SystemManager.h"
#include <windows.h>
#include <memory>
#include <tchar.h>
#include "Window.h"
#include <iostream>
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include "wrl.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;


using namespace DirectX;

class TextureLoader
{
public:
	TextureLoader();

	~TextureLoader();

	/// <summary>
    /// テクスチャを読み込む
    /// </summary>
    /// <param name="fileName">ファイルのパス</param>
    /// <returns>ロードの可否</returns>
	ComPtr<ID3D12Resource> LoadTexture(const std::string fileName);;

	static TextureLoader& GetInstance();

private:
	std::unordered_map<std::string, ComPtr<ID3D12Resource>> textures;

	/// <summary>
    /// DirectXの基幹システムを構築するクラスのシングルトンのインスタンス
    /// </summary>
	static std::unique_ptr<TextureLoader> instance;
};

#endif


