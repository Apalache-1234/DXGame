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
#include "TextureLoader.h"

using namespace DirectX;

class MeshLoader
{
public:
	MeshLoader();

	~MeshLoader();

	enum MESH
	{
		KNIGHT,
		SPARTAN,
		BOX
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
		std::vector<VertexData> VertexDatas;

		/// <summary>
		/// デフォルトは立方体のインデックス
		/// </summary>
		std::vector<int> indices;

		std::vector<VertexBoneData> vertexBoneDatas;

		ComPtr<ID3D12Resource> texResource;

		int baseVertexNumber;

		int baseIndexNumber;

		aiScene scene;
	};


	struct Mesh
	{
		std::unordered_map <MESH, std::vector<MeshData>> meshes;

		aiScene scene;

		std::vector<BoneInfo> boneInfos;

		std::unordered_map<std::string, UINT> boneMapping;
	};

	static MeshLoader& GetInstance();

	std::vector<MeshData> GetMesh(MESH type);
private:
	aiScene scene;

	Assimp::Importer import;

	/// <summary>
    /// メッシュを読み込む
    /// </summary>
    /// <param name="fileName">ファイルのパス</param>
    /// <returns>ロードの可否</returns>
	bool LoadMesh(const std::string fileName, MESH type);

	/// <summary>
    /// DirectXの基幹システムを構築するクラスのシングルトンのインスタンス
    /// </summary>
	static std::unique_ptr<MeshLoader> instance;

	/// <summary>
    /// 読み込んだメッシュの頂点情報を格納する
    /// </summary>
    /// <param name="scene">assimpのメッシュの情報を展開して格納されているaiScene変数</param>
	void StoreVertices(const std::string filePath, std::vector<MeshData> meshDatas);

	void InitBone(const aiMesh& mesh, MeshData& meshData);

	void InitMaterial(const std::string filePath, const aiMesh& mesh, MeshData& data);

};

#endif


