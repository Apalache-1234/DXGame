#include "MyIMGUI.h"
#include <tchar.h> 

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

	auto io = ImGui::GetIO();

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

void ShowInfo()
{
	string str = "FPS : ";
	str += to_string(GameManager::GetInstance().fps);

	ImGui::Text(str.c_str());

	str = "DisplaySize : ";
	str += " X " + std::to_string(Window::width) + " Y " + std::to_string(Window::height);

	ImGui::Text(str.c_str());

	POINT ptScreen;
	::GetCursorPos(&ptScreen);

	POINT ptClient = ptScreen;
	::ScreenToClient(Window::GetInstance().GetWH(), &ptClient);

	str = "ScreenPos : ";
	str += " X " + std::to_string(ptScreen.x) + " Y " + std::to_string(ptScreen.y);

	ImGui::Text(str.c_str());

	str = "ClientPos : ";
	str += " X " + std::to_string(ptClient.x) + " Y " + std::to_string(ptClient.y);

	ImGui::Text(str.c_str());

	auto io = ImGui::GetIO();

	str = "ImGuiCursorPos : ";
	str += " X " + std::to_string(io.MousePos.x) + " Y " + std::to_string(io.MousePos.y);

	ImGui::Text(str.c_str());

	str = "GameTime : " + std::to_string(GameManager::GetInstance().GameTime);

	ImGui::Text(str.c_str());

	str = "DeltaTime : " + std::to_string(GameManager::GetInstance().DeltaTime);

	ImGui::Text(str.c_str());
}

bool showEntityWindow = false;
bool showAnimationWindow = false;

shared_ptr<Entity> currentEntity = 0;

std::vector < std::shared_ptr<Entity>> entityList;

DirectX::XMVECTOR ShowAndSetVecInfo(string label , DirectX::XMVECTOR pos, float speed = 0.05)
{
	string str = label;

	float x = DirectX::XMVectorGetX(pos);
	float y = DirectX::XMVectorGetY(pos);
	float z = DirectX::XMVectorGetZ(pos);

	ImGui::Text(str.c_str());

	ImGui::PushID(label.c_str()); 

	ImGui::SetNextItemWidth(60);
	ImGui::DragFloat("X ", &x , speed);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	ImGui::DragFloat("Y ", &y, speed);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	ImGui::DragFloat("Z ", &z, speed);
	ImGui::SameLine();
	ImGui::NewLine();

	ImGui::PopID();

	return DirectX::XMVectorSet(x, y, z, 0);
}

ImVec2 ButtonRect = ImVec2(50, 20);

void EditColor(DirectX::XMVECTOR* color)
{
	float col[4];

	col[0] = DirectX::XMVectorGetX(*color);
	col[1] = DirectX::XMVectorGetY(*color);
	col[2] = DirectX::XMVectorGetZ(*color);
	col[3] = DirectX::XMVectorGetW(*color);

	ImGui::ColorEdit4(" : Color", col);

	*color = DirectX::XMVectorSet(col[0], col[1], col[2], col[3]);
}

