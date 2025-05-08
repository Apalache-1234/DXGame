#include "Window.h"
#include "DXRenderSystem.h"

#include <iostream>
#include <DirectXMath.h>
#include "entity.h"
#include "Utility.h"
#include "MyIMGUI.h"
#include "ColliderDetection.h"
#include "Collider.h"
#include "Camera.h"
#include "PhysicSystem.h"
#include "Rigidbody.h"

/*
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace std;

unsigned int const height = 1980;

unsigned int const width = 1080;

float colorBG[] =
{ 0.5 , 0.5 , 0.5, 1 };

const int frame_buffer = 2;

ComPtr<ID3D12Device> device;
ComPtr<IDXGIAdapter1> adapter1;
ComPtr<IDXGIFactory1> factory;
ComPtr<IDXGISwapChain3> swap_chain;
ComPtr<ID3D12CommandQueue> cmd_queue;
ComPtr<ID3D12CommandAllocator> cmd_allocator[frame_buffer];
ComPtr<ID3D12Resource> depth_buffer;
ComPtr<ID3D12GraphicsCommandList> cmd_list;
ComPtr<ID3D12DescriptorHeap> heap_rtv;
ComPtr<ID3D12DescriptorHeap> heap_dbv;
ComPtr<ID3D12Resource> rtv;
ComPtr<ID3D12Resource> color_buffer[frame_buffer];
ComPtr<ID3D12Fence> fence;
UINT16 fence_counter = 1;

HANDLE fence_event;

D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle[frame_buffer] = {};

D3D12_DEPTH_STENCIL_DESC ds_desc = {};

D3D12_CPU_DESCRIPTOR_HANDLE dbv_handle = {};

struct TextureView
{
	ComPtr<ID3D12Resource> resource;

	D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;

	D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle;
};

TextureView tex_view;

struct alignas(256) Transform
{
	XMMATRIX world;

	XMMATRIX view;

	XMMATRIX projection;

	XMFLOAT4 color;
};


UINT32 index[] =
{
	0,1,2,1,2,3
};

int backbuffer_index = 0;

auto cam_pos = XMVectorSet(0, 0, 2, 0);

auto target_pos = XMVectorZero();

auto upward = XMVectorSet(0, 1, 0, 0);

void Draw();

DirectX::VertexPositionTexture vertices[] =
{
	DirectX::VertexPositionTexture(XMFLOAT3(-1,1,0) , XMFLOAT2(0 , 0)),
	DirectX::VertexPositionTexture(XMFLOAT3(1,1,0)  , XMFLOAT2(1 , 0)),
	DirectX::VertexPositionTexture(XMFLOAT3(-1,-1,0) , XMFLOAT2(0 , 1)),
	DirectX::VertexPositionTexture(XMFLOAT3(1,-1,0) , XMFLOAT2(1 , 1))
};

class InstanceObject
{
private:
	Transform trans;

public:
	XMFLOAT3 position;
	XMFLOAT4 color;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;
	HRESULT hr;
	ComPtr<ID3D12Resource> constant_buffer[2];
	ComPtr<ID3D12DescriptorHeap> heap_cbv_srv;
	D3D12_CONSTANT_BUFFER_VIEW_DESC cb_view[2];
	ComPtr<ID3D12RootSignature> root_sig;
	ComPtr<ID3D12PipelineState> pipeline;
	ComPtr<ID3D12Resource> vertex_buffer;
	ComPtr<ID3D12Resource> index_buffer;
	D3D12_INDEX_BUFFER_VIEW ib_view;
	D3D12_VERTEX_BUFFER_VIEW vb_view;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissor;


	InstanceObject()
	{
		auto target_pos = XMVectorZero();

		auto upward = XMVectorSet(0, 1, 0, 0);

		trans.view = XMMatrixLookAtRH(cam_pos, target_pos, upward);

		auto aspect = (float)16 / (float)9;

		constexpr auto fov = XMConvertToRadians(40);

		trans.projection = XMMatrixPerspectiveFovRH(fov, aspect, 1, 1000);

		trans.color = XMFLOAT4(1,1,1,1);

		position = XMFLOAT3(0, 0, 0);

		color = XMFLOAT4(255, 255, 255 , 255);

		rotation = XMFLOAT3(0, 0, 0);

		scale = XMFLOAT3(1, 1, 1);
   	}

	void ConvertValues()
	{
		auto position_mat = XMMatrixTranslation(position.x, position.y, position.z);

		auto rotation_mat = XMMatrixRotationX(XMConvertToRadians(rotation.x)) * XMMatrixRotationY(XMConvertToRadians(rotation.y)) * XMMatrixRotationZ(XMConvertToRadians(rotation.z));

		auto scale_mat = XMMatrixScaling(scale.x, scale.y, scale.z);

		trans.world = position_mat * rotation_mat * scale_mat;

		auto color_normalized = XMFLOAT4(color.x / 255 , color.y / 255 , color.z / 255 , color.w / 255);

		trans.color = color_normalized;
	}

		void CopyCB()
		{
			ConvertValues();

			trans.view = XMMatrixLookAtRH(cam_pos, target_pos, upward);

			void* ptr[2] = { nullptr , nullptr };

			for (int i = 0; i < 2; i++)
			{
				ptr[i] = nullptr;

				hr = constant_buffer[i]->Map(0, nullptr, &ptr[i]);

				if (FAILED(hr))
				{
					std::cerr << "失敗z" << std::endl;

					return;
				}

				memcpy(ptr[i], &trans, sizeof(trans));


				constant_buffer[i]->Unmap(0, nullptr);
			}

		}

		void Init()
		{
			{
				D3D12_DESCRIPTOR_HEAP_DESC desc = {};

				desc.Flags =D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

				desc.NodeMask = 0;

				desc.NumDescriptors = 2;

				desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

				hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(heap_cbv_srv.GetAddressOf()));

				if (FAILED(hr))
				{
					std::cerr << "失敗z" << std::endl;

					return;
				}
			}

			for (int i = 0; i < 2; i++)
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

				desc.Width = sizeof(Transform);

				desc.Flags = D3D12_RESOURCE_FLAG_NONE;

				desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

				desc.SampleDesc.Count = 1;

				desc.SampleDesc.Quality = 0;

				desc.DepthOrArraySize = 1;

				desc.MipLevels = 1;

				desc.Format = DXGI_FORMAT_UNKNOWN;

				hr = device->CreateCommittedResource(
					&prop,
					D3D12_HEAP_FLAG_NONE,
					&desc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(constant_buffer[i].GetAddressOf())

				);

				if (FAILED(hr))
				{
					std::cerr << "失敗s" << std::endl;

					return;
				}
			}

			auto heap_ptr = heap_cbv_srv->GetCPUDescriptorHandleForHeapStart();

			auto increament = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			for (int i = 0; i < 2; i++)
			{
				cb_view[i].BufferLocation = constant_buffer[i]->GetGPUVirtualAddress();

				cb_view[i].SizeInBytes = sizeof(trans);

				device->CreateConstantBufferView(&cb_view[i], heap_ptr);

				heap_ptr.ptr += increament;
			}

			CopyCB();

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

				desc.Width = sizeof(vertices);

				desc.Flags = D3D12_RESOURCE_FLAG_NONE;

				desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

				desc.SampleDesc.Count = 1;

				desc.SampleDesc.Quality = 0;

				desc.DepthOrArraySize = 1;

				desc.MipLevels = 1;

				desc.Format = DXGI_FORMAT_UNKNOWN;

				hr = device->CreateCommittedResource(
					&prop,
					D3D12_HEAP_FLAG_NONE,
					&desc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(vertex_buffer.GetAddressOf())

				);

				if (FAILED(hr))
				{
					std::cerr << "失敗Zz" << std::endl;

					return;
				}
			}

			{
				void* ptr = nullptr;

				hr = vertex_buffer->Map(0, nullptr, &ptr);

				if (FAILED(hr))
				{
					std::cerr << "失敗z" << std::endl;

					return;
				}

				memcpy(ptr, vertices, sizeof(vertices));

				vertex_buffer->Unmap(0, nullptr);

				vb_view.BufferLocation = vertex_buffer->GetGPUVirtualAddress();

				vb_view.SizeInBytes = sizeof(vertices);

				vb_view.StrideInBytes = sizeof(DirectX::VertexPositionTexture);
			}

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

				desc.Width = sizeof(index);

				desc.Flags = D3D12_RESOURCE_FLAG_NONE;

				desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

				desc.SampleDesc.Count = 1;

				desc.SampleDesc.Quality = 0;

				desc.DepthOrArraySize = 1;

				desc.MipLevels = 1;

				desc.Format = DXGI_FORMAT_UNKNOWN;

				hr = device->CreateCommittedResource(
					&prop,
					D3D12_HEAP_FLAG_NONE,
					&desc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(index_buffer.GetAddressOf())

				);

				if (FAILED(hr))
				{
					std::cerr << "失敗Zz" << std::endl;

					return;
				}
			}

			{
				void* ptr = nullptr;

				hr = index_buffer->Map(0, nullptr, &ptr);

				if (FAILED(hr))
				{
					std::cerr << "失敗z" << std::endl;

					return;
				}

				memcpy(ptr, index, sizeof(index));

				index_buffer->Unmap(0, nullptr);

				ib_view.BufferLocation = index_buffer->GetGPUVirtualAddress();

				ib_view.SizeInBytes = sizeof(index);

				ib_view.Format = DXGI_FORMAT_R32_UINT;
			}

			{
				D3D12_ROOT_PARAMETER param[2] = {};

				param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

				param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;

				param[0].Descriptor.ShaderRegister = 0;

				param[0].Descriptor.RegisterSpace = 0;

				D3D12_DESCRIPTOR_RANGE range = {};

				range.NumDescriptors = 1;

				range.BaseShaderRegister = 0;

				range.RegisterSpace = 0;

				range.OffsetInDescriptorsFromTableStart = 0;

				range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

				param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

				param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

				param[1].DescriptorTable.NumDescriptorRanges = 1;

				param[1].DescriptorTable.pDescriptorRanges = &range;

				D3D12_STATIC_SAMPLER_DESC desc_sampler = {};

				desc_sampler.Filter = D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;

				desc_sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				desc_sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				desc_sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				desc_sampler.MipLODBias = D3D12_DEFAULT_MIP_LOD_BIAS;
				desc_sampler.MaxAnisotropy = 1;
				desc_sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
				desc_sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				desc_sampler.MaxLOD = D3D12_FLOAT32_MAX;
				desc_sampler.MinLOD = -D3D12_FLOAT32_MAX;
				desc_sampler.ShaderRegister = 0;
				desc_sampler.RegisterSpace = 0;
				desc_sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

				D3D12_ROOT_SIGNATURE_DESC desc = {};

				desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

				desc.NumParameters = 2;

				desc.NumStaticSamplers = 1;

				desc.pParameters = param;

				desc.pStaticSamplers = &desc_sampler;

				ComPtr<ID3DBlob> blob;
				ComPtr<ID3DBlob> error_blob;

				hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, blob.GetAddressOf(), error_blob.GetAddressOf());

				if (FAILED(hr))
				{
					std::cerr << "ルートシグネチャシリアライズ化失敗" << std::endl;

					return;
				}

				hr = device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(root_sig.GetAddressOf()));

				if (FAILED(hr))
				{
					std::cerr << "ルートシグネチャ作成失敗" << std::endl;

					return;
				}
			}

			{
				D3D12_INPUT_ELEMENT_DESC desc_element[2] = {};

				desc_element[0].SemanticName = "POSITION";

				desc_element[0].SemanticIndex = 0;

				desc_element[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;

				desc_element[0].InputSlot = 0;

				desc_element[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

				desc_element[0].InstanceDataStepRate = 0;

				desc_element[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

				desc_element[1].SemanticName = "TEXCOORD";

				desc_element[1].SemanticIndex = 0;

				desc_element[1].Format = DXGI_FORMAT_R32G32_FLOAT;

				desc_element[1].InputSlot = 0;

				desc_element[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

				desc_element[1].InstanceDataStepRate = 0;

				desc_element[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

				D3D12_RASTERIZER_DESC desc_ras = {};

				desc_ras.CullMode = D3D12_CULL_MODE_NONE;

				desc_ras.FillMode = D3D12_FILL_MODE_SOLID;

				desc_ras.FrontCounterClockwise = false;

				desc_ras.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;

				desc_ras.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;

				desc_ras.DepthClipEnable = false;

				desc_ras.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;

				desc_ras.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

				desc_ras.MultisampleEnable = false;

				desc_ras.AntialiasedLineEnable = false;

				desc_ras.ForcedSampleCount = 0;

				D3D12_RENDER_TARGET_BLEND_DESC desc_rt_blend = {};

				desc_rt_blend.BlendEnable = true;
				desc_rt_blend.BlendOp = D3D12_BLEND_OP_ADD;
				desc_rt_blend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
				desc_rt_blend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
				desc_rt_blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
				desc_rt_blend.LogicOp = D3D12_LOGIC_OP_NOOP;
				desc_rt_blend.SrcBlendAlpha = D3D12_BLEND_ONE;
				desc_rt_blend.DestBlendAlpha = D3D12_BLEND_ZERO;
				desc_rt_blend.LogicOpEnable = false;
				desc_rt_blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

				D3D12_BLEND_DESC desc_blend = {};

				desc_blend.AlphaToCoverageEnable = false;

				desc_blend.IndependentBlendEnable = false;

				for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;i++)
				{
					desc_blend.RenderTarget[i] = desc_rt_blend;
				}


				ComPtr<ID3DBlob> vs_blob;
				ComPtr<ID3DBlob> ps_blob;

				hr = D3DReadFileToBlob(L"VertexShader.cso", vs_blob.GetAddressOf());

				if (FAILED(hr))
				{
					std::cerr << "頂点シェーダー読み込みエラー" << std::endl;

					return;
				}

				hr = D3DReadFileToBlob(L"PixelShader.cso", ps_blob.GetAddressOf());

				if (FAILED(hr))
				{
					std::cerr << "ピクセルシェーダー読み込みエラー" << std::endl;

					return;
				}

				D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_pipe = {};

				desc_pipe.InputLayout = { desc_element , _countof(desc_element) };

				desc_pipe.pRootSignature = root_sig.Get();

				desc_pipe.VS = { vs_blob->GetBufferPointer() , vs_blob->GetBufferSize() };

				desc_pipe.PS = { ps_blob->GetBufferPointer() , ps_blob->GetBufferSize() };

				desc_pipe.RasterizerState = desc_ras;

				desc_pipe.BlendState = desc_blend;

				desc_pipe.DepthStencilState = ds_desc;

				desc_pipe.SampleMask = UINT_MAX;

				desc_pipe.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

				desc_pipe.NumRenderTargets = 1;

				for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;i++)
				{
					desc_pipe.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
				}

				desc_pipe.DSVFormat = DXGI_FORMAT_D32_FLOAT;

				desc_pipe.SampleDesc.Count = 1;

				desc_pipe.SampleDesc.Quality = 0;


				hr = device->CreateGraphicsPipelineState(&desc_pipe, IID_PPV_ARGS(pipeline.GetAddressOf()));

				if (FAILED(hr))
				{
					std::cerr << hr << std::endl;
					std::cerr << "sss" << std::endl;
				}

				viewport.TopLeftX = 0;
				viewport.TopLeftY = 0;

				viewport.MinDepth = 0;
				viewport.MaxDepth = 1;

				viewport.Height = height;

				viewport.Width = width;

				scissor.left = 0;
				scissor.top = 0;
				scissor.bottom = height;
				scissor.right = width;




			}

			{
				DirectX::ResourceUploadBatch batch(device.Get());

				batch.Begin();

				hr = DirectX::CreateDDSTextureFromFile(device.Get(), batch, L"C:/Users/yuichiro/source/repos/Project1/Project1/show.dds", tex_view.resource.GetAddressOf(), true);

				if (FAILED(hr))
				{
					std::cerr << "ddsファイル読み込みエラー" << std::endl;
				}

				auto load = batch.End(cmd_queue.Get());

				load.wait();

				tex_view.cpu_handle = heap_cbv_srv->GetCPUDescriptorHandleForHeapStart();
				tex_view.gpu_handle = heap_cbv_srv->GetGPUDescriptorHandleForHeapStart();

				auto texture_desc = tex_view.resource->GetDesc();

				D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

				desc.Format = texture_desc.Format;

				desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

				desc.Texture2D.MipLevels = texture_desc.MipLevels;
				desc.Texture2D.PlaneSlice = 0;
				desc.Texture2D.MostDetailedMip = 0;
				desc.Texture2D.ResourceMinLODClamp = 0;

				device->CreateShaderResourceView(tex_view.resource.Get() , &desc, tex_view.cpu_handle);
			}

		}

};

vector<InstanceObject*> objs;

void Init()
{

	HRESULT hr;

	hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, __uuidof(device), IID_PPV_ARGS_Helper(device.GetAddressOf()));

	if (FAILED(hr))
	{
		cout << "あ" << endl;

		return;
	}


	D3D12_COMMAND_QUEUE_DESC desc = {};

	desc.NodeMask = 0;

	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	desc.Priority = 0;

	hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(cmd_queue.GetAddressOf()));

	if (FAILED(hr))
	{
		cout << "あ" << endl;

		return;
	}

	IDXGIFactory4* factory;

	hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));

	if (FAILED(hr))
	{
		cout << "あ" << endl;

		return;
	}

	{
		DXGI_SWAP_CHAIN_DESC desc = {};

		desc.OutputWindow = wnd.GetWH();

		desc.BufferCount = frame_buffer;

		desc.BufferDesc.Width = width;

		desc.BufferDesc.Height = height;

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

		IDXGISwapChain * ptr_swap;

		hr = factory->CreateSwapChain(cmd_queue.Get(), &desc, &ptr_swap);

		if (FAILED(hr))
		{
			factory->Release();

			cout << "あsZ" << endl;

			return;
		}

		hr = ptr_swap->QueryInterface(IID_PPV_ARGS(swap_chain.GetAddressOf()));

		if (FAILED(hr))
		{
			factory->Release();

			ptr_swap->Release();

			cout << "あss" << endl;

			return;
		}

		factory->Release();

		ptr_swap->Release();
	}

	backbuffer_index = swap_chain->GetCurrentBackBufferIndex();

	for (int i = 0; i < frame_buffer; i++)
	{
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmd_allocator[i].GetAddressOf()));

		if (FAILED(hr))
		{
			cout << "あss" << endl;

			return;
		}
	}

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmd_allocator[backbuffer_index].Get(), nullptr, IID_PPV_ARGS(cmd_list.GetAddressOf()));

	if (FAILED(hr))
	{
		cout << "あss" << endl;

		return;
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};

		desc.NodeMask = 0;

		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		desc.NumDescriptors = frame_buffer;

		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

		hr = device->CreateDescriptorHeap(&desc , IID_PPV_ARGS(heap_rtv.GetAddressOf()));

		if (FAILED(hr))
		{
			cout << "あss" << endl;

			return;
		}
	}

	auto handle = heap_rtv->GetCPUDescriptorHandleForHeapStart();

	auto increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (int i = 0; i < frame_buffer; i++)
	{
		hr = swap_chain->GetBuffer(i, IID_PPV_ARGS(color_buffer[i].GetAddressOf()));

		if (FAILED(hr))
		{
			cout << "あs" << endl;

			return;
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

	hr = device->CreateFence(0 , D3D12_FENCE_FLAG_NONE , IID_PPV_ARGS(fence.GetAddressOf()) );

	if (FAILED(hr))
	{
		cout << "あs" << endl;

		return;
	}

	fence_event = CreateEvent(nullptr , false , false , nullptr);

	if(fence_event == nullptr)
	{
		cout << "あs" << endl;

		return;
	}

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

		desc.Height = height;

		desc.Width = width;

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
		}

		D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};

		heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		heap_desc.NodeMask = 0;

		heap_desc.NumDescriptors = 1;

		heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

		hr = device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(heap_dbv.GetAddressOf()));

		if (FAILED(hr))
		{
			std::cerr << "デスクリプターヒープ作成失敗" << std::endl;
		}

		D3D12_DEPTH_STENCIL_VIEW_DESC desc_depth = {};

		auto handle = heap_dbv->GetCPUDescriptorHandleForHeapStart();

		auto increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		desc_depth.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

		desc_depth.Flags = D3D12_DSV_FLAG_NONE;
		desc_depth.Format = DXGI_FORMAT_D32_FLOAT;
		desc_depth.Texture2D.MipSlice = 0;

		device->CreateDepthStencilView(depth_buffer.Get() ,&desc_depth , handle);

		dbv_handle = handle;
	}

	ds_desc.DepthEnable = true;
	ds_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	ds_desc.StencilWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	ds_desc.StencilEnable = false;
}

void WaitForRender()
{
	swap_chain->Present(1 , 0);

	auto cur_value = fence_counter;

	cmd_queue->Signal(fence.Get(), cur_value);

	fence_counter++;

	backbuffer_index = swap_chain->GetCurrentBackBufferIndex();

	if (fence->GetCompletedValue() < cur_value)
	{
		fence->SetEventOnCompletion(cur_value , fence_event);

		WaitForSingleObjectEx(fence_event , INFINITE , false);
	}
}

float rotation = 0;

void Draw()
{
	cmd_allocator[backbuffer_index]->Reset();

	cmd_list->Reset(cmd_allocator[backbuffer_index].Get(), nullptr);

	D3D12_RESOURCE_BARRIER barrier;

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

	barrier.Transition.pResource = color_buffer[backbuffer_index].Get();

	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

	vector<ID3D12CommandList*> lists;

	cmd_list->ResourceBarrier(1, &barrier);

	cmd_list->OMSetRenderTargets(1 , &rtv_handle[backbuffer_index] , false , &dbv_handle);

	cmd_list->ClearRenderTargetView(rtv_handle[backbuffer_index], colorBG, 0, nullptr);

	cmd_list->ClearDepthStencilView(dbv_handle, D3D12_CLEAR_FLAG_DEPTH , 1.0f , 0 , 0 ,nullptr);

	rotation += 0.06;

	POINT p;
	GetCursorPos(&p);

	ScreenToClient(FindWindowA("CreateWindow", nullptr), &p);

	std::cout << p.x << "  " <<  p.y << std::endl;

	for (int i = 0; i < objs.size(); i++)
	{


		objs[i]->rotation = XMFLOAT3(0 , 0 , std::sin((rotation * (i + 1) * 0.1)) * 360);

		objs[i]->CopyCB();

		cmd_list->SetGraphicsRootSignature(objs[i]->root_sig.Get());

		cmd_list->SetDescriptorHeaps(1, objs[i]->heap_cbv_srv.GetAddressOf());

		cmd_list->SetGraphicsRootConstantBufferView(0, objs[i]->cb_view[backbuffer_index].BufferLocation);

		cmd_list->SetGraphicsRootDescriptorTable(1 , tex_view.gpu_handle);

		cmd_list->SetPipelineState(objs[i]->pipeline.Get());

		cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		cmd_list->IASetVertexBuffers(0, 1, &objs[i]->vb_view);

		cmd_list->IASetIndexBuffer(&objs[i]->ib_view);

		cmd_list->RSSetScissorRects(1, &objs[i]->scissor);
		cmd_list->RSSetViewports(1, &objs[i]->viewport);

		cmd_list->DrawIndexedInstanced(6, 1, 0, 0, 0);

	}

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;

	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

	cmd_list->ResourceBarrier(1, &barrier);

	cmd_list->Close();
	 
	ID3D12CommandList* lists_array[] = { cmd_list.Get() };

	cmd_queue->ExecuteCommandLists(1, lists_array);

	WaitForRender();
}
*/

