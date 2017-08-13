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
		//�E�B���h�E�T�C�Y
		float m_width;  //��
		float m_height; //�c

		WCHAR* m_name;//�E�B���h�E��
		HWND m_hWnd;  //�E�B���h�E�n���h��

		//�S�̂łP��
		ID3D11Device* m_device;
		ID3D11DeviceContext* m_deviceContext;
		IDXGISwapChain* m_swapChain;
		ID3D11RenderTargetView* m_recderTargetView;
		ID3D11DepthStencilView* m_depthStencilView;
		ID3D11Texture2D* m_texture2D;
	
	public:
		//�E�B���h�E�쐬
		HRESULT Create(HINSTANCE);

		//DirectX�֘A�̏�����
		HRESULT InitD3D();

		//�v���V�[�W��
		LRESULT CALLBACK MsgProc(HWND hWnd, UINT iMag, WPARAM wParam, LPARAM lParam);

		//���b�Z�[�W���[�v
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
		//�R���X�g���N�^���f�X�g���N�^
		//�V���O���g���̂��߉B��
		Window() :
			m_width(640.0f),
			m_height(480.0f),
			m_name(L"�^�C�g��") {}

		~Window() {
			SAFE_RELEASE(m_swapChain);
			SAFE_RELEASE(m_recderTargetView);
			SAFE_RELEASE(m_deviceContext);
			SAFE_RELEASE(m_texture2D);
			SAFE_RELEASE(m_depthStencilView);
			SAFE_RELEASE(m_device);
		}

		//�Q�[���̍X�V
		void GameUpdate();

		//��ʃN���A
		void Clear();	
	};
}