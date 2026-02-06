
#include "Collider.h"
#include "Utility.h"
#include "Entity.h"

Collider::Collider(std::shared_ptr<Entity> ent) : Component::Component(ent)
{
	SetColliderType(Cube);
}

Collider::~Collider()
{
}

std::vector<DirectX::XMVECTOR> Collider::GetCurrentVerticesPos()
{
	std::vector<DirectX::XMVECTOR> cpy = verticesPos;

	for (auto& pos : cpy)
	{
		pos = DirectX::XMVector3Transform(pos , GetTRSMatrix());
	}

	return cpy;
}

DirectX::XMMATRIX Collider::GetTRSMatrix()
{
	auto positionMat = DirectX::XMMatrixTranslationFromVector(attachFrom.lock()->GetWorldPosition()) * DirectX::XMMatrixTranslationFromVector(position);

	auto qCol = GetRotQuaternion();

	auto resultQ = attachFrom.lock()->GetRotationAsQuaternion();
	resultQ = DirectX::XMQuaternionMultiply(resultQ, qCol);

	auto rotationMat = DirectX::XMMatrixRotationQuaternion(resultQ);

	auto resultS = DirectX::XMVectorMultiply(scale , attachFrom.lock()->GetScale());

	auto scaleMat = DirectX::XMMatrixScalingFromVector(resultS);

	return scaleMat * rotationMat * positionMat;
}

void Collider::SetColliderType(ColliderType type)
{
	verticesPos.clear();

	currentType = type;

	switch (currentType)
	{
	case Cube:
		mesh = MeshLoader::GetInstance().GetMesh(MeshLoader::BOX);
		break;
	case Sphere:
		mesh = MeshLoader::GetInstance().GetMesh(MeshLoader::SPHERE);
		break;
	default:
		mesh = MeshLoader::GetInstance().GetMesh(MeshLoader::BOX);
		break;
	}

	for (int i = 0;i < mesh->meshes[0].vertexDatas.size();i++)
	{
		verticesPos.emplace_back(DirectX::XMLoadFloat3(&mesh->meshes[0].vertexDatas[i].vertex));
	}
}

Collider::ColliderType Collider::GetColliderType()
{
	return currentType;
}

DirectX::XMVECTOR Collider::GetRotQuaternion()
{
	DirectX::XMVECTOR resultQ = DirectX::XMVectorSet(0, 0, 0, 0);

	auto rotation = rot;

	float rotX = DirectX::XMConvertToRadians(DirectX::XMVectorGetX(rotation));
	float rotY = DirectX::XMConvertToRadians(DirectX::XMVectorGetY(rotation));
	float rotZ = DirectX::XMConvertToRadians(DirectX::XMVectorGetZ(rotation));

	auto XQuaternion = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1, 0, 0, 0), rotX);
	auto YQuaternion = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 1, 0, 0), rotY);
	auto ZQuaternion = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0, 0, 1, 0), rotZ);

	resultQ = Utility::GetZYXMultiplyQuaternion(XQuaternion, YQuaternion, ZQuaternion);

	return resultQ;
}

std::pair < std::vector<int>, std::vector < DirectX::XMVECTOR > > Collider::GetTriangles()
{
	std::pair < std::vector<int>, std::vector <DirectX::XMVECTOR>> triangles;

	triangles.first = mesh->meshes[0].indices;
	triangles.second = GetCurrentVerticesPos();

	return triangles;
}

std::type_index Collider::GetBaseType()
{
	return typeid(Component);
}