void ShowRendererInfo(std::shared_ptr<Renderer> rend)
{
	EditColor(&rend->color);

	if (ImGui::TreeNode("Animations")) {

		auto animList = rend->GetAnimationNameList();

		for (int i = 0;i<animList.size();i++)
		{
			if (ImGui::Selectable(animList[i].c_str()))
			{
				rend->StartAnimation(i , 1 , true);

				showAnimationWindow = true;
			}
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Mesh")) {

		const char* meshNames[] = {
     	"KNIGHT",
	    "SPARTAN",
	    "BOX"
		};

		MeshLoader::MESHTYPE currentMeshType = rend->GetCurrentMeshType();

		int currentIndex = static_cast<int>(currentMeshType);
		if (ImGui::Combo("MeshType", &currentIndex, meshNames, IM_ARRAYSIZE(meshNames))) {
			currentMeshType = static_cast<MeshLoader::MESHTYPE>(currentIndex);

			rend->CreateMesh(currentMeshType);
		}

		ImGui::TreePop();
	}
}

void ShowRigidbodyInfo(std::shared_ptr<Rigidbody> rigid)
{
	ImGui::Checkbox("IsStatic" , &rigid->isStatic);

	auto vel = rigid->GetVelocity();

	ImGui::Text("Velocity : ");
	ImGui::SetNextItemWidth(60);
	float x = DirectX::XMVectorGetX(vel);
	ImGui::DragFloat("X ", &x, 0.1);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	float y = DirectX::XMVectorGetY(vel);
	ImGui::DragFloat("Y ", &y, 0.1);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	float z = DirectX::XMVectorGetZ(vel);
	ImGui::DragFloat("Z ", &z, 0.1);
	ImGui::SameLine();
	ImGui::NewLine();

	rigid->SetVelocity(DirectX::XMVectorSet(x, y, z, 0));

	float mass = rigid->GetMass();

	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat(" : Mass", &mass, 0.1);
	ImGui::SameLine();
	ImGui::NewLine();

	rigid->SetMass(mass);
}

void ShowColliderInfo(std::shared_ptr<Collider> col)
{
	auto pos = col->position;

	ImGui::PushID("a");

	ImGui::Text("Position : ");
	ImGui::SetNextItemWidth(60);
	float x = DirectX::XMVectorGetX(pos);
	ImGui::DragFloat("X ", &x, 0.1);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	float y = DirectX::XMVectorGetY(pos);
	ImGui::DragFloat("Y ", &y, 0.1);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	float z = DirectX::XMVectorGetZ(pos);
	ImGui::DragFloat("Z ", &z, 0.1);
	ImGui::SameLine();
	ImGui::NewLine();

	ImGui::PopID();

	col->position = (DirectX::XMVectorSet(x, y, z, 0));

	auto scale = col->scale;

	ImGui::PushID("v");

	ImGui::Text("Scale : ");
	ImGui::SetNextItemWidth(60);
	x = DirectX::XMVectorGetX(scale);
	ImGui::DragFloat("X ", &x, 0.1);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	y = DirectX::XMVectorGetY(scale);
	ImGui::DragFloat("Y ", &y, 0.1);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	z = DirectX::XMVectorGetZ(scale);
	ImGui::DragFloat("Z ", &z, 0.1);
	ImGui::SameLine();
	ImGui::NewLine();

	ImGui::PopID();

	col->scale = (DirectX::XMVectorSet(x, y, z, 0));
}

void ShowAnimationInfo(std::shared_ptr<Renderer> rend)
{
	if (!rend)
		return;

	if (ImGui::Begin("AnimationWindow", &showAnimationWindow))  // ウィンドウ名を指定
	{
		string str = "Time : ";

		std::ostringstream cut;

		cut << std::fixed << std::setprecision(2) << rend->GetAnimCurrentTime();

		str += cut.str();

		str += " / ";

		cut = std::ostringstream();

		cut << std::fixed << std::setprecision(2) << rend->GetAnimTime();

		str += cut.str();

		ImGui::Text(str.c_str());

		float curTime = rend->GetAnimCurrentTime();

		if (ImGui::SliderFloat(" / TimeBar", &curTime, 0, rend->GetAnimTime()))
		{
			rend->SetAnimTime(curTime);
		}

		ImGui::DragFloat("x Speed", &rend->animSpeed, 0.1);

		ImGui::Checkbox("Loop" , &rend->animLoop);

		if (ImGui::Button("Play", ButtonRect)) {
			if (rend->isAnimPlaying)
				rend->isAnimPlaying = true;
			else
				rend->StartAnimation(rend->GetAnimIndex(), rend->animSpeed, rend->animLoop);
		}

		ImGui::SameLine();

		if (ImGui::Button("Stop", ButtonRect)) {
			rend->isAnimPlaying = false;
		}

		ImGui::SameLine();

		ImGui::NewLine();

		ImGui::End();
	}

}

void ShowUIRendererInfo(std::shared_ptr<UIRenderer> uiRend)
{
	if (!uiRend)
		return;

	string str = "Width : ";
	str += std::to_string(uiRend->GetImageWidth());
	str += " Height : ";
	str += std::to_string(uiRend->GetImageHeight());

	ImGui::Text(str.c_str());

	EditColor(&uiRend->color);

	if (ImGui::Button("Open Image File", ImVec2(140, 20)))
	{
		OPENFILENAME ofn = {};

		const UINT MAXLENGTH = 200;

		TCHAR filePath[MAXLENGTH] = _T("");

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.lpstrFile = filePath;
		ofn.nMaxFile = MAXLENGTH;
		ofn.lpstrFilter = "画像ファイル (*.dds) \0*.dds\0";
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileName(&ofn))
		{
			string path(filePath);

			uiRend->CreateTexture(path);
		}
	}
}

#pragma warning(disable: 4996)  // strcpy に関する警告を無効化

