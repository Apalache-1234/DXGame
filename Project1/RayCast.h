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

	bool GetIsHit();
	DirectX::XMVECTOR GetStartPoint();
	DirectX::XMVECTOR GetDirection();
	float GetDistance();
	DirectX::XMVECTOR GetHitPoint();
	void ParseRayInteract(DirectX::XMVECTOR p1, DirectX::XMVECTOR p2, DirectX::XMVECTOR p3);
private:
	void CheckHit();

	std::vector<Collider> cols;

	DirectX::XMVECTOR startPoint;
	DirectX::XMVECTOR hitPoint;
	DirectX::XMVECTOR direction;
	float distance = 0;

	bool isHit = false;
};
#endif