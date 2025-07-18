#include "DebugGrids.h"
#include "Camera.h"
#include "Utility.h"

DebugGrids::DebugGrids(std::shared_ptr<Entity> ent) : DebugRenderer::DebugRenderer(ent)
{
}

DebugGrids::~DebugGrids()
{
}

std::type_index DebugGrids::GetBaseType()
{
	return typeid(DebugRenderer);
}

float ClampValue(float t , float val1 ,float val2)
{
	if (val1 > val2)
	{
		auto temp = val1;
		val1 = val2;
		val2 = temp;
	}

	val1 = floorf(val1);
	val2 = floorf(val2);

	if (val1 > t)
	{
		float sub = val1 - t;
		if (sub >= 1)
			return floor(val2 - sub - 1);
	}

	if (val2 < t)
	{
		float sub = val2 - t;
		if (sub <= -1)
			return floor(val1 - sub + 1);
	}

	return t;
}

void DebugGrids::Update()
{
	for (int i = 0; i < instances.size(); i++)
	{
		DirectX::XMVECTOR trans;
		DirectX::XMVECTOR scale;
		DirectX::XMVECTOR hoge;

		DirectX::XMMatrixDecompose(&scale ,&hoge,&trans , instances[i].world);

		auto camPosZ = DirectX::XMVectorGetZ(Camera::GetInstance().GetPosition());

		auto z = DirectX::XMVectorGetZ(trans);

		z = ClampValue(z, -GripsNum + camPosZ, GripsNum + camPosZ);

		trans = DirectX::XMVectorSetZ(trans , z);

		auto camPosX = DirectX::XMVectorGetX(Camera::GetInstance().GetPosition());

		auto x = DirectX::XMVectorGetX(trans);

		x = ClampValue(x, -GripsNum + camPosX, GripsNum + camPosX);

		trans = DirectX::XMVectorSetX(trans, x);

		if (i < HorizonNum)
		{
			trans = DirectX::XMVectorSetX(trans, camPosX);
			if (z == 0)
				instances[i].color = DirectX::XMFLOAT4(1,0,0,1);
			else
				instances[i].color = DirectX::XMFLOAT4(1, 1, 1, 1);
		}
		else
		{
			trans = DirectX::XMVectorSetZ(trans, camPosZ);
			if (x == 0)
				instances[i].color = DirectX::XMFLOAT4(0, 1, 0, 1);
			else
				instances[i].color = DirectX::XMFLOAT4(1, 1, 1, 1);
		}

		instances[i].world = DirectX::XMMatrixScalingFromVector(scale) * DirectX::XMMatrixTranslationFromVector(trans);
	}

	MapDVB();
}

void DebugGrids::InitDebugRenderer()
{
	LoadVertexShader(L"DebugPointLightVertexShader.cso");
	LoadPixelShader(L"DebugGridsPixelShader.cso");

	CreateMesh(MeshLoader::BOX);

	instances.clear();

	for (int i = -GripsNum; i < GripsNum; i++)
	{
		Instance inst = {};

		inst.world = DirectX::XMMatrixScalingFromVector(DirectX::XMVectorSet(GripsNum*2, 0.01f ,0.01f , 0)) * DirectX::XMMatrixTranslationFromVector(DirectX::XMVectorScale(Utility::forward, i));
		inst.color = DirectX::XMFLOAT4(1, 1, 1, 1);

		instances.push_back(inst);
	}

	HorizonNum = instances.size();

	for (int i = -GripsNum; i < GripsNum; i++)
	{
		Instance inst = {};

		inst.world = DirectX::XMMatrixScalingFromVector(DirectX::XMVectorSet(0.01f, 0.01f, GripsNum*2, 0)) * DirectX::XMMatrixTranslationFromVector(DirectX::XMVectorScale(Utility::right, i));
		inst.color = DirectX::XMFLOAT4(1, 1, 1, 1);

		instances.push_back(inst);
	}

	VerticalNum = instances.size();

	instanceCount = instances.size();

	D3D12_DEPTH_STENCIL_DESC dsDesc;

	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	dsDesc.StencilWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	dsDesc.StencilEnable = false;
}
