#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <memory>
#include <iostream>
#include <vector>
#include "Entity.h"
#include "windows.h"

class GameManager
{
public:

	GameManager();

	~GameManager();

	static GameManager& GetInstance();

	std::vector<std::shared_ptr<Entity>>& GetEntityList();

	void SetEntity(std::shared_ptr<Entity> entity);

	void MainLoop();

	void PauseTime();

	void UnpauseTime();

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

	/// <summary>
    /// 起動してからの総時間
    /// </summary>
	float GameTime = 0;

	/// <summary>
	/// 前回の処理と次の処理の間の秒数
	/// </summary>
	float DeltaTime = 0;

private:
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

	LARGE_INTEGER pauseClocks;

	LARGE_INTEGER unpauseClocks;

	/// <summary>
	/// 前回のGameTime
	/// </summary>
	float prevTime = 0;
};

#endif


