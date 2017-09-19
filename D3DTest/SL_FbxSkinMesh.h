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

//�V�F�[�_�[�ɓn���l

struct SHADER_GLOBAL0
{
	ShunLib::Vec4 lightDir;//���C�g����
	ShunLib::Vec4 eyePos;//�J�����ʒu
};

struct SHADER_GLOBAL1
{
	ShunLib::Matrix world;//���[���h�s��
	ShunLib::Matrix wvp;//���[���h����ˉe�܂ł̕ϊ��s��
	ShunLib::Vec4 ambient;//�A���r�G���g��
	ShunLib::Vec4 diffuse;//�f�B�t���[�Y�F
	ShunLib::Vec4 specular;//���ʔ���
};

//�V�F�[�_�[�ɓn���{�[���s��z��
struct SHADER_GLOBAL_BONES
{
	ShunLib::Matrix mBone[256];
};

//�I���W�i���@�}�e���A���\����
struct MY_MATERIAL
{
	CHAR szName[110];
	ShunLib::Vec4 Ka;//�A���r�G���g
	ShunLib::Vec4 Kd;//�f�B�t���[�Y
	ShunLib::Vec4 Ks;//�X�y�L�����[
	CHAR szTextureName[110];//�e�N�X�`���[�t�@�C����
	ID3D11ShaderResourceView* pTexture;
	DWORD dwNumFace;//���̃}�e���A���ł���|���S����
	MY_MATERIAL()
	{
		ZeroMemory(this,sizeof(MY_MATERIAL));
	}
	~MY_MATERIAL()
	{
		SAFE_RELEASE(pTexture);
	}
};

//���_�\����
struct MY_VERTEX
{
	ShunLib::Vec3 pos;//���_�ʒu
	ShunLib::Vec3 norm;//���_�@��
	ShunLib::Vec3 tex;//UV���W
	UINT bBoneIndex[4];//�{�[���@�ԍ�
	float bBoneWeight[4];//�{�[���@�d��
	MY_VERTEX()
	{
		ZeroMemory(this,sizeof(MY_VERTEX));
	}
};
//�{�[���\����
struct BONE
{
	ShunLib::Matrix bindPose;//�����|�[�Y�i�����ƕς��Ȃ��j
	ShunLib::Matrix newPose;//���݂̃|�[�Y�i���̓s�x�ς��j
};

//�P���_�̋��L�@�ő�20�|���S���܂�
struct POLY_TABLE
{
	int PolyIndex[20];//�|���S���ԍ� 
	int Index123[20];//3�̒��_�̂����A���Ԗڂ�
	int NumRef;//�����Ă���|���S����

	POLY_TABLE()
	{
		ZeroMemory(this,sizeof(POLY_TABLE));
	}
};


namespace ShunLib {
	//class FBX_SKINMESH
	//FBX_SKINMESH �I���W�i�����b�V���N���X
	class FbxSkinMesh
	{
	private:
		//�O���̃f�o�C�X�����
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

		//���b�V���֘A	
		DWORD m_dwNumVert;
		DWORD m_dwNumFace;
		DWORD m_dwNumUV;
		ID3D11Buffer* m_vertexBuffer;
		ID3D11Buffer** m_ppIndexBuffer;
		MY_MATERIAL* m_material;
		DWORD m_dwNumMaterial;
		Matrix m_mFinalWorld;//�ŏI�I�ȃ��[���h�s��i���̎p���Ń����_�����O����j

		//�{�[��
		int m_iNumBone;
		BONE* m_bone;
		FbxCluster** m_ppCluster;

		Vec3 m_vPos;
		float m_fYaw, m_fPitch, m_fRoll;
		float m_fScale;

	public:
		FbxSkinMesh();
		~FbxSkinMesh();
		
		//���\�b�h
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