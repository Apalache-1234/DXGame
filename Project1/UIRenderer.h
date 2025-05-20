#ifndef UIRENDERER_H
#define UIRENDERER_H

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

class UIRenderer : public Component, public std::enable_shared_from_this<UIRenderer>
{
public:
	UIRenderer(std::shared_ptr<Entity> ent);

	~UIRenderer();

	void AddRenderCommand(ComPtr<ID3D12GraphicsCommandList>& cmdList, int backbufferIndex);

	bool CreateTexture(const std::string fileName);

	/// <summary>
/// そのメッシュのカラーを決める
/// </summary>
	DirectX::XMVECTOR color = DirectX::XMVectorSet(1, 1, 1, 1);
private:
	MeshLoader::MeshData* meshData;

	std::unique_ptr<DescriptorHeapManager::PtrHandles> texHandle;

	/// <summary>
/// デスクリプタヒープ
/// </summary>
	DescriptorHeapManager heapManager;

	/// <summary>
	/// WinAPI系の関数の可否を格納する変数
	/// </summary>
	HRESULT hr;

	/// <summary>
	// バックバッファ込みのコンスタントバッファのリソース
	/// </summary>
	ComPtr<ID3D12Resource> constantBuffer[DXRenderSystem::frameBuffer];

	/// <summary>
	/// バックバッファ込みのコンスタントバッファビュー
	/// </summary>
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbView[DXRenderSystem::frameBuffer];

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
	bool InitVBVResource();

	/// <summary>
	/// 頂点バッファをマッピングしてGPUに頂点データを入力する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool MapVB();

	bool VBUploadToDefultHeap();

	/// <summary>
	/// インデックスバッファビューのリソースを作成する
	/// </summary>
	///  <returns>正常に終了したらtrueを返す</returns>
	bool InitIBVResource();

	/// <summary>
	/// インデックスバッファをマッピングしてGPUにインデックスデータを入力する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool MapIB();

	bool IBUploadToDefultHeap();

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

};

#endif

