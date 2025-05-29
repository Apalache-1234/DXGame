#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <list>
#include <typeindex>
#include <memory>
#include <iostream>
#include <unordered_map>
#include "Object.h"
#include "Component.h"
#include <DirectXMath.h>
#include <type_traits>

class Component;

class Entity : public Object
{
public:

	Entity(const std::string& name);

	~Entity();

	void Init();

	/// <summary>
	/// ワールド座標をXMVECTOR形式で返す
	/// </summary>
	/// <returns>ワールド座標</returns>
	DirectX::XMVECTOR GetWorldPosition();

	/// <summary>
    /// ローカル座標をXMVECTOR形式で返す
    /// </summary>
    /// <returns>ローカル座標</returns>
	DirectX::XMVECTOR GetLocalPosition();

	/// <summary>
    /// ワールドの大きさをXMVECTOR形式で返す
    /// </summary>
    /// <returns>ワールドの大きさ</returns>
	DirectX::XMVECTOR GetWorldScale();

	/// <summary>
	/// ローカルの大きさをXMVECTOR形式で返す
	/// </summary>
	/// <returns>ローカルの大きさ</returns>
	DirectX::XMVECTOR GetLocalScale();

	/// <summary>
    /// ワールドの大きさとローカルの大きさを考慮した実際の大きさをXMVECTOR形式で返す
    /// </summary>
    /// <returns>ローカルの大きさ</returns>
	DirectX::XMVECTOR GetScale();

	/// <summary>
    /// ワールドの回転をXMVECTOR形式でEulerで返す
    /// </summary>
    /// <returns>ワールドの回転</returns>
	DirectX::XMVECTOR GetWorldRotation();

	/// <summary>
	/// ワールドの回転をXMVECTOR形式で四元数で返す
	/// </summary>
	/// <returns>ワールドの回転</returns>
	DirectX::XMVECTOR GetWorldQuaternion();

	/// <summary>
    /// ローカルの回転をXMVECTOR形式でEulerで返す
    /// </summary>
    /// <returns>ローカルの回転</returns>
	DirectX::XMVECTOR GetLocalRotation();

	/// <summary>
	/// ローカルの回転をXMVECTOR形式で四元数で返す
	/// </summary>
	/// <returns>ローカルの回転</returns>
	DirectX::XMVECTOR GetLocalQuaternion();

	/// <summary>
	/// このオブジェクトの右方向
	/// </summary>
	/// <returns>右方向</returns>
	DirectX::XMVECTOR GetRight();

	/// <summary>
    /// このオブジェクトの上方向
    /// </summary>
    /// <returns>上方向</returns>
	DirectX::XMVECTOR GetUp();

	/// <summary>
    /// このオブジェクトの前方向
    /// </summary>
    /// <returns>前方向</returns>
	DirectX::XMVECTOR GetForward();

	/// <summary>
	/// ワールド座標をセットする
	/// </summary>
	/// <param name="position">セットする座標</param>
	void SetWorldPosition(DirectX::XMVECTOR position);

	/// <summary>
	/// 回転をクォータニオンでセットする
	/// </summary>
	/// <param name="q">セットするクォータニオン</param>
	void SetQuaternion(DirectX::XMVECTOR q);

	/// <summary>
	/// ローカル座標をセットする
	/// </summary>
	/// <param name="position">セットするローカル座標</param>
	void SetLocalPosition(DirectX::XMVECTOR position);

	/// <summary>
	/// ワールドの大きさをセットする
	/// </summary>
	/// <param name="scale">セットするワールドの大きさ</param>
	void SetWorldScale(DirectX::XMVECTOR scale);

	/// <summary>
	/// ローカルの大きさをセットする
	/// </summary>
	/// <param name="scale">セットするローカルの大きさ</param>
	void SetLocalScale(DirectX::XMVECTOR scale);

	/// <summary>
	/// ワールド回転をセットする
	/// </summary>
	/// <param name="rotation">セットするワールド回転</param>
	void SetWorldRotation(DirectX::XMVECTOR rotation);

	/// <summary>
	/// ローカル回転をセットする
	/// </summary>
	/// <param name="rotation">セットするローカル回転</param>
	void SetLocalRotation(DirectX::XMVECTOR rotation);

