#include "MyIMGUI.h"
#include "SkinnedRenderer.h"
#include "UIRenderer.h"
#include <tchar.h> 
#include "Camera.h"
#include "DebugOrigin.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "SceneManager.h"
#include "SkyBox.h"
#include "PhysicsSystem.h"
#include "PEEdgeFilter.h"
#include "PEMosaicFilter.h"
#include "PERadicalBlur.h"
#include "PEVerticalGaussianBlur.h"
#include "PEHorizonGaussianBlur.h"
#include "SceneManager.h"

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
	str += to_string(SystemManager::GetInstance().fps);

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

	str = "GameTime : " + std::to_string(SystemManager::GetInstance().GetGameTime());

	ImGui::Text(str.c_str());

	str = "GameDeltaTime : " + std::to_string(SystemManager::GetInstance().GetGameDeltaTime());

	ImGui::Text(str.c_str());

	str = "RealTime : " + std::to_string(SystemManager::GetInstance().GetRealTime());

	ImGui::Text(str.c_str());

	str = "RealDeltaTime : " + std::to_string(SystemManager::GetInstance().GetRealDeltaTime());

	ImGui::Text(str.c_str());
}

bool showEntityWindow = false;
bool showAnimationWindow = false;
bool showMaterialWindow = false;
bool showShadowMapWindow = false;

shared_ptr<Entity> currentEntity = 0;
shared_ptr<Renderer> currentRend = 0;

ImVec2 imageSize = ImVec2(64, 64);
ImVec2 imageSMSize = ImVec2(256, 256);

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

DirectX::XMFLOAT2 ShowAndSetVecInfo(string label, DirectX::XMFLOAT2 pos, float speed = 0.05)
{
	string str = label;

	float x = pos.x;
	float y = pos.y;

	ImGui::Text(str.c_str());

	ImGui::PushID(label.c_str());

	ImGui::SetNextItemWidth(60);
	ImGui::DragFloat("X ", &x, speed);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	ImGui::DragFloat("Y ", &y, speed);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	ImGui::NewLine();

	ImGui::PopID();

	return DirectX::XMFLOAT2(x,y);
}

ImVec2 ButtonRect = ImVec2(50, 20);

void EditColor(DirectX::XMVECTOR* color , string name)
{
	float col[4];

	col[0] = DirectX::XMVectorGetX(*color);
	col[1] = DirectX::XMVectorGetY(*color);
	col[2] = DirectX::XMVectorGetZ(*color);
	col[3] = DirectX::XMVectorGetW(*color);

	ImGui::ColorEdit4(name.c_str(), col);

	*color = DirectX::XMVectorSet(col[0], col[1], col[2], col[3]);
}

void EditColor(DirectX::XMFLOAT3* color, string name)
{
	float col[3];

	col[0] = color->x;
	col[1] = color->y;
	col[2] = color->z;

	ImGui::ColorEdit3(name.c_str(), col);

	*color = DirectX::XMFLOAT3(col[0], col[1], col[2]);
}

void ShowComponentInfo(std::shared_ptr<SkinnedRenderer> rend)
{
	EditColor(&rend->color , " : Color");

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
	    "BOX",
	    "PLANE",
		"SPHERE"
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

void ShowComponentInfo(std::shared_ptr<Rigidbody> rigid)
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

void ShowComponentInfo(std::shared_ptr<Collider> col)
{
	auto pos = col->position;

	ImGui::PushID("p");

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

	ImGui::PushID("s");

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

	auto rot = col->rot;

	ImGui::PushID("r");

	ImGui::Text("Scale : ");
	ImGui::SetNextItemWidth(60);
	x = DirectX::XMVectorGetX(rot);
	ImGui::DragFloat("X ", &x, 0.1);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	y = DirectX::XMVectorGetY(rot);
	ImGui::DragFloat("Y ", &y, 0.1);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	z = DirectX::XMVectorGetZ(rot);
	ImGui::DragFloat("Z ", &z, 0.1);
	ImGui::SameLine();
	ImGui::NewLine();

	ImGui::PopID();

	col->rot = (DirectX::XMVectorSet(x, y, z, 0));

	const char* colNames[] = {
	"CUBE",
	"SPHERE"
	};

	auto currentColType = col->GetColliderType();

	int currentIndex = static_cast<int>(currentColType);
	if (ImGui::Combo("ColliderType", &currentIndex, colNames, IM_ARRAYSIZE(colNames))) {

		std::cout << currentColType << std::endl;

		currentColType = static_cast<Collider::ColliderType>(currentIndex);

		std::cout << currentColType << std::endl;

		col->SetColliderType(currentColType);
	}
}

void ShowAnimationInfo(std::shared_ptr<SkinnedRenderer> rend)
{
	if (!rend)
		return;

	if (rend->GetAnimationNameList().size() == 0)
	{
		showAnimationWindow = false;
		return;
	}
		
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

ComPtr<ID3D12Resource> LoadImageResource()
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

		return TextureLoader::GetInstance().LoadTexture(path);
	}

	return nullptr;
}

