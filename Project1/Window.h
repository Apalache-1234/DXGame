#include <windows.h>
#include <memory>
#include <tchar.h>

#ifndef WINDOW_H
#define WINDOW_H



class Window
{
public:
	Window();

	~Window();

	bool Init();

	bool MainLoop();

	bool Term();

	HWND GetWH();

	/// <summary>
	/// シングルトンのインスタンスを返す
	/// </summary>
	/// <returns>シングルトンのポインタ</returns>
	static Window& GetInstance();

	//画面の縦幅
	const unsigned int height = 1080;

	//画面の横幅
	const unsigned int width = 1980;

private:
   	/// <summary>
    /// ウィンドウクラスのシングルトンのインスタンス
    /// </summary>
	static std::unique_ptr<Window> instance;

	HINSTANCE hInstance;

	HWND window_handle;
};

#endif
