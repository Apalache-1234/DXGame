#ifndef RAYCAST_H
#define RAYCAST_H

#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <DirectXMath.h>
#include "Collider.h"

class RayCast
{
public:
	RayCast(DirectX::XMVECTOR startPoint, DirectX::XMVECTOR dir, float distance);

	~RayCast();
	DirectX::XMVECTOR GetStartPoint();
	DirectX::XMVECTOR GetDirection();
	float GetDistance();
	bool GetIsHit();
	void ParseRayInteract(DirectX::XMVECTOR p1, DirectX::XMVECTOR p2, DirectX::XMVECTOR p3 , std::shared_ptr<Collider> col);

	struct Info {
		std::shared_ptr<Collider> colliders;

		DirectX::XMVECTOR hitPoint;
	};

	std::vector<Info> GetInfo();
private:
	std::vector<Info> infos;

	void CheckHit();

	std::vector<Collider> cols;

	DirectX::XMVECTOR startPoint;
	DirectX::XMVECTOR hitPoint;
	DirectX::XMVECTOR direction;
	float distance = 0;

	bool isHit = false;
};
#endif