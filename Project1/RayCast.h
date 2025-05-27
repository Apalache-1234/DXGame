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
	RayCast(DirectX::XMVECTOR start , DirectX::XMVECTOR end);

	~RayCast();

	bool GetIsHit();
private:
	void CheckHit();

	std::vector<Collider> cols;

	DirectX::XMVECTOR startPoint;
	DirectX::XMVECTOR endPoint;

	bool isHit = false;
};
#endif