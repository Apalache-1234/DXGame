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
	bool LoadTexture(const std::string fileName, MeshData& mesh);;
};

#endif


