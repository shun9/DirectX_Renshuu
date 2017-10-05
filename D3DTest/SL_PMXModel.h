//************************************************/
//* @file  :PMXModel.h
//* @brief :PMX���f����`�悷��
//* @date  :2017/10/05
//* @author:S.Katou
//************************************************/
#pragma once
#include <memory>
#include <d3d11.h>
#include <SL_Matrix.h>
#include <SL_MacroConstants.h>
#include "SL_PMXModelData.h"

namespace ShunLib
{
	namespace PMX
	{
		//�R���X�^���g�o�b�t�@�@�s��
		struct PMXConstantBuffer
		{
			Matrix world;
			Matrix view;
			Matrix proj;
		};

		//�R���X�^���g�o�b�t�@�@�}�e���A��
		struct PMXMaterialBuffer
		{
			Vec4 diffuse;
			Vec3 specular;
			float specularPower;
			Vec4 ambient;
		};

		class PMXModel
		{
		private:
			//���f�����
			std::unique_ptr<PMXModelData>m_data;

			//���f���̎�ނ���
			ID3D11InputLayout* m_vertexLayout;
			ID3D11VertexShader* m_vertexShader;
			ID3D11PixelShader* m_pixelShader;
			ID3D11Buffer* m_constantBuffer;

			//���f������
			ID3D11Buffer* m_vertexBuffer;
			ID3D11Buffer* m_indexBuffer;
			ID3D11Buffer* m_materialBuffer;
			ID3D11SamplerState* m_sampler;
			ID3D11ShaderResourceView** m_texture;

		public:
			PMXModel() :
				m_vertexLayout  (nullptr),
				m_vertexShader  (nullptr),
				m_pixelShader   (nullptr),
				m_constantBuffer(nullptr),
				m_vertexBuffer  (nullptr),
				m_indexBuffer   (nullptr),
				m_materialBuffer(nullptr),
				m_sampler       (nullptr),
				m_texture       (nullptr)
			{}
			~PMXModel() {
				for (int i = 0; i < m_data->Material()->count; i++)
				{
					SAFE_RELEASE(m_texture[i]);
				}
				SAFE_DELETE_ARRAY(m_texture);
				SAFE_RELEASE(m_sampler);
				SAFE_RELEASE(m_materialBuffer);
				SAFE_RELEASE(m_indexBuffer);
				SAFE_RELEASE(m_vertexBuffer);
				SAFE_RELEASE(m_constantBuffer);
				SAFE_RELEASE(m_pixelShader);
				SAFE_RELEASE(m_vertexLayout);
				SAFE_RELEASE(m_vertexShader);
			}


			bool Init(char* file);
			void Draw(const Matrix& world, const Matrix& view, const Matrix& proj);

		private:
			bool InitShader();
			bool InitPolygon();
			void SetMaterialBuffer(const PmxMaterialInfo* data, PMXMaterialBuffer* buf);
		};

	}
}