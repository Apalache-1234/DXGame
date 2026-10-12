#include <functional>
#include "SystemManager.h"
#include <algorithm>
#include "ITween.h"
#include "Utility.h"
#include <iostream>
#include <cmath>

#ifndef TWEEN
#define TWEEN

enum class EasingType {
	EaseInSine,
	EaseOutSine,
	EaseInOutSine,

	EaseInQuad,
	EaseOutQuad,
	EaseInOutQuad,

	EaseInCubic,
	EaseOutCubic,
	EaseInOutCubic,

	EaseInQuart,
	EaseOutQuart,
	EaseInOutQuart,

	EaseInQuint,
	EaseOutQuint,
	EaseInOutQuint,

	EaseInExpo,
	EaseOutExpo,
	EaseInOutExpo,

	EaseInCirc,
	EaseOutCirc,
	EaseInOutCirc,

	EaseInBack,
	EaseOutBack,
	EaseInOutBack,

	EaseInElastic,
	EaseOutElastic,
	EaseInOutElastic,

	EaseInBounce,
	EaseOutBounce,
	EaseInOutBounce,

	EaseLine
};

template<typename T>
class Tween : public ITween
{
public:
	Tween(T from , T to , EasingType type = EasingType::EaseOutSine , float time = 1, bool isOnRealTime = false) 
	{
		this->from = from;

		value = from;

		this->to = to;

		this->type = type;

		SetTime(time);

		onRealTime = isOnRealTime;

		isPlaying = true;
	}

	Tween()
	{

	}

	~Tween()
	{

	}

	void SetTime(float time)
	{
		this->time = std::clamp(time, 0.0f, FLT_MAX);
	}

	void ProcessTime() override
	{
		if (isPlaying == false)
		{
			return;
		}
		
		if (SystemManager::GetInstance().GetIsPause() && !onRealTime)
		{
			return;
		}

		if (currentTime == 0 && startFunc)
		{
			startFunc();
		}
		
		if (updateFunc && currentTime < time )
		{
			updateFunc();
		}

		float result = EasingFunction();

		if (currentTime >= time && !isLooping && !isEnd)
		{
			isEnd = true;
			isPlaying = false;
			if (updateFunc)
				updateFunc();

			if (completeFunc)
				completeFunc();
		}

		if (onRealTime)
			currentTime += SystemManager::GetInstance().GetRealDeltaTime();
		else
			currentTime += SystemManager::GetInstance().GetGameDeltaTime();

		if (isLooping)
			currentTime = std::fmod(currentTime, time);
		else
			currentTime = std::clamp(currentTime, 0.0f, time);

		if constexpr (std::is_same_v<T, int>) 
		{
			value = std::lerp(from, to, result);
		}
		else if constexpr (std::is_same_v<T, float>)
		{
			value = std::lerp(from, to, result);
		}
		else if constexpr (std::is_same_v<T, DirectX::XMVECTOR>)
		{
			value = DirectX::XMVectorLerp(from , to , result);
		}
		else {
			std::cout << "その型はサポートしていない" << std::endl;
			assert(true);
		}
	}

	void OnStart(std::function<void()> func)
	{
		startFunc = func;
	}

	void OnUpdate(std::function<void()> func)
	{
		updateFunc = func;
	}

	void OnComplete(std::function<void()> func)
	{
		completeFunc = func;
	}

	T GetValue()
	{
		return value;
	}

	float GetTime()
	{
		return time;
	}

	float GetCurrentTime()
	{
		return currentTime;
	}

	bool GetIsEnd()
	{
		return isEnd;
	}

	void Pause()
	{
		isPlaying = false;
	}

	void Play()
	{
		if (isEnd)
		{
			isEnd = false;
			currentTime = 0;
		}
		isPlaying = true;
	}

	void Replay()
	{
		isPlaying = true;
		currentTime = 0;
	}

	bool isLooping = false;

