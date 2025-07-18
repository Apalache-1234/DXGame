#ifndef POST_EFFECT
#define POST_EFFECT

#include "Renderer.h"
#include "Entity.h"
class PostEffect : public Renderer
{
public:
	PostEffect(shared_ptr<Entity> ent);

	~PostEffect();

	std::type_index GetBaseType() override;

	void InitRenderer() override;

	void AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList) override;

	virtual void InitPostEffect();

	template<typename T>
	void CreateConstantBuffer()
	{
		InitCB<T>();
		LinkPostEffectCBVToDescHeap<T>();

		D3D12_ROOT_PARAMETER param = {};

		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

		param.Descriptor.ShaderRegister = 3;

		param.Descriptor.RegisterSpace = 0;

		paramNum = GetRootParamNum();

		AddRootParam(param);
	}

	template<typename T>
	bool InitCB()
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

		desc.Width = sizeof(T);

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
			IID_PPV_ARGS(postEffectCB.GetAddressOf())

		);

		if (FAILED(hr))
		{
			std::cerr << "ポストエフェクトのCBのリソース作成失敗" << std::endl;

			return false;
		}

		return true;
	}

	template<typename T>
	bool LinkPostEffectCBVToDescHeap()
	{
		postEffectCBHandle = std::move(heapManager.GetHandle());

		postEffectCBView.BufferLocation = postEffectCB->GetGPUVirtualAddress();

		postEffectCBView.SizeInBytes = sizeof(T);

		DXRenderSystem::GetInstance().GetDevice()->CreateConstantBufferView(&postEffectCBView, postEffectCBHandle->cpuHandle);

		return true;
	}

	virtual bool MapPECB();

	bool flag = false;
protected:
	ComPtr<ID3D12Resource> postEffectCB;
	std::unique_ptr<DescriptorHeapManager::PtrHandles> postEffectCBHandle;
	D3D12_CONSTANT_BUFFER_VIEW_DESC postEffectCBView;

	int paramNum = 0;
};


#endif

