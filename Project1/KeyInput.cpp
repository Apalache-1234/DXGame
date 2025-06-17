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

    auto x = std::clamp(static_cast<int>(ptClient.x) , 0 , Window::width);
    auto y = std::clamp(static_cast<int>(ptClient.y) , 0 , Window::height);

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

    for (int vk = 0; vk < 256; ++vk) {
        SHORT state = GetAsyncKeyState(vk);
        keyInputsCur[vk] = (state & 0x8000) != 0;
    }
}
