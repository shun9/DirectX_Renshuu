#pragma once
#include <windows.h>
#include <d3d11.h>
#include <d3dCompiler.h>
#include <fbxsdk.h>

#include <SL_MacroConstants.h>
#include <SL_Matrix.h>
#include <SL_Vec2.h>
#include <SL_Vec3.h>
#include <SL_Vec4.h>

//警告非表示
#pragma warning(disable : 4244)

//シェーダーに渡す値
struct SHADER_GLOBAL1
{
	ShunLib::Matrix world;
	ShunLib::Matrix wvp;
	ShunLib::Vec4 ambient;
	ShunLib::Vec4 diffuse;
	ShunLib::Vec4 specular;
};

//マテリアル構造体
struct FBX_MATERIAL
{
	CHAR szName[100];
	ShunLib::Vec4 Ka;//アンビエント
	ShunLib::Vec4 Kd;//ディフューズ
	ShunLib::Vec4 Ks;//スペキュラー
	CHAR szTextureName[100];//テクスチャーファイル名
	ID3D11ShaderResourceView* pTexture;
	DWORD dwNumFace;//そのマテリアルであるポリゴン数
	FBX_MATERIAL()
	{
		ZeroMemory(this,sizeof(FBX_MATERIAL));
	}
	~FBX_MATERIAL()
	{
		SAFE_RELEASE(pTexture);
	}
};

//頂点構造体
struct FBX_VERTEX
{
	
	ShunLib::Vec3 pos;
	ShunLib::Vec3 norm;
	ShunLib::Vec2 tex;
};

class FBXMesh
{
private:
	FBXMesh** m_child;//自分の子へのポインター
	DWORD m_dwNumChild;//子の数

	//FBX
	FbxManager* m_manager;//SDK全体の管理
	FbxImporter* m_importer;//FBXを開くクラス
	FbxScene* m_pmyScene;//頂点情報などにアクセスするクラス

	//KFbxSdkManager *m_pSdkManager;
	//KFbxImporter* m_pImporter;
	//KFbxScene* m_pmyScene;

	//外部のデバイス等情報
	ID3D11SamplerState* m_sampleLinear;
	ID3D11Buffer* m_constantBuffer;

	//メッシュ関連	
	DWORD m_dwNumVert;
	DWORD m_dwNumFace;
	DWORD m_dwNumUV;
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer** m_indexBuffer;
	FBX_MATERIAL* m_material;
	DWORD m_dwNumMaterial;

	//アニメーション関連	
	ShunLib::Matrix m_parentOrientation;//親の姿勢行列
	ShunLib::Matrix m_FBXOrientation;//自分の姿勢行列（親から見た相対姿勢）
	ShunLib::Matrix m_finalWorld;//最終的なワールド行列（この姿勢でレンダリングする）


	FbxNode* m_FBXNode;//FBXから姿勢行列を取り出す際に使うFBXポインター

public:
	FBXMesh();
	~FBXMesh();

	HRESULT CreateIndexBuffer(DWORD dwSize,int* pIndex,ID3D11Buffer** ppIndexBuffer);
	void RenderMesh(const ShunLib::Matrix& world, const ShunLib::Matrix& view,const ShunLib::Matrix& proj);
	HRESULT InitFBX(CHAR* szFileName);
	HRESULT CreateFromFBX(CHAR* FileName);

	void SetSampleLinear(ID3D11SamplerState* sampler) { m_sampleLinear = sampler; }
	void ConstantBuffer(ID3D11Buffer* buffer) { m_constantBuffer = buffer; }

private:
	FbxMesh* SearchMesh(FbxNode* rootNode);
};