#pragma once
#include <SL_Singleton.h>
#include <SL_Vec2.h>
#include <SL_Vec3.h>
#include <SL_Vec4.h>
#include <SL_Matrix.h>
#include <d3d11.h>
#include <d3dcompiler.h>


//���_�̍\����
struct SimpleVertex
{
	ShunLib::Vec3 pos;   //�ʒu
	ShunLib::Vec2 texturePos;//�e�N�X�`���\���W
};

//Simple�V�F�[�_�[�p�̃R���X�^���g�o�b�t�@�[�̃A�v�����\���� 
//�V�F�[�_�[���̃R���X�^���g�o�b�t�@�[�ƈ�v���Ă���K�v����
struct SIMPLESHADER_CONSTANT_BUFFER
{
	//ShunLib::Matrix world; //���[���h�s��
	ShunLib::Matrix mWVP;  //���[���h�A�r���[�A�ˉe�̍����ϊ��s��
	//ShunLib::Vec4 lightDir;//���C�g�̕���
	ShunLib::Vec4 color;   //�|���S���F
	//ShunLib::Vec4 eyePos;  //�J�����ʒu
};

////���̂̍\����
//struct Model
//{
//	ShunLib::Vec3 pos;
//	ShunLib::Vec4 color;
//};

namespace ShunLib {
	class Graphics :public Singleton<Graphics>
	{
		friend Singleton<Graphics>;
	private:
		static const int MAX_MODEL = 2;

	private:
		//���f���̎�ނ��ƂɂP��
		//���f���̍\�������ׂē����Ȃ�S�̂łP��
		ID3D11InputLayout* m_vertexLayout;	//���_���C�A�E�g�@���_�������̑g�ݍ��킹�̂���
		ID3D11VertexShader* m_vertexShader; //�o�[�e�b�N�X�V�F�[�_�[
		ID3D11PixelShader* m_pixelShader;   //�s�N�Z���V�F�[�_�[
		ID3D11Buffer* m_constantBuffer;     //�R���X�^���g�o�b�t�@

		//���f�����ƂɕK�v
		ID3D11Buffer* m_vertexBuffer;//���_�o�b�t�@�[�@���_��ۑ����Ă����̈�
		ID3D11SamplerState* m_sampleLinear; //�e�N�X�`���\�̃T���v���[
		ID3D11ShaderResourceView* m_texture;//�e�N�X�`���\

		//Model m_model[MAX_MODEL];

		//���C�g�̕���
		//ShunLib::Vec4 m_lightDir;

	public:
		HRESULT InitShader();
		HRESULT InitPolygon();

		void TestUpdate();
		void TestRender();

	private:
		Graphics() { //m_lightDir = { 0,2.5f,-1.0f,0 }; 
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