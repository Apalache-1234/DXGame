#include "SkinnedRenderer.h"
#include "Utility.h"
#include <filesystem>

SkinnedRenderer::SkinnedRenderer(std::shared_ptr<Entity> ent) : Renderer::Renderer(ent)
{
}

SkinnedRenderer::~SkinnedRenderer()
{
	std::cout << "SkinnedRendererコンポーネントが破棄された" << std::endl;
}

void SkinnedRenderer::AddCommand(ComPtr<ID3D12GraphicsCommandList> cmdList)
{
	cmdList->SetGraphicsRootShaderResourceView(boneBufferParamNum, boneBuffer->GetGPUVirtualAddress());
}

bool SkinnedRenderer::InitBoneResource()
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

bool SkinnedRenderer::MapBB()
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

bool SkinnedRenderer::InitSRVForBoneBuffer()
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

void SkinnedRenderer::ReadHierarchyNode(float animationTime , aiNode& node , aiMatrix4x4& parentMat
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

void SkinnedRenderer::ProcessAnimation(float time)
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

float SkinnedRenderer::GetAnimCurrentTime()
{
	return animTimeCur;
}

float SkinnedRenderer::GetAnimTime()
{
	return animTime;
}

int SkinnedRenderer::GetAnimIndex()
{
	return animIndex;
}

void SkinnedRenderer::SetAnimTime(float time)
{
	animTimeCur = time;

	ProcessAnimation(animTimeCur);
}

void SkinnedRenderer::StartAnimation(UINT index,float speed = 1, bool isLoop = false)
{
	isAnimPlaying = true;

	animIndex = index;

	aiAnimation* anim = meshData->scene.mAnimations[animIndex];

	animSpeed = speed;

	animTime = anim->mDuration / anim->mTicksPerSecond;

	animTimeCur = 0;

	animLoop = isLoop;
}

std::vector<string> SkinnedRenderer::GetAnimationNameList()
{
	std::vector<string> names;

	for (int i = 0; i < meshData->scene.mNumAnimations;i++)
	{
		names.emplace_back(meshData->scene.mAnimations[i]->mName.C_Str());
	}

	return names;
}

void SkinnedRenderer::ResetAnimationStatue()
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

void SkinnedRenderer::InitRenderer()
{
	bool isSuccess = true;

	isSuccess &= InitBoneResource();

	isSuccess &= MapBB();

	isSuccess &= InitSRVForBoneBuffer();

	if (!isSuccess)
	{
		std::cerr << "SkinnedRendererコンポーネントの初期化に失敗しました" << std::endl;

		return;
	}

	LoadVertexShader(L"SkinnedVertexShader.cso");
	LoadPixelShader(L"SkinnedPixelShader.cso");

	D3D12_ROOT_PARAMETER param;

	param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;

	param.Descriptor.ShaderRegister = 20;

	param.Descriptor.RegisterSpace = 0;

	boneBufferParamNum = GetRootParamNum();

	AddRootParam(param);
}

void SkinnedRenderer::Update()
{
	if (isAnimPlaying)
	{
		animTimeCur += GameManager::GetInstance().GetGameDeltaTime() * animSpeed;

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
}

void SkinnedRenderer::InitInCreateMesh()
{
	ResetAnimationStatue();
}
