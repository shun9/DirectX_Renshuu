//************************************************/
//* @file  :PMXModel.cpp
//* @brief :PMX���f����`�悷��
//* @date  :2017/10/05
//* @author:S.Katou
//************************************************/
#include "SL_PMXModel.h"

#include<SL_Conversion.h>
#include "SL_Graphics.h"
#include "SL_Window.h"

using namespace ShunLib;
using namespace ShunLib::PMX;

bool PMXModel::Init(char * file)
{
	//���f���ǂݍ���
	m_data = std::make_unique<PMXModelData>();
	if (!m_data->LoadModel(file))
	{
		return false;
	}

	auto graphics = Graphics::GetInstance();
	//�V�F�[�_�֘A�̏�����
	if (!InitShader())
	{
		return false;
	}

	//�|���S���֘A�̏�����
	if (!InitPolygon())
	{
		return false;
	}

	return true;
}

void PMXModel::Draw(const Matrix & world, const Matrix & view, const Matrix & proj)
{
	auto context = Window::GetInstance()->DeviceContext();

	//�g�p����V�F�[�_�[�̓o�^
	context->VSSetShader(m_vertexShader, NULL, 0);
	context->PSSetShader(m_pixelShader, NULL, 0);

	//�V�F�[�_�[�̃R���X�^���g�o�b�t�@�[�Ɋe��f�[�^��n��
	D3D11_MAPPED_SUBRESOURCE data;
	PMXConstantBuffer cb;
	if (SUCCEEDED(context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data)))
	{
		//���[���h�A�J�����A�ˉe�s���n��
		cb.world = Matrix::Transpose(world);
		cb.view = Matrix::Transpose(view);
		cb.proj = Matrix::Transpose(proj);

		memcpy_s(data.pData, data.RowPitch, (void*)(&cb), sizeof(cb));
		context->Unmap(m_constantBuffer, 0);
	}

	//�o�[�e�b�N�X�o�b�t�@�[���Z�b�g
	UINT stride = sizeof(PMXVertexInfo);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	//���_�C���v�b�g���C�A�E�g���Z�b�g
	context->IASetInputLayout(m_vertexLayout);

	//�R���X�^���g�o�b�t�@���Z�b�g
	context->VSSetConstantBuffers(0, 1, &m_constantBuffer);

	// �C���f�b�N�X�o�b�t�@��ݒ�
	context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//�v���~�e�B�u�E�g�|���W�[���Z�b�g
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//�}�e���A�����ɕ`�揈��
	UINT startIndex = 0;
	PMXMaterialBuffer mb;
	for (int i = 0; i < m_data->Material()->count; i++)
	{
		//�}�e���A������n��
		SetMaterialBuffer(&(m_data->Material()->info[i]), &mb);

		if (SUCCEEDED(context->Map(m_materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data)))
		{
			memcpy_s(data.pData, data.RowPitch, (void*)(&mb), sizeof(mb));
			context->Unmap(m_materialBuffer, 0);
		}

		//�R���X�^���g�o�b�t�@�[���A�ǂ̃V�F�[�_�[�Ŏg�������w��
		context->PSSetConstantBuffers(1, 1, &m_materialBuffer);

		//�v���~�e�B�u�E�g�|���W�[���Z�b�g
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//�e�N�X�`���[���V�F�[�_�[�ɓn��
		context->PSSetSamplers(0, 1, &m_sampler);
		context->PSSetShaderResources(0, 1, &m_texture[i]);

		context->DrawIndexed(m_data->Material()->info[i].faceVertexCount, startIndex,0);
		startIndex += m_data->Material()->info[i].faceVertexCount;
	}
}


