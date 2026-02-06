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
#include "PENoiseFilter.h"
#include "DXRenderSystem.h"
#include "DirectionalLight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "Entity.h"
#include <map>

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


	bool isSuccess = true;

	isSuccess &= CreateDevice();

	isSuccess &= CreateCommandQueue();

	isSuccess &= CreateFactory();

	isSuccess &= CreateSwapChain();

	isSuccess &= CreateCommandAllocater();

	isSuccess &= CreateCommandList();

	heapManager = std::make_shared<DescriptorHeapManager>(30000);

	isSuccess &= InitLightInfoCB();

	isSuccess &= LinkLightInfoCBVToDescHeap();

	isSuccess &= MapLLICB();

	isSuccess &= InitTimeInfoCB();

	isSuccess &= LinkTimeInfoCBVToDescHeap();

	isSuccess &= MapTICB();

	isSuccess &= InitDirectionalLightBResource();

	isSuccess &= InitDLBV();

	isSuccess &= MapDLB();

	isSuccess &= InitPointLightBResource();

	isSuccess &= InitPLBV();

	isSuccess &= MapPLB();

	isSuccess &= InitSpotLightBResource();

	isSuccess &= InitSLBV();

	isSuccess &= MapSLB();

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

	isSuccess &= CreateShadowMapResource();

	isSuccess &= CreateShadowMapView();

	PrepareForRootSignature();

	isSuccess &= CreateRootSignature();

	heapSM.emplace(100);

	CreateShadowMapTextureSRV();

	CreateCSDividedDist();

	postEffectEntity = SystemManager::GetInstance().CreateEntity<PEEdgeFilter>("postEffect");
	postEffectEntity->AddComponent<PEMosaicFilter>();
	postEffectEntity->AddComponent<PERadicalBlur>();
	postEffectEntity->AddComponent<PEHorizonGaussianBlur>();
	postEffectEntity->AddComponent<PEVerticalGaussianBlur>();
	postEffectEntity->AddComponent<PENoiseFilter>();


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
	auto cur_value = fence_counter[backbuffer_index];

	cmd_queue->Signal(fence.Get(), cur_value);

	backbuffer_index = swap_chain->GetCurrentBackBufferIndex();
	frontbuffer_index = swap_chain->GetCurrentBackBufferIndex();
	frontbuffer_index = frontbuffer_index % 2;

	if (fence->GetCompletedValue() < fence_counter[backbuffer_index])
	{
		fence->SetEventOnCompletion(fence_counter[backbuffer_index], fence_event);

		WaitForSingleObjectEx(fence_event, INFINITE, false);
	}

	fence_counter[backbuffer_index] = cur_value + 1;
}

std::unique_ptr<DXRenderSystem> DXRenderSystem::instance = nullptr;

int in = 0;

int out = 1;

void DXRenderSystem::RenderPostEffect(bool whenOnUi)
{
	in = 0;

	out = 1;

	auto posts = SystemManager::GetInstance().GetComponentList<PostEffect>();

	posts.erase(std::remove_if(posts.begin(), posts.end(),
		[whenOnUi](shared_ptr<PostEffect> x) { return !x->flag || (x->onUI != whenOnUi); }),
		posts.end());

	if (posts.empty())
		return;

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

	ID3D12DescriptorHeap* ppHeaps[] = { heapManager->GetHeap().Get() }; // あなたの管理しているSRV用ヒープ
	cmd_list->SetDescriptorHeaps(1, ppHeaps);
}

void DXRenderSystem::CreateCSDividedDist()
{
	for (int i = 0; i < cascadeShadowCount; i++)
	{
		float val = (float)(i + 1) / (float)cascadeShadowCount;

		float log = pow(CSFarZ / CSNearZ, val) * CSNearZ;

		float liner = CSNearZ + (CSFarZ - CSNearZ) * val;

		float result = log * cascadeShadowLerp + liner * (1 - cascadeShadowLerp);

		CSDividedDist[i] = result;
	}
}

void DXRenderSystem::ResetCommands()
{
	cmd_allocator[backbuffer_index]->Reset();

	cmd_list->Reset(cmd_allocator[backbuffer_index].Get(), nullptr);

	ID3D12DescriptorHeap* ppHeaps[] = { heapManager->GetHeap().Get() }; // あなたの管理しているSRV用ヒープ

	cmd_list->SetDescriptorHeaps(1, ppHeaps);

	cmd_list->SetGraphicsRootSignature(rootSig.Get());

	cmd_list->SetGraphicsRootConstantBufferView(7, timeInfoCBView[backbuffer_index].BufferLocation);

	cmd_list->SetGraphicsRootConstantBufferView(2, lightInfoCBView[backbuffer_index].BufferLocation);

	cmd_list->SetGraphicsRootShaderResourceView(3, DLBuffer[backbuffer_index]->GetGPUVirtualAddress());

	cmd_list->SetGraphicsRootShaderResourceView(4, PLBuffer[backbuffer_index]->GetGPUVirtualAddress());

	cmd_list->SetGraphicsRootShaderResourceView(5, SLBuffer[backbuffer_index]->GetGPUVirtualAddress());
}

