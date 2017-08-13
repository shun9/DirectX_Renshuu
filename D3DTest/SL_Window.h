#pragma once
#include <Windows.h>
#include <SL_Singleton.h>

#include <d3d11.h>
#include <d3dcompiler.h>

namespace ShunLib
{
	class Window :public Singleton<Window>
	{
		friend Singleton<Window>;

	private:
		//ウィンドウサイズ
		float m_width;  //横
		float m_height; //縦

		WCHAR* m_name;//ウィンドウ名
		HWND m_hWnd;  //ウィンドウハンドル

		//全体で１つ
		ID3D11Device* m_device;
		ID3D11DeviceContext* m_deviceContext;
		IDXGISwapChain* m_swapChain;
		ID3D11RenderTargetView* m_recderTargetView;
		ID3D11DepthStencilView* m_depthStencilView;
		ID3D11Texture2D* m_texture2D;
	
	public:
		//ウィンドウ作成
		HRESULT Create(HINSTANCE);

		//DirectX関連の初期化
		HRESULT InitD3D();

		//プロシージャ
		LRESULT CALLBACK MsgProc(HWND hWnd, UINT iMag, WPARAM wParam, LPARAM lParam);

		//メッセージループ
		void Run();

		//Setter
		void Width(float width) { m_width = width; }
		void Height(float height) { m_height = height; }
		void Name(WCHAR* name) { m_name = name; }

		//Getter
		float Width() { return m_width; }
		float Height() { return m_height; }
		WCHAR* Name() { return m_name; }
		ID3D11Device* Device() { return m_device; }
		ID3D11DeviceContext* DeviceContext() { return m_deviceContext; }

	private:
		//コンストラクタ＆デストラクタ
		//シングルトンのため隠蔽
		Window() :
			m_width(640.0f),
			m_height(480.0f),
			m_name(L"タイトル") {}

		~Window() {
			SAFE_RELEASE(m_swapChain);
			SAFE_RELEASE(m_recderTargetView);
			SAFE_RELEASE(m_deviceContext);
			SAFE_RELEASE(m_texture2D);
			SAFE_RELEASE(m_depthStencilView);
			SAFE_RELEASE(m_device);
		}

		//ゲームの更新
		void GameUpdate();

		//画面クリア
		void Clear();	
	};
}