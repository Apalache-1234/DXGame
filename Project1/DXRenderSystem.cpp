#include "MyIMGUI.h"
#include "SkyBox.h"
#include "UIRenderer.h"
#include "SkinnedRenderer.h"
#include "DebugCollider.h"
#include "DebugOrigin.h"
#include "DebugRayCast.h"
#include "DebugDL.h"
#include "DebugPointLight.h"
#include "DebugSpotLight.h"
#include "DebugGrids.h"
#include "DebugUIRenderer.h"
#include "PEEdgeFilter.h"
#include "PEMosaicFilter.h"
#include "PERadicalBlur.h"
#include "PEVerticalGaussianBlur.h"
#include "PEHorizonGaussianBlur.h"

DXRenderSystem::DXRenderSystem()
{
}

DXRenderSystem::~DXRenderSystem()
{
}

DXRenderSystem& DXRenderSystem::GetInstance()
{
	if (!instance) {
		instance.reset(new DXRenderSystem());
	}
	return *instance;
}

bool DXRenderSystem::Init()
{
	ComPtr<ID3D12Debug> debug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) {
		debug->EnableDebugLayer();
	}

	bool isSuccess = true;

	isSuccess &= CreateDevice();

	isSuccess &= CreateCommandQueue();

	isSuccess &= CreateFactory();

	isSuccess &= CreateSwapChain();

	isSuccess &= CreateCommandAllocater();

	isSuccess &= CreateCommandList();

	isSuccess &= CreateRTVDescriptorHeap();

	isSuccess &= CreateRTV();

	isSuccess &= CreateDBVResouce();

	isSuccess &= CreateDBVDescripterHeap();

	isSuccess &= CreateDBV();

	isSuccess &= CheckMSAASupport();

	isSuccess &= CreateMSAAResource();

	isSuccess &= CreateMSAAView();

	isSuccess &= CreateFence();

	isSuccess &= CreatePostEffectResouce();

	isSuccess &= CreatePostEffectView();

	isSuccess &= SetScissorAndViewPort();

	postEffectEntity = SystemManager::GetInstance().CreateEntity<PEEdgeFilter>("postEffect");
	postEffectEntity->AddComponent<PEMosaicFilter>();
	postEffectEntity->AddComponent<PERadicalBlur>();
	postEffectEntity->AddComponent<PEHorizonGaussianBlur>();
	postEffectEntity->AddComponent<PEVerticalGaussianBlur>();

	if (!isSuccess)
	{
		std::cerr << "DXRenderSystemの初期化失敗" << std::endl;

		return false;
	}

	return true;
}

ComPtr<ID3D12CommandQueue>& DXRenderSystem::GetCommandQueue()
{
	return cmd_queue;
}

ComPtr<ID3D12GraphicsCommandList>& DXRenderSystem::GetCommandList()
{
	return cmd_list;
}

bool DXRenderSystem::CreateMSAAView()
{
	msaaRTVHandle = heapRT->GetHandle();

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format =DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

	device->CreateRenderTargetView(msaaRT.Get(), &rtvDesc, msaaRTVHandle->cpuHandle);

	msaaDSVHandle = heapDS->GetHandle();

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	device->CreateDepthStencilView(msaaDS.Get(), &dsvDesc, msaaDSVHandle->cpuHandle);

	return true;
}

void DXRenderSystem::WaitForRender()
{
	swap_chain->Present(0, 0);

	auto cur_value = fence_counter;

	cmd_queue->Signal(fence.Get(), cur_value);

	fence_counter++;

	backbuffer_index = swap_chain->GetCurrentBackBufferIndex();

	if (fence->GetCompletedValue() < cur_value)
	{
		fence->SetEventOnCompletion(cur_value, fence_event);

		WaitForSingleObjectEx(fence_event, INFINITE, false);
	}
}

std::unique_ptr<DXRenderSystem> DXRenderSystem::instance = nullptr;

