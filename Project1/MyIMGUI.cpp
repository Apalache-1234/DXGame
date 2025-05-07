#include "MyIMGUI.h"

std::unique_ptr<MyIMGUI> MyIMGUI::instance = nullptr;

MyIMGUI::MyIMGUI()
{
}

MyIMGUI::~MyIMGUI()
{
}

MyIMGUI& MyIMGUI::GetInstance()
{
	if (!instance) {
		instance.reset(new MyIMGUI());
	}
	return *instance;
}

MyIMGUI::ExampleDescriptorHeapAllocator g_pd3dSrvDescHeapAlloc;

void MyIMGUI::Init()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(Window::GetInstance().GetWH());

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 128;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;

	auto device = DXRenderSystem::GetInstance().GetDevice();

	auto cmdQueue = DXRenderSystem::GetInstance().GetCommandQueue();

	HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(forIMGUIHeap.GetAddressOf()));
	if (FAILED(hr))
	{
		std::cerr << "IMGUI用ヒープ設定失敗" << std::endl;
	}

	g_pd3dSrvDescHeapAlloc.Create(device.Get(), forIMGUIHeap.Get());

	ImGui_ImplDX12_InitInfo init_info = {};
	init_info.Device = device.Get();
	init_info.CommandQueue = cmdQueue.Get();
	init_info.NumFramesInFlight = DXRenderSystem::frameBuffer;
	init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	init_info.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	init_info.SrvDescriptorHeap = forIMGUIHeap.Get();
	init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) { return g_pd3dSrvDescHeapAlloc.Alloc(out_cpu_handle, out_gpu_handle); };
	init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) { return g_pd3dSrvDescHeapAlloc.Free(cpu_handle, gpu_handle); };
	ImGui_ImplDX12_Init(&init_info);
}

void MyIMGUI::Term()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void MyIMGUI::MainLoop()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	auto flagIns = Flags::GetInstance();



	//以下自由にIMGUIをカスタムするためのコード
	{

		ImGui::Begin("MainWindow");

		if (ImGui::TreeNode("Entites")) {

			auto list = GameManager::GetInstance().GetInstance().GetEntityList();

			for (auto entity : list)
			{
				if (!entity->HasParent())
				{
					if (ImGui::Selectable(entity->name.c_str()))
					{
	
					}
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Flags")) {

			ImGui::Checkbox("Debug", flagIns.isDebug.get());
			ImGui::Checkbox("Time", flagIns.isTimeStop.get());

			ImGui::TreePop();
		}
		
		ImGui::End();
	}
	//ここまで

	ImGui::Render();
		
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), DXRenderSystem::GetInstance().GetCommandList().Get());
}

ComPtr<ID3D12DescriptorHeap> MyIMGUI::GetHeap()
{
	return forIMGUIHeap;
}
