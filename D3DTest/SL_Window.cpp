#include "SL_Window.h"

#include <d3d11.h>
#include "SL_Graphics.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMag, WPARAM wParam, LPARAM lParam);

HRESULT ShunLib::Window::Create(HINSTANCE hInst)
{	
	//�E�B���h�E���@0�ŏ�����
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(wc));

	//�E�B���h�E�̏���ݒ�
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.lpszClassName = m_name;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	//�E�B���h�E�̓o�^
	RegisterClassEx(&wc);

	//�E�B���h�E���쐬
	m_hWnd = CreateWindow(m_name, m_name, WS_OVERLAPPEDWINDOW, 0, 0, (int)m_width, (int)m_height, 0, 0, hInst, 0);

	//�쐬�Ɏ��s������G���[
	if (!m_hWnd)return E_FAIL;

	//�E�B���h�E����ʂɕ\��
	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return S_OK;
}

/// <summary>
/// DirectX3D�̏�����
/// </summary>
HRESULT ShunLib::Window::InitD3D()
{
	// �f�o�C�X�ƃX���b�v�`�F�[���̍쐬
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = (UINT)m_width;                    //��ʉ���
	sd.BufferDesc.Height = (UINT)m_height;                  //��ʏc��
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//32bit�J���[
	sd.BufferDesc.RefreshRate.Numerator = 60;         //�t���b�V�����[�g�@60fps
	sd.BufferDesc.RefreshRate.Denominator = 1;        //�o�b�N�o�b�t�@�̐�
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


	//�X���b�v�`�F�C���������Ă���o�b�N�o�b�t�@���擾
	ID3D11Texture2D *backBuffer;
	m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);

	//�����_�[�^�[�Q�b�g�r���[�̍쐬
	//�����_�[�^�[�Q�b�g�ƃ��\�[�X���q�������
	m_device->CreateRenderTargetView(backBuffer, NULL, &m_recderTargetView);

	//�o�b�N�o�b�t�@�̎Q�ƃJ�E���^���P���炷
	//���j���ł͂Ȃ�
	SAFE_RELEASE(backBuffer);



	//�[�x�X�e���V���r���[�̍쐬
	//Z�o�b�t�@�ƃX�e���V���o�b�t�@�ɑ΂���r���[
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
	
	//�����_�[�^�[�Q�b�g�r���[�Ɛ[�x�X�e���V���r���[���p�C�v���C���Ɋ֘A�t����
	m_deviceContext->OMSetRenderTargets(1, &m_recderTargetView, m_depthStencilView);

	//�r���[�|�[�g�̐ݒ�
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)m_width;  //��ʕ�
	vp.Height = (FLOAT)m_height;//��ʕ�
	vp.MinDepth = 0.0f;         //Z���̕�
	vp.MaxDepth = 1.0f;         //Z���̕�
	vp.TopLeftX = 0;            //����̍��W
	vp.TopLeftY = 0;            //����̍��W
	m_deviceContext->RSSetViewports(1, &vp);

	//���X�^���C�Y�ݒ�
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
/// �E�B���h�E�v���V�[�W��
/// �EOS���烁�b�Z�[�W���󂯎�菈��������
/// </summary>
LRESULT CALLBACK ShunLib::Window::MsgProc(HWND hWnd, UINT iMag, WPARAM wParam, LPARAM lParam)
{
	switch (iMag)
	{
		//�L�[�������ꂽ
	case WM_KEYDOWN:
		switch ((char)wParam)
		{
			//Esc�L�[
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		break;

		//�E�B���h�E�������ꂽ
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	//�s�v�ȃ��b�Z�[�W��OS�ɏ���������
	return DefWindowProc(hWnd, iMag, wParam, lParam);
}


/// <summary>
/// ���b�Z�[�W���[�v
/// </summary>
void ShunLib::Window::Run()
{
	//���b�Z�[�W�@0�ŏ�����
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	//���b�Z�[�W���[�v
	while (msg.message != WM_QUIT) {
		//OS���烁�b�Z�[�W������΃v���V�[�W���ɓn��
		//OS���烁�b�Z�[�W��������΃A�v���P�[�V�������X�V����
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

			//���z�L�[���b�Z�[�W�𕶎����b�Z�[�W�֕ϊ�
			TranslateMessage(&msg);

			//���b�Z�[�W�𑗂�
			DispatchMessage(&msg);
		}
		else {
			//�Q�[���̍X�V
			GameUpdate();
		}
	}
}

/// <summary>
/// �Q�[���̍X�V
/// </summary>
void ShunLib::Window::GameUpdate()
{		
	//��ʃN���A
	Clear();

	auto graphics = Graphics::GetInstance();


	graphics->TestUpdate();
	graphics->TestRender();

	//�o�b�N�o�b�t�@�ƃt�����g�o�b�t�@������
	m_swapChain->Present(0, 0);
}

/// <summary>
/// ��ʂ̃N���A
/// </summary>
void ShunLib::Window::Clear()
{
	//��ʂ̐F
	float color[4] = { 0,1,1,1 };

	//��ʃN���A
	m_deviceContext->ClearRenderTargetView(m_recderTargetView, color);
	
	//�[�x�o�b�t�@�N���A
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH,1.0f,0);
}