void DXRenderSystem::RenderPostEffect()
{
	auto posts = SystemManager::GetInstance().GetComponentList<PostEffect>();

	posts.erase(std::remove_if(posts.begin(), posts.end(),
		[](shared_ptr<PostEffect> x) { return !x->flag; }),
		posts.end());

	if (posts.empty())
		return;

	int in = 0;

	int out = 1;

	D3D12_RESOURCE_BARRIER barrier;

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	barrier.Transition.pResource = msaaRT.Get();

	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;

	cmd_list->ResourceBarrier(1, &barrier);

	cmd_list->ResolveSubresource(
		postEffect[out].Get(),         // 解像先
		0,                  // MipLevel
		msaaRT.Get(),   // 解像元
		0,                  // MipLevel
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
	);

	barrier.Transition.pResource = msaaRT.Get();

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	cmd_list->ResourceBarrier(1, &barrier);

	int last = posts.size();

	for (int i = 0;i< last;i++)
	{
		if (!posts[i]->flag)
			continue;

		posts[i]->ApplyTextureOnMesh(0, Renderer::TEXTURE_TYPE::DIFFUSE, postEffect[out]);

		if (last - 1 == i)
			break;

		cmd_list->OMSetRenderTargets(1, &postEffectHandle[in]->cpuHandle, false, nullptr);

		cmd_list->ClearRenderTargetView(postEffectHandle[in]->cpuHandle, colorBG, 0, nullptr);

		cmd_list->RSSetScissorRects(1, &scissor);
		cmd_list->RSSetViewports(1, &viewport);

		posts[i]->AddRenderCommand(cmd_list, backbuffer_index);

		std::swap(in, out);
	}

	cmd_list->OMSetRenderTargets(1, &msaaRTVHandle->cpuHandle, false, &msaaDSVHandle->cpuHandle);

	cmd_list->ClearRenderTargetView(msaaRTVHandle->cpuHandle, colorBG, 0, nullptr);
	cmd_list->ClearDepthStencilView(msaaDSVHandle->cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	cmd_list->RSSetScissorRects(1, &scissor);
	cmd_list->RSSetViewports(1, &viewport);

	posts[last - 1]->AddRenderCommand(cmd_list, backbuffer_index);
}

void DXRenderSystem::RenderIMGUI()
{
	auto myIMGUIIns = MyIMGUI::GetInstance();

	if (myIMGUIIns.useIMGUI)
	{
		cmd_list->SetDescriptorHeaps(1, myIMGUIIns.GetHeap().GetAddressOf());
		myIMGUIIns.MainLoop();
	}
}

void DXRenderSystem::Draw()
{
	D3D12_RESOURCE_BARRIER barrier;

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	barrier.Transition.pResource = color_buffer[backbuffer_index].Get();

	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RESOLVE_DEST;

	cmd_list->ResourceBarrier(1, &barrier);

	vector<ID3D12CommandList*> lists;

	barrier.Transition.pResource = msaaRT.Get();

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;

	cmd_list->ResourceBarrier(1, &barrier);

	cmd_list->OMSetRenderTargets(1, &msaaRTVHandle->cpuHandle, false, &msaaDSVHandle->cpuHandle);

	cmd_list->ClearRenderTargetView(msaaRTVHandle->cpuHandle, colorBG, 0, nullptr);

	cmd_list->ClearDepthStencilView(msaaDSVHandle->cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	RenderComponent<SkyBox>();

	RenderComponent<SkinnedRenderer>();

	if (*Flags::GetInstance().isDebug)
	{
		RenderComponent<DebugCollider>();
		RenderComponent<DebugOrigin>();
		RenderComponent<DebugRayCast>();
		RenderComponent<DebugDirectionalLight>();
		RenderComponent<DebugPointLight>();
		RenderComponent<DebugSpotLight>();
		RenderComponent<DebugGrids>();
	}

	RenderPostEffect();

	RenderComponent<UIRenderer>();

	if (*Flags::GetInstance().isDebug)
	{
		RenderComponent<DebugUIRenderer>();
	}

	RenderIMGUI();

	barrier.Transition.pResource = msaaRT.Get();

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;

	cmd_list->ResourceBarrier(1, &barrier);

	cmd_list->ResolveSubresource(
		color_buffer[backbuffer_index].Get(),         // 解像先
		0,                  // MipLevel
		msaaRT.Get(),   // 解像元
		0,                  // MipLevel
		DXGI_FORMAT_R8G8B8A8_UNORM
	);

	barrier.Transition.pResource = color_buffer[backbuffer_index].Get();

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_DEST;

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	cmd_list->ResourceBarrier(1, &barrier);

	ExecuteCommands();

	if (uploadHeaps.size() > 0)
		uploadHeaps.clear();

	if (deletingHeaps.size() > 0)
		deletingHeaps.clear();

	if (deletingComps.size() > 0)
		deletingComps.clear();
}

void DXRenderSystem::ExecuteCommands()
{
	cmd_list->Close();

	ID3D12CommandList* lists_array[] = { cmd_list.Get() };

	cmd_queue->ExecuteCommandLists(1, lists_array);

	WaitForRender();

	cmd_allocator[backbuffer_index]->Reset();

	cmd_list->Reset(cmd_allocator[backbuffer_index].Get(), nullptr);
}

ComPtr<ID3D12Device>& DXRenderSystem::GetDevice()
{
	return device;
}

bool DXRenderSystem::CreateDevice()
{
	HRESULT hr;

	hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, __uuidof(device), IID_PPV_ARGS_Helper(device.GetAddressOf()));

	if (FAILED(hr))
	{
		cout << "デバイスの参照の作成失敗" << endl;

		return false;
	}

	return true;
}

bool DXRenderSystem::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC desc = {};

	desc.NodeMask = 0;

	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	desc.Priority = 0;

	hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(cmd_queue.GetAddressOf()));

	if (FAILED(hr))
	{
		cout << "コマンドキュー作成失敗" << endl;

		return false;
	}

	return true;
}

