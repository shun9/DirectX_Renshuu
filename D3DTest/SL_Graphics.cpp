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

	//バーテックスシェーダーを作成
	if (FAILED(this->CreateVertexShader(L"Simple.hlsl", "VS", "vs_5_0", layout, numElements, &m_vertexShader, &m_vertexLayout)))
	{
		//作成失敗
		return E_FAIL;
	}

	//ピクセルシェーダー作成
	if (FAILED(this->CreatePixleShader(L"Simple.hlsl", "PS", "ps_5_0", &m_pixelShader)))
	{
		//作成失敗
		return E_FAIL;
	}


	//コンスタントバッファー0作成 ライトと視点　渡し用
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
	//コンスタントバッファー1作成  FBXMesh内で使用 変換行列とマテリアル用
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

	//メッシュ作成
	m_fbxMesh = new FBXMesh;

	//	階層クラスに情報を渡す
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

	////ファイルを開いて内容を読み込む
	//FILE* fp = NULL;
	//fopen_s(&fp, fileName, "rt");

	//int i = 0;
	////まずは頂点数、ポリゴン数を調べる
	//while (!feof(fp))
	//{
	//	//キーワード読み込み
	//	fscanf_s(fp, "%s ", key, sizeof(key));
	//	//頂点
	//	if (strcmp(key, "v") == 0)
	//	{
	//		mesh->dwNumVert++;
	//	}
	//	//フェイス（ポリゴン）
	//	if (strcmp(key, "f") == 0)
	//	{
	//		mesh->dwNumFace++;
	//	}
	//	i++;
	//}
	////一時的なメモリ確保（頂点バッファとインデックスバッファ）
	//Vec3* pvVertexBuffer = new Vec3[mesh->dwNumVert];
	//int* piFaceBuffer = new int[mesh->dwNumFace * 3];//３頂点ポリゴンなので、1フェイス=3頂点(3インデックス)

	//												  //本読み込み
	//fseek(fp, SEEK_SET, 0);
	//DWORD dwVCount = 0;//読み込みカウンター
	//DWORD dwFCount = 0;//読み込みカウンター
	//while (!feof(fp))
	//{
	//	//キーワード 読み込み
	//	ZeroMemory(key, sizeof(key));
	//	fscanf_s(fp, "%s ", key, sizeof(key));
	//	//頂点 読み込み
	//	if (strcmp(key, "v") == 0)
	//	{
	//		fscanf_s(fp, "%f %f %f", &x, &y, &z);
	//		pvVertexBuffer[dwVCount].m_x = x;//OBJは右手座標系
	//		pvVertexBuffer[dwVCount].m_y = y;
	//		pvVertexBuffer[dwVCount].m_z = z;
	//		dwVCount++;
	//	}
	//	//フェイス（ポリゴン） 読み込み
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
	////バーテックスバッファー作成
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

	////インデックスバッファーを作成
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
//	//バーテックスバッファを作成
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
//	//バーテックスバッファをセット
//	UINT stride = sizeof(SimpleVertex);
//	UINT offset = 0;
//	context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
//
//	//テクスチャ―のサンプラーを作成
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
//	//テクスチャ―の読み込み
//	DirectX::TexMetadata metadata;
//	DirectX::ScratchImage image;
//	if (FAILED(LoadFromWICFile(L"IMG_0150.JPG", 0, &metadata, image))) {
//		return E_FAIL;
//	}
//
//	//テクスチャーからシェーダーリソースを作成
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

	//ビュー行列作成
	Vec3 eyePos(1.0f, 1.0f, 5.0f);
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
	if (SUCCEEDED(context->Map(m_constantBuffer0, 0, D3D11_MAP_WRITE_DISCARD, 0, &data)))
	{

		//ライトの方向を渡す
		Vec3 lightDir(3.0f, 5.0f, 0.0f);
		lightDir.Normalize();
		cb.vLightDir = Vec4(lightDir.m_x, lightDir.m_y, lightDir.m_z, 0.0f);

		//視点位置を渡す
		cb.vEye = Vec4(eyePos.m_x, eyePos.m_y, eyePos.m_z, 0.0f);


		memcpy_s(data.pData, data.RowPitch, (void*)(&cb), sizeof(cb));
		context->Unmap(m_constantBuffer0, 0);
	}

	//このコンスタントバッファーを、どのシェーダーで使うかを指定
	context->VSSetConstantBuffers(0, 1, &m_constantBuffer0);
	context->PSSetConstantBuffers(0, 1, &m_constantBuffer0);
	
	//頂点インプットレイアウトをセット
	context->IASetInputLayout(m_vertexLayout);

	//プリミティブをレンダリング
	m_fbxMesh->RenderMesh(world,view, proj);
}


