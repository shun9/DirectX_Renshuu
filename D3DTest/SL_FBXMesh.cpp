#include <stdlib.h>
#include <locale.h>
#include <vector>
#include <DirectXTex.h>
#include "SL_FBXMesh.h"
#include "SL_Graphics.h"
#include "SL_Window.h"

using namespace fbxsdk;

FBXMesh::FBXMesh()
{
	ZeroMemory(this, sizeof(FBXMesh));
}


FBXMesh::~FBXMesh()
{
	delete[] m_material;
	SAFE_RELEASE(m_vertexBuffer);
	for (DWORD i = 0; i < m_dwNumMaterial; i++)
	{
		SAFE_RELEASE(m_indexBuffer[i]);
	}
	delete[] m_indexBuffer;
	delete[] m_child;	
}


HRESULT FBXMesh::InitFBX(CHAR* szFileName)
{
	//マネージャーの作成
	m_manager = fbxsdk::FbxManager::Create();

	//インポーターの作成
	m_importer = FbxImporter::Create(m_manager, "importer");

	//ファイルを開く
	m_importer->Initialize(szFileName);

	//シーンの作成
	m_pmyScene = FbxScene::Create(m_manager, "scene");

	//ファイルからシーンを読み込む
	m_importer->Import(m_pmyScene);

	//不要になったインポーターを破棄
	m_importer->Destroy();
	return S_OK;
}