bool DXRenderSystem::CreateFactory()
{

	hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));

	if (FAILED(hr))
	{
		cout << "ファクトリー作成失敗" << endl;

		return false;
	}

	return true;
}

bool DXRenderSystem::CreateSwapChain()
{
	DXGI_SWAP_CHAIN_DESC desc = {};

	desc.OutputWindow = Window::GetInstance().GetWH();

	desc.BufferCount = frameBuffer;

	desc.BufferDesc.Width = Window::GetInstance().width;

	desc.BufferDesc.Height = Window::GetInstance().height;

	desc.BufferDesc.RefreshRate.Numerator = 60;

	desc.BufferDesc.RefreshRate.Denominator = 1;

	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.BufferDesc.Format =DXGI_FORMAT_R8G8B8A8_UNORM;

	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	desc.Windowed = true;

	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	IDXGISwapChain* ptr_swap;

	hr = factory->CreateSwapChain(cmd_queue.Get(), &desc, &ptr_swap);

	if (FAILED(hr))
	{

		cout << "スワップチェインの生成失敗" << endl;

		return false;
	}

	hr = ptr_swap->QueryInterface(IID_PPV_ARGS(swap_chain.GetAddressOf()));
	
	ptr_swap->Release();

	if (FAILED(hr))
	{
		cout << "スワップチェインのポインタ参照の失敗" << endl;

		return false;
	}

	backbuffer_index = swap_chain->GetCurrentBackBufferIndex();

	return true;
}

bool DXRenderSystem::CreateCommandAllocater()
{
	for (int i = 0; i < frameBuffer; i++)
	{
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmd_allocator[i].GetAddressOf()));

		if (FAILED(hr))
		{
			cout << "コマンドアロケータの作成" << endl;

			return false;
		}
	}

	return true;
}

bool DXRenderSystem::CreateCommandList()
{
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmd_allocator[backbuffer_index].Get(), nullptr, IID_PPV_ARGS(cmd_list.GetAddressOf()));

	if (FAILED(hr))
	{
		cout << "コマンドリストの作成失敗" << endl;

		return false;
	}

	return true;
}

bool DXRenderSystem::CheckMSAASupport()
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevels = {};
	qualityLevels.Format =DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	qualityLevels.SampleCount = msaaCount;

	device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&qualityLevels,
		sizeof(qualityLevels));

	msaaQuality = qualityLevels.NumQualityLevels;
	assert(msaaQuality > 0); 

	return true;
}

bool DXRenderSystem::CreateRTVDescriptorHeap()
{
	heapRT.emplace(100 , D3D12_DESCRIPTOR_HEAP_TYPE_RTV , D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	return true;
}

bool DXRenderSystem::CreateRTV()
{
	for (int i = 0; i < frameBuffer; i++)
	{
		hr = swap_chain->GetBuffer(i, IID_PPV_ARGS(color_buffer[i].GetAddressOf()));

		if (FAILED(hr))
		{
			cout << "モニタのカラーバッファの取得失敗" << endl;

			return false;
		}

		D3D12_RENDER_TARGET_VIEW_DESC desc;

		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		desc.Format =DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		desc.Texture2D.MipSlice = 0;

		desc.Texture2D.PlaneSlice = 0;

		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;  // MSAAの場合

		desc.Texture2DMS.UnusedField_NothingToDefine = 0;  // MSAA用の設定

		RTVHandle[i] = heapRT->GetHandle();

		device->CreateRenderTargetView(color_buffer[i].Get(), &desc, RTVHandle[i]->cpuHandle);
	}

	return true;
}

bool DXRenderSystem::CreateMSAAResource()
{
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	clearValue.Color[0] = 0.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 1.0f;

	D3D12_RESOURCE_DESC colorDesc = {};
	colorDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	colorDesc.Format =DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	colorDesc.Width = Window::width;
	colorDesc.Height = Window::height;
	colorDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	colorDesc.DepthOrArraySize = 1;
	colorDesc.MipLevels = 1;
	colorDesc.SampleDesc.Count = msaaCount;
	colorDesc.SampleDesc.Quality = msaaQuality - 1;

	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&colorDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearValue,
		IID_PPV_ARGS(&msaaRT));

	if (FAILED(hr))
	{
		cout << "MSAA用のRTバッファ作成失敗" << std::endl;

		return false;
	}

	D3D12_RESOURCE_DESC depthDesc = colorDesc;
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;


	hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearDepth,
		IID_PPV_ARGS(&msaaDS));

	if (FAILED(hr))
	{
		cout << "MSAA用のDSバッファ作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool DXRenderSystem::CreateFence()
{
	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));

	if (FAILED(hr))
	{
		cout << "フェンスの作成失敗" << endl;

		return false;
	}

	fence_event = CreateEvent(nullptr, false, false, nullptr);

	if (fence_event == nullptr)
	{
		cout << "フェンスイベントの作成失敗" << endl;

		return false;
	}

	return true;
}