	/// <summary>
	/// ワールド座標を足す
	/// </summary>
	/// <param name="position">足すワールド座標</param>
	void AddWorldPosition(DirectX::XMVECTOR position);

	/// <summary>
	/// ローカル座標を足す
	/// </summary>
	/// <param name="position">足すローカル座標</param>
	void AddLocalPosition(DirectX::XMVECTOR position);

	/// <summary>
	/// ワールド回転を足す
	/// </summary>
	/// <param name="rotation">足すワールド回転</param>
	void AddWorldRotation(DirectX::XMVECTOR rotation);

	/// <summary>
	/// ローカル回転を足す
	/// </summary>
	/// <param name="rotation">足すローカル回転</param>
	void AddLocalRotation(DirectX::XMVECTOR rotation);

	/// <summary>
	/// ワールドの大きさを足す
	/// </summary>
	/// <param name="scale">足すワールドの大きさ</param>
	void AddWorldScale(DirectX::XMVECTOR scale);

	void Destroy() override;

	/// <summary>
	/// ローカルの大きさを足す
	/// </summary>
	///<param name="scale">足すローカルの大きさ</param>
    void AddLocalScale(DirectX::XMVECTOR scale);


	/// <summary>
	/// コンポーネントを追加する
	/// </summary>
	/// <typeparam name="T">追加するタイプ</typeparam>
	template <typename T , typename... Args , std::enable_if_t<std::is_base_of_v<Component, T>>* = nullptr>
	void AddComponent(Args&&... args )
	{
		if (components.find(typeid(T)) == components.end())
		{
			// shared_from_this()を使用してEntityのshared_ptrを作成
			std::shared_ptr<Entity> ent = std::dynamic_pointer_cast<Entity>(shared_from_this());

			// コンポーネントを作成してマップに追加
			components[typeid(T)] = std::make_shared<T>(ent , std::forward<Args>(args)...);

			// コンポーネントの初期化
			components[typeid(T)]->Init();
		}
	}

	/// <summary>
    /// コンポーネントのシェアドポインタを返す、存在しない場合はnullptrを返す
    /// </summary>
    /// <typeparam name="T">返すタイプ</typeparam>
	template <typename T ,  std::enable_if_t<std::is_base_of_v<Component, T>>* = nullptr>
	std::shared_ptr<T> GetComponent()
	{
		if (components.find(typeid(T)) != components.end())
		{
			return  std::dynamic_pointer_cast<T>(components[typeid(T)]);
		}
		else
		{
			return nullptr;
		}
	}

	template <typename T , std::enable_if_t<std::is_base_of_v<Component, T>>* = nullptr>
	void RemoveComponent()
	{
		if (components.find(typeid(T)) != components.end())
		{
			components.erase(typeid(T));
		}
	}

private:
	/// <summary>
	/// 右、上、前のベクトルを現在の回転に合わせる
	/// </summary>
	void AdjustDirectionVector();

	/// <summary>
    /// ワールドの座標を表したベクトル
    /// </summary>
	DirectX::XMVECTOR worldPosition;

	/// <summary>
    /// ローカルの座標を表したベクトル
    /// </summary>
	DirectX::XMVECTOR localPosition;

	/// <summary>
    /// ワールドの大きさをを表したベクトル
    /// </summary>
	DirectX::XMVECTOR worldScale;

	/// <summary>
    /// ローカルの大きさを表したベクトル
    /// </summary>
	DirectX::XMVECTOR localScale;

	/// <summary>
    /// オブジェクト右の単位ベクトル
    /// </summary>
	DirectX::XMVECTOR right = DirectX::XMVectorSet(1, 0, 0, 0);

	/// <summary>
    /// オブジェクト上の単位ベクトル
    /// </summary>
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);

	/// <summary>
	/// オブジェクトの前の単位ベクトル
	/// </summary>
	DirectX::XMVECTOR forward = DirectX::XMVectorSet(0, 0, 1, 0);

	/// <summary>
	/// ローカルの回転をEulerで表したベクトル
	/// </summary>
	DirectX::XMVECTOR localRotation;

	/// <summary>
	/// ワールドの回転をEulerで表したベクトル
	/// </summary>
	DirectX::XMVECTOR worldRotation;

	/// <summary>
	/// コンポーネントの辞書型配列
	/// </summary>
	std::unordered_map<std::type_index, std::shared_ptr<Component>> components;
};

#endif