int currentMatIndex = 0;

void ShowMaterialInfo(std::shared_ptr<Renderer> rend)
{
	if (!rend)
		return;

	auto list = rend->GetMeshNameList(); 
	std::vector<const char*> cstrList;
	cstrList.reserve(list.size());

	for (const auto& s : list)
		cstrList.push_back(s.c_str());

	if (ImGui::Begin("MaterialWindow", &showMaterialWindow))
	{
		ImGui::Combo("SubMesh", &currentMatIndex, cstrList.data(), cstrList.size());

		auto cpuH = rend->GetTextureGPUHandle(currentMatIndex, Renderer::DIFFUSE);

		if (ImGui::ImageButton("Diffuse", (ImTextureID)cpuH.ptr, imageSize))
		{
			auto resource = LoadImageResource();

			if (resource)
			{
				rend->ApplyTextureOnMesh(currentMatIndex, Renderer::DIFFUSE , resource);
			}
		}

		ImGui::Text("Diffuse");

		cpuH = rend->GetTextureGPUHandle(currentMatIndex, Renderer::NORMAL);

		if (ImGui::ImageButton("Normal", (ImTextureID)cpuH.ptr, imageSize))
		{
			auto resource = LoadImageResource();

			if (resource)
			{
				rend->ApplyTextureOnMesh(currentMatIndex, Renderer::NORMAL, resource);
			}
		}

		ImGui::Text("Normal");

		auto mat = rend->GetMaterial(currentMatIndex);

		EditColor(&mat.DIFFUSE , " : DiffuseColor");
		EditColor(&mat.SPECULAR , " : SpecularColor");
		EditColor(&mat.AMBIENT , " : AmbientColor");
		EditColor(&mat.EMISSIVE , " : EmissiveColor");
		EditColor(&mat.REFLECTIVE , " : ReflectiveColor");
		EditColor(&mat.TRANSPARENT , " : TransparentColor");

		mat.UVPOS = ShowAndSetVecInfo("UVPosition" , mat.UVPOS , 0.001);
		mat.UVSCALE = ShowAndSetVecInfo("UVScale" , mat.UVSCALE);

		rend->SetMaterial(currentMatIndex , mat);

		ImGui::End();
	}

}

void ShowShadowMapInfo(std::shared_ptr<DirectionalLight> dl)
{
	if (!dl)
		return;

	UINT startOffset = dl->GetShadowMapIndex();
	UINT count = DXRenderSystem::GetInstance().GetCascadeShadowCount();


	if (ImGui::Begin("ShadowMapWindow", &showShadowMapWindow))
	{

		for (int i = 0;i < count;i++)
		{
			auto gpuH = DXRenderSystem::GetInstance().GetShadowMapTextureSliceSRVGPUHandle(startOffset + i);

			auto num = std::to_string(i + 1);

			auto name = "ShadowMap No." + num;

			ImGui::Text(name.c_str());

			ImGui::ImageButton(name.c_str(), (ImTextureID)gpuH.ptr, imageSMSize);
		}

		ImGui::End();
	}

}


void ShowComponentInfo(std::shared_ptr<DirectionalLight> DLight)
{
	auto color = DLight->GetColor();

	EditColor(&color , " : LightColor");

	DLight->SetColor(color);
	
	auto pow = DLight->GetPower();

	ImGui::DragFloat("Power ", &pow, 0.1);

	DLight->SetPower(pow);

	auto isShadow = DLight->GetIsCastShadow();

	ImGui::Checkbox("Shadow ", &isShadow);

	DLight->SetIsCastShadow(isShadow);

	showShadowMapWindow = isShadow;

	DLight->SetRotation(ShowAndSetVecInfo("Rotation", DLight->GetRotation()));
}

