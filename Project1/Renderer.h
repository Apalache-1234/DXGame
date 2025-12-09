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

#define PtrHandle std::unique_ptr<DescriptorHeapManager::PtrHandles>

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

class Renderer : public Component , public enable_shared_from_this<Renderer>
{
public:
	struct MeshData;

	enum TEXTURE_TYPE
	{
		DIFFUSE,
		NORMAL,
		CountLast
	};

	Renderer(std::shared_ptr<Entity> ent);

	virtual ~Renderer();

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

	std::type_index GetBaseType() override;
protected:
	void virtual Update();

	void virtual InitRenderer();

	void virtual InitInCreateMesh();

	void AddRootParam(D3D12_ROOT_PARAMETER param);

	int GetRootParamNum();

	void SetPrimitiveType(D3D12_PRIMITIVE_TOPOLOGY type);

	D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveType();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetDefaultPipelineDesc();

	void AddInputElementDesc(D3D12_INPUT_ELEMENT_DESC desc);

	void virtual AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList);

	void SetDescSampler(D3D12_STATIC_SAMPLER_DESC desc, SamplerType type);

	void PrepareForRootSignature();

	void PrepareForRenderPipeline();

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

	bool AddRenderPipeline(D3D12_GRAPHICS_PIPELINE_STATE_DESC desc);

	void ChangeRenderPipeline(int idx);

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

	ComPtr<ID3D12Resource> lightInfoCB;

	ComPtr<ID3D12Resource> timeInfoCB;

	DescriptorHeapManager heapManager;

	/// <summary>
	/// バックバッファ込みのコンスタントバッファビュー
	/// </summary>
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbView[DXRenderSystem::frameBuffer];
	D3D12_CONSTANT_BUFFER_VIEW_DESC lightInfoCBView;
	D3D12_CONSTANT_BUFFER_VIEW_DESC timeInfoCBView;

	std::unique_ptr<DescriptorHeapManager::PtrHandles> cbHandle[DXRenderSystem::frameBuffer];

	std::unique_ptr<DescriptorHeapManager::PtrHandles> lightInfoCBHandle;

	std::unique_ptr<DescriptorHeapManager::PtrHandles> timeInfoCBHandle;

	std::unique_ptr<DescriptorHeapManager::PtrHandles> materialHandle;
	ComPtr<ID3D12Resource> materialResource;

	std::vector<PtrHandle> texHandles;

	std::vector<PtrHandle> shadowMapTexHandles;

	/// <summary>
	/// ルートシグネチャのCOM
	/// </summary>
	ComPtr<ID3D12RootSignature> rootSig;

	/// <summary>
	/// 変換行列とカラーを格納するTransform変数
	/// </summary>
	Transform trans;

	LightInfo lightInfo;

	TimeInfo timeInfo;

	std::vector<Material> mats;
private:
	struct DirLight
	{
		DirectX::XMFLOAT3 direct;

		DirectX::XMFLOAT3 color;

		float power;

		float shadow;

		DirectX::XMMATRIX matVP 
	};

	struct PLight
	{
		float radius;

		DirectX::XMFLOAT3 color;

		DirectX::XMFLOAT3 pos;

		float power;

		float shadow;

		DirectX::XMMATRIX matVP
	};

	struct SLight
	{
		float angular;

		DirectX::XMFLOAT3 direct;

		DirectX::XMFLOAT3 color;

		DirectX::XMFLOAT3 pos;

		float power;

		float length;

		float shadow;

		DirectX::XMMATRIX matVP
	};

	D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	/// <summary>
    /// レンダーパイプラインのCOM
    /// </summary>
	std::vector<ComPtr<ID3D12PipelineState>> pipeline;

	ComPtr<ID3D12PipelineState> currentPipeline;

	ComPtr<ID3D12PipelineState> shadowMapPipeline;

	std::vector<DirLight> DirLights;

	ComPtr<ID3D12Resource> DLBuffer;

	std::unique_ptr<DescriptorHeapManager::PtrHandles> DLBHandle;

	std::vector<PLight> PLights;

	ComPtr<ID3D12Resource> PLBuffer;

	std::unique_ptr<DescriptorHeapManager::PtrHandles> PLBHandle;

	std::vector<SLight> SLights;

	ComPtr<ID3D12Resource> SLBuffer;

	std::unique_ptr<DescriptorHeapManager::PtrHandles> SLBHandle;

	std::vector<D3D12_STATIC_SAMPLER_DESC> descSamplers;

	std::vector<D3D12_ROOT_PARAMETER> param;

	D3D12_DESCRIPTOR_RANGE rangeCB;
	D3D12_DESCRIPTOR_RANGE rangeSRV[3];

	D3D12_DESCRIPTOR_RANGE shadowMapSRV[1];

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

	bool InitLightInfoCB();

	bool LinkLightInfoCBVToDescHeap();

	bool MapLLICB();

	bool InitTimeInfoCB();

	bool LinkTimeInfoCBVToDescHeap();

	bool MapTICB();

	bool InitMaterialResource();

	bool MapMB();

	bool InitSRVForMaterialResource();

	/// <summary>
	/// コンスタントバッファのリソースをビューを通じてデスクリプタヒープに登録する
	/// </summary>
	/// <returns>正常に終了したらtrueを返す</returns>
	bool LinkCBVResourceToDescHeap();

	void CreateSRV(ComPtr<ID3D12Resource> texResource, D3D12_CPU_DESCRIPTOR_HANDLE handle);

	void RegisterShadowMapTextureSRV();

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

	bool InitDirectionalLightBResource();

	bool MapDLB();

	bool InitDLBV();

	bool InitPointLightBResource();

	bool MapPLB();

	bool InitPLBV();

	bool InitSpotLightBResource();

	bool MapSLB();

	bool InitSLBV();
};

#endif