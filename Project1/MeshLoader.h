#include <memory>
#include <iostream>
#include <vector>
#include "Entity.h"
#define NOMINMAX
#include "windows.h"
#include "DirectXMath.h"
#include "GameManager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "TextureLoader.h"

#ifndef MESHLOADER_H
#define MESHLOADER_H

using namespace DirectX;

class MeshLoader
{
public:
	MeshLoader();

	~MeshLoader();

	enum MESHTYPE
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

	struct Mesh
	{
		/// <summary>
		/// デフォルトは立方体
		/// </summary>
		std::vector<VertexData> vertexDatas;

		/// <summary>
		/// デフォルトは立方体のインデックス
		/// </summary>
		std::vector<int> indices;

		std::vector<VertexBoneData> vertexBoneDatas;

		int baseVertexNumber;

		int baseIndexNumber;

		ComPtr<ID3D12Resource> texResource;

		ComPtr<ID3D12Resource> vertexBuffer;

		D3D12_VERTEX_BUFFER_VIEW vbView;

		ComPtr<ID3D12Resource> indexBuffer;

		D3D12_INDEX_BUFFER_VIEW ibView;
	};

	struct MeshData
	{
		std::vector<Mesh> meshes;

		std::vector<aiMatrix4x4> boneOffset;

		std::unordered_map<std::string, UINT> boneMapping;

		aiScene scene;

		Assimp::Importer import;

		MeshData() = default;

		void AddCommandForMesh(ComPtr<ID3D12GraphicsCommandList> cmdList)
		{
			for (int i = 0; i < meshes.size(); i++)
			{
				auto& mesh = meshes[i];

				cmdList->IASetVertexBuffers(0, 1, &mesh.vbView);

				cmdList->IASetIndexBuffer(&mesh.ibView);

				cmdList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
			}
		}
	};

	void Init();

	static MeshLoader& GetInstance();

	MeshData* GetMesh(MESHTYPE type);
private:
	aiScene currentScene;

	std::unordered_map<MESHTYPE, MeshData> meshData;

	Assimp::Importer import;

	HRESULT hr;

	/// <summary>
    /// メッシュを読み込む
    /// </summary>
    /// <param name="fileName">ファイルのパス</param>
    /// <returns>ロードの可否</returns>
	bool LoadMesh(const std::string fileName, MESHTYPE type);

	void RelateMeshWithBone(MeshData& meshData, aiNode& node);

	/// <summary>
    /// DirectXの基幹システムを構築するクラスのシングルトンのインスタンス
    /// </summary>
	static std::unique_ptr<MeshLoader> instance;

	/// <summary>
    /// 読み込んだメッシュの頂点情報を格納する
    /// </summary>
    /// <param name="scene">assimpのメッシュの情報を展開して格納されているaiScene変数</param>
	void StoreVertices(const std::string filePath, std::vector<Mesh>& meshes, MeshData& meshData);

	void InitBone(const aiMesh& aiMesh, Mesh& mesh, MeshData& meshData);

	void InitMaterial(const std::string filePath, const aiMesh& mesh, Mesh& data);

	bool InitVBVResource(MeshLoader::Mesh& mesh , ComPtr<ID3D12Resource>& uploadVertexbuffer);

	bool MapVB(MeshLoader::Mesh& mesh, ComPtr<ID3D12Resource>& uploadVertexbuffer);

	bool VBUploadToDefultHeap(MeshLoader::Mesh& mesh, ComPtr<ID3D12Resource>& uploadVertexbuffer);

	bool InitIBVResource(MeshLoader::Mesh& mesh, ComPtr<ID3D12Resource>& uploadIndexbuffer);

	bool MapIB(MeshLoader::Mesh& mesh, ComPtr<ID3D12Resource>& uploadIndexbuffer);

	bool IBUploadToDefultHeap(MeshLoader::Mesh& mesh, ComPtr<ID3D12Resource>& uploadIndexbuffer);

	void Init(MESHTYPE type);
};

#endif


