#include "KeyInput.h"
#include <Windows.h>
#include <bitset>

std::bitset<256> keyInputsCur;
std::bitset<256> keyInputsPrev;

KeyInput::KeyInput()
{
}

KeyInput::~KeyInput()
{
}

std::unique_ptr<KeyInput> KeyInput::instance = nullptr;

KeyInput& KeyInput::GetInstance()
{
    if (!instance) {
        instance.reset(new KeyInput());
    }
    return *instance;
}

bool KeyInput::GetKey(Key key)
{
    return keyInputsCur[key] && !keyInputsPrev[key];
}

bool KeyInput::GetKeyDown(Key key)
{
    return keyInputsCur[key];
 }

bool KeyInput::GetKeyUp(Key key)
{
    return !keyInputsCur[key] && keyInputsPrev[key];
}

void KeyInput::Update()
{
    keyInputsPrev = keyInputsCur;

    for (int vk = 0; vk < 256; ++vk) {
        SHORT state = GetAsyncKeyState(vk);
        keyInputsCur[vk] = (state & 0x8000) != 0;
    }
}
