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
#include "MeshLoader.h"
#include "DXRenderSystem.h"
#include "Utility.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

class Renderer : public Component
{
public:
	struct MeshData;

	enum TEXTURE_TYPE
	{
		DIFFUSE,
		NORMAL,
		CountLast
	};

    ~Renderer();

	void Init() override;

	/// <summary>
	/// メッシュを描画するレンダーコマンドをコマンドリストに格納する
	/// </summary>
	/// <param name="cmdList">格納するコマンドリスト</param>
	/// <param name="backBufferIndex">現在のバックバッファのインデックス</param>
	void virtual AddRenderCommand(ComPtr<ID3D12GraphicsCommandList>& cmdList, int backBufferIndex);

	std::vector<std::string> GetMeshNameList();

	Material& GetMaterial(int idx);

	void SetMaterial(int idx, const Material& mat);

	/// <summary>
	/// そのメッシュのカラーを決める
	/// </summary>
	DirectX::XMVECTOR color = DirectX::XMVectorSet(1, 1, 1, 1);

	MeshLoader::MESHTYPE GetCurrentMeshType();

	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureGPUHandle(int idx, Renderer::TEXTURE_TYPE type);
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureCPUHandle(int idx, Renderer::TEXTURE_TYPE type);

	bool CreateMesh(const MeshLoader::MESHTYPE type);

	void ApplyTextureOnMesh(int idx, Renderer::TEXTURE_TYPE type, ComPtr<ID3D12Resource> texResource);

	void ApplyTextureOnMesh(int idx, Renderer::TEXTURE_TYPE type, ComPtr<ID3D12Resource> texResource, D3D12_SHADER_RESOURCE_VIEW_DESC desc);

	void LoadVertexShader(LPCWSTR vsName);

	void LoadPixelShader(LPCWSTR psName);

	/// <summary>
    /// レンダーパイプラインを作成する
    /// </summary>
    /// <returns>正常に終了したらtrueを返す</returns>
	bool CreateRenderPipeline();

	void AddCommandListFunction(std::function< void(ComPtr<ID3D12GraphicsCommandList>)> f);
protected:
	Renderer(std::shared_ptr<Entity> ent);

	void virtual Update();

	void virtual InitRenderer();

	void virtual InitInCreateMesh();

	void SetPrimitiveType(D3D12_PRIMITIVE_TOPOLOGY type);

	D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveType();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetDefaultPipelineDesc();

	void AddInputElementDesc(D3D12_INPUT_ELEMENT_DESC desc);

	void virtual AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList);

	void virtual AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList, UINT meshInd);

	void PrepareForRenderPipeline();

	bool AddRenderPipeline(D3D12_GRAPHICS_PIPELINE_STATE_DESC desc);

	void ChangeRenderPipeline(int idx);

	void SetRasterizerDesc(D3D12_RASTERIZER_DESC desc);

	void SetRTBDesc(D3D12_RENDER_TARGET_BLEND_DESC desc);

	void SetDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC desc);

	int instanceCount = 1;

	bool isPipelineAlreadyCreated = false;

	/// <summary>
    /// WinAPI系の関数の可否を格納する変数
    /// </summary>
	HRESULT hr;

	MeshLoader::MeshData* meshData;

	MeshLoader::MESHTYPE currentMeshType;

	/// <summary>
	/// バックバッファ込みのコンスタントバッファのリソース
	/// </summary>
	std::vector<ComPtr<ID3D12Resource>> constantBuffer;

	UINT currentCBCount = 5;

	/// <summary>
	/// バックバッファ込みのコンスタントバッファビュー
	/// </summary>
	std::vector<D3D12_CONSTANT_BUFFER_VIEW_DESC> cbView;

	std::vector<std::unique_ptr<PtrHandles>> cbHandle;

	std::unique_ptr<PtrHandles> materialHandle;
	ComPtr<ID3D12Resource> materialResource;

	std::vector<std::unique_ptr<PtrHandles>> texHandles;

	/// <summary>
	/// 変換行列とカラーを格納するTransform変数
	/// </summary>
	Transform trans;

	std::vector<Material> mats;
private:
	std::vector<std::function< void(ComPtr<ID3D12GraphicsCommandList>)>> cmdFuncs;

	D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	/// <summary>
    /// レンダーパイプラインのCOM
    /// </summary>
	std::vector<ComPtr<ID3D12PipelineState>> pipeline;

	ComPtr<ID3D12PipelineState> currentPipeline;

	ComPtr<ID3D12PipelineState> shadowMapPipeline;

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

	bool InitMaterialResource();

	bool MapMB();

	bool InitSRVForMaterialResource();

	/// <summary>
	/// コンスタントバッファのリソースをビューを通じてデスクリプタヒープに登録する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool LinkCBVResourceToDescHeap();

	void CreateSRV(ComPtr<ID3D12Resource> texResource, D3D12_CPU_DESCRIPTOR_HANDLE handle);

	void RegisterTextureSRV();

	/// <summary>
	/// 座標行列を作成する
	/// </summary>
	void ConvertValues();

	/// <summary>
	/// コンスタントバッファをマッピングしてGPUに座標行列などを入力する
	/// </summary>
	///<returns>正常に終了したらtrueを返す</returns>
	bool MapCB(int backBuffer);
};

#endif