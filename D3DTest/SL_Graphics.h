#pragma once
#include <SL_Singleton.h>
#include <SL_Vec2.h>
#include <SL_Vec3.h>
#include <SL_Vec4.h>
#include <SL_Matrix.h>
#include <d3d11.h>
#include <d3dcompiler.h>
//#include "SL_FBXMesh.h"
#include "SL_FbxSkinMesh.h"
#include "SL_PMXModel.h"

namespace ShunLib {
	class Graphics :public Singleton<Graphics>
	{
		friend Singleton<Graphics>;

	private:
		////モデルの種類ごとに１つ
		////モデルの構造がすべて同じなら全体で１つ
		//ID3D11InputLayout* m_vertexLayout;	//頂点レイアウト　頂点が持つ情報の組み合わせのこと
		//ID3D11VertexShader* m_vertexShader; //バーテックスシェーダー
		//ID3D11PixelShader* m_pixelShader;   //ピクセルシェーダー
		//ID3D11Buffer* m_constantBuffer0;     //コンスタントバッファ
		//ID3D11Buffer* m_constantBuffer1;     //コンスタントバッファ

		////モデルごとに必要
		//ID3D11Buffer* m_vertexBuffer;//頂点バッファー　頂点を保存しておく領域
		//ID3D11SamplerState* m_sampleLinear; //テクスチャ―のサンプラー
		//ID3D11ShaderResourceView* m_texture;//テクスチャ―

		//FBX
		FbxManager* m_FBXManager;

		PMX::PMXModel* m_pmxModel;

	public:
		HRESULT InitShader();

		void TestUpdate();
		void TestRender();

		HRESULT CreateShaderResourceView(
			const wchar_t* path,
			ID3D11ShaderResourceView** srv);

		//頂点シェーダと頂点インプットレイアウトを作成
		HRESULT CreateVertexShader(
			const wchar_t* fileName,
			const char* entryPoint,
			const char* target,
			ID3D11VertexShader** vs,
			const D3D11_INPUT_ELEMENT_DESC layout[],
			const UINT& layoutSize,
			ID3D11InputLayout** il);

		//ピクセルシェーダーを作成
		HRESULT CreatePixleShader(
			const wchar_t* fileName,
			const char* entryPoint,
			const char* target,
			ID3D11PixelShader** ps);

		//コンスタントバッファを作成
		HRESULT CreateConstantBuffer(
			D3D11_BUFFER_DESC* cd,
			ID3D11Buffer** buf);

		//バーテックスバッファを作成
		HRESULT CreateVertexBuffer(
			D3D11_BUFFER_DESC* cd,
			D3D11_SUBRESOURCE_DATA* data,
			ID3D11Buffer** buf);

		FbxManager* FBXManager() { return m_FBXManager; }

	private:
		Graphics() {
			//m_vertexLayout   = nullptr;
			//m_vertexShader   = nullptr;
			//m_pixelShader    = nullptr;
			//m_constantBuffer0 = nullptr;
			//m_constantBuffer1 = nullptr;
			//m_vertexBuffer   = nullptr;
			//m_sampleLinear   = nullptr;
			//m_texture        = nullptr;

			m_FBXManager = FbxManager::Create();
		}
		~Graphics() {
			//SAFE_RELEASE(m_constantBuffer0);
			//SAFE_RELEASE(m_constantBuffer1);
			//SAFE_RELEASE(m_vertexShader);
			//SAFE_RELEASE(m_pixelShader);
			//SAFE_RELEASE(m_vertexBuffer);
			//SAFE_RELEASE(m_vertexLayout);
			SAFE_DELETE(m_pmxModel);

			m_FBXManager->Destroy();
		}


		HRESULT CreateInputLayout(
			const D3D11_INPUT_ELEMENT_DESC layout[],
			const UINT& layoutSize,
			ID3D11InputLayout** il);
	};
}