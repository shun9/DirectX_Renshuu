#pragma once
#include <SL_Singleton.h>
#include <SL_Vec2.h>
#include <SL_Vec3.h>
#include <SL_Vec4.h>
#include <SL_Matrix.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include "SL_FBXMesh.h"
//頂点の構造体
struct SimpleVertex
{
	ShunLib::Vec3 pos;   //位置
	ShunLib::Vec3 norm;  //法線
	ShunLib::Vec2 tex;   //テクスチャ
};

//Simpleシェーダー用のコンスタントバッファーのアプリ側構造体 
//シェーダー内のコンスタントバッファーと一致している必要あり
struct SIMPLESHADER_CONSTANT_BUFFER
{
	ShunLib::Vec4 vLightDir;//ライト方向
	ShunLib::Vec4 vEye;//カメラ位置
};

//オリジナルメッシュ
struct MESH
{
	DWORD dwNumVert;
	DWORD dwNumFace;//ポリゴン数
	ID3D11Buffer* pVertexBuffer;

	//頂点インデックスのバッファー
	//頂点に番号を付け使い回す
	ID3D11Buffer* pIndexBuffer;
};

namespace ShunLib {
	class Graphics :public Singleton<Graphics>
	{
		friend Singleton<Graphics>;

	private:
		//モデルの種類ごとに１つ
		//モデルの構造がすべて同じなら全体で１つ
		ID3D11InputLayout* m_vertexLayout;	//頂点レイアウト　頂点が持つ情報の組み合わせのこと
		ID3D11VertexShader* m_vertexShader; //バーテックスシェーダー
		ID3D11PixelShader* m_pixelShader;   //ピクセルシェーダー
		ID3D11Buffer* m_constantBuffer0;     //コンスタントバッファ
		ID3D11Buffer* m_constantBuffer1;     //コンスタントバッファ

		//モデルごとに必要
		ID3D11Buffer* m_vertexBuffer;//頂点バッファー　頂点を保存しておく領域
		ID3D11SamplerState* m_sampleLinear; //テクスチャ―のサンプラー
		ID3D11ShaderResourceView* m_texture;//テクスチャ―

		MESH m_mesh;	
		FBXMesh* m_fbxMesh;

	public:
		HRESULT InitShader();
		HRESULT InitStaticMesh(LPSTR fileName, MESH* mesh);

		void TestUpdate();
		void TestRender();

		MESH* Mesh() { return &m_mesh; }

		HRESULT CreateShaderResourceView(
			const wchar_t* path, 
			ID3D11ShaderResourceView** srv);

		HRESULT CreateVertexShader(
			const wchar_t* fileName,
			const char* entryPoint, 
			const char* target, 
			ID3D11VertexShader** vs);

		HRESULT CreatePixleShader(
			const wchar_t* fileName, 
			const char* entryPoint, 
			const char* target, 
			ID3D11PixelShader** ps);

		HRESULT CreateInputLayout(
			const D3D11_INPUT_ELEMENT_DESC layout[],
			const UINT& layoutSize,
			ID3D11InputLayout** il);
		

	private:
		Graphics() { 
			m_vertexLayout   = nullptr;
			m_vertexShader   = nullptr;
			m_pixelShader    = nullptr;
			m_constantBuffer0 = nullptr;
			m_constantBuffer1 = nullptr;
			m_vertexBuffer   = nullptr;
			m_sampleLinear   = nullptr;
			m_texture        = nullptr;

			ZeroMemory(&m_mesh, sizeof(m_mesh));
		}
		~Graphics() {
			SAFE_RELEASE(m_constantBuffer0);
			SAFE_RELEASE(m_constantBuffer1);
			SAFE_RELEASE(m_vertexShader);
			SAFE_RELEASE(m_pixelShader);
			SAFE_RELEASE(m_vertexBuffer);
			SAFE_RELEASE(m_vertexLayout);
		}
	};
}