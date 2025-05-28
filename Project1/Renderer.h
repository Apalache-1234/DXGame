#ifndef RENDERER_H
#define RENDERER_H

#include "Component.h"
#include <iostream>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "Window.h"
#undef min
#undef max
#include "DescriptorHeapManager.h"
#include "MeshLoader.h"
#include "DXRenderSystem.h"
#include "Utility.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

class Renderer : public Component , public enable_shared_from_this<Renderer>
{
public:
	struct MeshData;

	Renderer(std::shared_ptr<Entity> ent);

	virtual ~Renderer();

	void Init() override;

	/// <summary>
	/// メッシュを描画するレンダーコマンドをコマンドリストに格納する
	/// </summary>
	/// <param name="cmdList">格納するコマンドリスト</param>
	/// <param name="backBufferIndex">現在のバックバッファのインデックス</param>
	void virtual AddRenderCommand(ComPtr<ID3D12GraphicsCommandList>& cmdList, int backBufferIndex);

	/// <summary>
	/// そのメッシュのカラーを決める
	/// </summary>
	DirectX::XMVECTOR color = DirectX::XMVectorSet(1, 1, 1, 1);

	MeshLoader::MESHTYPE GetCurrentMeshType();

	bool CreateMesh(const MeshLoader::MESHTYPE type);

	D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
protected:
	void virtual Update();

	void virtual InitRenderer();

	void virtual InitInCreateMesh();

	void AddRootParam(D3D12_ROOT_PARAMETER param);

	void AddInputElementDesc(D3D12_INPUT_ELEMENT_DESC desc);

	void virtual AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList);

	void SetDescSampler(D3D12_STATIC_SAMPLER_DESC desc);

	void PrepareForRootSignature();

	void PrepareForRenderPipeline();

	void SetRasterizerDesc(D3D12_RASTERIZER_DESC desc);

	void SetRTBDesc(D3D12_RENDER_TARGET_BLEND_DESC desc);

	void LoadVertexShader(LPCWSTR vsName);

	void LoadPixelShader(LPCWSTR psName);

	void SetDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC desc);

	int instanceCount = 1;

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

	std::unique_ptr<DescriptorHeapManager::PtrHandles> cbHandle[DXRenderSystem::frameBuffer];

	std::vector<std::unique_ptr<DescriptorHeapManager::PtrHandles>> texHandles;

	/// <summary>
	/// ルートシグネチャのCOM
	/// </summary>
	ComPtr<ID3D12RootSignature> rootSig;

	/// <summary>
	/// レンダーパイプラインのCOM
	/// </summary>
	ComPtr<ID3D12PipelineState> pipeline;

	/// <summary>
	/// 変換行列とカラーを格納するTransform変数
	/// </summary>
	Transform trans;
private:
	D3D12_STATIC_SAMPLER_DESC descSampler;

	std::vector<D3D12_ROOT_PARAMETER> param;

	D3D12_DESCRIPTOR_RANGE range;

	std::vector<D3D12_INPUT_ELEMENT_DESC> descElement;

	D3D12_RASTERIZER_DESC descRas;

	D3D12_RENDER_TARGET_BLEND_DESC descRtBlend;

	ComPtr<ID3DBlob> vsBlob;

	ComPtr<ID3DBlob> psBlob;

	D3D12_BLEND_DESC descBlend;

	D3D12_DEPTH_STENCIL_DESC dsDesc;

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