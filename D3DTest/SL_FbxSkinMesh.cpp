#include "SL_FbxSkinMesh.h"

#include <stdlib.h>
#include <functional>
#include <locale.h>

#include "SL_Window.h"
#include "SL_Graphics.h"

using namespace std;
using namespace ShunLib;

FbxSkinMesh::FbxSkinMesh()
{
	ZeroMemory(this, sizeof(FbxSkinMesh));
}

FbxSkinMesh::~FbxSkinMesh()
{
	delete[] m_bone;
	delete[] m_material;
	delete[] m_ppCluster;
	SAFE_RELEASE(m_vertexBuffer);
	for (DWORD i = 0; i < m_dwNumMaterial; i++)
	{
		SAFE_RELEASE(m_ppIndexBuffer[i]);
	}
	delete[] m_ppIndexBuffer;
}


HRESULT FbxSkinMesh::Init(LPSTR FileName)
{
	if (FAILED(InitShader()))
	{
		MessageBox(0, L"メッシュ用シェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}
	if (FAILED(CreateFromFBX(FileName)))
	{
		MessageBox(0, L"メッシュ作成失敗", NULL, MB_OK);
		return E_FAIL;
	}
	//テクスチャー用サンプラー作成
	D3D11_SAMPLER_DESC SamDesc;
	ZeroMemory(&SamDesc, sizeof(D3D11_SAMPLER_DESC));

	SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	Window::GetInstance()->Device()->CreateSamplerState(&SamDesc, &m_sampleLinear);

	return S_OK;
}

/// <summary>
/// シェーダー作成
/// </summary>
HRESULT FbxSkinMesh::InitShader()
{
	auto graphic = Graphics::GetInstance();
	auto device = Window::GetInstance()->Device();

	//頂点インプットレイアウトを定義	
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION"   , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"     , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD"   , 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_INDEX" , 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	//バーテックスシェーダー作成
	if (FAILED(graphic->CreateVertexShader(L"SKIN_MESH.hlsl", "VSSkin", "vs_5_0", &m_vertexShader, layout, numElements, &m_vertexLayout)))
	{
		MessageBox(0, L"バーテックスシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}
	//ピクセルシェーダー作成
	if (FAILED(graphic->CreatePixleShader(L"SKIN_MESH.hlsl", "PSSkin", "ps_5_0", &m_pixelShader)))
	{
		MessageBox(0, L"ピクセルシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}

	//コンスタントバッファー作成　変換行列渡し用
	D3D11_BUFFER_DESC cb;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth = sizeof(SHADER_GLOBAL0);
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(device->CreateBuffer(&cb, NULL, &m_constantBuffer0)))
	{
		return E_FAIL;
	}

	//コンスタントバッファー作成  マテリアル渡し用
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth = sizeof(SHADER_GLOBAL1);
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(device->CreateBuffer(&cb, NULL, &m_constantBuffer1)))
	{
		return E_FAIL;
	}

	//コンスタントバッファーボーン用　作成
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth = sizeof(SHADER_GLOBAL_BONES);
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(device->CreateBuffer(&cb, NULL, &m_constantBufferBone)))
	{
		return E_FAIL;
	}

	return S_OK;
}


//HRESULT InitFBX(CHAR* szFileName)
//指定したファイルのFBXを初期化する
HRESULT FbxSkinMesh::InitFBX(CHAR* szFileName)
{
	auto graphic = Graphics::GetInstance();
	m_importer = FbxImporter::Create(graphic->FBXManager(), "my importer");

	int iFormat = -1;
	m_importer->Initialize((const char*)szFileName, iFormat);

	m_scene = FbxScene::Create(graphic->FBXManager(), "my scene");

	m_importer->Import(m_scene);

	return S_OK;
}

//
//HRESULT FBX_SKINMESH::ReadSkinInfo(KFbxMesh* pFbxMesh,MY_VERTEX* pvVB,POLY_TABLE* PolyTable)
//FBXからスキン関連の情報を読み出す　
HRESULT FbxSkinMesh::ReadSkinInfo(FbxMesh* pFbxMesh, MY_VERTEX* pvVB, POLY_TABLE* PolyTable)
{
	//FBXから抽出すべき情報は、頂点ごとのボーンインデックス、頂点ごとのボーンウェイト、バインド行列、ポーズ行列　の4項目

	int i, k;
	int iNumBone = 0;//ボーン数

	//デフォーマーを得る 
	FbxDeformer* pDeformer = pFbxMesh->GetDeformer(0);
	FbxSkin* pSkinInfo = static_cast<FbxSkin*>(pDeformer);
	//
	//ボーンを得る
	iNumBone = pSkinInfo->GetClusterCount();
	m_ppCluster = new fbxsdk::FbxCluster*[iNumBone];
	for (i = 0; i < iNumBone; i++)
	{
		m_ppCluster[i] = pSkinInfo->GetCluster(i);
	}

	//通常の場合　（頂点数＞=UV数　pvVBが頂点インデックスベースの場合）
	if (m_dwNumVert >= m_dwNumUV)
	{
		//それぞれのボーンに影響を受ける頂点を調べる　そこから逆に、頂点ベースでボーンインデックス・重みを整頓する
		for (i = 0; i < iNumBone; i++)
		{
			int iNumIndex = m_ppCluster[i]->GetControlPointIndicesCount();//このボーンに影響を受ける頂点数
			int* piIndex = m_ppCluster[i]->GetControlPointIndices();
			double* pdWeight = m_ppCluster[i]->GetControlPointWeights();
			//頂点側からインデックスをたどって、頂点サイドで整理する
			for (k = 0; k < iNumIndex; k++)
			{
				for (int m = 0; m < 4; m++)//FBXやCGソフトがボーン4本以内とは限らない。5本以上の場合は、重みの大きい順に4本に絞る 
				{
					if (pdWeight[k] > pvVB[piIndex[k]].bBoneWeight[m])
					{
						pvVB[piIndex[k]].bBoneIndex[m] = i;
						pvVB[piIndex[k]].bBoneWeight[m] = (float)(pdWeight[k]);
						break;
					}
				}
			}
		}
	}
	//UVインデックスベースの場合　（頂点数<UV数）
	else
	{
		int PolyIndex = 0;
		int UVIndex = 0;
		//それぞれのボーンに影響を受ける頂点を調べる　そこから逆に、頂点ベースでボーンインデックス・重みを整頓する
		for (i = 0; i < iNumBone; i++)
		{
			int iNumIndex = m_ppCluster[i]->GetControlPointIndicesCount();//このボーンに影響を受ける頂点数
			int* piIndex = m_ppCluster[i]->GetControlPointIndices();
			double* pdWeight = m_ppCluster[i]->GetControlPointWeights();
			//頂点側からインデックスをたどって、頂点サイドで整理する
			for (k = 0; k < iNumIndex; k++)
			{
				//その頂点を含んでいるポリゴンすべてに、このボーンとウェイトを適用
				for (int p = 0; p < PolyTable[piIndex[k]].NumRef; p++)
				{
					//頂点→属すポリゴン→そのポリゴンのUVインデックス　と逆引き
					PolyIndex = PolyTable[piIndex[k]].PolyIndex[p];
					UVIndex = pFbxMesh->GetTextureUVIndex(PolyIndex, PolyTable[piIndex[k]].Index123[p], FbxLayerElement::eTextureDiffuse);

					for (int m = 0; m < 4; m++)//FBXやCGソフトがボーン4本以内とは限らない。5本以上の場合は、重みの大きい順に4本に絞る 
					{
						if (pdWeight[k] > pvVB[UVIndex].bBoneWeight[m])
						{
							pvVB[UVIndex].bBoneIndex[m] = i;
							pvVB[UVIndex].bBoneWeight[m] = (float)(pdWeight[k]);
							break;
						}
					}
				}

			}
		}
	}

	//
	//ボーンを生成
	m_iNumBone = iNumBone;
	m_bone = new BONE[iNumBone];

	for (i = 0; i < m_iNumBone; i++)
	{
		FbxAMatrix mat;
		m_ppCluster[i]->GetTransformLinkMatrix(mat);

		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 4; y++)
			{
				m_bone[i].bindPose.m_value[y][x] = (float)(mat.Get(y, x));
				m_bone[i].bindPose.m_value[0][0] *= -1.0f;
			}
		}
	}
	return S_OK;
}

//
//HRESULT FBX_SKINMESH::CreateFromFBX(CHAR* szFileName)
//FBXからスキンメッシュを作成する　　注意）素材（FBX)のほうは、三角ポリゴンにすること
HRESULT FbxSkinMesh::CreateFromFBX(CHAR* szFileName)
{
	auto graphic = Graphics::GetInstance();
	auto device = Window::GetInstance()->Device();

	//FBXローダーを初期化
	if (FAILED(InitFBX(szFileName)))
	{
		MessageBox(0, L"FBXローダー初期化失敗", NULL, MB_OK);
		return E_FAIL;
	}

	FbxNode* pNode = m_scene->GetRootNode();

	int index = 0;
	FbxNodeAttribute* pAttr = pNode->GetNodeAttribute();
	while (!pAttr || pAttr->GetAttributeType() != FbxNodeAttribute::eMesh)//1メッシュを想定しているので、eMESHが見つかった時点で、それを使う
	{
		pNode = pNode->GetChild(index);
		index++;
		pAttr = pNode->GetNodeAttribute();
	}

	FbxMesh* pFbxMesh = pNode->GetMesh();

	//事前に頂点数、ポリゴン数等を調べる
	m_dwNumVert = pFbxMesh->GetControlPointsCount();
	m_dwNumUV = pFbxMesh->GetTextureUVCount();
	m_dwNumFace = pFbxMesh->GetPolygonCount();
	//一時的なメモリ確保（頂点バッファとインデックスバッファ）
	MY_VERTEX* pvVB = NULL;
	m_dwNumVert < m_dwNumUV ? pvVB = new MY_VERTEX[m_dwNumUV] : pvVB = new MY_VERTEX[m_dwNumVert];

	////ポリゴンごとに　頂点読み込み 、法線読み込み、UV読み込み
	auto SetIndexOrNormal = [](Vec3* set, FbxDouble* get)
	{
		set->m_x = -(float)get[0];
		set->m_y = (float)get[1];
		set->m_z = (float)get[2];
	};
	FbxVector4 Normal;
	for (DWORD i = 0; i < m_dwNumFace; i++)
	{
		int iIndex0 = 0;
		int iIndex1 = 0;
		int iIndex2 = 0;

		int iStartIndex = pFbxMesh->GetPolygonVertexIndex(i);
		int* piIndex = pFbxMesh->GetPolygonVertices();//（頂点インデックス）読み込み
		iIndex0 = piIndex[iStartIndex];
		iIndex1 = piIndex[iStartIndex + 1];
		iIndex2 = piIndex[iStartIndex + 2];

		if (m_dwNumVert < m_dwNumUV)//UV数のほうが多い場合は本来の（頂点ベース）インデックスを利用しない。UVインデックスを基準にする
		{
			iIndex0 = pFbxMesh->GetTextureUVIndex(i, 0, FbxLayerElement::eTextureDiffuse);
			iIndex1 = pFbxMesh->GetTextureUVIndex(i, 1, FbxLayerElement::eTextureDiffuse);
			iIndex2 = pFbxMesh->GetTextureUVIndex(i, 2, FbxLayerElement::eTextureDiffuse);
		}

		//頂点
		int index = pFbxMesh->GetPolygonVertex(i, 0);
		FbxVector4 Coord = pFbxMesh->GetControlPointAt(index);
		SetIndexOrNormal(&(pvVB[iIndex0].pos), Coord.mData);

		index = pFbxMesh->GetPolygonVertex(i, 1);
		Coord = pFbxMesh->GetControlPointAt(index);
		SetIndexOrNormal(&(pvVB[iIndex1].pos), Coord.mData); 

		index = pFbxMesh->GetPolygonVertex(i, 2);
		Coord = pFbxMesh->GetControlPointAt(index);
		SetIndexOrNormal(&(pvVB[iIndex2].pos), Coord.mData); 

		//法線		
		pFbxMesh->GetPolygonVertexNormal(i, 0, Normal);
		SetIndexOrNormal(&(pvVB[iIndex0].pos), Normal);

		pFbxMesh->GetPolygonVertexNormal(i, 1, Normal);
		SetIndexOrNormal(&(pvVB[iIndex1].pos), Normal);

		pFbxMesh->GetPolygonVertexNormal(i, 2, Normal);
		SetIndexOrNormal(&(pvVB[iIndex2].pos), Normal);

		//テクスチャー座標
		int UVindex = pFbxMesh->GetTextureUVIndex(i, 0, FbxLayerElement::eTextureDiffuse);
		FbxLayerElementUV* pUV = 0;

		FbxLayerElementUV* uv = pFbxMesh->GetLayer(0)->GetUVs();

		if (m_dwNumUV && uv->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
		{
			//この取得の仕方は、UVマッピングモードがeBY_POLYGON_VERTEXの時じゃないとできない
			UVindex = pFbxMesh->GetTextureUVIndex(i, 0, FbxLayerElement::eTextureDiffuse);
			pUV = pFbxMesh->GetLayer(0)->GetUVs();
			FbxVector2 v2 = pUV->GetDirectArray().GetAt(UVindex);
			pvVB[iIndex0].tex.m_x = (float)(v2.mData[0]);
			pvVB[iIndex0].tex.m_y = 1.0f - (float)(v2.mData[1]);

			UVindex = pFbxMesh->GetTextureUVIndex(i, 1, FbxLayerElement::eTextureDiffuse);
			v2 = pUV->GetDirectArray().GetAt(UVindex);
			pvVB[iIndex1].tex.m_x = (float)(v2.mData[0]);
			pvVB[iIndex1].tex.m_y = 1.0f - (float)(v2.mData[1]);

			UVindex = pFbxMesh->GetTextureUVIndex(i, 2, FbxLayerElement::eTextureDiffuse);
			v2 = pUV->GetDirectArray().GetAt(UVindex);
			pvVB[iIndex2].tex.m_x = (float)(v2.mData[0]);
			pvVB[iIndex2].tex.m_y = 1.0f - (float)(v2.mData[1]);
		}
	}

	FbxLayerElementUV* uv = pFbxMesh->GetLayer(0)->GetUVs();
	if (m_dwNumUV && uv->GetMappingMode() == FbxLayerElement::eByControlPoint)
	{
		FbxLayerElementUV* pUV = pFbxMesh->GetLayer(0)->GetUVs();
		for (DWORD k = 0; k < m_dwNumUV; k++)
		{
			FbxVector2 v2;
			v2 = pUV->GetDirectArray().GetAt(k);
			pvVB[k].tex.m_x = (float)(v2.mData[0]);
			pvVB[k].tex.m_y = 1.0f - (float)(v2.mData[1]);
		}
	}

	//マテリアル読み込み
	pNode = pFbxMesh->GetNode();
	m_dwNumMaterial = pNode->GetMaterialCount();

	m_material = new MY_MATERIAL[m_dwNumMaterial];

	//マテリアルの数だけインデックスバッファーを作成
	m_ppIndexBuffer = new ID3D11Buffer*[m_dwNumMaterial];

	auto SetMaterial = [](Vec4* set, FbxDouble* get)
	{
		set->m_x = (float)get[0];
		set->m_y = (float)get[1];
		set->m_z = (float)get[2];
		set->m_w = 1;
	};

	for (DWORD i = 0; i < m_dwNumMaterial; i++)
	{
		//フォンモデルを想定
		FbxSurfaceMaterial* pMaterial = pNode->GetMaterial(i);
		FbxSurfacePhong* pPhong = (FbxSurfacePhong*)pMaterial;

		//環境光
		fbxsdk::FbxDouble3 d3Ambient = pPhong->Ambient;
		auto a = d3Ambient.mData;
		SetMaterial(&m_material[i].Ka, a);

		//拡散反射光
		fbxsdk::FbxDouble3 d3Diffuse = pPhong->Diffuse;
		auto d = d3Diffuse.mData;
		SetMaterial(&m_material[i].Kd, d);


		//鏡面反射光
		fbxsdk::FbxDouble3 d3Specular = pPhong->Specular;
		auto s = d3Specular.mData;
		SetMaterial(&m_material[i].Ks, s);

		//テクスチャー（ディフューズテクスチャーのみ）
		fbxsdk::FbxProperty lProperty;
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
			if (FAILED(graphic->CreateShaderResourceView(wStrW, &m_material[i].pTexture)))
			{
				//ファイル名+拡張子のみにする
				char ext[10];
				auto path = m_material[i].szTextureName;

				_splitpath_s(path, nullptr, 0, nullptr, 0, m_material[i].szTextureName, sizeof(m_material[i].szTextureName), ext, sizeof(ext));

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
				if (FAILED(graphic->CreateShaderResourceView(wStrW, &m_material[i].pTexture)))
				{
					MessageBox(0, L"テクスチャー読み込み失敗", NULL, MB_OK);
					return E_FAIL;
				}
			}
		}
		//マテリアルの数だけインデックスバッファーを作成
		int iCount = 0;
		int* pIndex = new int[m_dwNumFace * 3];//とりあえず、メッシュ内のポリゴン数でメモリ確保（個々のポリゴングループはかならずこれ以下になるが）

		//そのマテリアルであるインデックス配列内の開始インデックスを調べる　さらにインデックスの個数も調べる		
		iCount = 0;
		for (DWORD k = 0; k < m_dwNumFace; k++)
		{
			FbxLayerElementMaterial* mat = pFbxMesh->GetLayer(0)->GetMaterials();//レイヤーが1枚だけを想定
			int matId = mat->GetIndexArray().GetAt(k);
			if (matId == i)
			{
				if (m_dwNumVert < m_dwNumUV)
				{
					pIndex[iCount] = pFbxMesh->GetTextureUVIndex(k, 0, FbxLayerElement::eTextureDiffuse);
					pIndex[iCount + 1] = pFbxMesh->GetTextureUVIndex(k, 1, FbxLayerElement::eTextureDiffuse);
					pIndex[iCount + 2] = pFbxMesh->GetTextureUVIndex(k, 2, FbxLayerElement::eTextureDiffuse);
				}
				else
				{
					pIndex[iCount] = pFbxMesh->GetPolygonVertex(k, 0);
					pIndex[iCount + 1] = pFbxMesh->GetPolygonVertex(k, 1);
					pIndex[iCount + 2] = pFbxMesh->GetPolygonVertex(k, 2);
				}
				iCount += 3;
			}
		}
		if (iCount > 0) CreateIndexBuffer(iCount * sizeof(int), pIndex, &m_ppIndexBuffer[i]);
		m_material[i].dwNumFace = iCount / 3;//そのマテリアル内のポリゴン数		
		delete[] pIndex;
	}

	//頂点からポリゴンを逆引きしたいので、逆引きテーブルを作る 
	POLY_TABLE* PolyTable = new POLY_TABLE[m_dwNumVert];

	for (DWORD i = 0; i < m_dwNumVert; i++)
	{
		for (DWORD k = 0; k < m_dwNumFace; k++)
		{
			for (int m = 0; m < 3; m++)
			{
				if (pFbxMesh->GetPolygonVertex(k, m) == i)
				{
					PolyTable[i].PolyIndex[PolyTable[i].NumRef] = k;
					PolyTable[i].Index123[PolyTable[i].NumRef] = m;
					PolyTable[i].NumRef++;
				}
			}
		}
	}

	//スキン情報（ジョイント、ウェイト　）読み込み
	ReadSkinInfo(pFbxMesh, pvVB, PolyTable);

	//バーテックスバッファーを作成
	if (m_dwNumVert < m_dwNumUV) m_dwNumVert = m_dwNumUV;
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(MY_VERTEX) *m_dwNumVert;
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

//HRESULT FBX_SKINMESH::CreateIndexBuffer(DWORD dwSize,int* pIndex,ID3D11Buffer** ppIndexBuffer)
//Direct3Dのインデックスバッファー作成
HRESULT FbxSkinMesh::CreateIndexBuffer(DWORD dwSize, int* pIndex, ID3D11Buffer** ppIndexBuffer)
{
	auto device = Window::GetInstance()->Device();

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


//void FBX_SKINMESH::SetNewPoseMatrices(int frame)
//ボーンを次のポーズ位置にセットする
void FbxSkinMesh::SetNewPoseMatrices(int frame)
{
	Matrix m;
	m.m_value[0][0] = -1.0f;

	int i;
	fbxsdk::FbxTime time;
	time.SetTime(0, 0, 0, frame, 0, 0, fbxsdk::FbxTime::eFrames30);//30フレーム/秒　と推定　厳密には状況ごとに調べる必要あり

	for (i = 0; i < m_iNumBone; i++)
	{
		FbxMatrix mat = m_ppCluster[i]->GetLink()->EvaluateGlobalTransform(time);

		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 4; y++)
			{
				m_bone[i].newPose.m_value[y][x] = (float)(mat.Get(y, x));
			}
		}
		m_bone[i].newPose = m * m_bone[i].newPose;//FBX右手座標系なのでｘが逆　補正する	
	}

	//
	//フレームを進めたことにより変化したポーズ（ボーンの行列）をシェーダーに渡す
	auto context = Window::GetInstance()->DeviceContext();
	D3D11_MAPPED_SUBRESOURCE pData;
	if (SUCCEEDED(context->Map(m_constantBufferBone, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		SHADER_GLOBAL_BONES sg;
		for (int i = 0; i < m_iNumBone; i++)
		{
			Matrix mat = GetCurrentPoseMatrix(i);
			mat = Matrix::Transpose(mat);
			sg.mBone[i] = mat;
		}
		memcpy_s(pData.pData, pData.RowPitch, (void*)&sg, sizeof(SHADER_GLOBAL_BONES));
		context->Unmap(m_constantBufferBone, 0);
	}
	context->VSSetConstantBuffers(2, 1, &m_constantBufferBone);
	context->PSSetConstantBuffers(2, 1, &m_constantBufferBone);
}

//D3DXMATRIX FBX_SKINMESH::GetCurrentPoseMatrix(int index)
//次の（現在の）ポーズ行列を返す
Matrix FbxSkinMesh::GetCurrentPoseMatrix(int index)
{
	Matrix inv;
	inv = Matrix::Inverse(m_bone[index].bindPose);
	Matrix ret = inv*m_bone[index].newPose;//バインドポーズの逆行列とフレーム姿勢行列をかける。なお、バインドポーズ自体が既に逆行列であるとする考えもある。（FBXの場合は違うが）

	return ret;
}



//レンダリング
void FbxSkinMesh::Render(const Matrix& world, const Matrix& view, const Matrix& proj,
	const Vec3& vLight, const Vec3& vEye)
{
	auto context = Window::GetInstance()->DeviceContext();


	D3D11_MAPPED_SUBRESOURCE pData;
	if (SUCCEEDED(context->Map(m_constantBuffer0, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		SHADER_GLOBAL0 sg;
		//ライトの方向を渡す
		sg.lightDir = Vec4(vLight.m_x, vLight.m_y, vLight.m_z, 0.0f);
		//視点位置を渡す
		sg.eyePos = Vec4(vEye.m_x, vEye.m_y, vEye.m_z, 0.0f);

		memcpy_s(pData.pData, pData.RowPitch, (void*)&sg, sizeof(SHADER_GLOBAL0));
		context->Unmap(m_constantBuffer0, 0);
	}
	
	//使用するシェーダーの登録	
	context->VSSetShader(m_vertexShader, NULL, 0);
	context->PSSetShader(m_pixelShader, NULL, 0);



	//このコンスタントバッファーを使うシェーダーの登録
	context->VSSetConstantBuffers(0, 1, &m_constantBuffer0);
	context->PSSetConstantBuffers(0, 1, &m_constantBuffer0);
	
	//頂点インプットレイアウトをセット
	context->IASetInputLayout(m_vertexLayout);

	//バーテックスバッファーをセット（バーテックスバッファーは一つでいい）
	UINT stride = sizeof(MY_VERTEX);
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
		context->IASetIndexBuffer(m_ppIndexBuffer[i], DXGI_FORMAT_R32_UINT, 0);

		//プリミティブ・トポロジーをセット
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		//マテリアルの各要素と変換行列をシェーダーに渡す
		D3D11_MAPPED_SUBRESOURCE pData;
		if (SUCCEEDED(context->Map(m_constantBuffer1, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
		{
			SHADER_GLOBAL1 sg;
			sg.world=Matrix::Transpose(world);
			sg.wvp = Matrix::Transpose((world * view * proj));
			
			sg.ambient = m_material[i].Ka;//アンビエントををシェーダーに渡す
			sg.diffuse = m_material[i].Kd;//ディフューズカラーをシェーダーに渡す
			sg.specular = m_material[i].Ks;//スペキュラーをシェーダーに渡す

			memcpy_s(pData.pData, pData.RowPitch, (void*)&sg, sizeof(SHADER_GLOBAL1));
			context->Unmap(m_constantBuffer1, 0);
		}
	
		context->VSSetConstantBuffers(1, 1, &m_constantBuffer1);
		context->PSSetConstantBuffers(1, 1, &m_constantBuffer1);

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