//素材（FBX)は、三角ポリゴンにする
HRESULT FBXMesh::CreateFromFBX(CHAR* FileName)
{
	//指定のファイルでFBX初期化
	if (FAILED(InitFBX(FileName)))
	{
		MessageBox(0, L"FBXローダー初期化失敗", NULL, MB_OK);
		return E_FAIL;
	}

	//シーンのルートノードを取得
	FbxNode* rootNode = m_pmyScene->GetRootNode();
	FbxMesh* fbxMesh = 0;
	FbxNodeAttribute::EType type;

	FbxNodeAttribute *pAttr = rootNode->GetNodeAttribute();
	if (pAttr != NULL)
	{
		type = pAttr->GetAttributeType();
		//タイプがメッシュの場合　普通はルートがメッシュそのものであることは少ない
		if (type == FbxNodeAttribute::EType::eMesh)
		{
			fbxMesh = rootNode->GetMesh();
		}
	}
	else//子がメッシュの可能性あり
	{
		fbxMesh = SearchMesh(rootNode);
		if (fbxMesh == nullptr)
		{
			MessageBox(0, L"メッシュの読み込み失敗", NULL, MB_OK);
			return E_FAIL;
		}
	}

	//事前に頂点数、ポリゴン数等を調べる
	m_dwNumVert = fbxMesh->GetControlPointsCount();
	m_dwNumUV = fbxMesh->GetTextureUVCount();
	m_dwNumFace = fbxMesh->GetPolygonCount();
	//一時的なメモリ確保（頂点バッファとインデックスバッファ）
	FBX_VERTEX* pvVB = NULL;
	m_dwNumVert < m_dwNumUV ? pvVB = new FBX_VERTEX[m_dwNumUV] : pvVB = new FBX_VERTEX[m_dwNumVert];
	
	/// ////ポリゴンごとに　頂点読み込み 、法線読み込み、UV読み込み
	FbxVector4 Normal;
	for (DWORD i = 0; i < m_dwNumFace; i++)
	{
		int iIndex0 = 0;
		int iIndex1 = 0;
		int iIndex2 = 0;

		int iStartIndex = fbxMesh->GetPolygonVertexIndex(i);
		int* piIndex = fbxMesh->GetPolygonVertices();//（頂点インデックス）読み込み
		iIndex0 = piIndex[iStartIndex];
		iIndex1 = piIndex[iStartIndex + 1];
		iIndex2 = piIndex[iStartIndex + 2];
		if (m_dwNumVert < m_dwNumUV)//UV数のほうが多い場合は本来の（頂点ベース）インデックスを利用しない。UVインデックスを基準にする
		{
			iIndex0 = fbxMesh->GetTextureUVIndex(i, 0, FbxLayerElement::EType::eTextureDiffuse);
			iIndex1 = fbxMesh->GetTextureUVIndex(i, 1, FbxLayerElement::EType::eTextureDiffuse);
			iIndex2 = fbxMesh->GetTextureUVIndex(i, 2, FbxLayerElement::EType::eTextureDiffuse);
		}
		//頂点
		int index = fbxMesh->GetPolygonVertex(i, 0);
		FbxVector4 Coord = fbxMesh->GetControlPointAt(index);
		pvVB[iIndex0].pos.m_x = -Coord.mData[0];//FBXは右手座標系なのでxあるいはｚを反転
		pvVB[iIndex0].pos.m_y = Coord.mData[1];
		pvVB[iIndex0].pos.m_z = Coord.mData[2];

		index = fbxMesh->GetPolygonVertex(i, 1);
		Coord = fbxMesh->GetControlPointAt(index);
		pvVB[iIndex1].pos.m_x = -Coord.mData[0];//FBXは右手座標系なのでxあるいはｚを反転
		pvVB[iIndex1].pos.m_y = Coord.mData[1];
		pvVB[iIndex1].pos.m_z = Coord.mData[2];

		index = fbxMesh->GetPolygonVertex(i, 2);
		Coord = fbxMesh->GetControlPointAt(index);
		pvVB[iIndex2].pos.m_x = -Coord.mData[0];//FBXは右手座標系なのでxあるいはｚを反転
		pvVB[iIndex2].pos.m_y = Coord.mData[1];
		pvVB[iIndex2].pos.m_z = Coord.mData[2];

		//法線		
		fbxMesh->GetPolygonVertexNormal(i, 0, Normal);
		pvVB[iIndex0].norm.m_x = -Normal[0];//FBXは右手座標系なのでxあるいはｚを反転
		pvVB[iIndex0].norm.m_y = Normal[1];
		pvVB[iIndex0].norm.m_z = Normal[2];

		fbxMesh->GetPolygonVertexNormal(i, 1, Normal);
		pvVB[iIndex1].norm.m_x = -Normal[0];//FBXは右手座標系なのでxあるいはｚを反転
		pvVB[iIndex1].norm.m_y = Normal[1];
		pvVB[iIndex1].norm.m_z = Normal[2];

		fbxMesh->GetPolygonVertexNormal(i, 2, Normal);
		pvVB[iIndex2].norm.m_x = -Normal[0];//FBXは右手座標系なのでxあるいはｚを反転
		pvVB[iIndex2].norm.m_y = Normal[1];
		pvVB[iIndex2].norm.m_z = Normal[2];

		//テクスチャー座標
		int UVindex = fbxMesh->GetTextureUVIndex(i, 0, FbxLayerElement::EType::eTextureDiffuse);
		FbxLayerElementUV* pUV = 0;

		FbxLayerElementUV* uv = fbxMesh->GetLayer(0)->GetUVs();

		if (m_dwNumUV && uv->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
		{
			//この取得の仕方は、UVマッピングモードがeBY_POLYGON_VERTEXの時じゃないとできない
			UVindex = fbxMesh->GetTextureUVIndex(i, 0, FbxLayerElement::EType::eTextureDiffuse);
			pUV = fbxMesh->GetLayer(0)->GetUVs();
			FbxVector2 v2 = pUV->GetDirectArray().GetAt(UVindex);
			pvVB[iIndex0].tex.m_x = v2.mData[0];
			pvVB[iIndex0].tex.m_y = 1.0f - v2.mData[1];

			UVindex = fbxMesh->GetTextureUVIndex(i, 1, FbxLayerElement::EType::eTextureDiffuse);
			v2 = pUV->GetDirectArray().GetAt(UVindex);
			pvVB[iIndex1].tex.m_x = v2.mData[0];
			pvVB[iIndex1].tex.m_y = 1.0f - v2.mData[1];

			UVindex = fbxMesh->GetTextureUVIndex(i, 2, FbxLayerElement::EType::eTextureDiffuse);
			v2 = pUV->GetDirectArray().GetAt(UVindex);
			pvVB[iIndex2].tex.m_x = v2.mData[0];
			pvVB[iIndex2].tex.m_y = 1.0f - v2.mData[1];
		}
	}

	FbxLayerElementUV* uv = fbxMesh->GetLayer(0)->GetUVs();
	if (m_dwNumUV && uv->GetMappingMode() == FbxLayerElement::eByControlPoint)
	{
		FbxLayerElementUV* pUV = fbxMesh->GetLayer(0)->GetUVs();
		for (DWORD k = 0; k < m_dwNumUV; k++)
		{
			FbxVector2 v2;
			v2 = pUV->GetDirectArray().GetAt(k);
			pvVB[k].tex.m_x = v2.mData[0];
			pvVB[k].tex.m_y = 1.0f - v2.mData[1];
		}
	}

	//マテリアル読み込み
	FbxNode* pNode = fbxMesh->GetNode();
	m_dwNumMaterial = pNode->GetMaterialCount();

	m_material = new FBX_MATERIAL[m_dwNumMaterial];

	//マテリアルの数だけインデックスバッファーを作成
	m_indexBuffer = new ID3D11Buffer*[m_dwNumMaterial];

	for (DWORD i = 0; i < m_dwNumMaterial; i++)
	{
		//フォンモデルを想定
		FbxSurfaceMaterial* pMaterial = pNode->GetMaterial(i);
		FbxSurfacePhong* pPhong = (FbxSurfacePhong*)pMaterial;

		//環境光
		FbxDouble3 d3Ambient = pPhong->Ambient;
		m_material[i].Ka.m_x = (float)d3Ambient.mData[0];
		m_material[i].Ka.m_y = (float)d3Ambient.mData[1];
		m_material[i].Ka.m_z = (float)d3Ambient.mData[2];
		//拡散反射光
		FbxDouble3 d3Diffuse = pPhong->Diffuse;
		auto d = d3Diffuse.mData;
		m_material[i].Kd.m_x = (float)d[0];
		m_material[i].Kd.m_y = (float)d[1];
		m_material[i].Kd.m_z = (float)d[2];
		//鏡面反射光
		FbxDouble3 d3Specular = pPhong->Specular;
		auto s = d3Specular.mData;
		m_material[i].Ks.m_x = (float)s[0];
		m_material[i].Ks.m_y = (float)s[1];
		m_material[i].Ks.m_z = (float)s[2];

		//テクスチャー（ディフューズテクスチャーのみ）
		FbxProperty lProperty;
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		FbxTexture* texture = FbxCast<FbxTexture>(lProperty.GetSrcObject(0));

		if (texture)
		{
			auto name = texture->GetName();
			strcpy_s(m_material[i].szTextureName, name);

			//変換文字列格納バッファ
			wchar_t	wStrW[256];
			//ロケール指定
			setlocale(LC_ALL, "japanese");
			//変換 wchar_t -> char
			errno_t err = mbstowcs_s(nullptr, wStrW, 120, m_material[i].szTextureName, _TRUNCATE);

			if (err != 0)
			{
				MessageBox(0, L"文字列の変換に失敗", NULL, MB_OK);
				return E_FAIL;
			}

			//テクスチャーを作成
			auto graphics = ShunLib::Graphics::GetInstance();
			if (FAILED(graphics->CreateShaderResourceView(wStrW, &m_material[i].pTexture)))
			{

				//ファイル名+拡張子のみにする
				char ext[10];
				auto path = m_material[i].szTextureName;

				_splitpath_s(path, nullptr, 0, nullptr, 0, m_material[i].szTextureName,sizeof(m_material[i].szTextureName),ext,sizeof(ext));

				strcat_s(m_material[i].szTextureName, ext);



				//変換文字列格納バッファ
				wchar_t	wStrW[50];
				//ロケール指定
				setlocale(LC_ALL, "japanese");
				//変換
				errno_t err = mbstowcs_s(nullptr, wStrW, 20, m_material[i].szTextureName, _TRUNCATE);

				if (err != 0)
				{
					MessageBox(0, L"パスの変換に失敗", NULL, MB_OK);
					return E_FAIL;
				}

				//ファイル名のみでトライ
				if (FAILED(graphics->CreateShaderResourceView(wStrW, &m_material[i].pTexture)))
				{
					MessageBox(0, L"テクスチャー読み込み失敗", NULL, MB_OK);
					return E_FAIL;
				}
			}
		}

		//マテリアルの数だけインデックスバッファーを作成
		int iCount = 0;
		int* index = new int[m_dwNumFace * 3];//メッシュ内のポリゴン数でメモリ確保（個々のポリゴングループはかならずこれ以下になる）

		//そのマテリアルであるインデックス配列内の開始インデックスを調べる　さらにインデックスの個数も調べる		
		iCount = 0;
		for (DWORD k = 0; k < m_dwNumFace; k++)
		{
			FbxLayerElementMaterial* mat = fbxMesh->GetLayer(0)->GetMaterials();//レイヤーが1枚だけを想定
			int matId = mat->GetIndexArray().GetAt(k);
			if (matId == i)
			{
				if (m_dwNumVert < m_dwNumUV)
				{
					index[iCount + 0] = fbxMesh->GetTextureUVIndex(k, 0, FbxLayerElement::EType::eTextureDiffuse);
					index[iCount + 1] = fbxMesh->GetTextureUVIndex(k, 1, FbxLayerElement::EType::eTextureDiffuse);
					index[iCount + 2] = fbxMesh->GetTextureUVIndex(k, 2, FbxLayerElement::EType::eTextureDiffuse);
				}
				else
				{
					index[iCount + 0] = fbxMesh->GetPolygonVertex(k, 0);
					index[iCount + 1] = fbxMesh->GetPolygonVertex(k, 1);
					index[iCount + 2] = fbxMesh->GetPolygonVertex(k, 2);
				}
				iCount += 3;
			}
		}
		CreateIndexBuffer(iCount * sizeof(int), index, &m_indexBuffer[i]);
		m_material[i].dwNumFace = iCount / 3;//そのマテリアル内のポリゴン数

		delete[] index;
	}

	auto device = ShunLib::Window::GetInstance()->Device();

	//バーテックスバッファーを作成
	if (m_dwNumVert < m_dwNumUV) m_dwNumVert = m_dwNumUV;
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(FBX_VERTEX) *m_dwNumVert;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = pvVB;
	if (FAILED(device->CreateBuffer(&bd, &InitData, &m_vertexBuffer)))
		return FALSE;

	//一時的な入れ物は、もはや不要
	delete[] pvVB;

	return S_OK;
}



HRESULT FBXMesh::CreateIndexBuffer(DWORD dwSize, int* pIndex, ID3D11Buffer** ppIndexBuffer)
{
	auto device = ShunLib::Window::GetInstance()->Device();
	
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = dwSize;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = pIndex;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	if (FAILED(device->CreateBuffer(&bd, &InitData, ppIndexBuffer)))
	{
		return FALSE;
	}

	return S_OK;
}


/// <summary>
/// メッシュの描画
/// </summary>
void FBXMesh::RenderMesh(const ShunLib::Matrix& world, const ShunLib::Matrix& view, const ShunLib::Matrix& proj)
{
	auto context = ShunLib::Window::GetInstance()->DeviceContext();
 
	//バーテックスバッファーをセット（バーテックスバッファーは一つ）
	UINT stride = sizeof(FBX_VERTEX);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	//マテリアルの数だけ、それぞれのマテリアルのインデックスバッファ－を描画
	for (DWORD i = 0; i < m_dwNumMaterial; i++)
	{
		//使用されていないマテリアル対策
		if (m_material[i].dwNumFace == 0)
		{
			continue;
		}
		//インデックスバッファーをセット
		stride = sizeof(int);
		offset = 0;
		context->IASetIndexBuffer(m_indexBuffer[i], DXGI_FORMAT_R32_UINT, 0);

		//プリミティブ・トポロジーをセット
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		//シェーダーに情報を渡す
		D3D11_MAPPED_SUBRESOURCE pData;
		if (SUCCEEDED(context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
		{
			SHADER_GLOBAL1 sg;
			sg.world = ShunLib::Matrix::Transpose(world);             //ワールド行列をシェーダーに渡す
			sg.wvp = ShunLib::Matrix::Transpose((sg.world*view*proj));//行列をシェーダーに渡す　ワールド×ビュー×プロジェクション　行列
			sg.ambient = m_material[i].Ka;                            //アンビエントをシェーダーに渡す
			sg.diffuse = m_material[i].Kd;                            //ディフューズカラーをシェーダーに渡す
			sg.specular = m_material[i].Ks;                           //スペキュラーをシェーダーに渡す

			sg.diffuse = ShunLib::Vec4(1.0f,1.0f,1.0f,1.0f);                            //ディフューズカラーをシェーダーに渡す

			memcpy_s(pData.pData, pData.RowPitch, (void*)&sg, sizeof(SHADER_GLOBAL1));
			context->Unmap(m_constantBuffer, 0);
		}

		//シェーダーにバッファーをセット
		context->VSSetConstantBuffers(1, 1, &m_constantBuffer);
		context->PSSetConstantBuffers(1, 1, &m_constantBuffer);

		//テクスチャーをシェーダーに渡す
		if (m_material[i].szTextureName[0] != NULL)
		{
			context->PSSetSamplers(0, 1, &m_sampleLinear);
			context->PSSetShaderResources(0, 1, &m_material[i].pTexture);
		}

		//Draw
		context->DrawIndexed(m_material[i].dwNumFace * 3, 0, 0);

	}
}

/// <summary>
/// メッシュノードを探す
/// </summary>
FbxMesh* FBXMesh::SearchMesh(FbxNode* rootNode)
{
	//子の数だけ
	int iNumChild = rootNode->GetChildCount();
	FbxNodeAttribute* attr;
	FbxNodeAttribute::EType type;
	for (int i = 0; i < iNumChild; i++)
	{
		FbxNode* childNode = rootNode->GetChild(i);
		if (!childNode) continue;
		attr = childNode->GetNodeAttribute();
		type = attr->GetAttributeType();

		//タイプがメッシュの場合
		if (type == FbxNodeAttribute::EType::eMesh)
		{
			auto mesh = childNode->GetMesh();
			return mesh;
		}
	}

	return nullptr;
}
