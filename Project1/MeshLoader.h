#ifndef MESHLOADER_H
#define MESHLOADER_H

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

class MeshLoader
{
public:
	MeshLoader();

	~MeshLoader();

	enum MESH
	{
		KNIGHT,
		SPARTAN
	};

	struct VertexData
	{
		XMFLOAT3 vertex;

		XMFLOAT2 texCoord;

		XMFLOAT3 normal;

		XMFLOAT3 tangent;

		XMFLOAT4 boneID;

		XMFLOAT4 weight;

		float parentBone = 0;
	};

	struct VertexBoneData {
		std::vector<UINT> boneID;

		std::vector<float> weight;
	};

	struct BoneInfo {
		aiMatrix4x4 offset;

		aiMatrix4x4 finalMat;
	};

	struct MeshData
	{
		/// <summary>
		/// デフォルトは立方体
		/// </summary>
		std::vector<VertexData> VertexDatas = {
		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0, 0), XMFLOAT3(0, 0, -1), XMFLOAT3(1, 0, 0) , XMFLOAT4(0, 0, 0 , 0), XMFLOAT4(0, 0, 0 , 0)},
		{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT2(1, 0), XMFLOAT3(0, 0,  1), XMFLOAT3(1, 0, 0) , XMFLOAT4(0, 0, 0 , 0), XMFLOAT4(0, 0, 0 , 0)},
		{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT2(0, 1), XMFLOAT3(0, 0, -1), XMFLOAT3(1, 0, 0)  , XMFLOAT4(0, 0, 0 , 0), XMFLOAT4(0, 0, 0 , 0)},
		{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT2(1, 1), XMFLOAT3(0, 0,  1), XMFLOAT3(1, 0, 0)  , XMFLOAT4(0, 0, 0 , 0), XMFLOAT4(0, 0, 0 , 0)},
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT2(0, 0), XMFLOAT3(0, 0, -1), XMFLOAT3(1, 0, 0) , XMFLOAT4(0, 0, 0 , 0), XMFLOAT4(0, 0, 0 , 0)},
		{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT2(1, 0), XMFLOAT3(0, 0,  1), XMFLOAT3(1, 0, 0) , XMFLOAT4(0, 0, 0 , 0), XMFLOAT4(0, 0, 0 , 0)},
		{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT2(0, 1), XMFLOAT3(0, 0, -1), XMFLOAT3(1, 0, 0) , XMFLOAT4(0, 0, 0 , 0), XMFLOAT4(0, 0, 0 , 0)},
		{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT2(1, 1), XMFLOAT3(0, 0,  1), XMFLOAT3(1, 0, 0) , XMFLOAT4(0, 0, 0 , 0), XMFLOAT4(0, 0, 0 , 0)}
		};

		/// <summary>
		/// デフォルトは立方体のインデックス
		/// </summary>
		std::vector<int> indices =
		{
				4, 5, 7,
				4, 7, 6,
				0, 2, 3,
				0, 3, 1,
				0, 4, 6,
				0, 6, 2,
				1, 3, 7,
				1, 7, 5,
				2, 6, 7,
				2, 7, 3,
				0, 1, 5,
				0, 5, 4
		};

		std::vector<VertexBoneData> vertexBoneDatas;

		std::vector<BoneInfo> boneInfos;

		int baseVertexNumber;

		int baseIndexNumber;
	};

	std::unordered_map <MESH, std::vector<MeshData>> meshes;

	/// <summary>
    /// メッシュを読み込む
    /// </summary>
    /// <param name="fileName">ファイルのパス</param>
    /// <returns>ロードの可否</returns>
	bool LoadMesh(const std::string fileName);
private:
	aiScene scene;

	Assimp::Importer import;

	std::unordered_map<std::string, UINT> boneMapping;

	/// <summary>
    /// 読み込んだメッシュの頂点情報を格納する
    /// </summary>
    /// <param name="scene">assimpのメッシュの情報を展開して格納されているaiScene変数</param>
	void StoreVertices(const std::string filePath, std::vector<MeshData> meshDatas);

	void InitBone(const aiMesh& mesh, MeshData& meshData);

	void InitMaterial(const std::string filePath, const aiMesh& mesh, MeshData& data, const int meshIndex);

};

#endif


