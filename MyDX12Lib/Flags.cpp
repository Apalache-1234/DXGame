#include "PCH/pch.h"
#include "Flags.h"

Flags::Flags()
{
}

Flags::~Flags()
{
}

std::unique_ptr<Flags> Flags::instance = nullptr;

Flags& Flags::GetInstance()
{
	if (!instance) {
		instance.reset(new Flags());
	}
	return *instance;
}
