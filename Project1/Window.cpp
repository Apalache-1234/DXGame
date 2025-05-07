#include "Window.h"
#include "imgui_impl_win32.h"

Window::Window() : hInstance(GetModuleHandle(NULL)){}

Window::~Window(){}

Window& Window::GetInstance() {
	if (!instance) {
		instance.reset(new Window());
	}
	return *instance;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, nMsg, wParam, lParam))
		return true;

	switch (nMsg)
	{
	case WM_PAINT:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcA(hWnd, nMsg, wParam, lParam);

	}

	return 0;
}

bool Window::MainLoop()
{
	MSG msg;

	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return false;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return true;
}

bool Window::Init()
{
	WNDCLASSEX wnd = 
	{
    sizeof(WNDCLASSEX),		// 構造体のサイズ
 	CS_HREDRAW | CS_VREDRAW,	// クラスのスタイル
	WindowProc,		// ウィンドウプロシージャ
	0,				// 補助メモリ
	0,				// 補助メモリ
	hInstance,			// このアプリのインスタンスハンドル
	LoadIcon(NULL, IDI_APPLICATION),// アイコン画像
	LoadCursor(NULL, IDC_ARROW),	// カーソル画像
	NULL,				// 背景ブラシ(背景色)
	NULL,				// メニュー名
	_T("CreateWindow"),		// クラス名							
	NULL				// 小さいアイコン
	};

	wnd.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);

	if (!RegisterClassEx(&wnd))
	{
		MessageBox(NULL,
			_T("RegisterClassExの処理に失敗しました"),
			_T("Sample02"),
			NULL);

		return false;
	}

	// ウィンドウ作成
	window_handle = CreateWindow(
		// 登録しているウィンドウクラス構造体の名前
		_T("CreateWindow"),
		// ウィンドウ名(タイトルバーに表示される)
		_T("ウィンドウ生成サンプル"),
		// ウィンドウスタイル
		(WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME),
		// 表示位置
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		// サイズ
		width,
		height,
		// 親ハンドル
		NULL,
		// メニューハンドル
		NULL,
		// インスタンスハンドル
		hInstance,
		// WM_CREATEメッセージでlpparamに渡したい値
		NULL);



	// ウィンドウを表示状態にする
	ShowWindow(window_handle, SW_SHOW);

	UpdateWindow(window_handle);

	SetFocus(window_handle);

	return true;
}

std::unique_ptr<Window> Window::instance = nullptr;

HWND Window::GetWH()
{
	return window_handle;
}

