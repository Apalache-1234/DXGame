#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <memory>
#include <iostream>
#include <vector>
#include "Entity.h"
#include "windows.h"
#include "DirectXMath.h"
#include "GameManager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Renderer.h"

using namespace DirectX;

class TextureLoader
{
public:
	TextureLoader();

	~TextureLoader();

	/// <summary>
    /// テクスチャのリソースやビューのCPUハンドルやレンダーに必要な情報のGPUハンドルを格納する構造体
    /// </summary>
	struct TextureView
	{
		ComPtr<ID3D12Resource> resource;

		D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;

		D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle;
	};

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


