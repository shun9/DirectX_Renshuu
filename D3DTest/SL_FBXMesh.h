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

//�x����\��
#pragma warning(disable : 4244)

//�V�F�[�_�[�ɓn���l
struct SHADER_GLOBAL1
{
	ShunLib::Matrix world;
	ShunLib::Matrix wvp;
	ShunLib::Vec4 ambient;
	ShunLib::Vec4 diffuse;
	ShunLib::Vec4 specular;
};

//�}�e���A���\����
struct FBX_MATERIAL
{
	CHAR szName[100];
	ShunLib::Vec4 Ka;//�A���r�G���g
	ShunLib::Vec4 Kd;//�f�B�t���[�Y
	ShunLib::Vec4 Ks;//�X�y�L�����[
	CHAR szTextureName[100];//�e�N�X�`���[�t�@�C����
	ID3D11ShaderResourceView* pTexture;
	DWORD dwNumFace;//���̃}�e���A���ł���|���S����
	FBX_MATERIAL()
	{
		ZeroMemory(this,sizeof(FBX_MATERIAL));
	}
	~FBX_MATERIAL()
	{
		SAFE_RELEASE(pTexture);
	}
};

//���_�\����
struct FBX_VERTEX
{
	
	ShunLib::Vec3 pos;
	ShunLib::Vec3 norm;
	ShunLib::Vec2 tex;
};

class FBXMesh
{
private:
	FBXMesh** m_child;//�����̎q�ւ̃|�C���^�[
	DWORD m_dwNumChild;//�q�̐�

	//FBX
	FbxManager* m_manager;//SDK�S�̂̊Ǘ�
	FbxImporter* m_importer;//FBX���J���N���X
	FbxScene* m_pmyScene;//���_���ȂǂɃA�N�Z�X����N���X

	//KFbxSdkManager *m_pSdkManager;
	//KFbxImporter* m_pImporter;
	//KFbxScene* m_pmyScene;

	//�O���̃f�o�C�X�����
	ID3D11SamplerState* m_sampleLinear;
	ID3D11Buffer* m_constantBuffer;

	//���b�V���֘A	
	DWORD m_dwNumVert;
	DWORD m_dwNumFace;
	DWORD m_dwNumUV;
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer** m_indexBuffer;
	FBX_MATERIAL* m_material;
	DWORD m_dwNumMaterial;

	//�A�j���[�V�����֘A	
	ShunLib::Matrix m_parentOrientation;//�e�̎p���s��
	ShunLib::Matrix m_FBXOrientation;//�����̎p���s��i�e���猩�����Ύp���j
	ShunLib::Matrix m_finalWorld;//�ŏI�I�ȃ��[���h�s��i���̎p���Ń����_�����O����j


	FbxNode* m_FBXNode;//FBX����p���s������o���ۂɎg��FBX�|�C���^�[

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