void ShowComponentInfo(std::shared_ptr<UIRenderer> uiRend)
{
	if (!uiRend)
		return;

	int width = uiRend->GetImageWidth();
	ImGui::DragInt("Width : ", &width);
	uiRend->SetImageWidth(width);

	int height = uiRend->GetImageHeight();
	ImGui::DragInt("Height : ", &height);
	uiRend->SetImageHeight(height);

	uiRend->colPosition = ShowAndSetVecInfo("ColliderPos" , uiRend->colPosition);
	uiRend->colScale = ShowAndSetVecInfo("ColliderScale" , uiRend->colScale);

	EditColor(&uiRend->color , " : Color");

	auto cpuH = uiRend->GetTextureGPUHandle();

	if (ImGui::ImageButton("UIImage", (ImTextureID)cpuH.ptr, imageSize))
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

void ShowComponentInfo(std::shared_ptr<PointLight> pl)
{
	auto color = pl->GetColor();

	EditColor(&color , " : LightColor");

	pl->SetColor(color);

	auto pow = pl->GetPower();

	ImGui::DragFloat("Power ", &pow, 0.1);

	pl->SetPower(pow);

	auto rad = pl->GetRadius();

	ImGui::DragFloat("Radius ", &rad, 0.1);

	pl->SetRadius(rad);

	auto isShadow = pl->GetIsCastShadow();

	ImGui::Checkbox("Shadow ", &isShadow);

	pl->SetIsCastShadow(isShadow);

	pl->position = ShowAndSetVecInfo("Position", pl->position);
}

void ShowComponentInfo(std::shared_ptr<SpotLight> sl)
{
	auto color = sl->GetColor();

	EditColor(&color , " : LightColor");

	sl->SetColor(color);

	auto pow = sl->GetPower();

	ImGui::DragFloat("Power ", &pow, 0.1);

	sl->SetPower(pow);

	auto ang = DirectX::XMConvertToDegrees(sl->GetAngular());

	ImGui::DragFloat("Angular ", &ang, 0.1);

	sl->SetAngular(DirectX::XMConvertToRadians(ang));

	auto len = sl->GetLength();

	ImGui::DragFloat("Length ", &len, 0.1);

	sl->SetLength(len);

	auto isShadow = sl->GetIsCastShadow();

	ImGui::Checkbox("Shadow ", &isShadow);

	sl->SetIsCastShadow(isShadow);

	sl->SetRotation(ShowAndSetVecInfo("Rotation", sl->GetRotation()));

	sl->position = ShowAndSetVecInfo("Position", sl->position);
}


void ShowComponentInfo(std::shared_ptr<SkyBox> sky)
{
	auto color = sky->color;

	EditColor(&color , " : Color");

	sky->color = color;
}

template <typename T>
void AddComponentInfo(std::string name)
{
	auto comp = currentEntity->GetComponent<T>();

	if (ImGui::TreeNode(name.c_str())) 
	{
		if (comp) 
		{
			auto rend = std::dynamic_pointer_cast<Renderer>(comp);

			if (rend)
			{
				if (ImGui::IsItemClicked())
				{
					showMaterialWindow = true;

					currentRend = rend;
				}
			}

			ShowComponentInfo(comp);

			if (ImGui::Button("Remove", ButtonRect)) 
			{
				currentEntity->RemoveComponent<T>();
			}
		}
		else
		{
			if (ImGui::Button("Add", ButtonRect))
			{
				currentEntity->AddComponent<T>();
			}
		}

		ImGui::TreePop();
	}
}

#pragma warning(disable: 4996)  // strcpy に関する警告を無効化

void ShowEntityInfo()
{
	string label = "EntityInfo : " + currentEntity->GetName();

	if (ImGui::Begin(label.c_str(), &showEntityWindow)) 
	{
		if (ImGui::Button("CreateChild", ImVec2(110, 22))) {
			auto child = SystemManager::GetInstance().CreateEntity("GameObject");

			currentEntity->AddChild(child);
		}

		ImGui::SameLine();

		if (ImGui::Button("Init", ImVec2(110, 22))) {
			currentEntity->Init();
		}

		if (currentEntity->GetComponent<UIRenderer>())
		{
			currentEntity->SetWorldPosition(ShowAndSetVecInfo("WorldPosition : ", currentEntity->GetWorldPosition(), 5));
			currentEntity->SetLocalPosition(ShowAndSetVecInfo("LocalPosition : ", currentEntity->GetLocalPosition() , 5));
		}
		else
		{
			currentEntity->SetWorldPosition(ShowAndSetVecInfo("WorldPosition : ", currentEntity->GetWorldPosition()));
			currentEntity->SetLocalPosition(ShowAndSetVecInfo("LocalPosition : ", currentEntity->GetLocalPosition()));
		}

		currentEntity->SetWorldScale(ShowAndSetVecInfo("WorldScale : ", currentEntity->GetWorldScale()));

		currentEntity->SetLocalScale(ShowAndSetVecInfo("LocalScale : ", currentEntity->GetLocalScale()));

		ShowAndSetVecInfo("FinalScale : ", currentEntity->GetScale());

		auto vec = ShowAndSetVecInfo("Rotation : ", currentEntity->GetRotation());

		if (!DirectX::XMVector3Equal(vec, currentEntity->GetRotation()))
			currentEntity->SetRotation(vec);

		vec = ShowAndSetVecInfo("WorldRotation : ", currentEntity->GetWorldRotation());

		if (!DirectX::XMVector3Equal(vec, currentEntity->GetWorldRotation()))
			currentEntity->SetWorldRotation(vec);

		vec = ShowAndSetVecInfo("LocalRotation : ", currentEntity->GetLocalRotation());

		if (!DirectX::XMVector3Equal(vec, currentEntity->GetLocalRotation()))
			currentEntity->SetLocalRotation(vec);

		AddComponentInfo<SkinnedRenderer>("SkinnedRendererComponent");
		AddComponentInfo<SkyBox>("SkyBox");
		AddComponentInfo<Collider>("ColliderComponent");
		AddComponentInfo<Rigidbody>("RigidbodyComponent");
		AddComponentInfo<UIRenderer>("UIRendererComponent");
		AddComponentInfo<DirectionalLight>("DirectionalLIghtComponent");
		AddComponentInfo<PointLight>("PointLIghtComponent");
		AddComponentInfo<SpotLight>("SpotLIghtComponent");

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button , ImVec4(0.3,0,0,1));
		if (ImGui::Button("Destroy", ImVec2(110, 22))) {
			currentEntity->Destroy();
			currentEntity = nullptr;
			showEntityWindow = false;
		}
		ImGui::PopStyleColor();
	}

	ImGui::End();

	if (showAnimationWindow)
	{
		ShowAnimationInfo(currentEntity->GetComponent<SkinnedRenderer>());
	}

	if (showMaterialWindow)
	{
		ShowMaterialInfo(currentRend);
	}

	if (showShadowMapWindow)
	{
		ShowShadowMapInfo(currentEntity->GetComponent<DirectionalLight>());
	}
}

