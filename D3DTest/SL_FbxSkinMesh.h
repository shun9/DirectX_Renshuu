#pragma once
#include <stdio.h>
#include <windows.h>
#include <d3d11.h>
#include <d3dCompiler.h>
#include <string.h>
#include <fbxsdk.h>

#include <SL_Vec4.h>
#include <SL_Matrix.h>
#include <SL_MacroConstants.h>

//シェーダーに渡す値

struct SHADER_GLOBAL0
{
	ShunLib::Vec4 lightDir;//ライト方向
	ShunLib::Vec4 eyePos;//カメラ位置
};

struct SHADER_GLOBAL1
{
	ShunLib::Matrix world;//ワールド行列
	ShunLib::Matrix wvp;//ワールドから射影までの変換行列
	ShunLib::Vec4 ambient;//アンビエント光
	ShunLib::Vec4 diffuse;//ディフューズ色
	ShunLib::Vec4 specular;//鏡面反射
};

//シェーダーに渡すボーン行列配列
struct SHADER_GLOBAL_BONES
{
	ShunLib::Matrix mBone[256];
};

//オリジナル　マテリアル構造体
struct MY_MATERIAL
{
	CHAR szName[110];
	ShunLib::Vec4 Ka;//アンビエント
	ShunLib::Vec4 Kd;//ディフューズ
	ShunLib::Vec4 Ks;//スペキュラー
	CHAR szTextureName[110];//テクスチャーファイル名
	ID3D11ShaderResourceView* pTexture;
	DWORD dwNumFace;//そのマテリアルであるポリゴン数
	MY_MATERIAL()
	{
		ZeroMemory(this,sizeof(MY_MATERIAL));
	}
	~MY_MATERIAL()
	{
		SAFE_RELEASE(pTexture);
	}
};

//頂点構造体
struct MY_VERTEX
{
	ShunLib::Vec3 pos;//頂点位置
	ShunLib::Vec3 norm;//頂点法線
	ShunLib::Vec3 tex;//UV座標
	UINT bBoneIndex[4];//ボーン　番号
	float bBoneWeight[4];//ボーン　重み
	MY_VERTEX()
	{
		ZeroMemory(this,sizeof(MY_VERTEX));
	}
};
//ボーン構造体
struct BONE
{
	ShunLib::Matrix bindPose;//初期ポーズ（ずっと変わらない）
	ShunLib::Matrix newPose;//現在のポーズ（その都度変わる）
};

//１頂点の共有　最大20ポリゴンまで
struct POLY_TABLE
{
	int PolyIndex[20];//ポリゴン番号 
	int Index123[20];//3つの頂点のうち、何番目か
	int NumRef;//属しているポリゴン数

	POLY_TABLE()
	{
		ZeroMemory(this,sizeof(POLY_TABLE));
	}
};


namespace ShunLib {
	//class FBX_SKINMESH
	//FBX_SKINMESH オリジナルメッシュクラス
	class FbxSkinMesh
	{
	private:
		//外部のデバイス等情報
		ID3D11SamplerState* m_pSampleLinear;
		ID3D11Buffer* m_pConstantBuffer0;
		ID3D11Buffer* m_pConstantBuffer1;
		ID3D11Buffer* m_pConstantBufferBone;
		ID3D11InputLayout* m_vertexLayout;
		ID3D11VertexShader* m_vertexShader;
		ID3D11PixelShader* m_pixelShader;
		Matrix m_mView;
		Matrix m_mProj;

		//FBX
		FbxManager *m_pSdkManager;
		FbxImporter* m_pImporter;
		FbxScene* m_pmyScene;

		//メッシュ関連	
		DWORD m_dwNumVert;
		DWORD m_dwNumFace;
		DWORD m_dwNumUV;
		ID3D11Buffer* m_vertexBuffer;
		ID3D11Buffer** m_ppIndexBuffer;
		MY_MATERIAL* m_material;
		DWORD m_dwNumMaterial;
		Matrix m_mFinalWorld;//最終的なワールド行列（この姿勢でレンダリングする）

		//ボーン
		int m_iNumBone;
		BONE* m_bone;
		FbxCluster** m_ppCluster;

		Vec3 m_vPos;
		float m_fYaw, m_fPitch, m_fRoll;
		float m_fScale;

	public:
		FbxSkinMesh();
		~FbxSkinMesh();
		
		//メソッド
		HRESULT CreateIndexBuffer(DWORD dwSize, int* pIndex, ID3D11Buffer** ppIndexBuffer);
		void Render(const Matrix& mView, const Matrix& mProj, const Vec3& vLight, const Vec3& vEye);
		HRESULT Init(LPSTR FileName);
		HRESULT InitShader();
		HRESULT InitFBX(CHAR* szFileName);
		HRESULT CreateFromFBX(CHAR* szFileName);
		HRESULT ReadSkinInfo(FbxMesh* pFbx, MY_VERTEX*, POLY_TABLE* PolyTable);
		void SetNewPoseMatrices(int frame);
		Matrix GetCurrentPoseMatrix(int index);

	};
}