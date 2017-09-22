#include "SL_Window.h"

#include <d3d11.h>
#include "SL_Graphics.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMag, WPARAM wParam, LPARAM lParam);

HRESULT ShunLib::Window::Create(HINSTANCE hInst)
{	
	//ウィンドウ情報　0で初期化
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(wc));

	//ウィンドウの情報を設定
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.lpszClassName = m_name;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	//ウィンドウの登録
	RegisterClassEx(&wc);

	//ウィンドウを作成
	m_hWnd = CreateWindow(m_name, m_name, WS_OVERLAPPEDWINDOW, 0, 0, (int)m_width, (int)m_height, 0, 0, hInst, 0);

	//作成に失敗したらエラー
	if (!m_hWnd)return E_FAIL;

	//ウィンドウを画面に表示
	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return S_OK;
}

/// <summary>
/// DirectX3Dの初期化
/// </summary>
HRESULT ShunLib::Window::InitD3D()
{
	// デバイスとスワップチェーンの作成
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = (UINT)m_width;                    //画面横幅
	sd.BufferDesc.Height = (UINT)m_height;                  //画面縦幅
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//32bitカラー
	sd.BufferDesc.RefreshRate.Numerator = 60;         //フレッシュレート　60fps
	sd.BufferDesc.RefreshRate.Denominator = 1;        //バックバッファの数
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	
	D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL* featureLevel = NULL;

	auto result = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		&featureLevels,
		1,
		D3D11_SDK_VERSION,
		&sd,
		&m_swapChain,
		&m_device,
		featureLevel,
		&m_deviceContext);

	if(FAILED(result))return FALSE;


	//スワップチェインが持っているバックバッファを取得
	ID3D11Texture2D *backBuffer;
	m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);

	//レンダーターゲットビューの作成
	//レンダーターゲットとリソースを繋げるもの
	m_device->CreateRenderTargetView(backBuffer, NULL, &m_recderTargetView);

	//バックバッファの参照カウンタを１つ減らす
	//※破棄ではない
	SAFE_RELEASE(backBuffer);



	//深度ステンシルビューの作成
	//Zバッファとステンシルバッファに対するビュー
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = (UINT)m_width;
	descDepth.Height = (UINT)m_height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	m_device->CreateTexture2D(&descDepth, NULL, &m_texture2D);
	m_device->CreateDepthStencilView(m_texture2D, NULL, &m_depthStencilView);
	
	//レンダーターゲットビューと深度ステンシルビューをパイプラインに関連付ける
	m_deviceContext->OMSetRenderTargets(1, &m_recderTargetView, m_depthStencilView);

	//ビューポートの設定
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)m_width;  //画面幅
	vp.Height = (FLOAT)m_height;//画面幅
	vp.MinDepth = 0.0f;         //Z軸の幅
	vp.MaxDepth = 1.0f;         //Z軸の幅
	vp.TopLeftX = 0;            //左上の座標
	vp.TopLeftY = 0;            //左上の座標
	m_deviceContext->RSSetViewports(1, &vp);

	//ラスタライズ設定
	D3D11_RASTERIZER_DESC rd;
	ZeroMemory(&rd, sizeof(rd));

	rd.CullMode = D3D11_CULL_NONE;
	rd.FillMode = D3D11_FILL_SOLID;

	ID3D11RasterizerState* irs = NULL;
	m_device->CreateRasterizerState(&rd, &irs);
	m_deviceContext->RSSetState(irs);
	SAFE_RELEASE(irs);

	return S_OK;
}

/// <summary>
/// ウィンドウプロシージャ
/// ・OSからメッセージを受け取り処理をする
/// </summary>
LRESULT CALLBACK ShunLib::Window::MsgProc(HWND hWnd, UINT iMag, WPARAM wParam, LPARAM lParam)
{
	switch (iMag)
	{
		//キーが押された
	case WM_KEYDOWN:
		switch ((char)wParam)
		{
			//Escキー
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		break;

		//ウィンドウが消された
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	//不要なメッセージをOSに処理させる
	return DefWindowProc(hWnd, iMag, wParam, lParam);
}


/// <summary>
/// メッセージループ
/// </summary>
void ShunLib::Window::Run()
{
	//メッセージ　0で初期化
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	//メッセージループ
	while (msg.message != WM_QUIT) {
		//OSからメッセージがあればプロシージャに渡す
		//OSからメッセージが無ければアプリケーションを更新する
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

			//仮想キーメッセージを文字メッセージへ変換
			TranslateMessage(&msg);

			//メッセージを送る
			DispatchMessage(&msg);
		}
		else {
			//ゲームの更新
			GameUpdate();
		}
	}
}

/// <summary>
/// ゲームの更新
/// </summary>
void ShunLib::Window::GameUpdate()
{		
	//画面クリア
	Clear();

	auto graphics = Graphics::GetInstance();


	graphics->TestUpdate();
	graphics->TestRender();

	//バックバッファとフロントバッファを交換
	m_swapChain->Present(0, 0);
}

/// <summary>
/// 画面のクリア
/// </summary>
void ShunLib::Window::Clear()
{
	//画面の色
	float color[4] = { 0,1,1,1 };

	//画面クリア
	m_deviceContext->ClearRenderTargetView(m_recderTargetView, color);
	
	//深度バッファクリア
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH,1.0f,0);
}