bool DXRenderSystem::CreatePostEffectResouce()
{
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	clearValue.Color[0] = 0.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 1.0f;

	for (int i = 0;i < sizeof(postEffect) / sizeof(postEffect[0]);i++)
	{
		D3D12_HEAP_PROPERTIES prop = {};

		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

		prop.Type = D3D12_HEAP_TYPE_DEFAULT;

		prop.CreationNodeMask = 1;

		prop.VisibleNodeMask = 1;

		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		D3D12_RESOURCE_DESC desc = color_buffer[0]->GetDesc();

		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		hr = device->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_RESOLVE_DEST,
			&clearValue,
			IID_PPV_ARGS(postEffect[i].GetAddressOf())

		);

		if (FAILED(hr))
		{
			std::cerr << "ポストエフェクト作成失敗" << std::endl;

			return false;
		}
	}

	return true;
}

bool DXRenderSystem::CreatePostEffectView()
{
	heap.emplace(100);

	for (int i = 0;i < sizeof(postEffect) / sizeof(postEffect[0]);i++)
	{
		postEffectHandle[i] = heapRT.value().GetHandle();

		D3D12_RENDER_TARGET_VIEW_DESC desc;

		desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		desc.Texture2D.MipSlice = 0;

		desc.Texture2D.PlaneSlice = 0;

		device->CreateRenderTargetView(postEffect[i].Get(), &desc, postEffectHandle[i]->cpuHandle);
	}

	return true;
}


bool DXRenderSystem::CreateDBVResouce()
{
	clearDepth = {};
	clearDepth.Format = DXGI_FORMAT_D32_FLOAT;
	clearDepth.DepthStencil.Depth = 1.0f;
	clearDepth.DepthStencil.Stencil = 0;

	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_DEFAULT;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	desc.Alignment = 0;

	desc.Height = Window::GetInstance().height;

	desc.Width = Window::GetInstance().width;

	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_D32_FLOAT;

	hr = device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearDepth,
		IID_PPV_ARGS(depth_buffer.GetAddressOf())

	);

	if (FAILED(hr))
	{
		std::cerr << "DSリソース作成失敗" << std::endl;

		return false;
	}


	return true;
}

bool DXRenderSystem::CreateDBVDescripterHeap()
{
	heapDS.emplace(100 , D3D12_DESCRIPTOR_HEAP_TYPE_DSV , D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	return  true;
}

bool DXRenderSystem::CreateDBV()
{
	DSVHandle = heapDS.value().GetHandle();

	D3D12_DEPTH_STENCIL_VIEW_DESC desc_depth = {};

	desc_depth.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	desc_depth.Flags = D3D12_DSV_FLAG_NONE;
	desc_depth.Format = DXGI_FORMAT_D32_FLOAT;
	desc_depth.Texture2D.MipSlice = 0;

	device->CreateDepthStencilView(depth_buffer.Get(), &desc_depth, DSVHandle->cpuHandle);

	return true;
}

ComPtr<ID3D12Resource>& DXRenderSystem::GetEmptyUploadHeap()
{
	uploadHeaps.emplace_back();

	return uploadHeaps.back();
}

void DXRenderSystem::SetDeletingHeap(ComPtr<ID3D12Resource> heap)
{
	deletingHeaps.push_back(heap);
}

void DXRenderSystem::SetDeletingComps(shared_ptr<Component> comp)
{
	deletingComps.push_back(comp);
}

void DXRenderSystem::SetDeletingComps(std::vector<shared_ptr<Component>> comps)
{
	deletingComps.insert(deletingComps.end() , comps.begin() , comps.end());
}

bool DXRenderSystem::SetScissorAndViewPort()
{
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	viewport.Height = Window::GetInstance().height;

	viewport.Width = Window::GetInstance().width;

	scissor.left = 0;
	scissor.top = 0;
	scissor.bottom = Window::GetInstance().height;
	scissor.right = Window::GetInstance().width;

	return true;
}