void DXRenderSystem::RenderDLShadowMap(std::shared_ptr<DirectionalLight> dl, int ind, XMMATRIX proj, XMMATRIX view)
{
	currentShadowMapIndex = ind;

	Camera::GetInstance().SetCSProjMat(proj);
	Camera::GetInstance().SetCSViewMat(view);

	int startOffset = dl->GetShadowMapIndex();

	cmd_list->OMSetRenderTargets(0, nullptr, false, &shadowMapHandle[startOffset + ind]->cpuHandle);

	cmd_list->ClearDepthStencilView(shadowMapHandle[startOffset + ind]->cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);

	RenderComponent<SkinnedRenderer>();
}

UINT DXRenderSystem::GetCurrentShadowMapIndex()
{
	return currentShadowMapIndex;
}

void DXRenderSystem::ComputeDLShadowMapRender(std::shared_ptr<DirectionalLight> dl)
{
	auto curNearZ = CSNearZ;
	auto curFarZ = CSDividedDist[0];

	for (int i = 0;i < cascadeShadowCount;i++)
	{
		curFarZ = CSDividedDist[i];

		DirectX::XMMATRIX curProj = XMMatrixPerspectiveFovLH(Camera::GetInstance().GetFov(), Camera::GetInstance().GetAspect(), curNearZ, curFarZ);
		isShadowMapRendering = false;
		auto viewProj = DirectX::XMMatrixMultiply(Camera::GetInstance().GetViewMat() , curProj);
		isShadowMapRendering = true;
		auto invViewProj = DirectX::XMMatrixInverse(nullptr , viewProj);

		XMVECTOR frustumCorners[8] = {
		{-1.0f,  1.0f, 0.0f, 1.0f}, { 1.0f,  1.0f, 0.0f, 1.0f},
		{ 1.0f, -1.0f, 0.0f, 1.0f}, {-1.0f, -1.0f, 0.0f, 1.0f},
		{-1.0f,  1.0f, 1.0f, 1.0f}, { 1.0f,  1.0f, 1.0f, 1.0f},
		{ 1.0f, -1.0f, 1.0f, 1.0f}, {-1.0f, -1.0f, 1.0f, 1.0f},
		};

		// 世界座標へ変換
		for (int j = 0; j < 8; ++j) {
			frustumCorners[j] = XMVector3TransformCoord(frustumCorners[j], invViewProj);
		}

		XMVECTOR center = XMVectorZero();
		for (int j = 0; j < 8; ++j) center = XMVectorAdd(center, frustumCorners[j]);
		center = XMVectorScale(center, 1.0f / 8.0f);

		float radius = 0.0f;
		for (int j = 0; j < 8; ++j) {
			float dist = XMVectorGetX(XMVector3Length(XMVectorSubtract(frustumCorners[j], center)));
			radius = std::max(radius, dist);
		}

		radius = std::ceil(radius * 16.0f) / 16.0f;

		float worldUnitsPerTexel = (radius * 2.0f) / (float)shadowMapRes;

		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		if (fabs(XMVectorGetY(dl->GetLightDir())) > 0.99f) up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

		XMMATRIX lightViewRotOnly = XMMatrixLookAtLH(XMVectorZero(), dl->GetLightDir(), up);

		// 中心をライト空間へ
		XMVECTOR centerLightSpace = XMVector3TransformCoord(center, lightViewRotOnly);

		// X, Y座標を「テクセルサイズの倍数」に丸める（Snap）
		float oldX = XMVectorGetX(centerLightSpace);
		float oldY = XMVectorGetY(centerLightSpace);
		float newX = floor(oldX / worldUnitsPerTexel) * worldUnitsPerTexel;
		float newY = floor(oldY / worldUnitsPerTexel) * worldUnitsPerTexel;

		// 丸めた座標との差分を世界座標に戻して、球の中心を補正する
		centerLightSpace = XMVectorSet(newX, newY, XMVectorGetZ(centerLightSpace), 1.0f);
		XMMATRIX invLightViewRot = XMMatrixInverse(nullptr, lightViewRotOnly);
		center = XMVector3TransformCoord(centerLightSpace, invLightViewRot);

		// -----------------------------------------------------------------------
		// 4. 最終的な LightView / LightProj 行列の作成
		// -----------------------------------------------------------------------

		// ライトの位置：補正した中心から、ライト逆方向へ半径分以上バックさせる
		// ※バックフェイスカリング等の影響を受けないよう、十分に引く
		float farAdjust = 1000.0f; // 適当な大きな値（シーンサイズ依存）
		XMVECTOR lightPos = XMVectorSubtract(center, XMVectorScale(dl->GetLightDir(), radius + farAdjust));

		XMMATRIX lightView = XMMatrixLookAtLH(lightPos, center, up);

		// 正射影行列
		// 球を使っているので、幅(w)と高さ(h)はどちらも「半径×2」の正方形になる
		// これにより「長方形を正方形に直す」という処理が自然に完了している
		float boxSize = radius;

		// Z範囲: 手前(radius + farAdjust) から 奥(radius + farAdjust + radius * 2) まで
		// 実際にはもっと余裕を持って取る
		float zRange = radius * 6.0f + farAdjust;

		XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(
			-boxSize, boxSize,      // Left, Right (中心からのオフセット)
			-boxSize, boxSize,      // Bottom, Top
			-zRange, zRange        // Near, Far (十分に取る)
		);

		dl->lightVP[i] = DirectX::XMMatrixMultiply(lightView,lightProj);

		RenderDLShadowMap(dl, i, lightProj, lightView);

		curNearZ = curFarZ;
	} 
}