float hoge = 0;

void GameMainLoop()
{

}

int main()
{
	auto window = &Window::GetInstance();
	auto renSys = &DXRenderSystem::GetInstance();
	auto myImGui = &MyIMGUI::GetInstance();
	auto gameManager = &GameManager::GetInstance();
	auto physicSystem = &PhysicSystem::GetInstance();


	window->Init();
	renSys->Init();
	if (myImGui->useIMGUI)
		myImGui->Init();

	shared_ptr<Entity> player = make_shared<Entity>("Player");
	shared_ptr<Entity> player2 = make_shared<Entity>("Player2");

	player2->AddComponent<Renderer>();
	player2->AddComponent<Collider>();
	player2->AddComponent<Rigidbody>();
	player->AddComponent<Collider>();
	player->AddComponent<Rigidbody>();

	player->GetComponent<Rigidbody>()->isStatic = true;

	player->SetWorldScale(DirectX::XMVectorSet(100,10,100,0));
	player->SetWorldPosition(DirectX::XMVectorSet(0,-5,0,0));
	player2->SetWorldScale(DirectX::XMVectorSet(1, 1, 1,0));

	player2->SetWorldPosition(DirectX::XMVectorSet(0,10,0,0));

	bool show_demo_window = true;

	player2->GetComponent<Renderer>()->LoadMesh("Models/Knight01/Animated_Low_Poly_Dark_Knight_BAKED.fbx");

	UINT fpsCounter = 0;

	float timeForFpsCounter = 0;

	while (window->MainLoop())
	{
		gameManager->MainLoop();
		physicSystem->MainLoop();

		if (myImGui->useIMGUI)
			myImGui->MainLoop();

		hoge += 10;

		auto rend = player2->GetComponent<Renderer>();

		player2->SetWorldRotation(DirectX::XMVectorSet(0, -hoge * 0.1 , 0, 0));

		DirectX::XMVECTOR velocity = DirectX::XMVectorSet(0,0,0,0);

		rend->StartAnimation(gameManager->GameTime);

		if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			velocity = DirectX::XMVectorAdd(velocity , Utility::left);
		}

		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			velocity = DirectX::XMVectorAdd(velocity, Utility::right);
		}

		if (GetAsyncKeyState(VK_UP) & 0x8000)
		{
			velocity = DirectX::XMVectorAdd(velocity, Utility::up);
		}

		if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			velocity = DirectX::XMVectorAdd(velocity, Utility::down);
		}

		if (GetAsyncKeyState(VK_NUMPAD4) & 0x8000)
		{
			velocity = DirectX::XMVectorAdd(velocity, Utility::forward);
		}

		if (GetAsyncKeyState(VK_NUMPAD6) & 0x8000)
		{
			velocity = DirectX::XMVectorAdd(velocity, Utility::backward);
		}

		if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		{
			player2->GetComponent<Rigidbody>()->SetVelocity(DirectX::XMVectorScale(Utility::up , 6));
		}

		velocity = DirectX::XMVectorScale(velocity , 0.07);

		player2->GetComponent<Rigidbody>()->AddVelocity(velocity);

		auto hoge1 = DirectX::XMVectorScale(player2->GetRight(),4);
		auto hoge2 = DirectX::XMVectorScale(player2->GetUp(),4);
		auto hoge3 = DirectX::XMVectorScale(player2->GetForward(),4);

		auto vec = player2->GetWorldPosition();

		hoge1 = DirectX::XMVectorAdd(hoge1 , vec);
		hoge2 = DirectX::XMVectorAdd(hoge2 , vec);
		hoge3 = DirectX::XMVectorAdd(hoge3 , vec);

		ColliderDetection::GetInstance().MainLoop();

		renSys->Draw();

		timeForFpsCounter += gameManager->DeltaTime;

		fpsCounter++;

		if (timeForFpsCounter >= 1)
		{
			std::cout << fpsCounter << std::endl;

			fpsCounter = 0;

			timeForFpsCounter = 0;
		}
	}

	myImGui->Term();

	return 0;
}


