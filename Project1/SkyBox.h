#ifndef SKYBOX_H
#define SKYBOX_H

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
#include "DXRenderSystem.h"
#include "DescriptorHeapManager.h"
#include "MeshLoader.h"

class SkyBox
{
public:
	SkyBox();

	~SkyBox();

	void AddRenderCommand(ComPtr<ID3D12GraphicsCommandList>& cmdList, int backbufferIndex);

	static SkyBox& GetInstance();

	bool Init();
private:
	/// <summary>
    /// スカイボックスを構築するクラスのシングルトンのインスタンス
    /// </summary>
	static std::unique_ptr<SkyBox> instance;

	/// <summary>
    /// WinAPI系の関数の可否を格納する変数
    /// </summary>
	HRESULT hr;

	/// <summary>
    /// デスクリプタヒープ
    /// </summary>
	DescriptorHeapManager heapManager;

	/// <summary>
    // バックバッファ込みのコンスタントバッファのリソース
    /// </summary>
	ComPtr<ID3D12Resource> constantBuffer[DXRenderSystem::frameBuffer];

	/// <summary>
    /// バックバッファ込みのコンスタントバッファビュー
    /// </summary>
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbView[DXRenderSystem::frameBuffer];

	DescriptorHeapManager::PtrHandles cbHandle[DXRenderSystem::frameBuffer];

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

	Transform trans;

	MeshLoader::MeshData* meshData;

	DescriptorHeapManager::PtrHandles texHandle;

	/// <summary>
    /// ルートシグネチャのCOM
    /// </summary>
	ComPtr<ID3D12RootSignature> rootSig;

	/// <summary>
    /// レンダーパイプラインのCOM
    /// </summary>
	ComPtr<ID3D12PipelineState> pipeline;

	void ConvertValues();

	bool MapCB();

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

	bool CreateMesh(const MeshLoader::MESHTYPE type);

	bool CreateTexture();
};

#endif

