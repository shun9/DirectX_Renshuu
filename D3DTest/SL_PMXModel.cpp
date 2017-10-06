//************************************************/
//* @file  :PMXModel.cpp
//* @brief :PMXモデルを描画する
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
	//モデル読み込み
	m_data = std::make_unique<PMXModelData>();
	if (!m_data->LoadModel(file))
	{
		return false;
	}

	auto graphics = Graphics::GetInstance();
	//シェーダ関連の初期化
	if (!InitShader())
	{
		return false;
	}

	//ポリゴン関連の初期化
	if (!InitPolygon())
	{
		return false;
	}

	return true;
}

void PMXModel::Draw(const Matrix & world, const Matrix & view, const Matrix & proj)
{
	auto context = Window::GetInstance()->DeviceContext();

	//使用するシェーダーの登録
	context->VSSetShader(m_vertexShader, NULL, 0);
	context->PSSetShader(m_pixelShader, NULL, 0);

	//シェーダーのコンスタントバッファーに各種データを渡す
	D3D11_MAPPED_SUBRESOURCE data;
	PMXConstantBuffer cb;
	if (SUCCEEDED(context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data)))
	{
		//ワールド、カメラ、射影行列を渡す
		cb.world = Matrix::Transpose(world);
		cb.view = Matrix::Transpose(view);
		cb.proj = Matrix::Transpose(proj);

		memcpy_s(data.pData, data.RowPitch, (void*)(&cb), sizeof(cb));
		context->Unmap(m_constantBuffer, 0);
	}

	//バーテックスバッファーをセット
	UINT stride = sizeof(PMXVertexInfo);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	//頂点インプットレイアウトをセット
	context->IASetInputLayout(m_vertexLayout);

	//コンスタントバッファをセット
	context->VSSetConstantBuffers(0, 1, &m_constantBuffer);

	// インデックスバッファを設定
	context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//プリミティブ・トポロジーをセット
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//マテリアル毎に描画処理
	UINT startIndex = 0;
	PMXMaterialBuffer mb;
	for (int i = 0; i < m_data->Material()->count; i++)
	{
		//マテリアル情報を渡す
		SetMaterialBuffer(&(m_data->Material()->info[i]), &mb);

		if (SUCCEEDED(context->Map(m_materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data)))
		{
			memcpy_s(data.pData, data.RowPitch, (void*)(&mb), sizeof(mb));
			context->Unmap(m_materialBuffer, 0);
		}

		//コンスタントバッファーを、どのシェーダーで使うかを指定
		context->PSSetConstantBuffers(1, 1, &m_materialBuffer);

		//プリミティブ・トポロジーをセット
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//テクスチャーをシェーダーに渡す
		context->PSSetSamplers(0, 1, &m_sampler);
		context->PSSetShaderResources(0, 1, &m_texture[i]);

		context->DrawIndexed(m_data->Material()->info[i].faceVertexCount, startIndex,0);
		startIndex += m_data->Material()->info[i].faceVertexCount;
	}
}


/// <summary>
///	シェーダ関連の初期化
/// </summary>
bool PMXModel::InitShader()
{
	auto graphics = Graphics::GetInstance();

	//頂点シェーダと頂点インプットレイアウトを作成
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

	//ピクセルシェーダを作成
	if (FAILED(graphics->CreatePixleShader(L"PMXShader.hlsl", "PS", "ps_5_0", &m_pixelShader)))
	{
		SAFE_DELETE(m_pixelShader);
		return false;
	}

	//コンスタントバッファー作成
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

	//マテリアルコンスタントバッファ作成
	cb.ByteWidth = sizeof(PMXMaterialBuffer);
	if (FAILED(graphics->CreateConstantBuffer(&cb, &m_materialBuffer))) {
		SAFE_DELETE(m_materialBuffer);
		return false;
	}

	return true;
}


/// <summary>
/// ポリゴン関連の初期化
/// </summary>
bool PMXModel::InitPolygon()
{
	auto graphics = Graphics::GetInstance();
	auto window = Window::GetInstance();

	//バーテックスバッファー作成
	D3D11_BUFFER_DESC bd;
	D3D11_SUBRESOURCE_DATA initData;
	{
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;                                //このバッファがどのように扱われるか
		bd.ByteWidth = sizeof(PMXVertexInfo) * m_data->Vertex()->count;//頂点数×頂点サイズ
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;                       //バッファの種類
		bd.CPUAccessFlags = 0;                                         //CPUのアクセス権限
		bd.MiscFlags = 0;                                              //オプション　なければ0


		ZeroMemory(&initData, sizeof(initData));
		initData.pSysMem = m_data->Vertex()->info.data();//頂点配列の先頭のポインタ

		if (FAILED(graphics->CreateVertexBuffer(&bd, &initData, &m_vertexBuffer)))
		{
			SAFE_DELETE(m_vertexBuffer);
			return false;
		}
	}

	//インデックスバッファ作成
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

	//テクスチャ用サンプラー作成
	{
		D3D11_SAMPLER_DESC samDesc;
		ZeroMemory(&samDesc, sizeof(D3D11_SAMPLER_DESC));
		samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		window->Device()->CreateSamplerState(&samDesc, &m_sampler);
	}

	//テクスチャ作成
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
/// マテリアル情報を設定する
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
