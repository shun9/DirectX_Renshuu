#include "SL_FbxSkinMesh.h"

#include <functional>

#include "SL_Window.h"
#include "SL_Graphics.h"

using namespace std;
using namespace ShunLib;
//
//FBX_SKINMESH::FBX_SKINMESH()
//コンストラクター
FbxSkinMesh::FbxSkinMesh()
{
	ZeroMemory(this,sizeof(FbxSkinMesh));
	m_fScale=1.0f;
}

//
//FBX_SKINMESH::~FBX_SKINMESH()
//デストラクター
FbxSkinMesh::~FbxSkinMesh()
{
	delete[] m_bone;
	delete[] m_pMaterial;
	delete[] m_ppCluster;
	SAFE_RELEASE(m_vertexBuffer);
	for(DWORD i=0;i<m_dwNumMaterial;i++)
	{
		SAFE_RELEASE(m_ppIndexBuffer[i]);
	}
	delete[] m_ppIndexBuffer;
	if(m_pSdkManager) m_pSdkManager->Destroy();
}
//
//
//
HRESULT FbxSkinMesh::Init(LPSTR FileName)
{
	if(FAILED(InitShader()))
	{
		MessageBox(0,L"メッシュ用シェーダー作成失敗",NULL,MB_OK);
		return E_FAIL;
	}
	if(FAILED(CreateFromFBX(FileName)))
	{
		MessageBox(0,L"メッシュ作成失敗",NULL,MB_OK);
		return E_FAIL;
	}
	//テクスチャー用サンプラー作成
	D3D11_SAMPLER_DESC SamDesc;
	ZeroMemory(&SamDesc,sizeof(D3D11_SAMPLER_DESC));

	SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	Window::GetInstance()->Device()->CreateSamplerState(&SamDesc, &m_pSampleLinear);

	return S_OK;
}
//
//
//
HRESULT FbxSkinMesh::InitShader()
{
	auto graphic = Graphics::GetInstance();
	auto device = Window::GetInstance()->Device();

	//バーテックスシェーダー作成
	if (FAILED(graphic->CreateVertexShader(L"SKIN_MESH.hlsl", "VSSkin", "vs_5_0",&m_vertexShader)))
	{
		MessageBox(0, L"バーテックスシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}

	//頂点インプットレイアウトを定義	
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION"   , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"     , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD"   , 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_INDEX" , 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout)/sizeof(layout[0]);



	//頂点インプットレイアウト作成
	if (FAILED(graphic->CreateInputLayout(layout, numElements, &m_vertexLayout)))
	{
		MessageBox(0, L"頂点インプットレイアウト作成失敗", NULL, MB_OK);
		return E_FAIL;
	}

	//ピクセルシェーダー作成
	if (FAILED(graphic->CreatePixleShader(L"SKIN_MESH.hlsl", "PSSkin", "ps_5_0",&m_pixelShader)))
	{
		MessageBox(0, L"ピクセルシェーダー作成失敗", NULL, MB_OK);
		return E_FAIL;
	}

	//コンスタントバッファー作成　変換行列渡し用
	D3D11_BUFFER_DESC cb;
	cb.BindFlags= D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth= sizeof( SHADER_GLOBAL0 );
	cb.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags	=0;
	cb.Usage=D3D11_USAGE_DYNAMIC;

	if( FAILED(device->CreateBuffer( &cb,NULL,&m_pConstantBuffer0)))
	{
		return E_FAIL;
	}
	
	//コンスタントバッファー作成  マテリアル渡し用
	cb.BindFlags= D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth= sizeof( SHADER_GLOBAL1 );
	cb.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags	=0;
	cb.Usage=D3D11_USAGE_DYNAMIC;

	if( FAILED(device->CreateBuffer( &cb,NULL,&m_pConstantBuffer1)))
	{
		return E_FAIL;
	}

	//コンスタントバッファーボーン用　作成
	cb.BindFlags= D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth= sizeof( SHADER_GLOBAL_BONES );
	cb.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags	=0;
	cb.Usage=D3D11_USAGE_DYNAMIC;

	if( FAILED(device->CreateBuffer( &cb,NULL,&m_pConstantBufferBone)))
	{
		return E_FAIL;
	}

	return S_OK;
}
//
//HRESULT InitFBX(CHAR* szFileName)
//指定したファイルのFBXを初期化する
HRESULT FbxSkinMesh::InitFBX(CHAR* szFileName)
{
	m_pSdkManager = FbxManager::Create();

	m_pImporter=FbxImporter::Create(m_pSdkManager,"my importer");

	int iFormat=-1;
	m_pImporter->Initialize((const char*)szFileName,iFormat);

	m_pmyScene = FbxScene::Create(m_pSdkManager, "my scene");

	m_pImporter->Import(m_pmyScene);

	return S_OK;
}

//
//HRESULT FBX_SKINMESH::ReadSkinInfo(KFbxMesh* pFbxMesh,MY_VERTEX* pvVB,POLY_TABLE* PolyTable)
//FBXからスキン関連の情報を読み出す　
HRESULT FbxSkinMesh::ReadSkinInfo(FbxMesh* pFbxMesh,MY_VERTEX* pvVB,POLY_TABLE* PolyTable)
{
	//FBXから抽出すべき情報は、頂点ごとのボーンインデックス、頂点ごとのボーンウェイト、バインド行列、ポーズ行列　の4項目

	int i,k;
	int iNumBone=0;//ボーン数

	//デフォーマーを得る 
	FbxDeformer* pDeformer=pFbxMesh->GetDeformer(0);
	FbxSkin* pSkinInfo=static_cast<FbxSkin*>(pDeformer);
	//
	//ボーンを得る
	iNumBone=pSkinInfo->GetClusterCount();
	m_ppCluster= new fbxsdk::FbxCluster*[ iNumBone ];
	for( i=0;i<iNumBone;i++)
	{
		m_ppCluster[i]=pSkinInfo->GetCluster(i);
	}

	//通常の場合　（頂点数＞=UV数　pvVBが頂点インデックスベースの場合）
	if(m_dwNumVert>=m_dwNumUV)
	{
		//それぞれのボーンに影響を受ける頂点を調べる　そこから逆に、頂点ベースでボーンインデックス・重みを整頓する
		for(i=0;i<iNumBone;i++)
		{
			int iNumIndex=m_ppCluster[i]->GetControlPointIndicesCount();//このボーンに影響を受ける頂点数
			int* piIndex=m_ppCluster[i]->GetControlPointIndices();
			double* pdWeight=m_ppCluster[i]->GetControlPointWeights();
			//頂点側からインデックスをたどって、頂点サイドで整理する
			for(k=0;k<iNumIndex;k++)
			{
				for(int m=0;m<4;m++)//FBXやCGソフトがボーン4本以内とは限らない。5本以上の場合は、重みの大きい順に4本に絞る 
				{
					if(pdWeight[k] > pvVB[ piIndex[k] ].bBoneWeight[m])
					{
						pvVB[ piIndex[k] ].bBoneIndex[ m]=i;
						pvVB[ piIndex[k] ].bBoneWeight[m]=pdWeight[k];				
						break;
					}
				}				
			}
		}
	}
	//UVインデックスベースの場合　（頂点数<UV数）
	else
	{
		int PolyIndex=0;
		int UVIndex=0;
		//それぞれのボーンに影響を受ける頂点を調べる　そこから逆に、頂点ベースでボーンインデックス・重みを整頓する
		for(i=0;i<iNumBone;i++)
		{
			int iNumIndex=m_ppCluster[i]->GetControlPointIndicesCount();//このボーンに影響を受ける頂点数
			int* piIndex=m_ppCluster[i]->GetControlPointIndices();
			double* pdWeight=m_ppCluster[i]->GetControlPointWeights();
			//頂点側からインデックスをたどって、頂点サイドで整理する
			for(k=0;k<iNumIndex;k++)
			{				
				//その頂点を含んでいるポリゴンすべてに、このボーンとウェイトを適用
				for(int p=0;p<PolyTable[ piIndex[k] ].NumRef;p++)
				{	
					//頂点→属すポリゴン→そのポリゴンのUVインデックス　と逆引き
					PolyIndex=PolyTable[ piIndex[k] ].PolyIndex[p];
					UVIndex=pFbxMesh->GetTextureUVIndex(PolyIndex,PolyTable[ piIndex[k] ].Index123[p],FbxLayerElement::eTextureDiffuse);
								
					for(int m=0;m<4;m++)//FBXやCGソフトがボーン4本以内とは限らない。5本以上の場合は、重みの大きい順に4本に絞る 
					{	
						if(pdWeight[k] > pvVB[ UVIndex ].bBoneWeight[m] )
						{
							pvVB[UVIndex].bBoneIndex[ m]=i;
							pvVB[UVIndex].bBoneWeight[m]=pdWeight[k];
							break;	
						}		
					}
				}	
		
			}
		}
	}

	//
	//ボーンを生成
	m_iNumBone=iNumBone;
	m_bone =new BONE[iNumBone];

	for(i=0;i<m_iNumBone;i++)
	{
		FbxAMatrix mat;
		m_ppCluster[i]->GetTransformLinkMatrix(mat);
		
		for(int x=0;x<4;x++)
		{
			for(int y=0;y<4;y++)
			{
				m_bone[i].bindPose.mData[y][x]=mat.Get(y,x);
				m_bone[i].bindPose.mData[0][0]*=-1;
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
	if(FAILED(InitFBX(szFileName)))
	{
		MessageBox(0,L"FBXローダー初期化失敗",NULL,MB_OK);
		return E_FAIL;
	}

	FbxNode* pNode=m_pmyScene->GetRootNode();

	int index=0;
	FbxNodeAttribute* pAttr=pNode->GetNodeAttribute();
	while(!pAttr || pAttr->GetAttributeType() != FbxNodeAttribute::eMesh)//1メッシュを想定しているので、eMESHが見つかった時点で、それを使う
	{
		pNode=pNode->GetChild(index);
		index++;
		pAttr=pNode->GetNodeAttribute();
	}

	FbxMesh* pFbxMesh=pNode->GetMesh();

	//事前に頂点数、ポリゴン数等を調べる
	m_dwNumVert=pFbxMesh->GetControlPointsCount();
	m_dwNumUV=pFbxMesh->GetTextureUVCount();
	m_dwNumFace=pFbxMesh->GetPolygonCount();	
	//一時的なメモリ確保（頂点バッファとインデックスバッファ）
	MY_VERTEX* pvVB=NULL;
	m_dwNumVert<m_dwNumUV ?  pvVB =new MY_VERTEX[ m_dwNumUV ] :  pvVB =new MY_VERTEX[ m_dwNumVert ];
	////ポリゴンごとに　頂点読み込み 、法線読み込み、UV読み込み
	FbxVector4 Normal;
	for(DWORD i=0;i<m_dwNumFace;i++)
	{	
		int iIndex0=0;
		int iIndex1=0;
		int iIndex2=0;	

		int iStartIndex=pFbxMesh->GetPolygonVertexIndex(i);
		int* piIndex=pFbxMesh->GetPolygonVertices();//（頂点インデックス）読み込み
		iIndex0=piIndex[ iStartIndex ];
		iIndex1=piIndex[ iStartIndex+1 ];
		iIndex2=piIndex[ iStartIndex+2 ];

		if(m_dwNumVert<m_dwNumUV)//UV数のほうが多い場合は本来の（頂点ベース）インデックスを利用しない。UVインデックスを基準にする
		{
			iIndex0=pFbxMesh->GetTextureUVIndex(i,0,FbxLayerElement::eTextureDiffuse);
			iIndex1=pFbxMesh->GetTextureUVIndex(i,1,FbxLayerElement::eTextureDiffuse);
			iIndex2=pFbxMesh->GetTextureUVIndex(i,2,FbxLayerElement::eTextureDiffuse);
		}
		//頂点
		int index=pFbxMesh->GetPolygonVertex(i,0);
		FbxVector4 Coord=pFbxMesh->GetControlPointAt(index);	
		pvVB[iIndex0].pos.m_x=-Coord.mData[0];
		pvVB[iIndex0].pos.m_y=Coord.mData[1];
		pvVB[iIndex0].pos.m_z=Coord.mData[2];

		index=pFbxMesh->GetPolygonVertex(i,1);
		Coord=pFbxMesh->GetControlPointAt(index);	
		pvVB[iIndex1].pos.m_x=-Coord.mData[0];
		pvVB[iIndex1].pos.m_y=Coord.mData[1];
		pvVB[iIndex1].pos.m_z=Coord.mData[2];

		index=pFbxMesh->GetPolygonVertex(i,2);
		Coord=pFbxMesh->GetControlPointAt(index);	
		pvVB[iIndex2].pos.m_x=-Coord.mData[0];
		pvVB[iIndex2].pos.m_y=Coord.mData[1];
		pvVB[iIndex2].pos.m_z=Coord.mData[2];

		//法線		
		pFbxMesh->GetPolygonVertexNormal(i,0,Normal);		
		pvVB[iIndex0].norm.m_x=-Normal[0];
		pvVB[iIndex0].norm.m_y=Normal[1];
		pvVB[iIndex0].norm.m_z=Normal[2];

		pFbxMesh->GetPolygonVertexNormal(i,1,Normal);
		pvVB[iIndex1].norm.m_x=-Normal[0];
		pvVB[iIndex1].norm.m_y=Normal[1];
		pvVB[iIndex1].norm.m_z=Normal[2];

		pFbxMesh->GetPolygonVertexNormal(i,2,Normal);
		pvVB[iIndex2].norm.m_x=-Normal[0];
		pvVB[iIndex2].norm.m_y=Normal[1];
		pvVB[iIndex2].norm.m_z=Normal[2];

		//テクスチャー座標
		int UVindex=pFbxMesh->GetTextureUVIndex(i,0,FbxLayerElement::eTextureDiffuse);
		FbxLayerElementUV* pUV=0;

		FbxLayerElementUV* uv= pFbxMesh->GetLayer(0)->GetUVs();

		if(m_dwNumUV && uv->GetMappingMode()==FbxLayerElement::eByPolygonVertex)
		{
			//この取得の仕方は、UVマッピングモードがeBY_POLYGON_VERTEXの時じゃないとできない
			UVindex=pFbxMesh->GetTextureUVIndex(i,0,FbxLayerElement::eTextureDiffuse);
			pUV=pFbxMesh->GetLayer(0)->GetUVs();
			FbxVector2 v2=pUV->GetDirectArray().GetAt(UVindex);
			pvVB[iIndex0].tex.m_x=v2.mData[0];
			pvVB[iIndex0].tex.m_y=1.0f-v2.mData[1];

			UVindex=pFbxMesh->GetTextureUVIndex(i,1,FbxLayerElement::eTextureDiffuse);
			v2=pUV->GetDirectArray().GetAt(UVindex);
			pvVB[iIndex1].tex.m_x=v2.mData[0];
			pvVB[iIndex1].tex.m_y=1.0f-v2.mData[1];

			UVindex=pFbxMesh->GetTextureUVIndex(i,2,FbxLayerElement::eTextureDiffuse);
			v2=pUV->GetDirectArray().GetAt(UVindex);
			pvVB[iIndex2].tex.m_x=v2.mData[0];
			pvVB[iIndex2].tex.m_y=1.0f-v2.mData[1];
		}	
	}

	FbxLayerElementUV* uv= pFbxMesh->GetLayer(0)->GetUVs();
	if(m_dwNumUV && uv->GetMappingMode()==FbxLayerElement::eByControlPoint)
	{
		FbxLayerElementUV* pUV=pFbxMesh->GetLayer(0)->GetUVs();
		for(DWORD k=0;k<m_dwNumUV;k++)
		{
			FbxVector2 v2;
			v2=pUV->GetDirectArray().GetAt(k);
			pvVB[k].tex.m_x=v2.mData[0];
			pvVB[k].tex.m_y=1.0f-v2.mData[1];
		}
	}

	//マテリアル読み込み
	pNode=pFbxMesh->GetNode();
	m_dwNumMaterial=pNode->GetMaterialCount();

	m_pMaterial=new MY_MATERIAL[m_dwNumMaterial];

	//マテリアルの数だけインデックスバッファーを作成
	m_ppIndexBuffer=new ID3D11Buffer*[m_dwNumMaterial];

	auto SetMaterial = [](Vec4* set, FbxDouble* get)
	{
		set->m_x = (float)get[0];
		set->m_y = (float)get[1];
		set->m_z = (float)get[2];
		set->m_w = 1;
	};

	for(DWORD i=0;i<m_dwNumMaterial;i++)
	{
		//フォンモデルを想定
		FbxSurfaceMaterial* pMaterial=pNode->GetMaterial(i);
		FbxSurfacePhong* pPhong=(FbxSurfacePhong*)pMaterial;
				
		//環境光
		FbxDouble3 d3Ambient=pPhong->Ambient;
		auto a = d3Ambient.mData;
		SetMaterial(&m_pMaterial[i].Ka, a);

		//拡散反射光
		FbxDouble3 d3Diffuse=pPhong->Diffuse;
		auto d = d3Diffuse.mData;
		SetMaterial(&m_pMaterial[i].Kd, d);


		//鏡面反射光
		FbxDouble3 d3Specular=pPhong->Specular;
		auto s = d3Specular.mData;
		SetMaterial(&m_pMaterial[i].Ks, s);

		//テクスチャー（ディフューズテクスチャーのみ）
		FbxProperty lProperty;
		lProperty=pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		FbxTexture* texture=FbxCast<FbxTexture>(lProperty.GetSrcObject(FbxTexture::ClassId, 0));

		if(texture)
		{
			strcpy_s(m_pMaterial[i].szTextureName,texture->GetName());
			//テクスチャーを作成
			if(FAILED(D3DX11CreateShaderResourceViewFromFileA(device, m_pMaterial[i].szTextureName, NULL, NULL, &m_pMaterial[i].pTexture, NULL )))//絶対パスファイル名は、まず失敗すると思うが、、、
			{	
				char ext[11];
				_splitpath(m_pMaterial[i].szTextureName, 0, 0,m_pMaterial[i].szTextureName,ext );
				//_splitpath_s(m_pMaterial[i].szTextureName, 0, 0,0,0,m_pMaterial[i].szTextureName,sizeof(m_pMaterial[i].szTextureName),ext,sizeof(ext) );
				strcat_s(m_pMaterial[i].szTextureName,ext);
		
				if(FAILED(D3DX11CreateShaderResourceViewFromFileA(device, m_pMaterial[i].szTextureName, NULL, NULL, &m_pMaterial[i].pTexture, NULL )))//ファイル名のみでトライ
				{			
					MessageBox(0,L"テクスチャー読み込み失敗",NULL,MB_OK);
					return E_FAIL;
				}			
			}
		}	
		//マテリアルの数だけインデックスバッファーを作成
		int iCount=0;
		int* pIndex=new int[m_dwNumFace*3];//とりあえず、メッシュ内のポリゴン数でメモリ確保（個々のポリゴングループはかならずこれ以下になるが）
	
		//そのマテリアルであるインデックス配列内の開始インデックスを調べる　さらにインデックスの個数も調べる		
		iCount=0;
		for(DWORD k=0;k<m_dwNumFace;k++)
		{
			FbxLayerElementMaterial* mat = pFbxMesh->GetLayer(0)->GetMaterials();//レイヤーが1枚だけを想定
			int matId=mat->GetIndexArray().GetAt(k);
			if(matId==i)
			{
				if(m_dwNumVert<m_dwNumUV) 
				{
					pIndex[iCount]=pFbxMesh->GetTextureUVIndex(k,0,FbxLayerElement::eTextureDiffuse);
					pIndex[iCount+1]=pFbxMesh->GetTextureUVIndex(k,1,FbxLayerElement::eTextureDiffuse);
					pIndex[iCount+2]=pFbxMesh->GetTextureUVIndex(k,2,FbxLayerElement::eTextureDiffuse);
				}
				else
				{
					pIndex[iCount]=pFbxMesh->GetPolygonVertex(k,0);
					pIndex[iCount+1]=pFbxMesh->GetPolygonVertex(k,1);
					pIndex[iCount+2]=pFbxMesh->GetPolygonVertex(k,2);
				}			
				iCount+=3;
			}
		}
		if(iCount>0) CreateIndexBuffer(iCount*sizeof(int),pIndex ,&m_ppIndexBuffer[i]);
		m_pMaterial[i].dwNumFace=iCount/3;//そのマテリアル内のポリゴン数		
		delete[] pIndex;
	}

	//頂点からポリゴンを逆引きしたいので、逆引きテーブルを作る 
	POLY_TABLE* PolyTable=new POLY_TABLE[m_dwNumVert];

	for(DWORD i=0;i<m_dwNumVert;i++)
	{
		for(DWORD k=0;k<m_dwNumFace;k++)
		{
			for(int m=0;m<3;m++)
			{
				if(pFbxMesh->GetPolygonVertex(k,m)==i)
				{
					PolyTable[i].PolyIndex[PolyTable[i].NumRef]=k;
					PolyTable[i].Index123[PolyTable[i].NumRef]=m;
					PolyTable[i].NumRef++;
				}
			}			
		}
	}

	//スキン情報（ジョイント、ウェイト　）読み込み
	ReadSkinInfo(pFbxMesh,pvVB,PolyTable);

	//バーテックスバッファーを作成
	if(m_dwNumVert<m_dwNumUV) m_dwNumVert=m_dwNumUV;
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( MY_VERTEX ) *m_dwNumVert;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = pvVB;
	if( FAILED(device->CreateBuffer( &bd, &InitData, &m_vertexBuffer ) ) )
		return FALSE;

	//一時的な入れ物は、もはや不要
	delete[] pvVB;

	return S_OK;
}
//
//HRESULT FBX_SKINMESH::CreateIndexBuffer(DWORD dwSize,int* pIndex,ID3D11Buffer** ppIndexBuffer)
//Direct3Dのインデックスバッファー作成
HRESULT FbxSkinMesh::CreateIndexBuffer(DWORD dwSize,int* pIndex,ID3D11Buffer** ppIndexBuffer)
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
	InitData.SysMemPitch=0;
	InitData.SysMemSlicePitch=0;
	if( FAILED(device->CreateBuffer( &bd, &InitData, ppIndexBuffer) ) )
	{
		return FALSE;
	}
	
	return S_OK;
}
//
//void FBX_SKINMESH::SetNewPoseMatrices(int frame)
//ボーンを次のポーズ位置にセットする
void FbxSkinMesh::SetNewPoseMatrices(int frame)
{
	Matrix m;
	m.m_value[0][0] = -1.0f;

	int i;
	FbxTime time;
	time.SetTime(0,0,0,frame,0, FbxTime::eFrames30,0);//30フレーム/秒　と推定　厳密には状況ごとに調べる必要あり

	for(i=0;i<m_iNumBone;i++)
	{		
		FbxMatrix mat=m_ppCluster[i]->GetLink()->GetGlobalFromCurrentTake( time );

		for(int x=0;x<4;x++)
		{
			for(int y=0;y<4;y++)
			{
				m_bone[i].newPose(y,x)=mat.Get(y,x);				
			}
		}
		m_bone[i].newPose*=m;//FBX右手座標系なのでｘが逆　補正する	
	}

	//
	//フレームを進めたことにより変化したポーズ（ボーンの行列）をシェーダーに渡す
	D3D11_MAPPED_SUBRESOURCE pData;
	if( SUCCEEDED( m_pDeviceContext->Map( m_pConstantBufferBone, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		SHADER_GLOBAL_BONES sg;
		for( int i = 0; i < m_iNumBone; i++ )
		{
			D3DXMATRIX mat = GetCurrentPoseMatrix(i);
			D3DXMatrixTranspose(&mat,&mat);
			sg.mBone[i]=mat;
		}
		memcpy_s( pData.pData, pData.RowPitch, (void*)&sg, sizeof( SHADER_GLOBAL_BONES ) );
		m_pDeviceContext->Unmap( m_pConstantBufferBone, 0 );
	}
	m_pDeviceContext->VSSetConstantBuffers(2,1,&m_pConstantBufferBone);
	m_pDeviceContext->PSSetConstantBuffers(2,1,&m_pConstantBufferBone );
}
//
//D3DXMATRIX FBX_SKINMESH::GetCurrentPoseMatrix(int index)
//次の（現在の）ポーズ行列を返す
Matrix FbxSkinMesh::GetCurrentPoseMatrix(int index)
{
	D3DXMATRIX inv;
	D3DXMatrixInverse(&inv,0,&m_BoneArray[index].mBindPose);//FBXのバインドポーズは初期姿勢（絶対座標）
	D3DXMATRIX ret=inv*m_BoneArray[index].mNewPose;//バインドポーズの逆行列とフレーム姿勢行列をかける。なお、バインドポーズ自体が既に逆行列であるとする考えもある。（FBXの場合は違うが）

	return ret;
}
//
//
//レンダリング
void FbxSkinMesh::Render(const Matrix& mView, const Matrix& mProj,
	const Vec3& vLight, const Vec3& vEye)
{
	D3DXMATRIX mWorld,mTran,mYaw,mPitch,mRoll,mScale;
	m_mView=mView;
	m_mProj=mProj;
	//ワールドトランスフォーム（絶対座標変換）
	D3DXMatrixScaling(&mScale,m_fScale,m_fScale,m_fScale);
	D3DXMatrixRotationY(&mYaw,m_fYaw);
	D3DXMatrixRotationX(&mPitch,m_fPitch);
	D3DXMatrixRotationZ(&mRoll,m_fRoll);
	D3DXMatrixTranslation(&mTran,m_vPos.x,m_vPos.y,m_vPos.z);

	mWorld=mScale*mYaw*mPitch*mRoll*mTran;
	m_mFinalWorld=mWorld;
	//バーテックスバッファーをセット（バーテックスバッファーは一つでいい）
	UINT stride = sizeof( MY_VERTEX );
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );
	//使用するシェーダーの登録	
	m_pDeviceContext->VSSetShader(m_pVertexShader,NULL,0);
	m_pDeviceContext->PSSetShader(m_pPixelShader,NULL,0);
	//頂点インプットレイアウトをセット
	m_pDeviceContext->IASetInputLayout( m_pVertexLayout );

	D3D11_MAPPED_SUBRESOURCE pData;
	if( SUCCEEDED( m_pDeviceContext->Map( m_pConstantBuffer0, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		SHADER_GLOBAL0 sg;
		//ライトの方向を渡す
		sg.vLightDir=D3DXVECTOR4(vLight.x,vLight.y,vLight.z,0.0f);
		//視点位置を渡す
		sg.vEye=D3DXVECTOR4(vEye.x,vEye.y,vEye.z,0);

		memcpy_s( pData.pData, pData.RowPitch, (void*)&sg, sizeof( SHADER_GLOBAL0 ) );
		m_pDeviceContext->Unmap( m_pConstantBuffer0, 0 );
	}
	m_pDeviceContext->VSSetConstantBuffers(0,1,&m_pConstantBuffer0 );
	m_pDeviceContext->PSSetConstantBuffers(0,1,&m_pConstantBuffer0 );
	//マテリアルの数だけ、それぞれのマテリアルのインデックスバッファ－を描画
	for(DWORD i=0;i<m_dwNumMaterial;i++)
	{
		//使用されていないマテリアル対策
		if(m_pMaterial[i].dwNumFace==0)
		{
			continue;
		}
		//インデックスバッファーをセット
		stride = sizeof( int );
		offset = 0;
		m_pDeviceContext->IASetIndexBuffer(m_ppIndexBuffer[i], DXGI_FORMAT_R32_UINT, 0 );

		//プリミティブ・トポロジーをセット
		m_pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		//マテリアルの各要素と変換行列をシェーダーに渡す

		//ワールド行列をシェーダーに渡す
		//ワールド・ビュー・プロジェクション行列をシェーダーに渡す
		//ディフューズカラーをシェーダーに渡す
		//スペキュラーをシェーダーに渡す
		//スペキュラーのパワーをシェーダーに渡す
		//アンビエントをシェーダーに渡す

		D3D11_MAPPED_SUBRESOURCE pData;
		if( SUCCEEDED( m_pDeviceContext->Map( m_pConstantBuffer1,0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
		{
			SHADER_GLOBAL1 sg;
			sg.mW=m_mFinalWorld;
			D3DXMatrixTranspose(&sg.mW, &sg.mW);
			sg.mWVP=m_mFinalWorld*m_mView*m_mProj;
			D3DXMatrixTranspose( &sg.mWVP, &sg.mWVP );
			sg.vAmbient=m_pMaterial[i].Ka;//アンビエントををシェーダーに渡す
			sg.vDiffuse=m_pMaterial[i].Kd;//ディフューズカラーをシェーダーに渡す
			sg.vSpecular=m_pMaterial[i].Ks;//スペキュラーをシェーダーに渡す
			memcpy_s( pData.pData, pData.RowPitch, (void*)&sg, sizeof( SHADER_GLOBAL1 ) );
			m_pDeviceContext->Unmap( m_pConstantBuffer1, 0 );
		}
		m_pDeviceContext->VSSetConstantBuffers(1,1,&m_pConstantBuffer1 );
		m_pDeviceContext->PSSetConstantBuffers(1,1,&m_pConstantBuffer1 );
		//テクスチャーをシェーダーに渡す
		if(m_pMaterial[i].szTextureName[0] != NULL)
		{
			m_pDeviceContext->PSSetSamplers(0,1,&m_pSampleLinear);
			m_pDeviceContext->PSSetShaderResources(0,1,&m_pMaterial[i].pTexture);
		}
		//Draw
		m_pDeviceContext->DrawIndexed(m_pMaterial[i].dwNumFace*3 , 0 ,0);
	}
}