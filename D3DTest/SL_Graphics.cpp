#include "SL_Graphics.h"
#include "SL_Window.h"
#include <SL_MacroConstants.h>
#include <SL_ConstantNumber.h>
#include <SL_RandomNumber.h>

#include <WICTextureLoader.h>
#include <DirectXTex.h>
HRESULT ShunLib::Graphics::InitShader()
{
	auto window = Window::GetInstance();
	auto device = window->Device();


	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	//�o�[�e�b�N�X�V�F�[�_�[���쐬
	if (FAILED(this->CreateVertexShader(L"Simple.hlsl", "VS", "vs_5_0", layout, numElements, &m_vertexShader, &m_vertexLayout)))
	{
		//�쐬���s
		return E_FAIL;
	}

	//�s�N�Z���V�F�[�_�[�쐬
	if (FAILED(this->CreatePixleShader(L"Simple.hlsl", "PS", "ps_5_0", &m_pixelShader)))
	{
		//�쐬���s
		return E_FAIL;
	}


	//�R���X�^���g�o�b�t�@�[0�쐬 ���C�g�Ǝ��_�@�n���p
	D3D11_BUFFER_DESC cb;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth = sizeof(SIMPLESHADER_CONSTANT_BUFFER);
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(device->CreateBuffer(&cb, NULL, &m_constantBuffer0)))
	{
		return E_FAIL;
	}
	//�R���X�^���g�o�b�t�@�[1�쐬  FBXMesh���Ŏg�p �ϊ��s��ƃ}�e���A���p
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth = sizeof(SHADER_GLOBAL1);
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(device->CreateBuffer(&cb, NULL, &m_constantBuffer1)))
	{
		return E_FAIL;
	}

	//���b�V���쐬
	m_fbxMesh = new FBXMesh;

	//	�K�w�N���X�ɏ���n��
	m_fbxMesh->SetSampleLinear(m_sampleLinear);
	m_fbxMesh->ConstantBuffer(m_constantBuffer1);

	if (FAILED(m_fbxMesh->CreateFromFBX("Enemy.fbx")))
	{
		return E_FAIL;
	}

	return S_OK;
}


HRESULT ShunLib::Graphics::InitStaticMesh(LPSTR fileName, MESH* mesh)
{
	//auto window = Window::GetInstance();
	//auto device = window->Device();

	//float x, y, z;
	//int v1 = 0, v2 = 0, v3 = 0;
	//char key[190] = { 0 };

	////�t�@�C�����J���ē��e��ǂݍ���
	//FILE* fp = NULL;
	//fopen_s(&fp, fileName, "rt");

	//int i = 0;
	////�܂��͒��_���A�|���S�����𒲂ׂ�
	//while (!feof(fp))
	//{
	//	//�L�[���[�h�ǂݍ���
	//	fscanf_s(fp, "%s ", key, sizeof(key));
	//	//���_
	//	if (strcmp(key, "v") == 0)
	//	{
	//		mesh->dwNumVert++;
	//	}
	//	//�t�F�C�X�i�|���S���j
	//	if (strcmp(key, "f") == 0)
	//	{
	//		mesh->dwNumFace++;
	//	}
	//	i++;
	//}
	////�ꎞ�I�ȃ������m�ہi���_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@�j
	//Vec3* pvVertexBuffer = new Vec3[mesh->dwNumVert];
	//int* piFaceBuffer = new int[mesh->dwNumFace * 3];//�R���_�|���S���Ȃ̂ŁA1�t�F�C�X=3���_(3�C���f�b�N�X)

	//												  //�{�ǂݍ���
	//fseek(fp, SEEK_SET, 0);
	//DWORD dwVCount = 0;//�ǂݍ��݃J�E���^�[
	//DWORD dwFCount = 0;//�ǂݍ��݃J�E���^�[
	//while (!feof(fp))
	//{
	//	//�L�[���[�h �ǂݍ���
	//	ZeroMemory(key, sizeof(key));
	//	fscanf_s(fp, "%s ", key, sizeof(key));
	//	//���_ �ǂݍ���
	//	if (strcmp(key, "v") == 0)
	//	{
	//		fscanf_s(fp, "%f %f %f", &x, &y, &z);
	//		pvVertexBuffer[dwVCount].m_x = x;//OBJ�͉E����W�n
	//		pvVertexBuffer[dwVCount].m_y = y;
	//		pvVertexBuffer[dwVCount].m_z = z;
	//		dwVCount++;
	//	}
	//	//�t�F�C�X�i�|���S���j �ǂݍ���
	//	if (strcmp(key, "f") == 0)
	//	{
	//		fscanf_s(fp, "%d// %d// %d//", &v1, &v2, &v3);
	//		piFaceBuffer[dwFCount * 3] = v1 - 1;
	//		piFaceBuffer[dwFCount * 3 + 1] = v2 - 1;
	//		piFaceBuffer[dwFCount * 3 + 2] = v3 - 1;
	//		dwFCount++;
	//	}

	//}

	//fclose(fp);
	////�o�[�e�b�N�X�o�b�t�@�[�쐬
	//D3D11_BUFFER_DESC bd;
	//bd.Usage = D3D11_USAGE_DEFAULT;
	//bd.ByteWidth = sizeof(Vec3) * mesh->dwNumVert * 3;
	//bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//bd.CPUAccessFlags = 0;
	//bd.MiscFlags = 0;
	//D3D11_SUBRESOURCE_DATA InitData;
	//InitData.pSysMem = pvVertexBuffer;
	//if (FAILED(device->CreateBuffer(&bd, &InitData, &mesh->pVertexBuffer)))
	//	return E_FAIL;

	////�C���f�b�N�X�o�b�t�@�[���쐬
	//bd.Usage = D3D11_USAGE_DEFAULT;
	//bd.ByteWidth = sizeof(int) * mesh->dwNumFace * 3;
	//bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	//bd.CPUAccessFlags = 0;
	//bd.MiscFlags = 0;
	//InitData.pSysMem = piFaceBuffer;
	//InitData.SysMemPitch = 0;
	//InitData.SysMemSlicePitch = 0;
	//if (FAILED(device->CreateBuffer(&bd, &InitData, &mesh->pIndexBuffer)))
	//	return E_FAIL;

	//delete[] pvVertexBuffer;
	//delete[] piFaceBuffer;

	return S_OK;
}


