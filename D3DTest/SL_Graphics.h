#pragma once
#include <SL_Singleton.h>
#include <SL_Vec3.h>
#include <SL_Vec4.h>
#include <SL_Matrix.h>
#include <d3d11.h>
#include <d3dcompiler.h>


//頂点の構造体
struct SimpleVertex
{
	ShunLib::Vec3 pos; //位置
	ShunLib::Vec3 Normal;//法線
};

//Simpleシェーダー用のコンスタントバッファーのアプリ側構造体 
//シェーダー内のコンスタントバッファーと一致している必要あり
struct SIMPLESHADER_CONSTANT_BUFFER
{
	ShunLib::Matrix world;//ワールド行列
	ShunLib::Matrix mWVP;//ワールド、ビュー、射影の合成変換行列
	ShunLib::Vec4 lightDir;//ライトの方向
	ShunLib::Vec4 color;//ポリゴン色
};

//物体の構造体
struct Model
{
	ShunLib::Vec3 pos;
	ShunLib::Vec4 color;
};

namespace ShunLib {
	class Graphics :public Singleton<Graphics>
	{
		friend Singleton<Graphics>;
	private:
		static const int MAX_MODEL = 2;

	private:
		//モデルの種類ごとに１つ
		//モデルの構造がすべて同じなら全体で１つ
		ID3D11InputLayout* m_vertexLayout;	//頂点レイアウト　頂点が持つ情報の組み合わせのこと
		ID3D11VertexShader* m_vertexShader; //バーテックスシェーダー
		ID3D11PixelShader* m_pixelShader;   //ピクセルシェーダー
		ID3D11Buffer* m_constantBuffer;     //コンスタントバッファ

		//モデルごとに必要
		ID3D11Buffer* m_vertexBuffer;//頂点バッファー　頂点を保存しておく領域

		Model m_model[MAX_MODEL];

		ShunLib::Vec4 m_lightDir;

	public:
		HRESULT InitShader();
		HRESULT InitPolygon();

		void TestUpdate();
		void TestRender();

	private:
		Graphics() { m_lightDir = { 0,10,-5.0f,0 }; }
		~Graphics() { 
			SAFE_RELEASE(m_constantBuffer);
			SAFE_RELEASE(m_vertexShader);
			SAFE_RELEASE(m_pixelShader);
			SAFE_RELEASE(m_vertexBuffer);
			SAFE_RELEASE(m_vertexLayout);
		}
	};
}