/// <summary>
///	�V�F�[�_�֘A�̏�����
/// </summary>
bool PMXModel::InitShader()
{
	auto graphics = Graphics::GetInstance();

	//���_�V�F�[�_�ƒ��_�C���v�b�g���C�A�E�g���쐬
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(Vec3), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, sizeof(Vec3) * 2, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = (UINT)(sizeof(layout) / sizeof(layout[0]));

	if (FAILED(graphics->CreateVertexShader(L"PMXShader.hlsl", "VS", "vs_5_0", &m_vertexShader, layout, numElements, &m_vertexLayout)))
	{
		SAFE_DELETE(m_vertexShader);
		SAFE_DELETE(m_vertexLayout);
		return false;
	}

	//�s�N�Z���V�F�[�_���쐬
	if (FAILED(graphics->CreatePixleShader(L"PMXShader.hlsl", "PS", "ps_5_0", &m_pixelShader)))
	{
		SAFE_DELETE(m_pixelShader);
		return false;
	}

	//�R���X�^���g�o�b�t�@�[�쐬
	D3D11_BUFFER_DESC cb;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth = sizeof(PMXConstantBuffer);
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(graphics->CreateConstantBuffer(&cb, &m_constantBuffer)))
	{
		SAFE_DELETE(m_constantBuffer);
		return false;
	}

	//�}�e���A���R���X�^���g�o�b�t�@�쐬
	cb.ByteWidth = sizeof(PMXMaterialBuffer);
	if (FAILED(graphics->CreateConstantBuffer(&cb, &m_materialBuffer))) {
		SAFE_DELETE(m_materialBuffer);
		return false;
	}

	return true;
}


/// <summary>
/// �|���S���֘A�̏�����
/// </summary>
bool PMXModel::InitPolygon()
{
	auto graphics = Graphics::GetInstance();
	auto window = Window::GetInstance();

	//�o�[�e�b�N�X�o�b�t�@�[�쐬
	D3D11_BUFFER_DESC bd;
	D3D11_SUBRESOURCE_DATA initData;
	{
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;                                //���̃o�b�t�@���ǂ̂悤�Ɉ����邩
		bd.ByteWidth = sizeof(PMXVertexInfo) * m_data->Vertex()->count;//���_���~���_�T�C�Y
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;                       //�o�b�t�@�̎��
		bd.CPUAccessFlags = 0;                                         //CPU�̃A�N�Z�X����
		bd.MiscFlags = 0;                                              //�I�v�V�����@�Ȃ����0


		ZeroMemory(&initData, sizeof(initData));
		initData.pSysMem = m_data->Vertex()->info.data();//���_�z��̐擪�̃|�C���^

		if (FAILED(graphics->CreateVertexBuffer(&bd, &initData, &m_vertexBuffer)))
		{
			SAFE_DELETE(m_vertexBuffer);
			return false;
		}
	}

	//�C���f�b�N�X�o�b�t�@�쐬
	{
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(int)* m_data->Face()->count;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		initData.pSysMem = m_data->Face()->info.data();
		if (FAILED(graphics->CreateVertexBuffer(&bd, &initData, &m_indexBuffer))) {
			return false;
		}
	}

	//�e�N�X�`���p�T���v���[�쐬
	{
		D3D11_SAMPLER_DESC samDesc;
		ZeroMemory(&samDesc, sizeof(D3D11_SAMPLER_DESC));
		samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		window->Device()->CreateSamplerState(&samDesc, &m_sampler);
	}

	//�e�N�X�`���쐬
	{
		m_texture = new ID3D11ShaderResourceView*[m_data->Material()->count];
		int texIndex = 0;
		for (int i = 0; i < m_data->Material()->count; i++)
		{
			texIndex = m_data->Material()->info[i].textureTableReferenceIndex;
			std::wstring path;
			ShunLib::ToWiden(m_data->Texture()->fileName[texIndex].str,path);
			path = m_filePath + path;
			if (FAILED(graphics->CreateShaderResourceView(path.c_str(),&m_texture[i])))
			{
				return false;
			}
		}
	}
	return true;
}


/// <summary>
/// �}�e���A������ݒ肷��
/// </summary>
void ShunLib::PMX::PMXModel::SetMaterialBuffer(const PmxMaterialInfo * data, PMXMaterialBuffer * buf)
{
	buf->diffuse       = data->diffuse;
	buf->specular      = data->specular;
	buf->specularPower = data->shininess;
	buf->ambient.m_x   = data->ambient.m_x;
	buf->ambient.m_y   = data->ambient.m_y;
	buf->ambient.m_z   = data->ambient.m_z;
	buf->ambient.m_w   = 1.0f;
}
