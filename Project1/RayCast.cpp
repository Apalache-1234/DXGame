#include "RayCast.h"

RayCast::RayCast(DirectX::XMVECTOR start , DirectX::XMVECTOR end)
{
	startPoint = start;
	endPoint = end;

	CheckHit();
}

RayCast::~RayCast()
{
}

bool RayCast::GetIsHit()
{
	return isHit;
}

void RayCast::CheckHit()
{
	

}