void ShowEntityInfo()
{
	string label = "EntityInfo : " + entityList[currentIndexEntity]->GetName();

	if (ImGui::Begin(label.c_str(), &showEntityWindow))  // ウィンドウ名を指定
	{
		if (ImGui::Button("CreateChild", ImVec2(150, 22))) {
			auto child = GameManager::GetInstance().CreateEntity("GameObject");

			entityList[currentIndexEntity]->AddChild(child);
		}

		if (entityList[currentIndexEntity]->GetComponent<UIRenderer>())
		{
			entityList[currentIndexEntity]->SetWorldPosition(ShowAndSetVecInfo("WorldPosition : ", entityList[currentIndexEntity]->GetWorldPosition(), 5));
			entityList[currentIndexEntity]->SetLocalPosition(ShowAndSetVecInfo("LocalPosition : ", entityList[currentIndexEntity]->GetLocalPosition() , 5));
		}
		else
		{
			entityList[currentIndexEntity]->SetWorldPosition(ShowAndSetVecInfo("WorldPosition : ", entityList[currentIndexEntity]->GetWorldPosition()));
			entityList[currentIndexEntity]->SetLocalPosition(ShowAndSetVecInfo("LocalPosition : ", entityList[currentIndexEntity]->GetLocalPosition()));
		}

		entityList[currentIndexEntity]->SetWorldScale(ShowAndSetVecInfo("WorldScale : ", entityList[currentIndexEntity]->GetWorldScale()));

		entityList[currentIndexEntity]->SetLocalScale(ShowAndSetVecInfo("LocalScale : ", entityList[currentIndexEntity]->GetLocalScale()));

		entityList[currentIndexEntity]->SetWorldRotation(ShowAndSetVecInfo("WorldRotation : ", entityList[currentIndexEntity]->GetWorldRotation()));

		entityList[currentIndexEntity]->SetLocalRotation(ShowAndSetVecInfo("LocalRotation : ", entityList[currentIndexEntity]->GetLocalRotation()));



		if (ImGui::TreeNode("RenderComponent")) {

			if (entityList[currentIndexEntity]->GetComponent<Renderer>()) {

				ShowRendererInfo(entityList[currentIndexEntity]->GetComponent<Renderer>());
			}
			else
			{
				if (ImGui::Button("Add", ButtonRect)) {
					entityList[currentIndexEntity]->AddComponent<Renderer>();
				}
			}


			ImGui::TreePop();
		}

		if (ImGui::TreeNode("ColliderComponent")) {

			if (entityList[currentIndexEntity]->GetComponent<Collider>()) {

				ShowColliderInfo(entityList[currentIndexEntity]->GetComponent<Collider>());
			}
			else
			{
				if (ImGui::Button("Add", ButtonRect)) {
					entityList[currentIndexEntity]->AddComponent<Collider>();
				}
			}


			ImGui::TreePop();
		}

		if (ImGui::TreeNode("RigidbodyComponent")) {

			if (entityList[currentIndexEntity]->GetComponent<Rigidbody>()) {

				ShowRigidbodyInfo(entityList[currentIndexEntity]->GetComponent<Rigidbody>());
			}
			else
			{
				if (ImGui::Button("Add", ButtonRect)) {
					entityList[currentIndexEntity]->AddComponent<Rigidbody>();
				}
			}


			ImGui::TreePop();
		}

		if (ImGui::TreeNode("UIRenderComponent")) {

			if (entityList[currentIndexEntity]->GetComponent<UIRenderer>()) {

				ShowUIRendererInfo(entityList[currentIndexEntity]->GetComponent<UIRenderer>());
			}
			else
			{
				if (ImGui::Button("Add", ButtonRect)) {
					entityList[currentIndexEntity]->AddComponent<UIRenderer>();
				}
			}


			ImGui::TreePop();
		}
		ImGui::End();
	}

	if (showAnimationWindow)
	{
		ShowAnimationInfo(entityList[currentIndexEntity]->GetComponent<Renderer>());
	}
}

void ShowEntityTree(shared_ptr<Entity> ent)
{
	if (ImGui::Selectable(ent->GetName().c_str()))
	{
		showEntityWindow = true;

		currentEntity = ent;

		if (ent->HasChild())
		{
			auto childs = ent->GetChild();

			for (auto child : childs)
			{
				auto childEnt = std::dynamic_pointer_cast<Entity>(child);

				if (childEnt)
					ShowEntityTree(childEnt);
			}
		}
	}
}

void ShowDebugTrees()
{
	if (ImGui::TreeNode("Entites")) {

		if (ImGui::Button("CreateObject", ImVec2(150, 22))) {
			GameManager::GetInstance().CreateEntity("GameObject");
		}

		entityList = GameManager::GetInstance().GetInstance().GetEntityList();

		for (int i = 0; i < entityList.size(); i++)
		{
			if (!entityList[i]->HasParent())
			{
				ShowEntityTree(entityList[i]);
			}
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Camera")) {

		Camera::GetInstance().SetCameraPosition(ShowAndSetVecInfo("Position", Camera::GetInstance().GetPosition()));
		Camera::GetInstance().SetCameraRotation(ShowAndSetVecInfo("Rotation", Camera::GetInstance().GetRotation()));

		ImGui::TreePop();
	}
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

		ImGui::SetWindowFontScale(1.5);

		if (ImGui::TreeNode("Info")) {

			ShowInfo();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Flags")) {

			ImGui::Checkbox("Debug", flagIns.isDebug.get());
			ImGui::Checkbox("Time", flagIns.isTimeStop.get());

			ImGui::TreePop();
		}

		if (*flagIns.isDebug)
		{
			ShowDebugTrees();
		}
		
		ImGui::End();

		if (*flagIns.isDebug)
		{
			if (showEntityWindow)
				ShowEntityInfo();
		}
	}
	//ここまで

	ImGui::Render();
		
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), DXRenderSystem::GetInstance().GetCommandList().Get());
}

ComPtr<ID3D12DescriptorHeap> MyIMGUI::GetHeap()
{
	return forIMGUIHeap;
}
