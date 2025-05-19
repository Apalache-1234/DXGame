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
#include "DescriptorHeapManager.h"

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
	/// そのメッシュのカラーを決める
	/// </summary>
	DirectX::XMVECTOR color = DirectX::XMVectorSet(1, 1, 1, 1);

	void StartAnimation(UINT index , float speed , bool isLoop);

	float GetAnimCurrentTime();

	float GetAnimTime();

	int GetAnimIndex();

	void SetAnimTime(float time);

	std::vector<string> GetAnimationNameList();

	MeshLoader::MESHTYPE GetCurrentMeshType();

	bool CreateMesh(const MeshLoader::MESHTYPE type);

	bool isAnimPlaying = false;

	float animSpeed = 1;

	bool animLoop = false;
private:
	/// <summary>
    /// WinAPI系の関数の可否を格納する変数
    /// </summary>
	HRESULT hr;

	MeshLoader::MeshData* meshData;

	MeshLoader::MESHTYPE currentMeshType;

	/// <summary>
	/// バックバッファ込みのコンスタントバッファのリソース
	/// </summary>
	ComPtr<ID3D12Resource> constantBuffer[DXRenderSystem::frameBuffer];

	DescriptorHeapManager heapManager;

	/// <summary>
	/// バックバッファ込みのコンスタントバッファビュー
	/// </summary>
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbView[DXRenderSystem::frameBuffer];

	DescriptorHeapManager::PtrHandles cbHandle[DXRenderSystem::frameBuffer];

	std::vector<DescriptorHeapManager::PtrHandles> texHandle;

	DescriptorHeapManager::PtrHandles boneHandle;

	/// <summary>
	/// ルートシグネチャのCOM
	/// </summary>
	ComPtr<ID3D12RootSignature> rootSig;

	/// <summary>
	/// レンダーパイプラインのCOM
	/// </summary>
	ComPtr<ID3D12PipelineState> pipeline;

	ComPtr<ID3D12Resource> boneBuffer;

	std::vector<XMMATRIX> finalBoneMats;

	float animTimeCur = 0;

	float animTime = 0;

	UINT animIndex = 0;

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
	/// ルートシグネチャを作成する
	/// </summary>
	///  <returns>正常に終了したらtrueを返す</returns>
	bool CreateRootSignature();

	/// <summary>
	/// レンダーパイプラインを作成する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool CreateRenderPipeline();

	void RegisterTextureSRV();

	bool InitBoneResource();

	bool MapBB();

	bool InitSRVForBoneBuffer();

	void ReadHierarchyNode(float animationTime, aiNode& node, aiMatrix4x4& parentMat, aiAnimation& anim);

	void ProcessAnimation(float time);

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