void DXRenderSystem::ComputeSLShadowMapRender(std::shared_ptr<SpotLight> sl)
{
	currentShadowMapIndex = 0;

	float fov = sl->GetAngular();

	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	if (fabs(XMVectorGetY(sl->GetLightDir())) > 0.99f) up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(sl->GetPosition(), DirectX::XMVectorAdd(sl->GetPosition() , sl->GetLightDir()), up);
	XMMATRIX proj = XMMatrixPerspectiveFovLH(fov * 2, 1, 0.001f, Camera::GetInstance().GetFarZ());

	sl->lightVP = DirectX::XMMatrixMultiply(view, proj);

	int startOffset = sl->GetShadowMapIndex();

	Camera::GetInstance().SetCSProjMat(proj);
	Camera::GetInstance().SetCSViewMat(view);

	cmd_list->OMSetRenderTargets(0, nullptr, false, &shadowMapHandle[startOffset]->cpuHandle);

	cmd_list->ClearDepthStencilView(shadowMapHandle[startOffset]->cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);

	RenderComponent<SkinnedRenderer>();
}

void DXRenderSystem::RenderShadowMap()
{
	Map();

	isShadowMapRendering = true;

	bool tempPerspective = Camera::GetInstance().isPerspective;

	D3D12_RESOURCE_BARRIER barrier;

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	barrier.Transition.pResource =shadowMap.Get();

	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	cmd_list->ResourceBarrier(1, &barrier);

	auto camPosTemp = Camera::GetInstance().GetPosition();

	auto camRotTemp = Camera::GetInstance().GetRotation();

	auto lights = SystemManager::GetInstance().GetComponentList<Light>();

	UINT ind = 0;

	for (int i = 0; i < lights.size(); i++)
	{
		if (!lights[i]->GetIsCastShadow())
			continue;

		auto dl = std::dynamic_pointer_cast<DirectionalLight>(lights[i]);
		auto pl = std::dynamic_pointer_cast<PointLight>(lights[i]);
		auto sl = std::dynamic_pointer_cast<SpotLight>(lights[i]);

		if (dl)
		{
			Camera::GetInstance().isPerspective = false;

			dl->SetShadowMapIndex(ind);

			ind += cascadeShadowCount;

			ComputeDLShadowMapRender(dl);
		}
		else if (pl)
		{
		}
		else if (sl)
		{
			sl->SetShadowMapIndex(ind);

			ind++;

			ComputeSLShadowMapRender(sl);
		}
	}

	Camera::GetInstance().SetCameraPosition(camPosTemp);

	Camera::GetInstance().SetCameraRotation(camRotTemp);

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;

	cmd_list->ResourceBarrier(1, &barrier);

	Camera::GetInstance().isPerspective = tempPerspective;

	isShadowMapRendering = false;
}

D3D12_CPU_DESCRIPTOR_HANDLE DXRenderSystem::GetShadowMapResourceCPUHandle(UINT index)
{
	return shadowMapHandle[index]->cpuHandle;
}

ComPtr<ID3D12Resource> DXRenderSystem::GetShadowMapResource()
{
	return shadowMap;
}

void DXRenderSystem::Map()
{
	UpdateLightCastingShadowNum();

	MapLLICB();

	MapDLB();

	MapPLB();

	MapSLB();

	MapTICB();
}

