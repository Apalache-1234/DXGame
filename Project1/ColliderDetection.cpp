#include "ColliderDetection.h"

ColliderDetection::ColliderDetection()
{
}

ColliderDetection::~ColliderDetection()
{
}

std::unique_ptr<ColliderDetection> ColliderDetection::instance = nullptr;

DirectX::XMVECTOR vecAtoB;

ColliderDetection& ColliderDetection::GetInstance()
{
	if (!instance) {
		instance.reset(new ColliderDetection());
	}
	return *instance;
}

std::vector<DirectX::XMVECTOR> ReturnMinkowskiDif(std::vector<DirectX::XMVECTOR> verA, std::vector<DirectX::XMVECTOR> verB)
{
	std::vector<DirectX::XMVECTOR> minkowskiDifVer;

	for (auto posA : verA)
	{
		for (auto posB : verB)
		{
			minkowskiDifVer.emplace_back(DirectX::XMVectorSubtract(posA, posB));
		}
	}

	return minkowskiDifVer;
}

void UpdateVelocity(std::shared_ptr<Collider> col , DirectX::XMVECTOR nd)
{
	auto comp = col->attachFrom->GetComponent<Rigidbody>();

	if (comp != nullptr)
	{
		if (!comp->isStatic)
		{
			col->attachFrom->AddWorldPosition(nd);

			auto vel = col->attachFrom->GetComponent<Rigidbody>()->GetVelocity();

			auto n = DirectX::XMVector3Normalize(nd);

			auto dot = DirectX::XMVector3Dot(vel, n).m128_f32[0];

			if (DirectX::XMVector3Dot(n, DirectX::XMVectorScale(n, -dot)).m128_f32[0] > 0)
			{
				n = DirectX::XMVectorScale(n, -dot);

				col->attachFrom->GetComponent<Rigidbody>()->AddVelocity(n);
			}
	    }
	}
	else
	{
		col->attachFrom->AddWorldPosition(nd);
	}

}

void ColliderDetection::MainLoop()
{
	auto list = GameManager::GetInstance().GetComponentList<Collider>();

	for (int i = 0;i < list.size()- 1;i++)
	{
		auto colA = list[i];

		for (int j = i + 1;j < list.size();j++)
		{
			auto colB = list[j];

			DirectX::XMVECTOR ndA = Utility::zero3D;
			DirectX::XMVECTOR ndB = Utility::zero3D;;

			CheckCollision(colA , colB , ndA);
			CheckCollision(colB , colA , ndB);

			UpdateVelocity(colA, ndA);
			UpdateVelocity(colB, ndB);
		
		}
	}
}

bool ReturnFarthestPoint(DirectX::XMVECTOR point, std::vector<DirectX::XMVECTOR> dif , DirectX::XMVECTOR& dest)
{
	auto vecToOrigin = DirectX::XMVectorNegate(point);

	vecToOrigin = DirectX::XMVector3Normalize(vecToOrigin);

	float maxLength = -FLT_MAX;

	int index = 0;

	for (int i = 0;i < dif.size();i++)
	{
		auto vecSub = DirectX::XMVectorSubtract(dif[i], point);

		float dot = DirectX::XMVector3Dot(vecToOrigin, vecSub).m128_f32[0];

		if (dot > maxLength)
		{
			maxLength = dot;

			index = i;
		}
	}

	dest = dif[index];

	if (maxLength <= 0 || DirectX::XMVector3Equal(point , dif[index]))
	{
		return true;
	}
	
	return false;
}

/// <summary>
/// 射影係数を求めて最近接点をどこにあるかを調べる
/// </summary>
/// <param name="p1">ポイント</param>
/// <param name="p2">ポイント</param>
/// <returns>最近接点の座標</returns>
DirectX::XMVECTOR ReturnNearestPoint(DirectX::XMVECTOR p1 , DirectX::XMVECTOR p2)
{
	auto subVec = DirectX::XMVectorSubtract(p2 , p1);

	auto negateVec = DirectX::XMVectorNegate(p1);

	float length = DirectX::XMVector3LengthSq(subVec).m128_f32[0];

	float dot = DirectX::XMVector3Dot(subVec , negateVec).m128_f32[0];

	float t = dot / length;

	if (t >= 1)
		return p2;
	else if (t <= 0)
		return p1;
	else
	{
		return DirectX::XMVectorAdd(p1, DirectX::XMVectorScale(subVec, t));
	}
}

