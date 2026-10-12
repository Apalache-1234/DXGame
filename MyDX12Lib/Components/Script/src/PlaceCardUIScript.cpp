#include "PCH/pch.h"
#include "PlaceCardUIScript.h"
#include "UIRenderer.h"
#include "ReadyUIScript.h"
#include "Entity.h"

PlaceCardUIScript::PlaceCardUIScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

PlaceCardUIScript::~PlaceCardUIScript()
{
}

void PlaceCardUIScript::OnStart()
{
	sRend = attachFrom.lock()->AddComponent<UIRenderer>();

	sRend->SetImageWidth(Window::width);
	sRend->SetImageHeight(Window::height);

	sRend->LoadPixelShader(L"PlaceCardUIPixelShader.cso");

	InitCB();

	LinkCBVToDescHeap();

	MapCB();

	std::function<void(ComPtr<ID3D12GraphicsCommandList>)> f = [=](ComPtr<ID3D12GraphicsCommandList> i) { i->SetGraphicsRootConstantBufferView(14, CBView.BufferLocation);};

	sRend->AddCommandListFunction(f);

	readyEnt = sceneManager->GetCurrentGameScene()->CreateEntity<ReadyUIScript>("PlaceCardUI");

	readyRend = readyEnt->GetComponent<UIRenderer>();

	readyRend->color = DirectX::XMVectorSet(1,1,1,0);
}

void  PlaceCardUIScript::MapCB()
{
	HRESULT hr;

	void* ptr = nullptr;

	hr = CB->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "CBのマッピング失敗" << std::endl;
	}

	memcpy(ptr, &info, sizeof(UIInfo));

	CB->Unmap(0, nullptr);
}

void  PlaceCardUIScript::InitCB()
{
	HRESULT hr;

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

	desc.Width = sizeof(UIInfo);

	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.SampleDesc.Count = 1;

	desc.SampleDesc.Quality = 0;

	desc.DepthOrArraySize = 1;

	desc.MipLevels = 1;

	desc.Format = DXGI_FORMAT_UNKNOWN;

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(CB.GetAddressOf())

	);

	if (FAILED(hr))
	{
		std::cerr << "PlaceCardUIScriptのCBのリソース作成失敗" << std::endl;
	}
}

bool  PlaceCardUIScript::LinkCBVToDescHeap()
{
	CBHandle = std::move(DXRenderSystem::GetInstance().GetHeapManager()->GetHandle());

	CBView.BufferLocation = CB->GetGPUVirtualAddress();

	CBView.SizeInBytes = sizeof(UIInfo);

	DXRenderSystem::GetInstance().GetDevice()->CreateConstantBufferView(&CBView, CBHandle->cpuHandle);

	return true;
}

void PlaceCardUIScript::StartAnim(std::function<void()> whenEndF)
{
	animPat = FIRST;
	time = 0;
	f = whenEndF;
	isFuncExec = false;
}

PlaceCardUIScript::AnimStatus PlaceCardUIScript::GetAnimStatus()
{
	return animPat;
}

void PlaceCardUIScript::OnUpdate()
{
	MapCB();

	if (animPat == END)
	{
		if (!isFuncExec)
		{
			if (f)
				f();
			isFuncExec = true;
		}

		info.time = 0;
		time = 0;
		info.color = DirectX::XMVectorZero();
		return;
	}

	time += systemManager->GetGameDeltaTime() / interval;

	if (time > 1)
	{
		animPat = (AnimStatus)((int)animPat + 1);
		time = 0;
	}

	switch (animPat)
	{
	case FIRST:
		break;
	case SECOND:
		info.time = time * 0.5f;
		info.color = DirectX::XMVectorLerp(DirectX::XMVectorSet(0, 0, 0, 0), DirectX::XMVectorSet(0, 0, 0, 0.8), time);
		break;
	case THIRD:
		info.time = 0.5;
		readyRend->color = DirectX::XMVectorLerp(DirectX::XMVectorSet(1, 1, 1, 0) , DirectX::XMVectorSplatOne() , std::sin(time * Utility::PI));
		readyEnt->SetWorldPosition(DirectX::XMVectorLerp(DirectX::XMVectorSet(100,0,0,0) , DirectX::XMVectorSet(-100,0,0,0) , time));
		info.color = DirectX::XMVectorSet(0, 0, 0, 0.8);
		break;
	case FOURTH:
		info.time = 0.5f + time * 0.5f;
		readyRend->color = DirectX::XMVectorZero();
		info.color = DirectX::XMVectorLerp(DirectX::XMVectorSet(0, 0, 0, 0), DirectX::XMVectorSet(0, 0, 0, 0.8), 1 - std::pow(time, 2));
		break;
	case END:
		if (!isFuncExec)
		{
			if (f)
				f();
			isFuncExec = true;
		}
		break;
	}
}
