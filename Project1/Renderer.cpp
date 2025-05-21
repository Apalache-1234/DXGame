#include "Renderer.h"
#include "Utility.h"
#include <filesystem>

void Renderer::ConvertValues()
{
	auto worldPos = attachFrom->GetWorldPosition();

	float posX = DirectX::XMVectorGetX(worldPos);
	float posY = DirectX::XMVectorGetY(worldPos);
	float posZ = DirectX::XMVectorGetZ(worldPos);

	auto position_mat = XMMatrixTranslation(posX, posY, posZ);

	auto mat1 = DirectX::XMMatrixRotationQuaternion(attachFrom->GetWorldQuaternion());
	auto mat2 = DirectX::XMMatrixRotationQuaternion(attachFrom->GetLocalQuaternion());

	auto rotation_mat = mat1 * mat2;

	auto scale = attachFrom->GetScale();

	float scaleX = DirectX::XMVectorGetX(scale);
	float scaleY = DirectX::XMVectorGetY(scale);
	float scaleZ = DirectX::XMVectorGetZ(scale);

	auto scale_mat = XMMatrixScaling(scaleX, scaleY, scaleZ);

	trans.world = scale_mat * rotation_mat * position_mat;

	float colR = DirectX::XMVectorGetX(color);
	float colG = DirectX::XMVectorGetY(color);
	float colB = DirectX::XMVectorGetZ(color);
	float colA = DirectX::XMVectorGetW(color);

	auto color_normalized = XMFLOAT4(colR, colG, colB , colA);

	trans.color = color_normalized;
}

bool Renderer::MapCB()
{
	ConvertValues();

	trans.view = XMMatrixLookAtLH(Camera::GetInstance().GetPosition(), Camera::GetInstance().GetTargetPos(), DirectX::XMVectorSet(0,1,0,0));

	trans.projection = XMMatrixPerspectiveFovLH(Camera::GetInstance().fov, Camera::GetInstance().aspect, 1, 1000);

	void* ptr[2] = { nullptr , nullptr };

	for (int i = 0; i < 2; i++)
	{
		ptr[i] = nullptr;

		hr = constantBuffer[i]->Map(0, nullptr, &ptr[i]);

		if (FAILED(hr))
		{
			std::cerr << "座標行列が収納されたCBのマッピング失敗" << std::endl;

			return false;
		}

		memcpy(ptr[i], &trans, sizeof(Transform));


		constantBuffer[i]->Unmap(0, nullptr);
	}

	return true;
}

Renderer::Renderer(std::shared_ptr<Entity> ent) : Component::Component(ent)
{

	bool isSuccess = true;

	isSuccess &= InitCBVResource();

	isSuccess &= LinkCBVResourceToDescHeap();

	isSuccess &= MapCB();

	isSuccess &= CreateMesh(MeshLoader::BOX);

	isSuccess &= InitBoneResource();

	isSuccess &= MapBB();

	isSuccess &= InitSRVForBoneBuffer();

	isSuccess &= CreateRootSignature();

	isSuccess &= CreateRenderPipeline();

	if (!isSuccess)
	{
		std::cerr << "レンダーコンポネントの初期化失敗" << std::endl;
	}
}

Renderer::~Renderer()
{
}

void Renderer::AddRenderCommand(ComPtr<ID3D12GraphicsCommandList>& cmdList, int backbufferIndex)
{
	MapCB();

	if (isAnimPlaying)
	{
		animTimeCur += GameManager::GetInstance().DeltaTime * animSpeed;

		if (animTimeCur >= animTime)
		{
			if (animLoop)
				animTimeCur = 0;
			else
			{
				animTimeCur = animTime;
				isAnimPlaying = false;
			}
			
		}
		else if (0 > animTimeCur)
		{
			if (animLoop)
				animTimeCur = animTime;
			else
				animTimeCur = 0;
		}

		ProcessAnimation(animTimeCur);
	}

	cmdList->SetGraphicsRootSignature(rootSig.Get());

	cmdList->SetDescriptorHeaps(1, heapManager.GetHeap().GetAddressOf());

	cmdList->SetGraphicsRootConstantBufferView(0, cbView[backbufferIndex].BufferLocation);

	cmdList->SetPipelineState(pipeline.Get());

	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (int i =0;i<meshData->meshes.size();i++)
	{
		auto& mesh = meshData->meshes[i];

		cmdList->SetGraphicsRootDescriptorTable(1, texHandle[i]->gpuHandle);

		cmdList->SetGraphicsRootShaderResourceView(2, boneBuffer->GetGPUVirtualAddress());

		cmdList->IASetVertexBuffers(0, 1, &mesh.vbView);

		cmdList->IASetIndexBuffer(&mesh.ibView);

		cmdList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}

}//uploadIndexBuffer = 0x0000021b7c486440 < 情報はありません。d3d12SDKLayers.dll のシンボルが読み込まれていません > vertexBuffer = 0x0000021b6ff54450 < 情報はありません。d3d12SDKLayers.dll のシンボルが読み込まれていません >

