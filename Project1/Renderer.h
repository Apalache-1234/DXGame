#ifndef RENDERER_H
#define RENDERER_H

#include "Component.h"
#include <iostream>
#include <d3d12.h>
#include <d3dx12.h>
#pragma comment(lib, "d3d12.lib")
#include <dxgi1_4.h>
#pragma comment(lib, "dxgi.lib")
#include <D3Dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include <DirectXMath.h>
#pragma comment(lib, "dxguid.lib")
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include <ResourceUploadBatch.h>
#include <DDSTextureLoader.h>
#include <VertexTypes.h>
#include "Camera.h"
#include "Window.h"
#undef min
#undef max
#include "DXRenderSystem.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "TextureLoader.h"
#include "MeshLoader.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

class Renderer : public Component , public std::enable_shared_from_this<Renderer>
{
public:
	struct MeshData;

	Renderer(std::shared_ptr<Entity> ent);

	~Renderer();

	/// <summary>
	/// メッシュを描画するレンダーコマンドをコマンドリストに格納する
	/// </summary>
	/// <param name="cmdList">格納するコマンドリスト</param>
	/// <param name="backBufferIndex">現在のバックバッファのインデックス</param>
	void AddRenderCommand(ComPtr<ID3D12GraphicsCommandList>& cmdList , int backBufferIndex);

	/// <summary>
	/// テクスチャを読み込む
	/// </summary>
	/// <param name="fileName">ファイルのパス</param>
	/// <returns>ロードの可否</returns>
	bool LoadTexture(const std::string fileName, MeshData& mesh);;

	/// <summary>
	/// そのメッシュのカラーを決める
	/// </summary>
	DirectX::XMVECTOR color = DirectX::XMVectorSet(1, 1, 1, 1);

	void StartAnimation(UINT index , float speed , bool isLoop);

	float GetAnimCurrentTime();

	float GetAnimTime();

	int GetAnimIndex();

	void SetAnimTime(float time);

	std::vector<string> GetAnimationNameList();

	bool CreateMesh(const MeshLoader::MESH type);

	bool isAnimPlaying = false;

	float animSpeed = 1;

	bool animLoop = false;
private:
	/// <summary>
    /// WinAPI系の関数の可否を格納する変数
    /// </summary>
	HRESULT hr;

	/// <summary>
	/// バックバッファ込みのコンスタントバッファのリソース
	/// </summary>
	ComPtr<ID3D12Resource> constantBuffer[DXRenderSystem::frameBuffer];

	/// <summary>
	/// デスクリプタヒープ
	/// </summary>
	ComPtr<ID3D12DescriptorHeap> heap;

	/// <summary>
	/// バックバッファ込みのコンスタントバッファビュー
	/// </summary>
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbView[DXRenderSystem::frameBuffer];

	/// <summary>
	/// ルートシグネチャのCOM
	/// </summary>
	ComPtr<ID3D12RootSignature> rootSig;

	/// <summary>
	/// レンダーパイプラインのCOM
	/// </summary>
	ComPtr<ID3D12PipelineState> pipeline;

	ComPtr<ID3D12Resource> boneBuffer;

	UINT numHandling = 0;

	/// <summary>
	/// 頂点情報を格納する構造体
	/// </summary>
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

	struct BoneInfo {
		aiMatrix4x4 offset;

		aiMatrix4x4 finalMat;
	};


	std::unordered_map<string, UINT> boneMapping;

	std::vector<XMMATRIX> finalBoneMats;

	std::vector<BoneInfo> boneInfos;

	struct VertexBoneData {
		std::vector<UINT> boneID;

		std::vector<float> weight;
	};

	std::vector<MeshLoader::MeshData> meshDatas = { MeshLoader::MeshData() };

	Assimp::Importer import;

	float animTimeCur = 0;

	float animTime = 0;

	UINT animIndex = 0;

	aiScene scene;

	struct alignas(256) Transform
	{
		//ワールド変換行列
		XMMATRIX world;

		//ビュー行列
		XMMATRIX view;

		//投影行列
		XMMATRIX projection;

		//メッシュに適用する色
		XMFLOAT4 color;
	};

	/// <summary>
	/// 変換行列とカラーを格納するTransform変数
	/// </summary>
	Transform trans;

	UINT GetHandlingNum();

	/// <summary>
	/// コンスタントバッファビューのデスクリプタヒープを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool InitDescHeap();

	/// <summary>
	/// コンスタントバッファビューのリソースを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool InitCBVResource();

	/// <summary>
	/// コンスタントバッファのリソースをビューを通じてデスクリプタヒープに登録する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool LinkCBVResourceToDescHeap();

	/// <summary>
	/// 頂点バッファビューのリソースを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool InitVBVResource(MeshData& mesh);

	/// <summary>
	/// 頂点バッファをマッピングしてGPUに頂点データを入力する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool MapVB(MeshData& mesh);

	bool VBUploadToDefultHeap(MeshData& mesh);

	/// <summary>
	/// インデックスバッファビューのリソースを作成する
	/// </summary>
	///  <returns>正常に終了したらtrueを返す</returns>
	bool InitIBVResource(MeshData& mesh);

	/// <summary>
	/// インデックスバッファをマッピングしてGPUにインデックスデータを入力する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool MapIB(MeshData& mesh);

	bool IBUploadToDefultHeap(MeshData& mesh);

	/// <summary>
	/// ルートシグネチャを作成する
	/// </summary>
	///  <returns>正常に終了したらtrueを返す</returns>
	bool CreateRootSignature();

	/// <summary>
	/// レンダーパイプラインを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool CreateRenderPipeline();

	bool InitBoneResource();

	bool MapBB();

	bool InitSRVForBoneBuffer();

	void ReadHierarchyNode(float animationTime, aiNode& node, aiMatrix4x4& parentMat, aiAnimation& anim);

	void ProcessAnimation(float time);

	/// <summary>
	/// 読み込んだメッシュの頂点情報を格納する
	/// </summary>
	/// <param name="scene">assimpのメッシュの情報を展開して格納されているaiScene変数</param>
	void StoreVertices(const std::string filePath);

	void InitBone(const aiMesh& mesh, MeshData& meshData);

	void InitMaterial(const std::string filePath,  const aiMesh& mesh, MeshData& data, const int meshIndex);

	/// <summary>
	/// 座標行列を作成する
	/// </summary>
	void ConvertValues();

	 /// <summary>
	 /// コンスタントバッファをマッピングしてGPUに座標行列などを入力する
	 /// </summary>
	 ///<returns>正常に終了したらtrueを返す</returns>
	 bool MapCB();
};

#endif