/// <summary>
/// シェーダーリソースを作成
/// </summary>
HRESULT ShunLib::Graphics::CreateShaderResourceView(
	const wchar_t* path,
	ID3D11ShaderResourceView** srv)
{
	//テクスチャ―の読み込み
	DirectX::TexMetadata metadata;
	DirectX::ScratchImage image;
	if (FAILED(LoadFromWICFile(path, 0, &metadata, image))) {
		//読み込み失敗
		return E_FAIL;
	}

	auto device = ShunLib::Window::GetInstance()->Device();

	//テクスチャーからシェーダーリソースを作成
	if (FAILED(DirectX::CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, srv))) {
		//作成失敗
		*srv = nullptr;
		return E_FAIL;
	}

	return S_OK;
}


/// <summary>
/// バーテックスシェーダーと頂点インプットレイアウトを作成
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

	//hlslファイル読み込み用ブロブ
	//ブロブの時点では何のシェーダーなのか分からない
	ID3DBlob* compiledShader = NULL;
	ID3DBlob* error = NULL;

	//バーテックスシェーダーのブロブを作成
	if (FAILED(D3DCompileFromFile((LPCWSTR)fileName, NULL, NULL, (LPCSTR)entryPoint, (LPCSTR)target, 0, 0, &compiledShader, &error)))
	{
		return E_FAIL;
	}
	SAFE_RELEASE(error);

	//ブロブからバーテックスシェーダー作成
	if (FAILED(device->CreateVertexShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, vs)))
	{
		SAFE_RELEASE(compiledShader);
		return E_FAIL;
	}
	
	//頂点インプットレイアウトを作成
	if (FAILED(device->CreateInputLayout(layout, layoutSize, compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), il)))
	{
		SAFE_RELEASE(compiledShader);
		return E_FAIL;
	}

	SAFE_RELEASE(compiledShader);

	return S_OK;
}


/// <summary>
/// ピクセルシェーダーを作成
/// </summary>
HRESULT ShunLib::Graphics::CreatePixleShader(
	const wchar_t* fileName,
	const char* entryPoint,
	const char* target,
	ID3D11PixelShader** ps) 
{
	auto window = Window::GetInstance();
	auto device = window->Device();

	//hlslファイル読み込み用ブロブ
	//ブロブの時点では何のシェーダーなのか分からない
	ID3DBlob* compiledShader = NULL;
	ID3DBlob* error = NULL;

	//バーテックスシェーダーのブロブを作成
	if (FAILED(D3DCompileFromFile((LPCWSTR)fileName, NULL, NULL, (LPCSTR)entryPoint, (LPCSTR)target, 0, 0, &compiledShader, &error)))
	{
		return E_FAIL;
	}
	SAFE_RELEASE(error);

	//ピクセルシェーダー作成
	if (FAILED(device->CreatePixelShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, ps)))
	{
		SAFE_RELEASE(compiledShader);
		return E_FAIL;
	}
	
	SAFE_RELEASE(compiledShader);

	return S_OK;
}

