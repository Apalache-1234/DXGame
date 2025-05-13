#ifndef COLLIDER_DETECTION_H
#define COLLIDER_DETECTION_H

#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <DirectXMath.h>
#include "Component.h"
#include "GameManager.h"
#include "Entity.h"
#include "Collider.h"
#include "Utility.h"
#include "Renderer.h"
#include "Rigidbody.h"

class ColliderDetection
{
public:
	ColliderDetection();

	~ColliderDetection();

	static ColliderDetection& GetInstance();

	/// <summary>
	/// コライダーが接触しているかを調べるために必要なメインループ
	/// </summary>
	void MainLoop();

private:

	/// <summary>
	/// コライダーが接触しているかを判定する
	/// </summary>
	/// <param name="colA">コライダー</param>
	/// <param name="colB">もう片方のコライダー</param>
	/// <param name="dest">貫通深度を掛けた法線を返す</param>
	/// <returns>接触したかどうかを返す</returns>
	bool CheckCollision(std::shared_ptr<Collider> colA, std::shared_ptr<Collider> colB, DirectX::XMVECTOR& dest);

	/// <summary>
	/// GJKアルゴリズムを用いて接触しているかを調べる
	/// </summary>
	/// <param name="dif">ミンコフスキーの差を入れたXMVECTOR形式の配列</param>
	///  <param name="dest">GJKアルゴリズムによって導出した原点を含んだ四面体を格納する配列</param>
	/// <returns>接触しているかどうかをboolで返す</returns>
	bool GJKAlgorithm(std::vector<DirectX::XMVECTOR> dif, std::vector<DirectX::XMVECTOR>& dest);

	DirectX::XMVECTOR EPAAlgorithm(std::vector<DirectX::XMVECTOR> tetrahedron, std::vector<DirectX::XMVECTOR> dif);

	/// <summary>
	/// 当たり判定を形成するColliderクラス同士が接触しているかを調べるためのクラスのシングルトンのインスタンス
	/// </summary>
	static std::unique_ptr<ColliderDetection> instance;
};
#endif