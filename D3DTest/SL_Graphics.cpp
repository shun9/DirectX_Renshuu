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
	
	//hlsl�t�@�C���ǂݍ��ݗp�u���u
	//�u���u�̎��_�ł͉��̃V�F�[�_�[�Ȃ̂�������Ȃ�
	ID3DBlob* compiledShader = NULL;
	ID3DBlob* error = NULL;

	//�o�[�e�b�N�X�V�F�[�_�[�̃u���u���쐬
	if (FAILED(D3DCompileFromFile(L"Simple.hlsl", NULL, NULL, "VS", "vs_5_0", 0, 0, &compiledShader, &error)))
	{
		MessageBox(0, L"hlsl�ǂݍ��ݎ��s",NULL,MB_OK);
		return E_FAIL;
	}
	SAFE_RELEASE(error);
	
	//�u���u����o�[�e�b�N�X�V�F�[�_�[�쐬
	if (FAILED(device->CreateVertexShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_vertexShader)))
	{
		SAFE_RELEASE(compiledShader);
		MessageBox(0, L"�o�[�e�b�N�X�V�F�[�_�[�쐬���s", NULL, MB_OK);
		return E_FAIL;
	}

	//���_�C���v�b�g���C�A�E�g���`
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElement = sizeof(layout) / sizeof(layout[0]);

	//���_�C���v�b�g���C�A�E�g���쐬
	if (FAILED(device->CreateInputLayout(layout,numElement,compiledShader->GetBufferPointer(),compiledShader->GetBufferSize(),&m_vertexLayout)))
	{
		return FALSE;
	}

	//�u���u����s�N�Z���V�F�[�_�[�쐬
	if (FAILED(D3DCompileFromFile(L"Simple.hlsl", NULL, NULL, "PS", "ps_5_0", 0, 0, &compiledShader, &error)))
	{
		MessageBox(0, L"hlsl�ǂݍ��ݎ��s", NULL, MB_OK);
		return E_FAIL;
	}
	SAFE_RELEASE(error);

	//�u���u����s�N�Z���V�F�[�_�[�쐬
	if (FAILED(device->CreatePixelShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_pixelShader)))
	{
		SAFE_RELEASE(compiledShader);
		MessageBox(0, L"�s�N�Z���V�F�[�_�[�쐬���s", NULL, MB_OK);
		return E_FAIL;
	}
	SAFE_RELEASE(compiledShader);

	//�R���X�^���g�o�b�t�@�[�쐬
	D3D11_BUFFER_DESC cd;
	cd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cd.ByteWidth = sizeof(SIMPLESHADER_CONSTANT_BUFFER);
	cd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cd.MiscFlags = 0;
	//cd.StructureByteStride = 0;
	cd.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(device->CreateBuffer(&cd, NULL,&m_constantBuffer)))
	{
		return E_FAIL;
	}

	return S_OK;
}


HRESULT ShunLib::Graphics::InitStaticMesh(LPSTR fileName, MESH* mesh)
{
	auto window = Window::GetInstance();
	auto device = window->Device();

	float x, y, z;
	int v1 = 0, v2 = 0, v3 = 0;
	char key[190] = { 0 };

	//�t�@�C�����J���ē��e��ǂݍ���
	FILE* fp = NULL;
	fopen_s(&fp, fileName, "rt");

	int i = 0;
	//�܂��͒��_���A�|���S�����𒲂ׂ�
	while (!feof(fp))
	{
		//�L�[���[�h�ǂݍ���
		fscanf_s(fp, "%s ", key, sizeof(key));
		//���_
		if (strcmp(key, "v") == 0)
		{
			mesh->dwNumVert++;
		}
		//�t�F�C�X�i�|���S���j
		if (strcmp(key, "f") == 0)
		{
			mesh->dwNumFace++;
		}
		i++;
	}
	//�ꎞ�I�ȃ������m�ہi���_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@�j
	Vec3* pvVertexBuffer = new Vec3[mesh->dwNumVert];
	int* piFaceBuffer = new int[mesh->dwNumFace * 3];//�R���_�|���S���Ȃ̂ŁA1�t�F�C�X=3���_(3�C���f�b�N�X)

													  //�{�ǂݍ���	
	fseek(fp, SEEK_SET, 0);
	DWORD dwVCount = 0;//�ǂݍ��݃J�E���^�[
	DWORD dwFCount = 0;//�ǂݍ��݃J�E���^�[
	while (!feof(fp))
	{
		//�L�[���[�h �ǂݍ���
		ZeroMemory(key, sizeof(key));
		fscanf_s(fp, "%s ", key, sizeof(key));
		//���_ �ǂݍ���
		if (strcmp(key, "v") == 0)
		{
			fscanf_s(fp, "%f %f %f", &x, &y, &z);
			pvVertexBuffer[dwVCount].m_x = x;//OBJ�͉E����W�n�Ȃ̂�x���邢�͂��𔽓]
			pvVertexBuffer[dwVCount].m_y = y;
			pvVertexBuffer[dwVCount].m_z = z;
			dwVCount++;
		}
		//�t�F�C�X�i�|���S���j �ǂݍ���
		if (strcmp(key, "f") == 0)
		{
			fscanf_s(fp, "%d// %d// %d//", &v1, &v2, &v3);
			piFaceBuffer[dwFCount * 3] = v1 - 1;
			piFaceBuffer[dwFCount * 3 + 1] = v2 - 1;
			piFaceBuffer[dwFCount * 3 + 2] = v3 - 1;
			dwFCount++;
		}

	}

	fclose(fp);
	//�o�[�e�b�N�X�o�b�t�@�[�쐬
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vec3) * mesh->dwNumVert * 3;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = pvVertexBuffer;
	if (FAILED(device->CreateBuffer(&bd, &InitData, &mesh->pVertexBuffer)))
		return E_FAIL;

	//�C���f�b�N�X�o�b�t�@�[���쐬
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(int) * mesh->dwNumFace * 3;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	InitData.pSysMem = piFaceBuffer;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	if (FAILED(device->CreateBuffer(&bd, &InitData, &mesh->pIndexBuffer)))
		return E_FAIL;

	delete[] pvVertexBuffer;
	delete[] piFaceBuffer;

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
	Vec3 eyePos(1.0f, 1.0f, 4.0f);
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
	if (SUCCEEDED(context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data)))
	{
		static float pos = 0.0f;
		pos += 0.0001f;
		world = Matrix::CreateTranslation(Vec3(0.0f, 0.0f,pos));

		//�s���n��
		Matrix m = world*view*proj;
		cb.mWVP = Matrix::Transpose(m);
		
		memcpy_s(data.pData, data.RowPitch, (void*)(&cb), sizeof(cb));
		context->Unmap(m_constantBuffer, 0);
	}

	//���̃R���X�^���g�o�b�t�@�[���A�ǂ̃V�F�[�_�[�Ŏg�������w��
	context->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	context->PSSetConstantBuffers(0, 1, &m_constantBuffer);

	//���_�C���v�b�g���C�A�E�g���Z�b�g		 	
	context->IASetInputLayout(m_vertexLayout);
	//�v���~�e�B�u�E�g�|���W�[���Z�b�g
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	
	UINT stride = sizeof(Vec3);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_mesh.pVertexBuffer, &stride, &offset);
	//�C���f�b�N�X�o�b�t�@�[���Z�b�g
	stride = sizeof(int);
	offset = 0;
	context->IASetIndexBuffer(m_mesh.pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//�v���~�e�B�u�������_�����O
	context->DrawIndexed(m_mesh.dwNumFace * 3, 0, 0);
}