/// <summary>
/// 3つの頂点で形成された三角形内で原点の最近接点を求める
/// </summary>
/// <param name="p1">頂点1</param>
/// <param name="p2">頂点2</param>
/// <param name="p3">頂点3</param>
/// <returns>最近接点</returns>
DirectX::XMVECTOR ReturnNearestPointInTriangle(const DirectX::XMVECTOR p1, const DirectX::XMVECTOR p2, const DirectX::XMVECTOR p3)
{
	auto abVec = DirectX::XMVectorSubtract(p2, p1);
	auto acVec = DirectX::XMVectorSubtract(p3, p1);
	auto toOriginVec = DirectX::XMVectorNegate(p1);

	auto n = DirectX::XMVector3Cross(abVec, acVec);

	n = DirectX::XMVector3Normalize(n);

	auto dot = DirectX::XMVector3Dot(n, toOriginVec).m128_f32[0];

	auto multipliedN = DirectX::XMVectorScale(n, dot);

	auto projectionVec = DirectX::XMVectorSubtract(Utility::zero3D, multipliedN);
	projectionVec = DirectX::XMVectorSubtract(projectionVec, p1);

	auto dot00 = DirectX::XMVector3Dot(abVec, abVec).m128_f32[0];
	auto dot01 = DirectX::XMVector3Dot(abVec, acVec).m128_f32[0];
	auto dot02 = DirectX::XMVector3Dot(abVec, projectionVec).m128_f32[0];
	auto dot11 = DirectX::XMVector3Dot(acVec, acVec).m128_f32[0];
	auto dot12 = DirectX::XMVector3Dot(acVec, projectionVec).m128_f32[0];

	auto invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
	auto u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	auto v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	DirectX::XMVECTOR nearestPoint;

	if (u >= 0 && v >= 0 && u + v <= 1)
	{
		nearestPoint = DirectX::XMVectorSubtract(Utility::zero3D, multipliedN);
	}
	else
	{
		std::vector<DirectX::XMVECTOR> lengths;

		auto abNPoint = ReturnNearestPoint(p2, p1);
		auto bcNPoint = ReturnNearestPoint(p3, p2);
		auto caNPoint = ReturnNearestPoint(p1, p3);

		lengths.emplace_back(abNPoint);
		lengths.emplace_back(bcNPoint);
		lengths.emplace_back(caNPoint);
		lengths.emplace_back(p1);
		lengths.emplace_back(p2);
		lengths.emplace_back(p3);

		float minLength = FLT_MAX;

		int index = 0;

		for (int i = 0;i < lengths.size();i++)
		{
			float value = DirectX::XMVector3Length(lengths[i]).m128_f32[0];

			if (value < minLength)
			{
				minLength = value;

				index = i;
			}
		}

		nearestPoint = lengths[index];
	}

	return nearestPoint;
}

bool ColliderDetection::CheckCollision(std::shared_ptr<Collider> colA, std::shared_ptr<Collider> colB, DirectX::XMVECTOR& dest)
{
	auto difList = ReturnMinkowskiDif(colB->GetCurrentVerticesPos(), colA->GetCurrentVerticesPos());

	auto entityAPos = colA->attachFrom->GetWorldPosition();
	auto entityBPos = colB->attachFrom->GetWorldPosition();

	vecAtoB = DirectX::XMVectorSubtract(entityBPos, entityAPos);

	std::vector<DirectX::XMVECTOR> points;

	if (GJKAlgorithm(difList, points))
	{
		if(colA->attachFrom->name == "Player2")
		dest = EPAAlgorithm(points, difList);

		return true;
	}

	return false;
}

