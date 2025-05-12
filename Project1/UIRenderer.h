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

	bool LoadTexture(const std::string fileName);

	/// <summary>
    /// そのメッシュのカラーを決める
    /// </summary>
	DirectX::XMVECTOR color = DirectX::XMVectorSet(1, 1, 1, 1);
private:

	/// <summary>
	/// WinAPI系の関数の可否を格納する変数
	/// </summary>
	HRESULT hr;

	/// <summary>
	/// デスクリプタヒープ
	/// </summary>
	ComPtr<ID3D12DescriptorHeap> heap;

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
	/// 頂点情報を格納する構造体
	/// </summary>
	struct VertexData
	{
		XMFLOAT3 vertex;
		XMFLOAT2 uv;
	};

	/// <summary>
    /// デフォルトは立方体
    /// </summary>
	std::vector<VertexData> vertexDatas = {
	{ XMFLOAT3(-Window::width /2, Window::height / 2, 0) , XMFLOAT2(0, 0)},
	{ XMFLOAT3(Window::width / 2, Window::height / 2, 0) , XMFLOAT2(1, 0)},
	{ XMFLOAT3(Window::width / 2, -Window::height / 2, 0) , XMFLOAT2(1, 1)},
	{ XMFLOAT3(-Window::width / 2, -Window::height / 2, 0) , XMFLOAT2(0, 1)},
	};

	/// <summary>
	/// デフォルトは立方体のインデックス
	/// </summary>
	std::vector<int> indices =
	{
	0, 1, 2,
	2, 3, 0 
	};

	DXRenderSystem::TextureView texView;

	/// <summary>
	/// 頂点バッファのアップロードリソース
	/// </summary>
	ComPtr<ID3D12Resource> uploadVertexBuffer;

	/// <summary>
	/// 頂点バッファのリソース
	/// </summary>
	ComPtr<ID3D12Resource> vertexBuffer;

	/// <summary>
	/// インデックスバッファのアップロードリソース
	/// </summary>
	ComPtr<ID3D12Resource> uploadIndexBuffer;

	/// <summary>
	/// インデックスバッファのリソース
	/// </summary>
	ComPtr<ID3D12Resource> indexBuffer;

	/// <summary>
	/// インデックスビュー
	/// </summary>
	D3D12_INDEX_BUFFER_VIEW ibView;

	/// <summary>
	/// 頂点ビュー
	/// </summary>
	D3D12_VERTEX_BUFFER_VIEW vbView;

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