	bool onRealTime = false;
private:
	float EasingFunction() {

		float t = currentTime / time;

		switch (type) {
		case EasingType::EaseInSine:      return easeInSine(t);
		case EasingType::EaseOutSine:     return easeOutSine(t);
		case EasingType::EaseInOutSine:   return easeInOutSine(t);

		case EasingType::EaseInQuad:      return easeInQuad(t);
		case EasingType::EaseOutQuad:     return easeOutQuad(t);
		case EasingType::EaseInOutQuad:   return easeInOutQuad(t);

		case EasingType::EaseInCubic:     return easeInCubic(t);
		case EasingType::EaseOutCubic:    return easeOutCubic(t);
		case EasingType::EaseInOutCubic:  return easeInOutCubic(t);

		case EasingType::EaseInQuart:     return easeInQuart(t);
		case EasingType::EaseOutQuart:    return easeOutQuart(t);
		case EasingType::EaseInOutQuart:  return easeInOutQuart(t);

		case EasingType::EaseInQuint:     return easeInQuint(t);
		case EasingType::EaseOutQuint:    return easeOutQuint(t);
		case EasingType::EaseInOutQuint:  return easeInOutQuint(t);

		case EasingType::EaseInExpo:      return easeInExpo(t);
		case EasingType::EaseOutExpo:     return easeOutExpo(t);
		case EasingType::EaseInOutExpo:   return easeInOutExpo(t);

		case EasingType::EaseInCirc:      return easeInCirc(t);
		case EasingType::EaseOutCirc:     return easeOutCirc(t);
		case EasingType::EaseInOutCirc:   return easeInOutCirc(t);

		case EasingType::EaseInBack:      return easeInBack(t);
		case EasingType::EaseOutBack:     return easeOutBack(t);
		case EasingType::EaseInOutBack:   return easeInOutBack(t);

		case EasingType::EaseInElastic:   return easeInElastic(t);
		case EasingType::EaseOutElastic:  return easeOutElastic(t);
		case EasingType::EaseInOutElastic:return easeInOutElastic(t);

		case EasingType::EaseInBounce:    return easeInBounce(t);
		case EasingType::EaseOutBounce:   return easeOutBounce(t);
		case EasingType::EaseInOutBounce: return easeInOutBounce(t);

		case EasingType::EaseLine: return t;

		default: return 9999;
		}
	}

	float easeInSine(float t) {
		return 1.0f - cosf((t * Utility::PI) / 2.0f);
	}

	float easeOutSine(float t) {
		return sinf((t * Utility::PI) / 2.0f);
	}

	float easeInOutSine(float t) {
		return -(cosf(Utility::PI * t) - 1.0f) / 2.0f;
	}

	float easeInQuad(float t) {
		return t * t;
	}

	float easeOutQuad(float t) {
		return 1.0f - (1.0f - t) * (1.0f - t);
		// または return t * (2 - t);
	}

	float easeInOutQuad(float t) {
		return (t < 0.5f) ? (2.0f * t * t) : (1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f);
	}

	float easeInCubic(float t) {
		return t * t * t;
	}

	float easeOutCubic(float t) {
		float p = 1.0f - t;
		return 1.0f - (p * p * p);
	}

	float easeInOutCubic(float t) {
		return (t < 0.5f)
			? 4.0f * t * t * t
			: 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
	}

	float easeInQuart(float t) {
		return t * t * t * t;
	}

	float easeOutQuart(float t) {
		float p = 1.0f - t;
		return 1.0f - (p * p * p * p);
	}

	float easeInOutQuart(float t) {
		return (t < 0.5f)
			? 8.0f * t * t * t * t
			: 1.0f - powf(-2.0f * t + 2.0f, 4.0f) / 2.0f;
	}

	float easeInQuint(float t) {
		return t * t * t * t * t;
	}

	float easeOutQuint(float t) {
		float p = 1.0f - t;
		return 1.0f - (p * p * p * p * p);
	}