bool ColliderDetection::GJKAlgorithm(std::vector<DirectX::XMVECTOR> dif, std::vector<DirectX::XMVECTOR>& dest)
{
	std::vector<std::shared_ptr<DirectX::XMVECTOR>> points;

	// shared_ptr を使って動的に確保
	points.emplace_back(std::make_shared<DirectX::XMVECTOR>(Utility::zero3D));
	points.emplace_back(std::make_shared<DirectX::XMVECTOR>(Utility::zero3D));
	points.emplace_back(std::make_shared<DirectX::XMVECTOR>(Utility::zero3D));
	points.emplace_back(std::make_shared<DirectX::XMVECTOR>(Utility::zero3D));

	// それぞれの XMVECTOR にアクセスする方法
	auto p1 = points[0];
	auto p2 = points[1];
	auto p3 = points[2];
	auto p4 = points[3];

	//1,適当な点を選びその原点に向かって一番遠い点を求める
	{
		DirectX::XMVECTOR d = DirectX::XMVectorAdd(dif[0] , DirectX::XMVectorSet(Utility::ep, Utility::ep, Utility::ep,0));

		if (DirectX::XMVector3LengthSq(vecAtoB).m128_f32[0] > Utility::ep)
			d = vecAtoB;

		if (ReturnFarthestPoint(d, dif, *p1))
				return false;
	}

	//2,1で取得した点から原点に向かって遠い点を求める
	{
		if (ReturnFarthestPoint(*p1, dif, *p2))
			return false;
	}

	for (int repeat = 0;repeat < 100;repeat++)
	{

		//3,二つの求めた点で繋いだ辺から原点から最近接点を求めて、そこから原点に向かう方向で一番遠い点を求める
		{
			auto point = ReturnNearestPoint(*p1, *p2);

			if (DirectX::XMVector3Equal(point, Utility::zero3D))
			{
				if (abs(p1->m128_f32[0]) > 0)
				{
					point = DirectX::XMVectorSet(0, 1,0,0);
				}
				else if (abs(p1->m128_f32[1]) > 0)
				{
					point = DirectX::XMVectorSet(0, 0, 1, 0);
				}
				else if (abs(p1->m128_f32[2]) > 0)
				{
					point = DirectX::XMVectorSet(1, 0, 0, 0);
				}
			}

			if (ReturnFarthestPoint(point, dif, *p3))
				return false;
		}



		//4,次に四面体にするために原点から三角形への最近接点を求めて、そこ点から原点への方向で一番遠い点を求める
		{
			auto nearestPoint = ReturnNearestPointInTriangle(*p1 , *p2 , *p3);

			if (DirectX::XMVector3Equal(nearestPoint, Utility::zero3D))
			{
				auto n = DirectX::XMVector3Cross(DirectX::XMVectorSubtract(*p1,*p2) , DirectX::XMVectorSubtract(*p1, *p3));

				nearestPoint = DirectX::XMVectorAdd(nearestPoint , DirectX::XMVector3Normalize(n));
			}

			if (ReturnFarthestPoint(nearestPoint, dif, *p4))
				return false;
		}



		//5,四面体の中に原点が含まれるかを調べる、なければ6に進む
		{
			bool isOutbound = false;
			auto toOriginVec = DirectX::XMVectorNegate(*p1);

			float dotToD;
			float dotToO;

			DirectX::XMVECTOR n;

			//四面体を形成する３点の平面に対する法線ベクトルに残りの一点と原点の内積をとって互いに結果の値の符号が一致しなかった場合原点は含んでいないと見做す
			int indices[] = { 0, 1, 2, 3 };
			for (int i = 0; i < 4; ++i)
			{
				int a = indices[i];
				int b = indices[(i + 1) % 4];
				int c = indices[(i + 2) % 4];
				int d = indices[(i + 3) % 4];

				auto ab = DirectX::XMVectorSubtract(*points[b], *points[a]);
				auto ac = DirectX::XMVectorSubtract(*points[c], *points[a]);
				n = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(ab, ac));

				auto toOrigin = DirectX::XMVectorNegate(*points[a]);

				dotToD = DirectX::XMVector3Dot(n, DirectX::XMVectorSubtract(*points[d], *points[a])).m128_f32[0];
				dotToO = DirectX::XMVector3Dot(n, toOrigin).m128_f32[0];

				if ( (dotToD * dotToO) < 0)
				{
					isOutbound = true;
					break;
				}
			}

			if (!isOutbound)
			{
				dest.emplace_back(*p1);
				dest.emplace_back(*p2);
				dest.emplace_back(*p3);
				dest.emplace_back(*p4);

				return true;
			}
		
		}

		//6,もし原点が含まれていなかったら原点から一番遠い点を除外して3にもどる
		{
			std::vector<float> lengths;

			auto p1length = DirectX::XMVector3Length(*p1).m128_f32[0];
			auto p2length = DirectX::XMVector3Length(*p2).m128_f32[0];
			auto p3length = DirectX::XMVector3Length(*p3).m128_f32[0];
			auto p4length = DirectX::XMVector3Length(*p4).m128_f32[0];

			lengths.emplace_back(p1length);
			lengths.emplace_back(p2length);
			lengths.emplace_back(p3length);
			lengths.emplace_back(p4length);

			int index = 0;

			float maxLength = -FLT_MAX;

			for (int i = 0;i < lengths.size() - 1;i++)
			{
				for (int j = i+1;j < lengths.size();j++)
				{
					if (lengths[i] >= lengths[j])
					{
						auto temp1 = lengths[i];
						lengths[i] = lengths[j];
						lengths[j] = temp1;

						auto temp2 = *points[i];
						*points[i] = *points[j];
						*points[j] = temp2;
					}
				}
			}
		}
	}

	return false;
}

struct Triangle
{
	DirectX::XMVECTOR p1;
	DirectX::XMVECTOR p2;
	DirectX::XMVECTOR p3;

	DirectX::XMVECTOR n;

	DirectX::XMVECTOR toO;

	std::vector<std::pair<DirectX::XMVECTOR, DirectX::XMVECTOR>> edges;