bool Renderer::InitCBVResource()
{
	for (int i = 0; i < DXRenderSystem::frameBuffer; i++)
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

		hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(constantBuffer[i].GetAddressOf())

		);

		if (FAILED(hr))
		{
			std::cerr << "CBVのリソース作成失敗" << std::endl;

			return false;
		}
	}

	return true;
}

bool Renderer::LinkCBVResourceToDescHeap()
{
	for (int i = 0; i < DXRenderSystem::frameBuffer; i++)
	{
		cbHandle[i] = std::move(heapManager.GetHandle());

		cbView[i].BufferLocation = constantBuffer[i]->GetGPUVirtualAddress();

		cbView[i].SizeInBytes = sizeof(Transform);

		DXRenderSystem::GetInstance().GetDevice()->CreateConstantBufferView(&cbView[i], cbHandle[i]->cpuHandle);
	}

	return true;
}



bool Renderer::CreateRootSignature()
{
	D3D12_ROOT_PARAMETER param[3] = {};

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

	param[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	param[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;

	param[2].Descriptor.ShaderRegister = 1;

	param[2].Descriptor.RegisterSpace = 0;

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

	desc.NumParameters = sizeof(param) / sizeof(D3D12_ROOT_PARAMETER);

	desc.NumStaticSamplers = 1;

	desc.pParameters = param;

	desc.pStaticSamplers = &desc_sampler;

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

bool Renderer::CreateRenderPipeline()
{
	D3D12_INPUT_ELEMENT_DESC desc_element[7] = {
	{"POSITION" , 0 , DXGI_FORMAT_R32G32B32_FLOAT , 0 , D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
	{"TEXCOORD" , 0 , DXGI_FORMAT_R32G32_FLOAT , 0 , D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
	{"TANGENT" , 0 , DXGI_FORMAT_R32G32B32_FLOAT , 0 , D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
	{"NORMAL" , 0 , DXGI_FORMAT_R32G32B32_FLOAT , 0 , D3D12_APPEND_ALIGNED_ELEMENT , D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0},
	{ "BONEID",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "WEIGHT",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "PARENT",   0, DXGI_FORMAT_R32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

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

		return false;
	}

	hr = D3DReadFileToBlob(L"PixelShader.cso", ps_blob.GetAddressOf());

	if (FAILED(hr))
	{
		std::cerr << "ピクセルシェーダー読み込みエラー" << std::endl;

		return false;
	}

	D3D12_DEPTH_STENCIL_DESC dsDesc;

	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	dsDesc.StencilWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	dsDesc.StencilEnable = false;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_pipe = {};

	desc_pipe.InputLayout = { desc_element , _countof(desc_element) };

	desc_pipe.pRootSignature = rootSig.Get();

	desc_pipe.VS = { vs_blob->GetBufferPointer() , vs_blob->GetBufferSize() };

	desc_pipe.PS = { ps_blob->GetBufferPointer() , ps_blob->GetBufferSize() };

	desc_pipe.RasterizerState = desc_ras;

	desc_pipe.BlendState = desc_blend;

	desc_pipe.DepthStencilState = dsDesc;

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

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateGraphicsPipelineState(&desc_pipe, IID_PPV_ARGS(pipeline.GetAddressOf()));

	if (FAILED(hr))
	{
		std::cerr << "パイプラインの作成失敗" << std::endl;

		return false;
	}

	return true;
}

void Renderer::RegisterTextureSRV()
{
	texHandle.clear();

	for (int i = 0;i < meshData->meshes.size();i++)
	{
		texHandle.push_back(std::move(heapManager.GetHandle()));

		auto texture_desc = meshData->meshes[i].texResource->GetDesc();

		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};

		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		desc.Format = texture_desc.Format;

		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		desc.Texture2D.MipLevels = texture_desc.MipLevels;
		desc.Texture2D.MostDetailedMip = 0;
		desc.Texture2D.ResourceMinLODClamp = 0;
		desc.Texture2D.PlaneSlice = 0;

		DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(meshData->meshes[i].texResource.Get(), &desc, texHandle[i]->cpuHandle);
	}
}

bool Renderer::InitBoneResource()
{
	finalBoneMats.resize(400, DirectX::XMMatrixIdentity());

	UINT bufferSize = sizeof(XMMATRIX) * finalBoneMats.size();

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

	hr = DXRenderSystem::GetInstance().GetDevice()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(boneBuffer.GetAddressOf())
	);

	if (FAILED(hr))
	{
		std::cerr << "ボーンバッファのリソースの作成失敗" << std::endl;

		return false;
	}

	return true;
}

bool Renderer::MapBB()
{
	void* ptr= nullptr;

	hr = boneBuffer->Map(0, nullptr, &ptr);

	if (FAILED(hr))
	{
		std::cerr << "ボーンの最終座標が収納されたBBのマッピング失敗" << std::endl;

		return false;
	}

	memcpy(ptr, finalBoneMats.data(), sizeof(XMMATRIX) * finalBoneMats.size());

	boneBuffer->Unmap(0, nullptr);

	return true;
}

bool Renderer::InitSRVForBoneBuffer()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = finalBoneMats.size();
	srvDesc.Buffer.StructureByteStride = sizeof(XMMATRIX);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	boneHandle = std::move(heapManager.GetHandle());

	DXRenderSystem::GetInstance().GetDevice()->CreateShaderResourceView(boneBuffer.Get(), &srvDesc, boneHandle->cpuHandle);

	return true;
}

aiNodeAnim* ParseAnimNode(aiAnimation& anim, string nodeName)
{
	for (int i = 0;i < anim.mNumChannels;i++)
	{
		if (anim.mChannels[i]->mNodeName.C_Str() == nodeName)
		{
			return anim.mChannels[i];
		}
	}

	return nullptr;
}

UINT FindScalingKey(float animationTime ,const aiNodeAnim* nodeAnim)
{
	for (int i = 0;i < nodeAnim->mNumScalingKeys - 1;i++)
	{
		auto nextNode = nodeAnim->mScalingKeys[i + 1];

		if (animationTime < nextNode.mTime)
		{
			return i;
		}
	}
}

UINT FindQuaternionKey(float animationTime, const aiNodeAnim* nodeAnim)
{
	for (int i = 0;i < nodeAnim->mNumRotationKeys - 1;i++)
	{
		auto nextNode = nodeAnim->mRotationKeys[i + 1];

		if (animationTime < nextNode.mTime)
		{
			return i;
		}
	}
}

UINT FindPositionKey(float animationTime, const aiNodeAnim* nodeAnim)
{
	for (int i = 0;i < nodeAnim->mNumPositionKeys - 1;i++)
	{
		auto nextNode = nodeAnim->mPositionKeys[i + 1];

		if (animationTime < nextNode.mTime)
		{
			return i;
		}
	}
}

void interpolateScaling(aiVector3D& scale , float animationTime , const aiNodeAnim* nodeAnim)
{
	if (nodeAnim->mNumScalingKeys <= 1)
	{
		if (nodeAnim->mNumScalingKeys == 1)
		{
			scale = nodeAnim->mScalingKeys[0].mValue;
		}
		else 
		{
			scale = aiVector3D(1, 1, 1);
		}

		return;
	}

	auto currentKeyIndex = FindScalingKey(animationTime , nodeAnim);
	auto nextKeyIndex = currentKeyIndex + 1;
	auto curKey = nodeAnim->mScalingKeys[currentKeyIndex];
	auto nextKey = nodeAnim->mScalingKeys[nextKeyIndex];
	auto intervalTime = nextKey.mTime - curKey.mTime;
	auto factor = (animationTime - curKey.mTime) / intervalTime;
	auto curKeyVec = curKey.mValue;
	auto nextKeyVec = nextKey.mValue;
	auto subVec = nextKeyVec - curKeyVec;
	subVec *= factor;
	auto resultVec = curKeyVec + subVec;
	scale = resultVec;
}

void interpolateQuaternion(aiQuaternion& quat, float animationTime, const aiNodeAnim* nodeAnim)
{
	if (nodeAnim->mNumRotationKeys <= 1)
	{
		if (nodeAnim->mNumScalingKeys == 1)
		{
			quat = nodeAnim->mRotationKeys[0].mValue;
		}
		else
		{
			quat = aiQuaternion(1, 0, 0, 0);
		}

		return;
	}

	auto currentKeyIndex = FindQuaternionKey(animationTime, nodeAnim);
	auto nextKeyIndex = currentKeyIndex + 1;
	auto curKey = nodeAnim->mRotationKeys[currentKeyIndex];
	auto nextKey = nodeAnim->mRotationKeys[nextKeyIndex];
	auto intervalTime = nextKey.mTime - curKey.mTime;
	auto factor = (animationTime - curKey.mTime) / intervalTime;
	auto curKeyQuat = curKey.mValue;
	auto nextKeyQuat = nextKey.mValue;
    aiQuaternion::Interpolate(quat , curKeyQuat, nextKeyQuat,factor);
	quat.Normalize();
}

void InterpolateTranslation(aiVector3D& trans, float animationTime, const aiNodeAnim* nodeAnim)
{
	if (nodeAnim->mNumPositionKeys <= 1)
	{
		if (nodeAnim->mNumPositionKeys == 1)
		{
			trans = nodeAnim->mPositionKeys[0].mValue;
		}
		else
		{
			trans = aiVector3D(0, 0, 0);
		}

		return;
	}

	auto currentKeyIndex = FindPositionKey(animationTime, nodeAnim);
	auto nextKeyIndex = currentKeyIndex + 1;
	auto curKey = nodeAnim->mPositionKeys[currentKeyIndex];
	auto nextKey = nodeAnim->mPositionKeys[nextKeyIndex];
	auto intervalTime = nextKey.mTime - curKey.mTime;
	auto factor = (animationTime - curKey.mTime) / intervalTime;
	auto curKeyVec = curKey.mValue;
	auto nextKeyVec = nextKey.mValue;
	auto subVec = nextKeyVec - curKeyVec;
	subVec *= factor;
	auto resultVec = curKeyVec + subVec;
	trans = resultVec;
}

void Renderer::ReadHierarchyNode(float animationTime , aiNode& node , aiMatrix4x4& parentMat
 , aiAnimation& anim)
{
	string name(node.mName.C_Str());

	aiMatrix4x4 nodeTransformation(node.mTransformation);

	auto channel = ParseAnimNode(anim , name);

	aiMatrix4x4 resultMat;

	if (channel)
	{
		aiVector3D scaling;
		interpolateScaling(scaling , animationTime , channel);
		aiMatrix4x4 scalingMat;
		aiMatrix4x4::Scaling(scaling , scalingMat);

		aiQuaternion quat;
		interpolateQuaternion(quat, animationTime, channel);
		aiMatrix4x4 rotMat = aiMatrix4x4(quat.GetMatrix());

		aiVector3D trans;
		InterpolateTranslation(trans , animationTime , channel);
		aiMatrix4x4 transMat;
		aiMatrix4x4::Translation(trans, transMat);

		nodeTransformation = transMat * rotMat;
	}

	resultMat = parentMat * nodeTransformation;

	if (meshData->boneMapping.find(name) != meshData->boneMapping.end())
	{
		auto animationMat = resultMat * meshData->boneOffset[meshData->boneMapping[name]];

		finalBoneMats[meshData->boneMapping[name]] = Utility::ConvertAiMatrix4x4ToXMMATRIX(animationMat);
	}

	for (int i = 0;i < node.mNumChildren;i++)
	{
		ReadHierarchyNode(animationTime , *node.mChildren[i] , resultMat , anim);
	}

}

void Renderer::ProcessAnimation(float time)
{
	aiAnimation* anim = meshData->scene.mAnimations[animIndex];

	auto ticksPerSecond = anim->mTicksPerSecond;

	auto ticksInSecond = time * ticksPerSecond;

	float animationTime = fmod(ticksInSecond, anim->mDuration);

	aiMatrix4x4 mat;

	mat = aiMatrix4x4();

	ReadHierarchyNode(animationTime, *meshData->scene.mRootNode, mat, *anim);

	InitBoneResource();

	MapBB();
}

float Renderer::GetAnimCurrentTime()
{
	return animTimeCur;
}

float Renderer::GetAnimTime()
{
	return animTime;
}

int Renderer::GetAnimIndex()
{
	return animIndex;
}

void Renderer::SetAnimTime(float time)
{
	animTimeCur = time;

	ProcessAnimation(animTimeCur);
}

void Renderer::StartAnimation(UINT index,float speed = 1, bool isLoop = false)
{
	isAnimPlaying = true;

	animIndex = index;

	aiAnimation* anim = meshData->scene.mAnimations[animIndex];

	animSpeed = speed;

	animTime = anim->mDuration / anim->mTicksPerSecond;

	animTimeCur = 0;

	animLoop = isLoop;
}

std::vector<string> Renderer::GetAnimationNameList()
{
	std::vector<string> names;

	for (int i = 0; i < meshData->scene.mNumAnimations;i++)
	{
		names.emplace_back(meshData->scene.mAnimations[i]->mName.C_Str());
	}

	return names;
}

MeshLoader::MESHTYPE  Renderer::GetCurrentMeshType()
{
	return currentMeshType;
}

void Renderer::ResetAnimationStatue()
{
	finalBoneMats.clear();

	finalBoneMats.resize(meshData->boneMapping.size(), DirectX::XMMatrixIdentity());

	isAnimPlaying = false;

	animIndex = 0;

	aiAnimation* anim = nullptr;

	animSpeed = 1;

	animLoop = false;

	InitBoneResource();

	MapBB();
}

bool Renderer::CreateMesh(const MeshLoader::MESHTYPE type)
{
	currentMeshType = type;

	meshData = MeshLoader::GetInstance().GetMesh(type);

	ResetAnimationStatue();

	RegisterTextureSRV();

	return true;
}
