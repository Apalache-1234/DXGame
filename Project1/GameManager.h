#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <memory>
#include <iostream>
#include <vector>
#include "Entity.h"
#include "windows.h"
#include <sstream>

class GameManager
{
public:

	GameManager();

	~GameManager();

	static GameManager& GetInstance();

	std::vector<std::shared_ptr<Entity>>& GetEntityList();

	bool isExitNameEntity(std::string name);

	void SetEntity(std::shared_ptr<Entity> entity);

	std::shared_ptr<Entity> CreateEntity(std::string name);

	template <typename T, std::enable_if_t<std::is_base_of_v<Component, T>>* = nullptr>
	std::shared_ptr<Entity> CreateEntity(std::string name)
	{
		auto ent = CreateEntity(name);

		ent->AddComponent<T>();

		return ent;
	}

	void MainLoop();

	void PauseTime();

	void UnpauseTime();

	DirectX::XMFLOAT3 GetAmbientColor();

	void SetAmbientColor(DirectX::XMFLOAT3 col);

	/// <summary>
    /// 全てのEntityが現在持つ指定されたコンポーネントの配列を返す
    /// </summary>
    /// <typeparam name="T">返すタイプ</typeparam>
	template <typename T>
	std::vector<std::shared_ptr<T>> GetComponentList()
	{
		auto list = GetEntityList();

		std::vector<std::shared_ptr<T>> componentList;

		for (auto entity : list)
		{
			auto comp = entity->GetComponent<T>();

			if (comp != nullptr)
			{
				componentList.emplace_back(comp);
			}
		}

		return componentList;
	}

	UINT fps = 0;

	float GetGameTime();
	float GetGameDeltaTime();
	float GetRealTime();
	float GetRealDeltaTime();

private:
	/// <summary>
    /// 起動してからの総時間、ポーズの影響を受ける
    /// </summary>
	float GameTime = 0;

	/// <summary>
	/// 前回の処理と次の処理の間の秒数、ポーズの影響を受ける
	/// </summary>
	float GameDeltaTime = 0;

	/// <summary>
	/// 起動してからの総時間、ポーズの影響を受けない
	/// </summary>
	float RealTime = 0;

	/// <summary>
	/// 前回の処理と次の処理の間の秒数、ポーズの影響を受けない
	/// </summary>
	float RealDeltaTime = 0;

	bool pause = false;

	/// <summary>
	/// 現在生成されているエンティティ
	/// </summary>
	std::vector<std::shared_ptr<Entity>> entities;

	/// <summary>
	/// ゲーム上のあらゆる情報を確保するGameManagerクラスのシングルトンのインスタンス
	/// </summary>
	static std::unique_ptr<GameManager> instance;

	/// <summary>
	/// cpuの一秒間にカウントするクロック数
	/// </summary>
	LARGE_INTEGER freq;

	/// <summary>
    /// cpuが今までカウントした総クロック数
    /// </summary>
	LARGE_INTEGER base;

	LARGE_INTEGER baseReal;

	LARGE_INTEGER pauseClocks;

	LARGE_INTEGER unpauseClocks;

	DirectX::XMFLOAT3 ambientColor = DirectX::XMFLOAT3(1,1,1);

	/// <summary>
	/// 前回のGameTime
	/// </summary>
	float prevGameTime = 0;

	float prevRealTime = 0;
};

#endif


