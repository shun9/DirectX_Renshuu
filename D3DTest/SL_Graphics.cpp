#include "SL_Graphics.h"
#include "SL_Window.h"
#include <SL_MacroConstants.h>
#include <SL_ConstantNumber.h>
#include <SL_RandomNumber.h>

#include <WICTextureLoader.h>
#include <DirectXTex.h>

HRESULT ShunLib::Graphics::InitShader()
{
	m_pmxModel = new PMX::PMXModel();
	if (!m_pmxModel->Init("IkazuchiModel\\Ikazuchi.pmx"))
	{
		return E_FAIL;
	}

	return S_OK;
}

void ShunLib::Graphics::TestUpdate()
{
}


void ShunLib::Graphics::TestRender()
{
	auto window = Window::GetInstance();
	auto device = window->Device();
	auto context = window->DeviceContext();

	Matrix world;
	Matrix view;
	Matrix proj;

	//�r���[�s��쐬
	Vec3 eyePos(5.0f, 10.0f, -20.0f);
	Vec3 lookAt(0.0f, 10.0f, 0.0f);
	Vec3 Up(0.0f, 1.0f, 0.0f);
	view = Matrix::CreateLookAt(eyePos, lookAt, Up);

	proj = Matrix::CreateProj(ConstantNumber::PI / 4.0f, (window->Width()) / (window->Height()), 0.001f, 1000.0f);

	m_pmxModel->Draw(world, view, proj);

	//���C�g�̕���
	Vec3 lightDir(3.0f, 5.0f, 0.0f);
	lightDir.Normalize();
}


/// <summary>
/// �V�F�[�_�[���\�[�X���쐬
/// </summary>
HRESULT ShunLib::Graphics::CreateShaderResourceView(
	const wchar_t* path,
	ID3D11ShaderResourceView** srv)
{
	//�e�N�X�`���\�̓ǂݍ���
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage image;
	if (FAILED(LoadFromWICFile(path, 0, &metadata, image))) {
		//�ǂݍ��ݎ��s
		return E_FAIL;
	}

	auto device = ShunLib::Window::GetInstance()->Device();

	//�e�N�X�`���[����V�F�[�_�[���\�[�X���쐬
	if (FAILED(DirectX::CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, srv))) {
		//�쐬���s
		*srv = nullptr;
		return E_FAIL;
	}

	return S_OK;
}


/// <summary>
/// �o�[�e�b�N�X�V�F�[�_�[�ƒ��_�C���v�b�g���C�A�E�g���쐬
/// </summary>
HRESULT ShunLib::Graphics::CreateVertexShader(
	const wchar_t* fileName,
	const char* entryPoint,
	const char* target,
	ID3D11VertexShader** vs,
	const D3D11_INPUT_ELEMENT_DESC layout[],
	const UINT& layoutSize,
	ID3D11InputLayout** il)
{
	auto device = Window::GetInstance()->Device();

	//hlsl�t�@�C���ǂݍ��ݗp�u���u
	//�u���u�̎��_�ł͉��̃V�F�[�_�[�Ȃ̂�������Ȃ�
	ID3DBlob* compiledShader = NULL;
	ID3DBlob* error = NULL;

	//�o�[�e�b�N�X�V�F�[�_�[�̃u���u���쐬
	if (FAILED(D3DCompileFromFile((LPCWSTR)fileName, NULL, NULL, (LPCSTR)entryPoint, (LPCSTR)target, 0, 0, &compiledShader, &error)))
	{
		return E_FAIL;
	}
	SAFE_RELEASE(error);

	//�u���u����o�[�e�b�N�X�V�F�[�_�[�쐬
	if (FAILED(device->CreateVertexShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, vs)))
	{
		SAFE_RELEASE(compiledShader);
		return E_FAIL;
	}

	//���_�C���v�b�g���C�A�E�g���쐬
	if (FAILED(device->CreateInputLayout(layout, layoutSize, compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), il)))
	{
		SAFE_RELEASE(compiledShader);
		return E_FAIL;
	}

	SAFE_RELEASE(compiledShader);

	return S_OK;
}


/// <summary>
/// �s�N�Z���V�F�[�_�[���쐬
/// </summary>
HRESULT ShunLib::Graphics::CreatePixleShader(
	const wchar_t* fileName,
	const char* entryPoint,
	const char* target,
	ID3D11PixelShader** ps)
{
	auto window = Window::GetInstance();
	auto device = window->Device();

	//hlsl�t�@�C���ǂݍ��ݗp�u���u
	//�u���u�̎��_�ł͉��̃V�F�[�_�[�Ȃ̂�������Ȃ�
	ID3DBlob* compiledShader = NULL;
	ID3DBlob* error = NULL;

	//�s�N�Z���V�F�[�_�[�̃u���u���쐬
	if (FAILED(D3DCompileFromFile((LPCWSTR)fileName, NULL, NULL, (LPCSTR)entryPoint, (LPCSTR)target, 0, 0, &compiledShader, &error)))
	{
		return E_FAIL;
	}
	SAFE_RELEASE(error);

	//�s�N�Z���V�F�[�_�[�쐬
	if (FAILED(device->CreatePixelShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, ps)))
	{
		SAFE_RELEASE(compiledShader);
		return E_FAIL;
	}

	SAFE_RELEASE(compiledShader);

	return S_OK;
}

/// <summary>
/// �R���X�^���g�o�b�t�@���쐬
/// </summary>
HRESULT ShunLib::Graphics::CreateConstantBuffer(D3D11_BUFFER_DESC * cd, ID3D11Buffer ** buf)
{
	auto device = Window::GetInstance()->Device();

	if (FAILED(device->CreateBuffer(cd, NULL, buf)))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT ShunLib::Graphics::CreateVertexBuffer(D3D11_BUFFER_DESC * bd, D3D11_SUBRESOURCE_DATA* data, ID3D11Buffer ** buf)
{
	auto device = Window::GetInstance()->Device();

	if (FAILED(device->CreateBuffer(bd, data, buf)))
	{
		return E_FAIL;

	}
	return S_OK;
}



