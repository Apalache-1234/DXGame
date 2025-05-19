#include "DXRenderSystem.h"

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
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {

		std::cout << "DirectXデバックモードON" << std::endl;

		debugController->EnableDebugLayer();
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

	isSuccess &= CreateFence();

	isSuccess &= CreateDBVResouce();

	isSuccess &= CreateDBVDescripterHeap();

	isSuccess &= CreateDBV();

	isSuccess &= SetScissorAndViewPort();

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

void DXRenderSystem::Draw()
{
	D3D12_RESOURCE_BARRIER barrier;

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	barrier.Transition.pResource = color_buffer[backbuffer_index].Get();

	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	vector<ID3D12CommandList*> lists;

	cmd_list->ResourceBarrier(1, &barrier);

	cmd_list->OMSetRenderTargets(1, &rtv_handle[backbuffer_index], false, &dbv_handle);

	cmd_list->ClearRenderTargetView(rtv_handle[backbuffer_index], colorBG, 0, nullptr);

	cmd_list->ClearDepthStencilView(dbv_handle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	auto rendererComponents = GameManager::GetInstance().GetComponentList<Renderer>();
	auto UIRendererComponents = GameManager::GetInstance().GetComponentList<UIRenderer>();

	cmd_list->RSSetScissorRects(1, &scissor);
	cmd_list->RSSetViewports(1, &viewport);

	SkyBox::GetInstance().AddRenderCommand(cmd_list, backbuffer_index);

	for (int i = 0; i < rendererComponents.size(); i++)
	{
		cmd_list->RSSetScissorRects(1, &scissor);
		cmd_list->RSSetViewports(1, &viewport);

		rendererComponents[i]->AddRenderCommand(cmd_list , backbuffer_index);
	}

	for (int i = 0; i < UIRendererComponents.size(); i++)
	{
		cmd_list->RSSetScissorRects(1, &scissor);
		cmd_list->RSSetViewports(1, &viewport);

		UIRendererComponents[i]->AddRenderCommand(cmd_list, backbuffer_index);
	}
	
	auto myIMGUIIns = MyIMGUI::GetInstance();
	
	if (myIMGUIIns.useIMGUI)
	{
		cmd_list->SetDescriptorHeaps(1, myIMGUIIns.GetHeap().GetAddressOf());
		myIMGUIIns.MainLoop();
	}

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	cmd_list->ResourceBarrier(1, &barrier);

	ExecuteCommands();

	if (uploadHeaps.size() > 0)
		uploadHeaps.clear();
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

	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

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

bool DXRenderSystem::CreateRTVDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};

	desc.NodeMask = 0;

	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	desc.NumDescriptors = frameBuffer;

	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(heap_rtv.GetAddressOf()));

	if (FAILED(hr))
	{
		cout << "レンダーターゲットビュー用のデスクリプタヒープの作成失敗" << endl;

		return false;
	}

	return true;
}

bool DXRenderSystem::CreateRTV()
{
	auto handle = heap_rtv->GetCPUDescriptorHandleForHeapStart();

	auto increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

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

		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

		desc.Texture2D.MipSlice = 0;

		desc.Texture2D.PlaneSlice = 0;

		device->CreateRenderTargetView(color_buffer[i].Get(), &desc, handle);

		rtv_handle[i] = handle;

		handle.ptr += increment;
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

bool DXRenderSystem::CreateDBVResouce()
{
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

	D3D12_CLEAR_VALUE clear_depth = {};

	clear_depth.Format = DXGI_FORMAT_D32_FLOAT;
	clear_depth.DepthStencil.Depth = 1.0f;
	clear_depth.DepthStencil.Stencil = 0;

	hr = device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clear_depth,
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
	D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};

	heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	heap_desc.NodeMask = 0;

	heap_desc.NumDescriptors = 1;

	heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	hr = device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(heap_dbv.GetAddressOf()));

	if (FAILED(hr))
	{
		std::cerr << "デスクリプターヒープ作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool DXRenderSystem::CreateDBV()
{
	D3D12_DEPTH_STENCIL_VIEW_DESC desc_depth = {};

	auto handle = heap_dbv->GetCPUDescriptorHandleForHeapStart();

	auto increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	desc_depth.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	desc_depth.Flags = D3D12_DSV_FLAG_NONE;
	desc_depth.Format = DXGI_FORMAT_D32_FLOAT;
	desc_depth.Texture2D.MipSlice = 0;

	device->CreateDepthStencilView(depth_buffer.Get(), &desc_depth, handle);

	dbv_handle = handle;

	return true;
}

ComPtr<ID3D12Resource>& DXRenderSystem::GetEmptyUploadHeap()
{
	uploadHeaps.emplace_back();

	return uploadHeaps.back();
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