bool DXRenderSystem::InitLightInfoCB()
{
	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_UPLOAD;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	desc.Alignment = 0;

	desc.Height = 1;

	desc.Width = sizeof(LightInfo);

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;

	for (int i = 0;i < frameBuffer;i++)
	{
		hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(lightInfoCB[i].GetAddressOf()));
	}



	if (FAILED(hr))
	{
		std::cerr << "CBVのリソース作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool DXRenderSystem::LinkLightInfoCBVToDescHeap()
{
	lightInfoCBHandle = std::move(DXRenderSystem::GetInstance().GetHeapManager()->GetSerialHandle(frameBuffer));

	for (int i = 0;i < frameBuffer;i++)
	{
		lightInfoCBView[i].BufferLocation = lightInfoCB[i]->GetGPUVirtualAddress();

		lightInfoCBView[i].SizeInBytes = sizeof(LightInfo);

		DXRenderSystem::GetInstance().GetDevice()->CreateConstantBufferView(&lightInfoCBView[i], lightInfoCBHandle[i]->cpuHandle);

	}

	return true;
}

bool DXRenderSystem::MapLLICB()
{
	lightInfo.ambientLightColor = DXRenderSystem::GetInstance().GetAmbientColor();

	auto dirLNum = SystemManager::GetInstance().GetComponentList<DirectionalLight>().size();

	lightInfo.DirectionalLightNum = DirectX::XMFLOAT4(dirLNum, dirLNum, dirLNum, dirLNum);

	auto pLNum = SystemManager::GetInstance().GetComponentList<PointLight>().size();

	lightInfo.PointLightNum = DirectX::XMFLOAT4(pLNum, pLNum, pLNum, pLNum);

	auto sLNum = SystemManager::GetInstance().GetComponentList<SpotLight>().size();

	lightInfo.SpotLightNum = DirectX::XMFLOAT4(sLNum, sLNum, sLNum, sLNum);

	lightInfo.cameraPos = Camera::GetInstance().GetPosition();

	lightInfo.cascadeShadowCount = DXRenderSystem::GetInstance().GetCascadeShadowCount();
	lightInfo.cascadeShadowDist = DXRenderSystem::GetInstance().GetCascadeShadowDist();

	DirectX::XMStoreFloat3(&lightInfo.fogColor, DXRenderSystem::GetInstance().GetFogColor());

	lightInfo.fogRange = DXRenderSystem::GetInstance().GetFogRange();

	lightInfo.fogFlag = DXRenderSystem::GetInstance().fogFlag;

	void* ptr = nullptr;

	hr = lightInfoCB[frontbuffer_index]->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "座標行列が収納されたCBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, &lightInfo, sizeof(LightInfo));

	return true;
}

bool DXRenderSystem::InitDirectionalLightBResource()
{
	DirLights.resize(100);

	UINT bufferSize = sizeof(DirLight) * DirLights.size();

	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_UPLOAD;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	desc.Alignment = 0;

	desc.Height = 1;

	desc.Width = bufferSize;

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;


	for (int i = 0;i < frameBuffer;i++)
	{
		hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(DLBuffer[i].GetAddressOf())
		);

	}

	if (FAILED(hr))
	{
		std::cerr << "ダイレクショナルライトのリソースの作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool DXRenderSystem::MapDLB()
{
	auto list = SystemManager::GetInstance().GetComponentList<DirectionalLight>();;

	for (int i = 0; i < list.size(); i++)
	{
		DirLight l;

		DirectX::XMStoreFloat3(&l.color, list[i]->GetColor());
		DirectX::XMStoreFloat3(&l.direct, list[i]->GetLightDir());
		l.shadow = list[i]->GetIsCastShadow();
		l.power = list[i]->GetPower();
		l.shadowMapIndex = list[i]->GetShadowMapIndex();
		for (int j = 0;j < DXRenderSystem::GetInstance().GetCascadeShadowCount();j++)
			l.matVP[j] = list[i]->lightVP[j];
		DirLights[i] = l;
	}

	void* ptr = nullptr;

	hr = DLBuffer[frontbuffer_index]->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "ダイレクショナルライトのDLBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, DirLights.data(), sizeof(DirLight) * DirLights.size());

	return true;
}

bool DXRenderSystem::InitDLBV()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = DirLights.size();
	srvDesc.Buffer.StructureByteStride = sizeof(DirLight);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	DLBHandle = std::move(DXRenderSystem::GetInstance().GetHeapManager()->GetSerialHandle(frameBuffer));

	for (int i = 0;i < frameBuffer;i++)
	{
		DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(DLBuffer[i].Get(), &srvDesc, DLBHandle[i]->cpuHandle);
	}

	return true;
}


bool DXRenderSystem::InitPointLightBResource()
{
	PLights.resize(100);

	UINT bufferSize = sizeof(PLight) * PLights.size();

	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_UPLOAD;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	desc.Alignment = 0;

	desc.Height = 1;

	desc.Width = bufferSize;

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;

	for (int i = 0;i < frameBuffer;i++)
	{
		hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(PLBuffer[i].GetAddressOf())
		);
	}

	if (FAILED(hr))
	{
		std::cerr << "ボーンバッファのリソースの作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool DXRenderSystem::MapPLB()
{
	auto list = SystemManager::GetInstance().GetComponentList<PointLight>();

	for (int i = 0; i < list.size(); i++)
	{
		PLight l;
		DirectX::XMStoreFloat3(&l.color, list[i]->GetColor());
		DirectX::XMStoreFloat3(&l.pos, list[i]->GetPosition());
		l.power = list[i]->GetPower();
		l.radius = list[i]->GetRadius();
		l.shadow = list[i]->GetIsCastShadow();
		l.shadowMapIndex = list[i]->GetShadowMapIndex();

		PLights[i] = l;
	}

	void* ptr = nullptr;

	hr = PLBuffer[frontbuffer_index]->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "ポイントライトのPLBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, PLights.data(), sizeof(PLight) * PLights.size());

	return true;
}

bool DXRenderSystem::InitPLBV()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = PLights.size();
	srvDesc.Buffer.StructureByteStride = sizeof(PLight);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	PLBHandle = std::move(DXRenderSystem::GetInstance().GetHeapManager()->GetSerialHandle(frameBuffer));

	for (int i = 0;i < frameBuffer;i++)
	{
		DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(PLBuffer[i].Get(), &srvDesc, PLBHandle[i]->cpuHandle);
	}

	return true;
}

bool DXRenderSystem::InitSpotLightBResource()
{
	SLights.resize(100);

	UINT bufferSize = sizeof(SLight) * SLights.size();

	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_UPLOAD;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	desc.Alignment = 0;

	desc.Height = 1;

	desc.Width = bufferSize;

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;



	for (int i = 0;i < frameBuffer;i++)
	{
		hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(SLBuffer[i].GetAddressOf())
		);
	}

	if (FAILED(hr))
	{
		std::cerr << "ボーンバッファのリソースの作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool DXRenderSystem::MapSLB()
{
	auto list = SystemManager::GetInstance().GetComponentList<SpotLight>();

	for (int i = 0; i < list.size(); i++)
	{
		SLight l;
		DirectX::XMStoreFloat3(&l.color, list[i]->GetColor());
		DirectX::XMStoreFloat3(&l.pos, list[i]->GetPosition());
		DirectX::XMStoreFloat3(&l.direct, list[i]->GetLightDir());
		l.length = list[i]->GetLength();
		l.angular = list[i]->GetAngular();
		l.power = list[i]->GetPower();
		l.shadow = list[i]->GetIsCastShadow();
		l.shadowMapIndex = list[i]->GetShadowMapIndex();
		l.matVP = list[i]->lightVP;
		SLights[i] = l;
	}

	void* ptr = nullptr;

	hr = SLBuffer[frontbuffer_index]->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "ポイントライトのPLBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, SLights.data(), sizeof(PLight) * SLights.size());
	return true;
}

bool DXRenderSystem::InitSLBV()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = SLights.size();
	srvDesc.Buffer.StructureByteStride = sizeof(SLight);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	SLBHandle = std::move(DXRenderSystem::GetInstance().GetHeapManager()->GetSerialHandle(frameBuffer));

	for (int i = 0;i < frameBuffer;i++)
	{
		DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(SLBuffer[i].Get(), &srvDesc, SLBHandle[i]->cpuHandle);
	}

	return true;
}


bool DXRenderSystem::InitTimeInfoCB()
{
	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_UPLOAD;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {  };

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;

	desc.Alignment = 0;

	desc.Height = 1;

	desc.Width = sizeof(TimeInfo);

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;

	for (int i = 0;i < frameBuffer;i++)
	{
		hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(timeInfoCB[i].GetAddressOf())

		);
	}

	if (FAILED(hr))
	{
		std::cerr << "時間の情報を持ったTIBVのリソース作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool DXRenderSystem::LinkTimeInfoCBVToDescHeap()
{
	timeInfoCBHandle = std::move(DXRenderSystem::GetInstance().GetHeapManager()->GetSerialHandle(2));

	for (int i = 0;i < frameBuffer;i++)
	{
		timeInfoCBView[i].BufferLocation = timeInfoCB[i]->GetGPUVirtualAddress();

		timeInfoCBView[i].SizeInBytes = sizeof(TimeInfo);

		DXRenderSystem::GetInstance().GetDevice()->CreateConstantBufferView(&timeInfoCBView[i], timeInfoCBHandle[i]->cpuHandle);
	}

	return true;
}

bool DXRenderSystem::MapTICB()
{
	auto system = SystemManager::GetInstance();

	timeInfo.GameTime = system.GetGameTime();
	timeInfo.GameDeltaTime = system.GetGameDeltaTime();
	timeInfo.RealTime = system.GetRealTime();
	timeInfo.RealDeltaTime = system.GetRealDeltaTime();

	void* ptr = nullptr;

	hr = timeInfoCB[frontbuffer_index]->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "時間の情報が収納されたCBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, &timeInfo, sizeof(TimeInfo));

	return true;
}

void DXRenderSystem::PrepareForRootSignature()
{
	param.resize(15);

	param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

	param[0].Descriptor.ShaderRegister = 0;

	param[0].Descriptor.RegisterSpace = 0;

	rangeSRV[0].NumDescriptors = 1;

	rangeSRV[0].BaseShaderRegister = 0;

	rangeSRV[0].RegisterSpace = 0;

	rangeSRV[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rangeSRV[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	rangeSRV[1].NumDescriptors = 1;

	rangeSRV[1].BaseShaderRegister = 1;

	rangeSRV[1].RegisterSpace = 0;

	rangeSRV[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rangeSRV[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

	param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[1].DescriptorTable.NumDescriptorRanges = 2;

	param[1].DescriptorTable.pDescriptorRanges = rangeSRV;

	param[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

	param[2].Descriptor.ShaderRegister = 1;

	param[2].Descriptor.RegisterSpace = 0;

	param[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;

	param[3].Descriptor.ShaderRegister = 10;

	param[3].Descriptor.RegisterSpace = 0;

	param[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;

	param[4].Descriptor.ShaderRegister = 11;

	param[4].Descriptor.RegisterSpace = 0;

	param[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;

	param[5].Descriptor.ShaderRegister = 12;

	param[5].Descriptor.RegisterSpace = 0;

	param[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;

	param[6].Descriptor.ShaderRegister = 30;

	param[6].Descriptor.RegisterSpace = 0;

	param[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

	param[7].Descriptor.ShaderRegister = 2;

	param[7].Descriptor.RegisterSpace = 0;

	shadowMapSRV[0].NumDescriptors = 1;

	shadowMapSRV[0].BaseShaderRegister = 100;

	shadowMapSRV[0].RegisterSpace = 0;

	shadowMapSRV[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	shadowMapSRV[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	param[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

	param[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[8].DescriptorTable.NumDescriptorRanges = 1;

	param[8].DescriptorTable.pDescriptorRanges = shadowMapSRV;

	param[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

	param[9].Descriptor.ShaderRegister = 3;

	param[9].Descriptor.RegisterSpace = 0;

	param[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	param[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;

	param[10].Descriptor.ShaderRegister = 20;

	param[10].Descriptor.RegisterSpace = 0;

	param[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

	param[11].Descriptor.ShaderRegister = 21;

	param[11].Descriptor.RegisterSpace = 0;

	param[12].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

	param[12].Descriptor.ShaderRegister = 33;

	param[12].Descriptor.RegisterSpace = 0;

	param[13].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

	param[13].Descriptor.ShaderRegister = 34;

	param[13].Descriptor.RegisterSpace = 0;

	param[14].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	param[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

	param[14].Descriptor.ShaderRegister = 35;

	param[14].Descriptor.RegisterSpace = 0;

	D3D12_STATIC_SAMPLER_DESC desSam;

	desSam = {};

	desSam.Filter = D3D12_FILTER_ANISOTROPIC;

	desSam.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	desSam.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	desSam.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	desSam.MipLODBias = D3D12_DEFAULT_MIP_LOD_BIAS;
	desSam.MaxAnisotropy = 4;
	desSam.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	desSam.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	desSam.MaxLOD = D3D12_FLOAT32_MAX;
	desSam.MinLOD = -D3D12_FLOAT32_MAX;
	desSam.ShaderRegister = 0;
	desSam.RegisterSpace = 0;
	desSam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descSamplers.push_back(desSam);

	desSam = {};

	desSam.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;

	desSam.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	desSam.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	desSam.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	desSam.MipLODBias = D3D12_DEFAULT_MIP_LOD_BIAS;
	desSam.MaxAnisotropy = 4;
	desSam.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	desSam.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	desSam.MaxLOD = D3D12_FLOAT32_MAX;
	desSam.MinLOD = -D3D12_FLOAT32_MAX;
	desSam.ShaderRegister = 1;
	desSam.RegisterSpace = 0;
	desSam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descSamplers.push_back(desSam);
}

bool DXRenderSystem::CreateRootSignature()
{
	D3D12_ROOT_SIGNATURE_DESC desc = {};

	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	desc.NumParameters = param.size();

	desc.NumStaticSamplers = descSamplers.size();

	desc.pParameters = param.data();

	desc.pStaticSamplers = descSamplers.data();

	ComPtr<ID3DBlob> blob;
	ComPtr<ID3DBlob> error_blob;

	hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, blob.GetAddressOf(), error_blob.GetAddressOf());

	if (FAILED(hr))
	{
		std::cerr << "ルートシグネチャシリアライズ化失敗" << std::endl;

		return false;
	}

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(rootSig.GetAddressOf()));

	if (FAILED(hr))
	{
		std::cerr << "ルートシグネチャ作成失敗" << std::endl;

		return false;
	}

	return true;
}

void DXRenderSystem::AddRootParam(D3D12_ROOT_PARAMETER param)
{
	this->param.emplace_back(param);
}

int DXRenderSystem::GetRootParamNum()
{
	return param.size();
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

	if (GetShadowMapNum() > 0)
		RenderShadowMap();

	Map();

	cmd_list->ResourceBarrier(1, &barrier);

	cmd_list->OMSetRenderTargets(1, &msaaRTVHandle->cpuHandle, false, &msaaDSVHandle->cpuHandle);

	cmd_list->ClearRenderTargetView(msaaRTVHandle->cpuHandle, colorBG, 0, nullptr);

	cmd_list->ClearDepthStencilView(msaaDSVHandle->cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	RenderComponent<SkyBox>();

	RenderComponent<SkinnedRenderer>();

	if (Flags::GetInstance().isDebug)
	{
		RenderComponent<DebugCollider>();
		RenderComponent<DebugOrigin>();
		RenderComponent<DebugRayCast>();
		RenderComponent<DebugDirectionalLight>();
		RenderComponent<DebugPointLight>();
		RenderComponent<DebugSpotLight>();
		RenderComponent<DebugGrids>();
	}

	RenderPostEffect(false);

	RenderComponent<UIRenderer>();

	RenderPostEffect(true);

	if (Flags::GetInstance().isDebug)
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

	for (int i = 0;i < deletingEnts.size();i++)
	{
		deletingEnts[i].first--;
	}

	auto it = std::remove_if(deletingEnts.begin(), deletingEnts.end(),
		[this](const std::pair<int, shared_ptr<Entity>>& e) {
			
			return e.first == 0;
		});

	deletingEnts.erase(it, deletingEnts.end());

	for (int i = 0;i < deletingResources.size();i++)
	{
		deletingResources[i].first--;
	}

	auto it2 = std::remove_if(deletingResources.begin(), deletingResources.end(),
		[this](const std::pair<int, ComPtr<ID3D12Resource>>& e) {

			return e.first == 0;
		});

	deletingResources.erase(it2, deletingResources.end());
}

std::shared_ptr<DescriptorHeapManager> DXRenderSystem::GetHeapManager()
{
	return heapManager;
}

void DXRenderSystem::ExecuteCommands()
{
	cmd_list->Close();

	ID3D12CommandList* lists_array[] = { cmd_list.Get() };

	cmd_queue->ExecuteCommandLists(1, lists_array);

	swap_chain->Present(0, 0);

	WaitForRender();

	ResetCommands();
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

ComPtr<ID3D12RootSignature> DXRenderSystem::GetRootSignature()
{
	return rootSig;
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

bool DXRenderSystem::CreateShadowMapResource()
{
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	clearValue.Color[0] = 0.0f;
	clearValue.Color[1] = 0.0f;
	clearValue.Color[2] = 0.0f;
	clearValue.Color[3] = 1.0f;

	D3D12_HEAP_PROPERTIES prop = {};

	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;

	prop.Type = D3D12_HEAP_TYPE_DEFAULT;

	prop.CreationNodeMask = 1;

	prop.VisibleNodeMask = 1;

	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = color_buffer[0]->GetDesc();

	desc.Height = shadowMapRes;

	desc.Width = shadowMapRes;

	desc.DepthOrArraySize = std::max(GetShadowMapNum() , 1U);

	desc.Format = DXGI_FORMAT_R32_TYPELESS;

	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	hr = device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
		&clearDepth,
		IID_PPV_ARGS(shadowMap.GetAddressOf()));

	if (FAILED(hr))
	{
		std::cerr << "シャドウマップ作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool DXRenderSystem::CreateShadowMapView()
{
	shadowMapHandle.clear();

	shadowMapHandle = heapDS.value().GetSerialHandle(std::max(GetShadowMapNum(), 1U));

	for (int i = 0; i < shadowMapHandle.size(); i++)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};

		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
		desc.Flags = D3D12_DSV_FLAG_NONE;
		desc.Texture2DMSArray.FirstArraySlice = i;
		desc.Texture2DMSArray.ArraySize = 1;

		device->CreateDepthStencilView(shadowMap.Get(), &desc, shadowMapHandle[i]->cpuHandle);

	}

	return true;
}

void DXRenderSystem::CreateShadowMapTextureSRV()
{
	auto han = heapManager->GetHandle();

	auto texture_desc = shadowMap->GetDesc();

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	desc.Format = DXGI_FORMAT_R32_FLOAT;
	desc.Texture2DArray.MipLevels = 1;
	desc.Texture2DArray.MostDetailedMip = 0;
	desc.Texture2DArray.ResourceMinLODClamp = 0;
	desc.Texture2DArray.PlaneSlice = 0;
	desc.Texture2DArray.FirstArraySlice = 0;
	desc.Texture2DArray.ArraySize = std::max(DXRenderSystem::GetInstance().GetShadowMapNum(), 1U);

	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(shadowMap.Get(), &desc, han->cpuHandle);

	shadowMapTexHandle = std::move(han);

	shadowMapTexSliceHandles.clear();

	auto hans = heapManager->GetSerialHandle(std::max(GetShadowMapNum() , 1U));

	desc.Texture2DArray.ArraySize = 1;

	desc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0,0,0,0);

	for (int i = 0;i < hans.size();i++)
	{
		desc.Texture2DArray.FirstArraySlice = i;

		DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(shadowMap.Get(), &desc, hans[i]->cpuHandle);

		shadowMapTexSliceHandles.emplace_back(std::move(hans[i]));
	}
}


D3D12_CPU_DESCRIPTOR_HANDLE DXRenderSystem::GetShadowMapTextureSRVCPUHandle()
{
	return shadowMapTexHandle->cpuHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DXRenderSystem::GetShadowMapTextureSRVGPUHandle()
{
	return shadowMapTexHandle->gpuHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE DXRenderSystem::GetShadowMapTextureSliceSRVCPUHandle(UINT ind)
{
	return shadowMapTexSliceHandles[ind]->cpuHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DXRenderSystem::GetShadowMapTextureSliceSRVGPUHandle(UINT ind)
{
	return shadowMapTexSliceHandles[ind]->gpuHandle;
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

void DXRenderSystem::SetDeletingEnt(shared_ptr<Entity> ent)
{
	deletingEnts.emplace_back(std::make_pair(2 , ent));
}

void DXRenderSystem::SetDeletingResource(ComPtr<ID3D12Resource> rsc)
{
	deletingResources.emplace_back(std::make_pair(2 , rsc));
}

bool DXRenderSystem::GetIsShadowMapRendering()
{
	return isShadowMapRendering;
}

void DXRenderSystem::UpdateLightCastingShadow()
{
	bool isValueChanged  = UpdateLightCastingShadowNum();

	if (isValueChanged)
	{
		CreateShadowMapResource();

		CreateShadowMapView();

		CreateShadowMapTextureSRV();
	}
}

UINT DXRenderSystem::GetShadowMapNum()
{
	return DlightCastingShadowNum * cascadeShadowCount + PlightCastingShadowNum * 4 + SlightCastingShadowNum;
}

void DXRenderSystem::SetCascadeShadowCount(UINT num)
{
	cascadeShadowCount = std::clamp(num , 0U ,4U);

	CreateCSDividedDist();
}

void DXRenderSystem::SetCascadeShadowFarZ(float val)
{
	if (CSNearZ >= val)
		CSFarZ = CSNearZ + Utility::ep;
	else
		CSFarZ = val;

	CreateCSDividedDist();
}

void DXRenderSystem::SetCascadeShadowNearZ(float val)
{
	if (CSFarZ <= val)
		CSNearZ = CSFarZ - Utility::ep;
	else
		CSNearZ = val;

	CreateCSDividedDist();
}

void DXRenderSystem::SetCascadeShadowLerp(float val)
{
	val = std::clamp(val , 0.0f , 1.0f);

	cascadeShadowLerp = val;

	CreateCSDividedDist();
}

UINT DXRenderSystem::GetCascadeShadowCount()
{
	return cascadeShadowCount;
}

float DXRenderSystem::GetCascadeShadowFarZ()
{
	return CSFarZ;
}

float DXRenderSystem::GetCascadeShadowNearZ()
{
	return CSNearZ;
}

float DXRenderSystem::GetCascadeShadowLerp()
{
	return cascadeShadowLerp;
}

DirectX::XMFLOAT3 DXRenderSystem::GetCascadeShadowDist()
{
	return DirectX::XMFLOAT3(CSDividedDist[0], CSDividedDist[1], CSDividedDist[2]);
}

void DXRenderSystem::SetFogColor(DirectX::XMVECTOR color)
{
	fogColor = DirectX::XMVectorSaturate(color);
}

void DXRenderSystem::SetFogRange(float range)
{
	fogRange = std::clamp(range , Camera::GetInstance().GetNearZ() , Camera::GetInstance().GetFarZ());
}

DirectX::XMVECTOR DXRenderSystem::GetFogColor()
{
	return fogColor;
}

float DXRenderSystem::GetFogRange()
{
	return fogRange;
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

	shadowViewport.TopLeftX = 0;
	shadowViewport.TopLeftY = 0;

	shadowViewport.MinDepth = 0;
	shadowViewport.MaxDepth = 1;

	shadowViewport.Height = shadowMapRes;

	shadowViewport.Width = shadowMapRes;

	shadowScissor.left = 0;
	shadowScissor.top = 0;
	shadowScissor.bottom = shadowMapRes;
	shadowScissor.right = shadowMapRes;

	return true;
}

bool DXRenderSystem::UpdateLightCastingShadowNum()
{
	auto lights = SystemManager::GetInstance().GetComponentList<Light>();

	UINT num = 0;

	UINT tempDlightNum = 0;
	UINT tempSlightNum = 0;
	UINT tempPlightNum = 0;

	for (int i = 0; i < lights.size(); i++)
	{
		auto l = std::dynamic_pointer_cast<Light>(lights[i]);

		if (!l->GetIsCastShadow())
			continue;

		auto dl = std::dynamic_pointer_cast<DirectionalLight>(lights[i]);
		auto pl = std::dynamic_pointer_cast<PointLight>(lights[i]);
		auto sl = std::dynamic_pointer_cast<SpotLight>(lights[i]);

		if (dl)
			tempDlightNum++;
		else if (pl)
			tempPlightNum++;
		else if (sl)
			tempSlightNum++;
	}

	bool isChanged = false;

	if (tempDlightNum != DlightCastingShadowNum)
	{
		DlightCastingShadowNum = tempDlightNum;
		isChanged = true;
	}
	else if (tempSlightNum != SlightCastingShadowNum)
	{
		SlightCastingShadowNum = tempSlightNum;
		isChanged = true;

		std::cout << SlightCastingShadowNum << std::endl;
	}
	else if (tempPlightNum != PlightCastingShadowNum)
	{
		PlightCastingShadowNum = tempPlightNum;
		isChanged = true;
	}

	return isChanged;
}

DirectX::XMFLOAT3 DXRenderSystem::GetAmbientColor()
{
	return ambientColor;
}

void DXRenderSystem::SetAmbientColor(DirectX::XMFLOAT3 col)
{
	auto xmvec = DirectX::XMVectorClamp(DirectX::XMLoadFloat3(&col), DirectX::XMVectorZero(), DirectX::XMVectorSplatOne());

	DirectX::XMStoreFloat3(&ambientColor, xmvec);
}