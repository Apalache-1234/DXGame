#ifndef COLLIDER_H
#define COLLIDER_H

#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <DirectXMath.h>
#include "Component.h"
#include "MeshLoader.h"

class Entity;

class Collider : public Component
{
public:
	enum ColliderType 
	{ Cube , Sphere};

	Collider(std::shared_ptr<Entity> ent);

	~Collider();

	/// <summary>
	/// コライダーの位置や大きさ、そしてこれをつけているエンティティーの位置や大きさに考慮して計算された結果の頂点座標を配列で返す
	/// </summary>
	/// <returns>調整後の頂点座標</returns>
	std::vector<DirectX::XMVECTOR> GetCurrentVerticesPos();

	DirectX::XMMATRIX GetTRSMatrix();

	void SetColliderType(ColliderType type);

	ColliderType GetColliderType();

	DirectX::XMVECTOR GetRotQuaternion();

	std::pair < std::vector<int>, std::vector <DirectX::XMVECTOR>> GetTriangles();

	/// <summary>
	/// コライダーの位置
	/// </summary>
	DirectX::XMVECTOR position = DirectX::XMVectorSet(0,0,0,0);

	/// <summary>
	/// コライダーの大きさ
	/// </summary>
	DirectX::XMVECTOR scale = DirectX::XMVectorSet(1, 1, 1, 0);

	DirectX::XMVECTOR rot = DirectX::XMVectorSet(0, 0, 0, 0);

	bool isColliding = false;
private:
	/// <summary>
	/// 頂点座標
	/// </summary>
	std::vector<DirectX::XMVECTOR> verticesPos;

	ColliderType currentType = Cube;

	MeshLoader::MeshData* mesh;
};

#endif