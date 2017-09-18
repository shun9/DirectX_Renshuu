#pragma once
#include <SL_Singleton.h>
#include <SL_Vec2.h>
#include <SL_Vec3.h>
#include <SL_Vec4.h>
#include <SL_Matrix.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include "SL_FBXMesh.h"
//���_�̍\����
struct SimpleVertex
{
	ShunLib::Vec3 pos;   //�ʒu
	ShunLib::Vec3 norm;  //�@��
	ShunLib::Vec2 tex;   //�e�N�X�`��
};

//Simple�V�F�[�_�[�p�̃R���X�^���g�o�b�t�@�[�̃A�v�����\���� 
//�V�F�[�_�[���̃R���X�^���g�o�b�t�@�[�ƈ�v���Ă���K�v����
struct SIMPLESHADER_CONSTANT_BUFFER
{
	ShunLib::Vec4 vLightDir;//���C�g����
	ShunLib::Vec4 vEye;//�J�����ʒu
};

//�I���W�i�����b�V��
struct MESH
{
	DWORD dwNumVert;
	DWORD dwNumFace;//�|���S����
	ID3D11Buffer* pVertexBuffer;

	//���_�C���f�b�N�X�̃o�b�t�@�[
	//���_�ɔԍ���t���g����
	ID3D11Buffer* pIndexBuffer;
};

namespace ShunLib {
	class Graphics :public Singleton<Graphics>
	{
		friend Singleton<Graphics>;

	private:
		//���f���̎�ނ��ƂɂP��
		//���f���̍\�������ׂē����Ȃ�S�̂łP��
		ID3D11InputLayout* m_vertexLayout;	//���_���C�A�E�g�@���_�������̑g�ݍ��킹�̂���
		ID3D11VertexShader* m_vertexShader; //�o�[�e�b�N�X�V�F�[�_�[
		ID3D11PixelShader* m_pixelShader;   //�s�N�Z���V�F�[�_�[
		ID3D11Buffer* m_constantBuffer0;     //�R���X�^���g�o�b�t�@
		ID3D11Buffer* m_constantBuffer1;     //�R���X�^���g�o�b�t�@

		//���f�����ƂɕK�v
		ID3D11Buffer* m_vertexBuffer;//���_�o�b�t�@�[�@���_��ۑ����Ă����̈�
		ID3D11SamplerState* m_sampleLinear; //�e�N�X�`���\�̃T���v���[
		ID3D11ShaderResourceView* m_texture;//�e�N�X�`���\

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