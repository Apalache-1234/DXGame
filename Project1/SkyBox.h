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
	};

	/// <summary>
/// デフォルトは立方体
/// </summary>
	std::vector<VertexData> vertexDatas = {
	{ XMFLOAT3(-1, -1, -1)},
	{ XMFLOAT3(-1, -1,  1)},
	{ XMFLOAT3(-1,  1, -1)},
	{ XMFLOAT3(-1,  1,  1)},
	{ XMFLOAT3(1, -1, -1)},
	{ XMFLOAT3(1, -1,  1)},
	{ XMFLOAT3(1,  1, -1)},
	{ XMFLOAT3(1,  1,  1)}
	};

	/// <summary>
	/// デフォルトは立方体のインデックス
	/// </summary>
	std::vector<int> indices =
	{
			4, 5, 7,
			4, 7, 6,
			0, 2, 3,
			0, 3, 1,
			0, 4, 6,
			0, 6, 2,
			1, 3, 7,
			1, 7, 5,
			2, 6, 7,
			2, 7, 3,
			0, 1, 5,
			0, 5, 4
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
	bool LoadTexture();
};

#endif