//HRESULT ShunLib::Graphics::InitPolygon()
//{
//	auto window = Window::GetInstance();
//	auto device = window->Device();
//	auto context = window->DeviceContext();
//
//	//�o�[�e�b�N�X�o�b�t�@���쐬
//	SimpleVertex vertex[] = {
//		{Vec3(-0.5f,-0.5f,0.0f),Vec2(0.0f,1.0f)},
//		{Vec3(-0.5f,0.5f,0.0f),Vec2(0.0f,0.0f)},
//		{Vec3(0.5f,-0.5f,0.0f),Vec2(1.0f,1.0f)},
//		{Vec3(0.5f,0.5f,0.0f),Vec2(1.0f,0.0f)},
//	};
//
//	D3D11_BUFFER_DESC bd;
//	bd.Usage = D3D11_USAGE_DEFAULT;
//	bd.ByteWidth = sizeof(SimpleVertex) * 4;
//	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	bd.CPUAccessFlags = 0;
//	bd.MiscFlags = 0;
//
//	D3D11_SUBRESOURCE_DATA initData;
//	initData.pSysMem = vertex;
//	if (FAILED(device->CreateBuffer(&bd,&initData,&m_vertexBuffer)))
//	{
//		return E_FAIL;
//	}
//
//	//�o�[�e�b�N�X�o�b�t�@���Z�b�g
//	UINT stride = sizeof(SimpleVertex);
//	UINT offset = 0;
//	context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
//
//	//�e�N�X�`���\�̃T���v���[���쐬
//	D3D11_SAMPLER_DESC samDesc;
//	ZeroMemory(&samDesc, sizeof(D3D11_SAMPLER_DESC));
//	samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
//	samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
//	samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
//	samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
//	if (FAILED(device->CreateSamplerState(&samDesc, &m_sampleLinear)))
//	{
//		return E_FAIL;
//	}
//
//	//�e�N�X�`���\�̓ǂݍ���
//	DirectX::TexMetadata metadata;
//	DirectX::ScratchImage image;
//	if (FAILED(LoadFromWICFile(L"IMG_0150.JPG", 0, &metadata, image))) {
//		return E_FAIL;
//	}
//
//	//�e�N�X�`���[����V�F�[�_�[���\�[�X���쐬
//	if (FAILED(DirectX::CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, &m_texture))) {
//		return E_FAIL;
//	}
//
//	context->PSSetSamplers(0,1,&m_sampleLinear);
//	context->PSSetShaderResources(0, 1, &m_texture);
//	return S_OK;
//}


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
	Vec3 eyePos(1.0f, 1.0f, 5.0f);
	Vec3 lookAt(0.0f, 0.0f, 0.0f);
	Vec3 Up(0.0f, 1.0f, 0.0f);


	view = Matrix::CreateLookAt(eyePos, lookAt, Up);

	proj = Matrix::CreateProj(ConstantNumber::PI / 4.0f, (window->Width()) / (window->Height()), 0.001f, 100.0f);

	//�g�p����V�F�[�_�[�̓o�^
	context->VSSetShader(m_vertexShader, NULL, NULL);
	context->PSSetShader(m_pixelShader, NULL, NULL);

	//�V�F�[�_�[�̃R���X�^���g�o�b�t�@�Ƀf�[�^��n��
	D3D11_MAPPED_SUBRESOURCE data;
	SIMPLESHADER_CONSTANT_BUFFER cb;
	if (SUCCEEDED(context->Map(m_constantBuffer0, 0, D3D11_MAP_WRITE_DISCARD, 0, &data)))
	{

		//���C�g�̕�����n��
		Vec3 lightDir(3.0f, 5.0f, 0.0f);
		lightDir.Normalize();
		cb.vLightDir = Vec4(lightDir.m_x, lightDir.m_y, lightDir.m_z, 0.0f);

		//���_�ʒu��n��
		cb.vEye = Vec4(eyePos.m_x, eyePos.m_y, eyePos.m_z, 0.0f);


		memcpy_s(data.pData, data.RowPitch, (void*)(&cb), sizeof(cb));
		context->Unmap(m_constantBuffer0, 0);
	}

	//���̃R���X�^���g�o�b�t�@�[���A�ǂ̃V�F�[�_�[�Ŏg�������w��
	context->VSSetConstantBuffers(0, 1, &m_constantBuffer0);
	context->PSSetConstantBuffers(0, 1, &m_constantBuffer0);
	
	//���_�C���v�b�g���C�A�E�g���Z�b�g
	context->IASetInputLayout(m_vertexLayout);

	//�v���~�e�B�u�������_�����O
	m_fbxMesh->RenderMesh(world,view, proj);
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
	const D3D11_INPUT_ELEMENT_DESC layout[],
	const UINT& layoutSize,
	ID3D11VertexShader** vs,
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

	//�o�[�e�b�N�X�V�F�[�_�[�̃u���u���쐬
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

