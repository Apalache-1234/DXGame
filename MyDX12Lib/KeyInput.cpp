#include "PCH/pch.h"
#include "KeyInput.h"
#include "Window.h"
#include <bitset>
#include <algorithm>

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

DirectX::XMFLOAT2 KeyInput::GetMousePos()
{
    POINT ptScreen;
    ::GetCursorPos(&ptScreen);
    POINT ptClient = ptScreen;
    ::ScreenToClient(Window::GetInstance().GetWH(), &ptClient);

    auto x = std::clamp(static_cast<float>(ptClient.x) , 0.0f , Window::width);
    auto y = std::clamp(static_cast<float>(ptClient.y) , 0.0f, Window::height);

    return DirectX::XMFLOAT2(x, Window::height - y);
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

    BYTE keyState[256];

    if (GetKeyboardState(keyState))
    {
        for (int vk = 0; vk < 256; ++vk) {
            keyInputsCur[vk] = (keyState[vk] & 0x80) != 0;
        }
    }
}
