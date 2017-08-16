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
		//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

	//ファイルを開いて内容を読み込む
	FILE* fp = NULL;
	fopen_s(&fp, fileName, "rt");

	int i = 0;
	//まずは頂点数、ポリゴン数を調べる
	while (!feof(fp))
	{
		//キーワード読み込み
		fscanf_s(fp, "%s ", key, sizeof(key));
		//頂点
		if (strcmp(key, "v") == 0)
		{
			mesh->dwNumVert++;
		}
		//フェイス（ポリゴン）
		if (strcmp(key, "f") == 0)
		{
			mesh->dwNumFace++;
		}
		i++;
	}
	//一時的なメモリ確保（頂点バッファとインデックスバッファ）
	Vec3* pvVertexBuffer = new Vec3[mesh->dwNumVert];
	int* piFaceBuffer = new int[mesh->dwNumFace * 3];//３頂点ポリゴンなので、1フェイス=3頂点(3インデックス)

													  //本読み込み	
	fseek(fp, SEEK_SET, 0);
	DWORD dwVCount = 0;//読み込みカウンター
	DWORD dwFCount = 0;//読み込みカウンター
	while (!feof(fp))
	{
		//キーワード 読み込み
		ZeroMemory(key, sizeof(key));
		fscanf_s(fp, "%s ", key, sizeof(key));
		//頂点 読み込み
		if (strcmp(key, "v") == 0)
		{
			fscanf_s(fp, "%f %f %f", &x, &y, &z);
			pvVertexBuffer[dwVCount].m_x = x;//OBJは右手座標系なのでxあるいはｚを反転
			pvVertexBuffer[dwVCount].m_y = y;
			pvVertexBuffer[dwVCount].m_z = z;
			dwVCount++;
		}
		//フェイス（ポリゴン） 読み込み
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
	//バーテックスバッファー作成
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

	//インデックスバッファーを作成
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
	Vec3 eyePos(1.0f, 1.0f, 4.0f);
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
		static float pos = 0.0f;
		pos += 0.0001f;
		world = Matrix::CreateTranslation(Vec3(0.0f, 0.0f,pos));

		//行列を渡す
		Matrix m = world*view*proj;
		cb.mWVP = Matrix::Transpose(m);
		
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
	
	UINT stride = sizeof(Vec3);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_mesh.pVertexBuffer, &stride, &offset);
	//インデックスバッファーをセット
	stride = sizeof(int);
	offset = 0;
	context->IASetIndexBuffer(m_mesh.pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//プリミティブをレンダリング
	context->DrawIndexed(m_mesh.dwNumFace * 3, 0, 0);
}