	float easeInOutQuint(float t) {
		return (t < 0.5f)
			? 16.0f * t * t * t * t * t
			: 1.0f - powf(-2.0f * t + 2.0f, 5.0f) / 2.0f;
	}

	float easeInExpo(float t) {
		return (t == 0.0f) ? 0.0f : powf(2.0f, 10.0f * (t - 1.0f));
	}

	float easeOutExpo(float t) {
		return (t == 1.0f) ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
	}

	float easeInOutExpo(float t) {
		if (t == 0.0f) return 0.0f;
		if (t == 1.0f) return 1.0f;
		return (t < 0.5f)
			? powf(2.0f, 20.0f * t - 10.0f) / 2.0f
			: (2.0f - powf(2.0f, -20.0f * t + 10.0f)) / 2.0f;
	}

	float easeInCirc(float t) {
		return 1.0f - sqrtf(1.0f - t * t);
	}

	float easeOutCirc(float t) {
		return sqrtf(1.0f - (t - 1.0f) * (t - 1.0f));
	}

	float easeInOutCirc(float t) {
		return (t < 0.5f)
			? (1.0f - sqrtf(1.0f - 4.0f * t * t)) / 2.0f
			: (sqrtf(1.0f - powf(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f;
	}

	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;

	float easeInBack(float t) {
		return c3 * t * t * t - c1 * t * t;
	}

	float easeOutBack(float t) {
		float p = t - 1.0f;
		return 1.0f + c3 * p * p * p + c1 * p * p;
	}

	float easeInOutBack(float t) {
		const float c2 = c1 * 1.525f;
		return (t < 0.5f)
			? (powf(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f
			: (powf(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
	}

	float easeInElastic(float t) {
		if (t == 0.0f) return 0.0f;
		if (t == 1.0f) return 1.0f;
		return -powf(2.0f, 10.0f * t - 10.0f) * sinf((t * 10.0f - 10.75f) * (2 * Utility::PI / 3.0f));
	}

	float easeOutElastic(float t) {
		if (t == 0.0f) return 0.0f;
		if (t == 1.0f) return 1.0f;
		return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * (2 * Utility::PI / 3.0f)) + 1.0f;
	}

	float easeInOutElastic(float t) {
		if (t == 0.0f) return 0.0f;
		if (t == 1.0f) return 1.0f;
		if (t < 0.5f) {
			return -0.5f * powf(2.0f, 20.0f * t - 10.0f) * sinf((20.0f * t - 11.125f) * (2 * Utility::PI / 4.5f));
		}
		else {
			return powf(2.0f, -20.0f * t + 10.0f) * sinf((20.0f * t - 11.125f) * (2 * Utility::PI / 4.5f)) * 0.5f + 1.0f;
		}
	}

	float easeOutBounce(float t) {
		const float n1 = 7.5625f;
		const float d1 = 2.75f;

		if (t < 1.0f / d1) {
			return n1 * t * t;
		}
		else if (t < 2.0f / d1) {
			t -= 1.5f / d1;
			return n1 * t * t + 0.75f;
		}
		else if (t < 2.5f / d1) {
			t -= 2.25f / d1;
			return n1 * t * t + 0.9375f;
		}
		else {
			t -= 2.625f / d1;
			return n1 * t * t + 0.984375f;
		}
	}

	float easeInBounce(float t) {
		return 1.0f - easeOutBounce(1.0f - t);
	}

	float easeInOutBounce(float t) {
		return (t < 0.5f)
			? (1.0f - easeOutBounce(1.0f - 2.0f * t)) / 2.0f
			: (1.0f + easeOutBounce(2.0f * t - 1.0f)) / 2.0f;
	}



	std::function<void()> startFunc;

	std::function<void()> updateFunc;

	std::function<void()> completeFunc;

	EasingType type;

	T value;

	T from;

	T to;

	float currentTime = 0;

	bool isPlaying = false;

	bool isEnd = false;

	float time = 0;
};

#endif // !TWEEN