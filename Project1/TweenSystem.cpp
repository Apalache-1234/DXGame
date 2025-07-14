#include "TweenSystem.h"

auto Delete = [](std::shared_ptr<ITween> n) { return n == nullptr; };

TweenSystem::TweenSystem()
{
}

TweenSystem::~TweenSystem()
{
}

std::unique_ptr<TweenSystem> TweenSystem::instance = nullptr;

TweenSystem& TweenSystem::GetInstance()
{
	if (!instance) {
		instance.reset(new TweenSystem());
	}
	return *instance;
}

void TweenSystem::Update()
{
	for (auto& t : tween)
	{
		if (t.use_count() == 1)
		{
			t = nullptr;
			continue;
		}
	
		t->ProcessTime();
	}

	tween.erase(remove_if(tween.begin(), tween.end(), Delete), tween.end());
}