	Triangle(DirectX::XMVECTOR a, DirectX::XMVECTOR b, DirectX::XMVECTOR c) : p1(a), p2(b), p3(c)
	{
		auto p1TOp2 = DirectX::XMVectorSubtract(p2 , p1);
		auto p1TOp3 = DirectX::XMVectorSubtract(p3 , p1);

		n = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(p1TOp2, p1TOp3));

		toO = DirectX::XMVectorNegate(p1);

		if (DirectX::XMVector3Dot(n, toO).m128_f32[0] > 0)
		{
			auto temp = p2;
			p2 = p3;
			p3 = temp;

			p1TOp2 = DirectX::XMVectorSubtract(p2, p1);
			p1TOp3 = DirectX::XMVectorSubtract(p3, p1);

			n = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(p1TOp2, p1TOp3));
		}

		edges = {
			std::make_pair(p1,p2),
		    std::make_pair(p2,p3),
	    	std::make_pair(p3,p1)
		};
	}
};

DirectX::XMVECTOR ColliderDetection::EPAAlgorithm(std::vector<DirectX::XMVECTOR> tetrahedron , std::vector<DirectX::XMVECTOR> dif)
{
	std::vector<Triangle> tries;

	auto p = tetrahedron;

	tries.emplace_back(Triangle(p[0] , p[1] , p[2]));
	tries.emplace_back(Triangle(p[1] , p[2] , p[3]));
	tries.emplace_back(Triangle(p[2], p[3], p[0]));
	tries.emplace_back(Triangle(p[3] , p[0] , p[1]));

	int repeatNum = 100;

	for (int repeat = 0;repeat < repeatNum; repeat++)
	{
		double minLength = FLT_MAX;
		int index = 0;

		for (int i = 0;i < tries.size();i++)
		{
			double val = fabs(DirectX::XMVector3Dot(tries[i].n, tries[i].toO).m128_f32[0]);

			if (fabs(minLength) > val)
			{
				minLength = DirectX::XMVector3Dot(tries[i].n, tries[i].toO).m128_f32[0];
				index = i;
			}
		}

		Triangle nearestTri = tries[index];

		auto nearestPoint = ReturnNearestPointInTriangle(nearestTri.p1, nearestTri.p2, nearestTri.p3);

		DirectX::XMVECTOR supportPoint = DirectX::XMVectorSet(0,0,0,0);

		bool isEnd = ReturnFarthestPoint(DirectX::XMVectorNegate(nearestPoint), dif , supportPoint);

		double length = DirectX::XMVector3Dot(supportPoint, nearestTri.n).m128_f32[0];

		if (length - minLength < Utility::ep || isEnd || repeat == repeatNum - 1)
		{

			return DirectX::XMVectorScale(nearestTri.n, -minLength + 0.001);
		}

		std::vector<Triangle> triesDelete;

		for (int i = 0;i < tries.size();i++)
		{
			if (DirectX::XMVector3Dot(tries[i].n, supportPoint).m128_f32[0] > 0)
			{
				triesDelete.emplace_back(tries[i]);

				tries.erase(tries.begin() + i);

				i--;
			}
		}


		if (tries.size() == 0)
		{
			Utility::Debug(supportPoint);
			std::cout << std::endl;


			return Utility::zero3D;
		}


		for (int i = 0;i < triesDelete.size() - 1;i++)
		{
			if (triesDelete.size() == 0)
				break;

			auto& edgesA = triesDelete[i].edges;

			for (int j = 0;j < edgesA.size();j++)
			{
				auto edgeA = edgesA[j];

				bool isDuplicated = false;

				for (int k = i+1;k < triesDelete.size();k++)
				{
					auto& edgesB = triesDelete[k].edges;

					for (int l = 0;l < edgesB.size();l++)
					{
						auto edgeB = edgesB[l];

						if (DirectX::XMVector3Equal(edgeA.second, edgeB.first) && DirectX::XMVector3Equal(edgeA.first, edgeB.second))
						{
							edgesB.erase(edgesB.begin() + l);
							l--;

							isDuplicated = true;
						}
						if (DirectX::XMVector3Equal(edgeA.second, edgeB.second) && DirectX::XMVector3Equal(edgeA.first, edgeB.first))
						{
							edgesB.erase(edgesB.begin() + l);
							l--;

							isDuplicated = true;
						}
					}
				}

				if (isDuplicated)
				{
					edgesA.erase(edgesA.begin() + j);
					j--;
				}
			}
		}

		for (int i = 0;i < triesDelete.size();i++)
		{
			auto edges = triesDelete[i].edges;

			for (auto edge : edges)
			{
				auto triangle = Triangle(edge.first, edge.second, supportPoint);

				if (DirectX::XMVector3Dot(triangle.n, triangle.toO).m128_f32[0] != 0)
				{
					tries.emplace_back(triangle);
				}

			}
		}



	}

	return Utility::zero3D;
}

