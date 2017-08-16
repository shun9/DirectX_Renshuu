#include <Windows.h>

#include "SL_Window.h"
#include "SL_Graphics.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMag, WPARAM wParam, LPARAM lParam);

HRESULT InitWindow(HINSTANCE hInst);
HRESULT InitGraphics();

/// <summary>
/// エントリーポイント
/// ・ここからプログラムが始まる
/// ・ウィンドウを作成
/// ・メッセージループの実装
/// </summary>
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR szStr, INT iCmpShow)
{
	auto window = ShunLib::Window::GetInstance();
	
	//ウィンドウ作成
	if (SUCCEEDED(InitWindow(hInst)))
	{
		//DirectX関連の初期化
		if (SUCCEEDED(InitGraphics()))
		{
			//作成に成功したらループ
			window->Run();
		}
	}
	ShunLib::Graphics::Destroy();
	ShunLib::Window::Destroy();

	//終了
	return 0;
}


/// <summary>
/// ウィンドウプロシージャ
/// </summary>
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMag, WPARAM wParam, LPARAM lParam)
{
	auto window = ShunLib::Window::GetInstance();
	return window->MsgProc(hWnd, iMag, wParam, lParam);
}


/// <summary>
/// ウィンドウ初期化
/// </summary>
HRESULT InitWindow(HINSTANCE hInst)
{
	auto window = ShunLib::Window::GetInstance();

	//ウィンドウ情報設定
	window->Width(640.0f);
	window->Height(480.0f);
	window->Name(L"ポリゴン複数表示");

	//ウィンドウ作成
	if (SUCCEEDED(window->Create(hInst)))
	{
		//DirectX関連の初期化
		if (SUCCEEDED(window->InitD3D()))
		{
			return S_OK;
		}
	}

	return E_FAIL;
}

/// <summary>
/// 描画関連初期化
/// </summary>
HRESULT InitGraphics()
{
	auto graphics = ShunLib::Graphics::GetInstance();

	//シェーダーの読み込み
	if (SUCCEEDED(graphics->InitShader()))
	{
		//描画関連の設定
		if (SUCCEEDED(graphics->InitStaticMesh("cube.obj",graphics->Mesh())))
		{
			return S_OK;
		}
	}

	return E_FAIL;
}
