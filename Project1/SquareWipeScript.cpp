#include "SquareWipeScript.h"
#include "UIRenderer.h"

SquareWipeScript::SquareWipeScript(std::shared_ptr<Entity> ent) : ScriptComponent::ScriptComponent(ent)
{
}

SquareWipeScript::~SquareWipeScript()
{
}

void SquareWipeScript::OnStart()
{
    rend = attachFrom->AddComponent<UIRenderer>();

	rend->SetImageWidth(Window::width);
	rend->SetImageHeight(Window::height);
	rend->color = DirectX::XMVectorSet(0,0,0,1);

	rend->LoadPixelShader(L"SquareWipePixelShader.cso");

	InitCB();

	LinkCBVToDescHeap();

	MapCB();

	std::function<void(ComPtr<ID3D12GraphicsCommandList>)> f = [=](ComPtr<ID3D12GraphicsCommandList> i) { i->SetGraphicsRootConstantBufferView(12, CBView.BufferLocation);};

	rend->AddCommandListFunction(f);

	tws.emplace_back(tweenSystem->DoTween(0.0f, 0.01f, EasingType::EaseOutSine, 1));
	tws.emplace_back(tweenSystem->DoTween(0.01f, 0.01f, EasingType::EaseOutSine, 0.5f));
	tws.emplace_back(tweenSystem->DoTween(0.01f, 1.0f, EasingType::EaseOutSine, 0.6f));

	tws[1]->Pause();
	tws[2]->Pause();
}

void SquareWipeScript::OnUpdate()
{
	for (auto& tw : tws)
	{
		info.rotate = DirectX::XMMatrixRotationZ(systemManager->GetGameTime() * 5);
		if (!tw->GetIsEnd())
		{
			tw->Play();
			info.scale = tw->GetValue();
			MapCB();
			break;
		}
	}

	if (tws.back()->GetIsEnd())
	{
		isWipeEnd = true;
	}


}

bool SquareWipeScript::GetIsWipeEnd()
{
	return isWipeEnd;
}

void SquareWipeScript::MapCB()
{
	HRESULT hr;

	void* ptr = nullptr;

	hr = CB->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "CBのマッピング失敗" << std::endl;
	}

	memcpy(ptr, &info, sizeof(SquareInfo));

	CB->Unmap(0, nullptr);
}

void SquareWipeScript::InitCB()
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

	desc.Width = sizeof(SquareInfo);

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
		std::cerr << "SquareWipeのCBのリソース作成失敗" << std::endl;
	}
}

bool SquareWipeScript::LinkCBVToDescHeap()
{
	CBHandle = std::move(DXRenderSystem::GetInstance().GetHeapManager()->GetHandle());

	CBView.BufferLocation = CB->GetGPUVirtualAddress();

	CBView.SizeInBytes = sizeof(SquareInfo);

	DXRenderSystem::GetInstance().GetDevice()->CreateConstantBufferView(&CBView, CBHandle->cpuHandle);

	return true;
}