bool IsSelectedChild(shared_ptr<Entity> ent , bool isSelected = false)
{
	if (ent->HasChild())
	{
		auto childs = ent->GetChild();

		for (auto child : childs)
		{
			auto childEnt = std::dynamic_pointer_cast<Entity>(child);

			if (currentEntity == childEnt)
			{
				return true;
			}

			if (childEnt)
				isSelected = IsSelectedChild(childEnt , isSelected);
		}
	}

	return isSelected;
}

void ShowEntityTree(shared_ptr<Entity> ent)
{
	if (currentEntity == ent)
	{
		bool isPop = false;

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));

		if (ImGui::TreeNode(ent->GetName().c_str()))
		{
			ImGui::PopStyleColor();

			isPop = true;

			if (ImGui::IsItemClicked())
			{
				showEntityWindow = true;

				currentEntity = ent;
			}

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

			ImGui::TreePop();
		}

		if(!isPop)
			ImGui::PopStyleColor();
	}
	else
	{
		if (ImGui::TreeNode(ent->GetName().c_str()))
		{
			if (ImGui::IsItemClicked())
			{
				showEntityWindow = true;

				currentEntity = ent;
			}

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

			ImGui::TreePop();
		}
	}
}

void ShowTrees()
{
	if (ImGui::TreeNode("Scenes")) {
		auto sceneList = SceneManager::GetInstance().GetScenes();

		for (int i = 0; i < sceneList.size(); i++)
		{
			if (ImGui::Selectable(sceneList[i]->GetName().c_str()))
			{
				SceneManager::GetInstance().ChangeGameScene(i);
			}
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Entites")) {

		if (ImGui::Button("CreateObject", ImVec2(150, 22))) {
			SceneManager::GetInstance().GetCurrentGameScene()->CreateEntity<SkinnedRenderer>("GameObject");
		}

		entityList = SceneManager::GetInstance().GetCurrentGameScene()->GetEntities();

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

		auto fov = DirectX::XMConvertToDegrees(Camera::GetInstance().GetFov());

		ImGui::DragFloat("Fov" , &fov);

		Camera::GetInstance().SetFov(DirectX::XMConvertToRadians(fov));

		auto nearZ = Camera::GetInstance().GetNearZ();

		ImGui::DragFloat("NearZ", &nearZ);

		Camera::GetInstance().SetNearZ(nearZ);

		auto farZ = Camera::GetInstance().GetFarZ();

		ImGui::DragFloat("farZ", &farZ);

		Camera::GetInstance().SetFarZ(farZ);

		ImGui::Checkbox("isPerspective", &Camera::GetInstance().isPerspective);

		auto wid = Camera::GetInstance().GetOrthoDiameter();

		ImGui::DragFloat("OrthoDiameter", &wid);

		Camera::GetInstance().SetOrthoDiameter(wid);

		ImGui::TreePop();
	}
}

void DebugMenu()
{
	auto debugOrigin = SystemManager::GetInstance().GetComponentList<DebugOrigin>();

	const char* typeNames[] = {"WORLD","LOCAL"};

	auto currentType = debugOrigin[0]->type;

	int currentIndex = static_cast<int>(currentType);
	if (ImGui::Combo("Type", &currentIndex, typeNames, IM_ARRAYSIZE(typeNames))) {
		currentType = static_cast<DebugOrigin::TYPE>(currentIndex);

		debugOrigin[0]->type = currentType;
	}
}

void ShowPostEffectFlags(shared_ptr<PostEffect> ps)
{
	ImGui::Checkbox("Flag", &ps->flag);
	ImGui::Checkbox("OnUI", &ps->onUI);
}

void ShowEdgeFilterPostEffect()
{
	if (ImGui::TreeNode("EdgeFilter")) {

		auto edgeF = SystemManager::GetInstance().GetComponentList<PEEdgeFilter>();

		ShowPostEffectFlags(edgeF.front());

		auto trans = edgeF.front()->GetTransition();

		ImGui::DragFloat("Transition", &trans, 0.05f, 0, 1);

		edgeF.front()->SetTransition(trans);

		ImGui::TreePop();
	}
}

void ShowMosaicFilterPostEffect()
{
	if (ImGui::TreeNode("MosaicFilter")) {

		auto mosaicF = SystemManager::GetInstance().GetComponentList<PEMosaicFilter>();

		ShowPostEffectFlags(mosaicF.front());

		auto intens = mosaicF.front()->GetIntensify();

		ImGui::DragFloat("Intensify", &intens);

		mosaicF.front()->SetIntensify(intens);

		ImGui::TreePop();
	}
}

void ShowRadicalBlurPostEffect()
{
	if (ImGui::TreeNode("RadicalBlur")) {

		auto rad = SystemManager::GetInstance().GetComponentList<PERadicalBlur>();

		ShowPostEffectFlags(rad.front());

		auto samp = rad.front()->GetSample();

		ImGui::DragInt("Sample", &samp);

		rad.front()->SetSample(samp);

		auto str = rad.front()->GetBlurStrength();

		ImGui::DragFloat("Strength", &str , 0.05f);

		rad.front()->SetBlurStrength(str);

		rad.front()->SetScreenCenter(ShowAndSetVecInfo("CenterPos" , rad.front()->GetScreenCenter()));

		ImGui::TreePop();
	}
}

void ShowGaussianBlurPostEffect()
{
	if (ImGui::TreeNode("GaussianBlur")) {

		auto hg = SystemManager::GetInstance().GetComponentList<PEHorizonGaussianBlur>();

		ShowPostEffectFlags(hg.front());

		auto samp = hg.front()->GetKernelRadius();

		ImGui::DragInt("Sample", &samp);

		hg.front()->SetKernelRadius(samp);

		auto str = hg.front()->GetSigma();

		ImGui::DragFloat("Strength", &str, 0.05f);

		hg.front()->SetSigma(str);

		auto vg = SystemManager::GetInstance().GetComponentList<PEVerticalGaussianBlur>();

		vg.front()->flag = hg.front()->flag;

		vg.front()->SetKernelRadius(samp);

		vg.front()->SetSigma(str);

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

		ShowTrees();

		if (ImGui::TreeNode("AmbientLight")) {

			auto hoge = SystemManager::GetInstance().GetAmbientColor();

			auto ambientCol = DirectX::XMLoadFloat3(&hoge);

			ambientCol = DirectX::XMVectorSetW(ambientCol, 1);

			EditColor(&ambientCol , " : AmbientLightColor");

			DirectX::XMFLOAT3 col;

			DirectX::XMStoreFloat3(&col, ambientCol);

			SystemManager::GetInstance().SetAmbientColor(col);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Physics")) {

			auto& physics = PhysicsSystem::GetInstance();

			auto pow = physics.gravityPow;

			ImGui::PushItemWidth(80);
			ImGui::DragFloat("GravityPower" , &pow);
			ImGui::PopItemWidth();

			physics.gravityPow = pow;

			physics.SetGravityVec(ShowAndSetVecInfo("GravityVec", physics.GetGravityVec()));

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("PostEffect")) {

			ShowEdgeFilterPostEffect();

			ShowMosaicFilterPostEffect();

			ShowRadicalBlurPostEffect();

			ShowGaussianBlurPostEffect();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("ShadowMap")) {

			int count = DXRenderSystem::GetInstance().GetCascadeShadowCount();

			ImGui::SliderInt("CSCount" , &count ,1  , 4);

			DXRenderSystem::GetInstance().SetCascadeShadowCount(count);

			float nearZ = DXRenderSystem::GetInstance().GetCascadeShadowNearZ();

			ImGui::DragFloat("NearZ", &nearZ);

			DXRenderSystem::GetInstance().SetCascadeShadowNearZ(nearZ);

			float farZ = DXRenderSystem::GetInstance().GetCascadeShadowFarZ();

			ImGui::DragFloat("FarZ", &farZ);

			DXRenderSystem::GetInstance().SetCascadeShadowFarZ(farZ);

			float lerp = DXRenderSystem::GetInstance().GetCascadeShadowLerp();

			ImGui::DragFloat("Lerp", &lerp);

			DXRenderSystem::GetInstance().SetCascadeShadowLerp(lerp);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Fog")) {

			ImGui::Checkbox("IsFog", &DXRenderSystem::GetInstance().fogFlag);

			auto col = DXRenderSystem::GetInstance().GetFogColor();

			EditColor(&col, " : FogColor");

			DXRenderSystem::GetInstance().SetFogColor(col);

			auto range = DXRenderSystem::GetInstance().GetFogRange();

			ImGui::DragFloat("FogRange : ", &range, 1, Camera::GetInstance().GetNearZ(), Camera::GetInstance().GetFarZ());

			DXRenderSystem::GetInstance().SetFogRange(range);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Flags")) {

			ImGui::Checkbox("Debug", &flagIns.isDebug);

			if (flagIns.isDebug)
				DebugMenu();

			ImGui::Checkbox("Time", &flagIns.isTimeStop);

			ImGui::TreePop();
		}
		
		ImGui::End();

		if (showEntityWindow)
			ShowEntityInfo();
	}
	//ここまで

	ImGui::Render();
		
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), DXRenderSystem::GetInstance().GetCommandList().Get());
}

ComPtr<ID3D12DescriptorHeap> MyIMGUI::GetHeap()
{
	return forIMGUIHeap;
}
