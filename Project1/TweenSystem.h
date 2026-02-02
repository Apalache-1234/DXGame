#ifndef TWEEN_SYSTEM
#define TWEEN_SYSTEM

#include <unordered_map>
#include <functional>
#include "Tween.h"

class TweenSystem
{
public:
	TweenSystem();

	~TweenSystem();

	void Update();

	static TweenSystem& GetInstance();

	template<typename T>
	std::shared_ptr<Tween<T>> DoTween(T from, T to, EasingType type = EasingType::EaseOutSine, float time = 1, bool isOnRealTime = false)
	{
		std::shared_ptr<ITween> base_tween = std::make_shared<Tween<T>>(from, to, type, time, isOnRealTime);

		tween.push_back(base_tween);

		return std::dynamic_pointer_cast<Tween<T>>(tween.back());
	}
private:
	/// <summary>
    /// ゲーム上のTweenを確保するTweenSystemクラスのシングルトンのインスタンス
    /// </summary>
	static std::unique_ptr<TweenSystem> instance;

	std::vector<std::shared_ptr<ITween>> tween;
};

#endif






