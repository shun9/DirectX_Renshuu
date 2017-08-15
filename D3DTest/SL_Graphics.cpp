#include "SL_Graphics.h"
#include "SL_Window.h"
#include <SL_MacroConstants.h>
#include <SL_ConstantNumber.h>
#include <SL_RandomNumber.h>

#include <WICTextureLoader.h>
#include <DirectXTex.h>
const int ShunLib::Graphics::MAX_MODEL;

HRESULT ShunLib::Graphics::InitShader()
{
	auto window = Window::GetInstance();
	auto device = window->Device();
	
	//hlslファイル読み込み用ブロブ
	//ブロブの時点では何のシェーダーなのか分からない
	ID3DBlob* compiledShader = NULL;
	ID3DBlob* error = NULL;

	//バーテックスシェーダーのブロブを作成
	if (FAILED(D3DCompileFromFile(L"Simple.hlsl", NULL, NULL, "VS", "vs_5_0", 0, 0, &compiledShader, &error)))
	{
		MessageBox(0, L"hlsl読み込み失敗",NULL,MB_OK);
		return E_FAIL;
	}
	SAFE_RELEASE(error);
	
	//ブロブからバーテックスシェーダー作成
	if (FAILED(device->CreateVertexShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_vertexShader)))
	{
		SAFE_RELEASE(compiledShader);
		MessageBox(0, L"バーテックスシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}

	//頂点インプットレイアウトを定義
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElement = sizeof(layout) / sizeof(layout[0]);

	//頂点インプットレイアウトを作成
	if (FAILED(device->CreateInputLayout(layout,numElement,compiledShader->GetBufferPointer(),compiledShader->GetBufferSize(),&m_vertexLayout)))
	{
		return FALSE;
	}

	//ブロブからピクセルシェーダー作成
	if (FAILED(D3DCompileFromFile(L"Simple.hlsl", NULL, NULL, "PS", "ps_5_0", 0, 0, &compiledShader, &error)))
	{
		MessageBox(0, L"hlsl読み込み失敗", NULL, MB_OK);
		return E_FAIL;
	}
	SAFE_RELEASE(error);

	//ブロブからピクセルシェーダー作成
	if (FAILED(device->CreatePixelShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_pixelShader)))
	{
		SAFE_RELEASE(compiledShader);
		MessageBox(0, L"ピクセルシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}
	SAFE_RELEASE(compiledShader);

	//コンスタントバッファー作成
	D3D11_BUFFER_DESC cd;
	cd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cd.ByteWidth = sizeof(SIMPLESHADER_CONSTANT_BUFFER);
	cd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cd.MiscFlags = 0;
	cd.StructureByteStride = 0;
	cd.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(device->CreateBuffer(&cd, NULL,&m_constantBuffer)))
	{
		return E_FAIL;
	}

	return S_OK;
}


HRESULT ShunLib::Graphics::InitPolygon()
{
	auto window = Window::GetInstance();
	auto device = window->Device();
	auto context = window->DeviceContext();

	//バーテックスバッファを作成
	SimpleVertex vertex[] = {
		{Vec3(-0.5f,-0.5f,0.0f),Vec2(0.0f,1.0f)},
		{Vec3(-0.5f,0.5f,0.0f),Vec2(0.0f,0.0f)},
		{Vec3(0.5f,-0.5f,0.0f),Vec2(1.0f,1.0f)},
		{Vec3(0.5f,0.5f,0.0f),Vec2(1.0f,0.0f)},
	};

	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = vertex;
	if (FAILED(device->CreateBuffer(&bd,&initData,&m_vertexBuffer)))
	{
		return E_FAIL;
	}

	//バーテックスバッファをセット
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	
	//テクスチャ―のサンプラーを作成
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory(&samDesc, sizeof(D3D11_SAMPLER_DESC));
	samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	if (FAILED(device->CreateSamplerState(&samDesc, &m_sampleLinear)))
	{
		return E_FAIL;
	}

	//テクスチャ―の読み込み
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage image;
	if (FAILED(LoadFromWICFile(L"IMG_0150.JPG", 0, &metadata, image))) {
		return E_FAIL;
	}
	
	//テクスチャーからシェーダーリソースを作成
	if (FAILED(DirectX::CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, &m_texture))) {
		return E_FAIL;
	}

	context->PSSetSamplers(0,1,&m_sampleLinear);
	context->PSSetShaderResources(0, 1, &m_texture);
	return S_OK;
}


void ShunLib::Graphics::TestUpdate()
{
	//if (GetKeyState(VK_RIGHT))
	//{
	//	for (int i = 0; i < MAX_MODEL; i++)
	//	{
	//		m_model[i].pos.m_x += 0.01f;
	//	}
	//}
}


void ShunLib::Graphics::TestRender()
{
	auto window = Window::GetInstance();
	auto device = window->Device();
	auto context = window->DeviceContext();

	Matrix world;
	Matrix view;
	Matrix proj;
	
	//ビュー行列作成
	Vec3 eyePos(1.0f, 1.0f, 2.0f);
	Vec3 lookAt(0.0f, 0.0f, 0.0f);
	Vec3 Up(0.0f, 1.0f, 0.0f);
	

	view = Matrix::CreateLookAt(eyePos, lookAt, Up);

	proj = Matrix::CreateProj(ConstantNumber::PI / 4.0f, (window->Width()) / (window->Height()), 0.001f, 100.0f);

	//使用するシェーダーの登録
	context->VSSetShader(m_vertexShader, NULL, NULL);
	context->PSSetShader(m_pixelShader, NULL, NULL);

														   
	//シェーダーのコンスタントバッファにデータを渡す
	D3D11_MAPPED_SUBRESOURCE data;
	SIMPLESHADER_CONSTANT_BUFFER cb;
	if (SUCCEEDED(context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data)))
	{
		//行列を渡す
		//world = Matrix::CreateTranslation(m_model[0].pos);
		Matrix m = world*view*proj;
		cb.mWVP = Matrix::Transpose(m);
		//cb.world = Matrix::Transpose(world);

		//色を渡す
		//m_model[0].color = { 1,1,0,1 };
		//cb.color = m_model[0].color;
		cb.color = Vec4(1, 0, 0, 1);

		//ライトの方向を渡す
		//cb.lightDir = m_lightDir.Normalize();

		//視点を渡す
		//cb.eyePos = Vec4(eyePos.m_x, eyePos.m_y, eyePos.m_z,0.0f);
		
		memcpy_s(data.pData, data.RowPitch, (void*)(&cb), sizeof(cb));
		context->Unmap(m_constantBuffer, 0);
	}

	//このコンスタントバッファーを、どのシェーダーで使うかを指定
	context->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	context->PSSetConstantBuffers(0, 1, &m_constantBuffer);

	//頂点インプットレイアウトをセット		 	
	context->IASetInputLayout(m_vertexLayout);
	//プリミティブ・トポロジーをセット
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	
	//プリミティブをレンダリング
	context->Draw(4, 0);
}