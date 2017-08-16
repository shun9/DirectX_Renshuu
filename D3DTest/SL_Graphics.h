#pragma once
#include <SL_Singleton.h>
#include <SL_Vec2.h>
#include <SL_Vec3.h>
#include <SL_Vec4.h>
#include <SL_Matrix.h>
#include <d3d11.h>
#include <d3dcompiler.h>


//頂点の構造体
struct SimpleVertex
{
	ShunLib::Vec3 pos;   //位置
};

//Simpleシェーダー用のコンスタントバッファーのアプリ側構造体 
//シェーダー内のコンスタントバッファーと一致している必要あり
struct SIMPLESHADER_CONSTANT_BUFFER
{
	ShunLib::Matrix mWVP;  //ワールド、ビュー、射影の合成変換行列
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
		ID3D11Buffer* m_constantBuffer;     //コンスタントバッファ

		//モデルごとに必要
		ID3D11Buffer* m_vertexBuffer;//頂点バッファー　頂点を保存しておく領域
		ID3D11SamplerState* m_sampleLinear; //テクスチャ―のサンプラー
		ID3D11ShaderResourceView* m_texture;//テクスチャ―

		MESH m_mesh;

	public:
		HRESULT InitShader();
		HRESULT InitStaticMesh(LPSTR fileName, MESH* mesh);

		void TestUpdate();
		void TestRender();

		MESH* Mesh() { return &m_mesh; }

	private:
		Graphics() { 
			m_vertexLayout   = nullptr;
			m_vertexShader   = nullptr;
			m_pixelShader    = nullptr;
			m_constantBuffer = nullptr;
			m_vertexBuffer   = nullptr;
			m_sampleLinear   = nullptr;
			m_texture        = nullptr;

			ZeroMemory(&m_mesh, sizeof(m_mesh));
		}
		~Graphics() {
			SAFE_RELEASE(m_constantBuffer);
			SAFE_RELEASE(m_vertexShader);
			SAFE_RELEASE(m_pixelShader);
			SAFE_RELEASE(m_vertexBuffer);
			SAFE_RELEASE(m_vertexLayout);
		}
	};
}