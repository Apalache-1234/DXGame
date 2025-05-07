#ifndef COLLIDER_H
#define COLLIDER_H

#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <DirectXMath.h>
#include "Component.h"
#include "Entity.h"
#include "Utility.h"

class Collider : public Component
{
public:
	enum ColliderType 
	{ Cube};

	Collider(std::shared_ptr<Entity> ent);

	~Collider();

	/// <summary>
	/// コライダーの位置や大きさ、そしてこれをつけているエンティティーの位置や大きさに考慮して計算された結果の頂点座標を配列で返す
	/// </summary>
	/// <returns>調整後の頂点座標</returns>
	std::vector<DirectX::XMVECTOR> GetCurrentVerticesPos();

	void SetColliderType(ColliderType type);

	ColliderType GetColliderType();

	/// <summary>
	/// コライダーの位置
	/// </summary>
	DirectX::XMVECTOR position = DirectX::XMVectorSet(0,0,0,0);

	/// <summary>
	/// コライダーの大きさ
	/// </summary>
	DirectX::XMVECTOR scale = DirectX::XMVectorSet(1, 1, 1, 0);;


private:
	/// <summary>
	/// 頂点座標
	/// </summary>
	std::vector<DirectX::XMVECTOR> verticesPos;

	int sphereCorrectness = 18;

	ColliderType currentType = Cube;
};

#endif