#include "SL_FbxSkinMesh.h"

#include <functional>

#include "SL_Window.h"
#include "SL_Graphics.h"

using namespace std;
using namespace ShunLib;
//
//FBX_SKINMESH::FBX_SKINMESH()
//�R���X�g���N�^�[
FbxSkinMesh::FbxSkinMesh()
{
	ZeroMemory(this,sizeof(FbxSkinMesh));
	m_fScale=1.0f;
}

//
//FBX_SKINMESH::~FBX_SKINMESH()
//�f�X�g���N�^�[
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
		MessageBox(0,L"���b�V���p�V�F�[�_�[�쐬���s",NULL,MB_OK);
		return E_FAIL;
	}
	if(FAILED(CreateFromFBX(FileName)))
	{
		MessageBox(0,L"���b�V���쐬���s",NULL,MB_OK);
		return E_FAIL;
	}
	//�e�N�X�`���[�p�T���v���[�쐬
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

	//�o�[�e�b�N�X�V�F�[�_�[�쐬
	if (FAILED(graphic->CreateVertexShader(L"SKIN_MESH.hlsl", "VSSkin", "vs_5_0",&m_vertexShader)))
	{
		MessageBox(0, L"�o�[�e�b�N�X�V�F�[�_�[�쐬���s", NULL, MB_OK);
		return E_FAIL;
	}

	//���_�C���v�b�g���C�A�E�g���`	
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION"   , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"     , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD"   , 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_INDEX" , 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout)/sizeof(layout[0]);



	//���_�C���v�b�g���C�A�E�g�쐬
	if (FAILED(graphic->CreateInputLayout(layout, numElements, &m_vertexLayout)))
	{
		MessageBox(0, L"���_�C���v�b�g���C�A�E�g�쐬���s", NULL, MB_OK);
		return E_FAIL;
	}

	//�s�N�Z���V�F�[�_�[�쐬
	if (FAILED(graphic->CreatePixleShader(L"SKIN_MESH.hlsl", "PSSkin", "ps_5_0",&m_pixelShader)))
	{
		MessageBox(0, L"�s�N�Z���V�F�[�_�[�쐬���s", NULL, MB_OK);
		return E_FAIL;
	}

	//�R���X�^���g�o�b�t�@�[�쐬�@�ϊ��s��n���p
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
	
	//�R���X�^���g�o�b�t�@�[�쐬  �}�e���A���n���p
	cb.BindFlags= D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth= sizeof( SHADER_GLOBAL1 );
	cb.CPUAccessFlags=D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags	=0;
	cb.Usage=D3D11_USAGE_DYNAMIC;

	if( FAILED(device->CreateBuffer( &cb,NULL,&m_pConstantBuffer1)))
	{
		return E_FAIL;
	}

	//�R���X�^���g�o�b�t�@�[�{�[���p�@�쐬
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
//�w�肵���t�@�C����FBX������������
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
//FBX����X�L���֘A�̏���ǂݏo���@
HRESULT FbxSkinMesh::ReadSkinInfo(FbxMesh* pFbxMesh,MY_VERTEX* pvVB,POLY_TABLE* PolyTable)
{
	//FBX���璊�o���ׂ����́A���_���Ƃ̃{�[���C���f�b�N�X�A���_���Ƃ̃{�[���E�F�C�g�A�o�C���h�s��A�|�[�Y�s��@��4����

	int i,k;
	int iNumBone=0;//�{�[����

	//�f�t�H�[�}�[�𓾂� 
	FbxDeformer* pDeformer=pFbxMesh->GetDeformer(0);
	FbxSkin* pSkinInfo=static_cast<FbxSkin*>(pDeformer);
	//
	//�{�[���𓾂�
	iNumBone=pSkinInfo->GetClusterCount();
	m_ppCluster= new fbxsdk::FbxCluster*[ iNumBone ];
	for( i=0;i<iNumBone;i++)
	{
		m_ppCluster[i]=pSkinInfo->GetCluster(i);
	}

	//�ʏ�̏ꍇ�@�i���_����=UV���@pvVB�����_�C���f�b�N�X�x�[�X�̏ꍇ�j
	if(m_dwNumVert>=m_dwNumUV)
	{
		//���ꂼ��̃{�[���ɉe�����󂯂钸�_�𒲂ׂ�@��������t�ɁA���_�x�[�X�Ń{�[���C���f�b�N�X�E�d�݂𐮓ڂ���
		for(i=0;i<iNumBone;i++)
		{
			int iNumIndex=m_ppCluster[i]->GetControlPointIndicesCount();//���̃{�[���ɉe�����󂯂钸�_��
			int* piIndex=m_ppCluster[i]->GetControlPointIndices();
			double* pdWeight=m_ppCluster[i]->GetControlPointWeights();
			//���_������C���f�b�N�X�����ǂ��āA���_�T�C�h�Ő�������
			for(k=0;k<iNumIndex;k++)
			{
				for(int m=0;m<4;m++)//FBX��CG�\�t�g���{�[��4�{�ȓ��Ƃ͌���Ȃ��B5�{�ȏ�̏ꍇ�́A�d�݂̑傫������4�{�ɍi�� 
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
	//UV�C���f�b�N�X�x�[�X�̏ꍇ�@�i���_��<UV���j
	else
	{
		int PolyIndex=0;
		int UVIndex=0;
		//���ꂼ��̃{�[���ɉe�����󂯂钸�_�𒲂ׂ�@��������t�ɁA���_�x�[�X�Ń{�[���C���f�b�N�X�E�d�݂𐮓ڂ���
		for(i=0;i<iNumBone;i++)
		{
			int iNumIndex=m_ppCluster[i]->GetControlPointIndicesCount();//���̃{�[���ɉe�����󂯂钸�_��
			int* piIndex=m_ppCluster[i]->GetControlPointIndices();
			double* pdWeight=m_ppCluster[i]->GetControlPointWeights();
			//���_������C���f�b�N�X�����ǂ��āA���_�T�C�h�Ő�������
			for(k=0;k<iNumIndex;k++)
			{				
				//���̒��_���܂�ł���|���S�����ׂĂɁA���̃{�[���ƃE�F�C�g��K�p
				for(int p=0;p<PolyTable[ piIndex[k] ].NumRef;p++)
				{	
					//���_�������|���S�������̃|���S����UV�C���f�b�N�X�@�Ƌt����
					PolyIndex=PolyTable[ piIndex[k] ].PolyIndex[p];
					UVIndex=pFbxMesh->GetTextureUVIndex(PolyIndex,PolyTable[ piIndex[k] ].Index123[p],FbxLayerElement::eTextureDiffuse);
								
					for(int m=0;m<4;m++)//FBX��CG�\�t�g���{�[��4�{�ȓ��Ƃ͌���Ȃ��B5�{�ȏ�̏ꍇ�́A�d�݂̑傫������4�{�ɍi�� 
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
	//�{�[���𐶐�
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
//FBX����X�L�����b�V�����쐬����@�@���Ӂj�f�ށiFBX)�̂ق��́A�O�p�|���S���ɂ��邱��
HRESULT FbxSkinMesh::CreateFromFBX(CHAR* szFileName)
{
	auto graphic = Graphics::GetInstance();
	auto device = Window::GetInstance()->Device();

	//FBX���[�_�[��������
	if(FAILED(InitFBX(szFileName)))
	{
		MessageBox(0,L"FBX���[�_�[���������s",NULL,MB_OK);
		return E_FAIL;
	}

	FbxNode* pNode=m_pmyScene->GetRootNode();

	int index=0;
	FbxNodeAttribute* pAttr=pNode->GetNodeAttribute();
	while(!pAttr || pAttr->GetAttributeType() != FbxNodeAttribute::eMesh)//1���b�V����z�肵�Ă���̂ŁAeMESH�������������_�ŁA������g��
	{
		pNode=pNode->GetChild(index);
		index++;
		pAttr=pNode->GetNodeAttribute();
	}

	FbxMesh* pFbxMesh=pNode->GetMesh();

	//���O�ɒ��_���A�|���S�������𒲂ׂ�
	m_dwNumVert=pFbxMesh->GetControlPointsCount();
	m_dwNumUV=pFbxMesh->GetTextureUVCount();
	m_dwNumFace=pFbxMesh->GetPolygonCount();	
	//�ꎞ�I�ȃ������m�ہi���_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@�j
	MY_VERTEX* pvVB=NULL;
	m_dwNumVert<m_dwNumUV ?  pvVB =new MY_VERTEX[ m_dwNumUV ] :  pvVB =new MY_VERTEX[ m_dwNumVert ];
	////�|���S�����ƂɁ@���_�ǂݍ��� �A�@���ǂݍ��݁AUV�ǂݍ���
	FbxVector4 Normal;
	for(DWORD i=0;i<m_dwNumFace;i++)
	{	
		int iIndex0=0;
		int iIndex1=0;
		int iIndex2=0;	

		int iStartIndex=pFbxMesh->GetPolygonVertexIndex(i);
		int* piIndex=pFbxMesh->GetPolygonVertices();//�i���_�C���f�b�N�X�j�ǂݍ���
		iIndex0=piIndex[ iStartIndex ];
		iIndex1=piIndex[ iStartIndex+1 ];
		iIndex2=piIndex[ iStartIndex+2 ];

		if(m_dwNumVert<m_dwNumUV)//UV���̂ق��������ꍇ�͖{���́i���_�x�[�X�j�C���f�b�N�X�𗘗p���Ȃ��BUV�C���f�b�N�X����ɂ���
		{
			iIndex0=pFbxMesh->GetTextureUVIndex(i,0,FbxLayerElement::eTextureDiffuse);
			iIndex1=pFbxMesh->GetTextureUVIndex(i,1,FbxLayerElement::eTextureDiffuse);
			iIndex2=pFbxMesh->GetTextureUVIndex(i,2,FbxLayerElement::eTextureDiffuse);
		}
		//���_
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

		//�@��		
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

		//�e�N�X�`���[���W
		int UVindex=pFbxMesh->GetTextureUVIndex(i,0,FbxLayerElement::eTextureDiffuse);
		FbxLayerElementUV* pUV=0;

		FbxLayerElementUV* uv= pFbxMesh->GetLayer(0)->GetUVs();

		if(m_dwNumUV && uv->GetMappingMode()==FbxLayerElement::eByPolygonVertex)
		{
			//���̎擾�̎d���́AUV�}�b�s���O���[�h��eBY_POLYGON_VERTEX�̎�����Ȃ��Ƃł��Ȃ�
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

	//�}�e���A���ǂݍ���
	pNode=pFbxMesh->GetNode();
	m_dwNumMaterial=pNode->GetMaterialCount();

	m_pMaterial=new MY_MATERIAL[m_dwNumMaterial];

	//�}�e���A���̐������C���f�b�N�X�o�b�t�@�[���쐬
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
		//�t�H�����f����z��
		FbxSurfaceMaterial* pMaterial=pNode->GetMaterial(i);
		FbxSurfacePhong* pPhong=(FbxSurfacePhong*)pMaterial;
				
		//����
		FbxDouble3 d3Ambient=pPhong->Ambient;
		auto a = d3Ambient.mData;
		SetMaterial(&m_pMaterial[i].Ka, a);

		//�g�U���ˌ�
		FbxDouble3 d3Diffuse=pPhong->Diffuse;
		auto d = d3Diffuse.mData;
		SetMaterial(&m_pMaterial[i].Kd, d);


		//���ʔ��ˌ�
		FbxDouble3 d3Specular=pPhong->Specular;
		auto s = d3Specular.mData;
		SetMaterial(&m_pMaterial[i].Ks, s);

		//�e�N�X�`���[�i�f�B�t���[�Y�e�N�X�`���[�̂݁j
		FbxProperty lProperty;
		lProperty=pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		FbxTexture* texture=FbxCast<FbxTexture>(lProperty.GetSrcObject(FbxTexture::ClassId, 0));

		if(texture)
		{
			strcpy_s(m_pMaterial[i].szTextureName,texture->GetName());
			//�e�N�X�`���[���쐬
			if(FAILED(D3DX11CreateShaderResourceViewFromFileA(device, m_pMaterial[i].szTextureName, NULL, NULL, &m_pMaterial[i].pTexture, NULL )))//��΃p�X�t�@�C�����́A�܂����s����Ǝv�����A�A�A
			{	
				char ext[11];
				_splitpath(m_pMaterial[i].szTextureName, 0, 0,m_pMaterial[i].szTextureName,ext );
				//_splitpath_s(m_pMaterial[i].szTextureName, 0, 0,0,0,m_pMaterial[i].szTextureName,sizeof(m_pMaterial[i].szTextureName),ext,sizeof(ext) );
				strcat_s(m_pMaterial[i].szTextureName,ext);
		
				if(FAILED(D3DX11CreateShaderResourceViewFromFileA(device, m_pMaterial[i].szTextureName, NULL, NULL, &m_pMaterial[i].pTexture, NULL )))//�t�@�C�����݂̂Ńg���C
				{			
					MessageBox(0,L"�e�N�X�`���[�ǂݍ��ݎ��s",NULL,MB_OK);
					return E_FAIL;
				}			
			}
		}	
		//�}�e���A���̐������C���f�b�N�X�o�b�t�@�[���쐬
		int iCount=0;
		int* pIndex=new int[m_dwNumFace*3];//�Ƃ肠�����A���b�V�����̃|���S�����Ń������m�ہi�X�̃|���S���O���[�v�͂��Ȃ炸����ȉ��ɂȂ邪�j
	
		//���̃}�e���A���ł���C���f�b�N�X�z����̊J�n�C���f�b�N�X�𒲂ׂ�@����ɃC���f�b�N�X�̌������ׂ�		
		iCount=0;
		for(DWORD k=0;k<m_dwNumFace;k++)
		{
			FbxLayerElementMaterial* mat = pFbxMesh->GetLayer(0)->GetMaterials();//���C���[��1��������z��
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
		m_pMaterial[i].dwNumFace=iCount/3;//���̃}�e���A�����̃|���S����		
		delete[] pIndex;
	}

	//���_����|���S�����t�����������̂ŁA�t�����e�[�u������� 
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

	//�X�L�����i�W���C���g�A�E�F�C�g�@�j�ǂݍ���
	ReadSkinInfo(pFbxMesh,pvVB,PolyTable);

	//�o�[�e�b�N�X�o�b�t�@�[���쐬
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

	//�ꎞ�I�ȓ��ꕨ�́A���͂�s�v
	delete[] pvVB;

	return S_OK;
}
//
//HRESULT FBX_SKINMESH::CreateIndexBuffer(DWORD dwSize,int* pIndex,ID3D11Buffer** ppIndexBuffer)
//Direct3D�̃C���f�b�N�X�o�b�t�@�[�쐬
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
//�{�[�������̃|�[�Y�ʒu�ɃZ�b�g����
void FbxSkinMesh::SetNewPoseMatrices(int frame)
{
	Matrix m;
	m.m_value[0][0] = -1.0f;

	int i;
	FbxTime time;
	time.SetTime(0,0,0,frame,0, FbxTime::eFrames30,0);//30�t���[��/�b�@�Ɛ���@�����ɂ͏󋵂��Ƃɒ��ׂ�K�v����

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
		m_bone[i].newPose*=m;//FBX�E����W�n�Ȃ̂ł����t�@�␳����	
	}

	//
	//�t���[����i�߂����Ƃɂ��ω������|�[�Y�i�{�[���̍s��j���V�F�[�_�[�ɓn��
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
//���́i���݂́j�|�[�Y�s���Ԃ�
Matrix FbxSkinMesh::GetCurrentPoseMatrix(int index)
{
	D3DXMATRIX inv;
	D3DXMatrixInverse(&inv,0,&m_BoneArray[index].mBindPose);//FBX�̃o�C���h�|�[�Y�͏����p���i��΍��W�j
	D3DXMATRIX ret=inv*m_BoneArray[index].mNewPose;//�o�C���h�|�[�Y�̋t�s��ƃt���[���p���s���������B�Ȃ��A�o�C���h�|�[�Y���̂����ɋt�s��ł���Ƃ���l��������B�iFBX�̏ꍇ�͈Ⴄ���j

	return ret;
}
//
//
//�����_�����O
void FbxSkinMesh::Render(const Matrix& mView, const Matrix& mProj,
	const Vec3& vLight, const Vec3& vEye)
{
	D3DXMATRIX mWorld,mTran,mYaw,mPitch,mRoll,mScale;
	m_mView=mView;
	m_mProj=mProj;
	//���[���h�g�����X�t�H�[���i��΍��W�ϊ��j
	D3DXMatrixScaling(&mScale,m_fScale,m_fScale,m_fScale);
	D3DXMatrixRotationY(&mYaw,m_fYaw);
	D3DXMatrixRotationX(&mPitch,m_fPitch);
	D3DXMatrixRotationZ(&mRoll,m_fRoll);
	D3DXMatrixTranslation(&mTran,m_vPos.x,m_vPos.y,m_vPos.z);

	mWorld=mScale*mYaw*mPitch*mRoll*mTran;
	m_mFinalWorld=mWorld;
	//�o�[�e�b�N�X�o�b�t�@�[���Z�b�g�i�o�[�e�b�N�X�o�b�t�@�[�͈�ł����j
	UINT stride = sizeof( MY_VERTEX );
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers( 0, 1, &m_pVertexBuffer, &stride, &offset );
	//�g�p����V�F�[�_�[�̓o�^	
	m_pDeviceContext->VSSetShader(m_pVertexShader,NULL,0);
	m_pDeviceContext->PSSetShader(m_pPixelShader,NULL,0);
	//���_�C���v�b�g���C�A�E�g���Z�b�g
	m_pDeviceContext->IASetInputLayout( m_pVertexLayout );

	D3D11_MAPPED_SUBRESOURCE pData;
	if( SUCCEEDED( m_pDeviceContext->Map( m_pConstantBuffer0, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
	{
		SHADER_GLOBAL0 sg;
		//���C�g�̕�����n��
		sg.vLightDir=D3DXVECTOR4(vLight.x,vLight.y,vLight.z,0.0f);
		//���_�ʒu��n��
		sg.vEye=D3DXVECTOR4(vEye.x,vEye.y,vEye.z,0);

		memcpy_s( pData.pData, pData.RowPitch, (void*)&sg, sizeof( SHADER_GLOBAL0 ) );
		m_pDeviceContext->Unmap( m_pConstantBuffer0, 0 );
	}
	m_pDeviceContext->VSSetConstantBuffers(0,1,&m_pConstantBuffer0 );
	m_pDeviceContext->PSSetConstantBuffers(0,1,&m_pConstantBuffer0 );
	//�}�e���A���̐������A���ꂼ��̃}�e���A���̃C���f�b�N�X�o�b�t�@�|��`��
	for(DWORD i=0;i<m_dwNumMaterial;i++)
	{
		//�g�p����Ă��Ȃ��}�e���A���΍�
		if(m_pMaterial[i].dwNumFace==0)
		{
			continue;
		}
		//�C���f�b�N�X�o�b�t�@�[���Z�b�g
		stride = sizeof( int );
		offset = 0;
		m_pDeviceContext->IASetIndexBuffer(m_ppIndexBuffer[i], DXGI_FORMAT_R32_UINT, 0 );

		//�v���~�e�B�u�E�g�|���W�[���Z�b�g
		m_pDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		//�}�e���A���̊e�v�f�ƕϊ��s����V�F�[�_�[�ɓn��

		//���[���h�s����V�F�[�_�[�ɓn��
		//���[���h�E�r���[�E�v���W�F�N�V�����s����V�F�[�_�[�ɓn��
		//�f�B�t���[�Y�J���[���V�F�[�_�[�ɓn��
		//�X�y�L�����[���V�F�[�_�[�ɓn��
		//�X�y�L�����[�̃p���[���V�F�[�_�[�ɓn��
		//�A���r�G���g���V�F�[�_�[�ɓn��

		D3D11_MAPPED_SUBRESOURCE pData;
		if( SUCCEEDED( m_pDeviceContext->Map( m_pConstantBuffer1,0, D3D11_MAP_WRITE_DISCARD, 0, &pData ) ) )
		{
			SHADER_GLOBAL1 sg;
			sg.mW=m_mFinalWorld;
			D3DXMatrixTranspose(&sg.mW, &sg.mW);
			sg.mWVP=m_mFinalWorld*m_mView*m_mProj;
			D3DXMatrixTranspose( &sg.mWVP, &sg.mWVP );
			sg.vAmbient=m_pMaterial[i].Ka;//�A���r�G���g�����V�F�[�_�[�ɓn��
			sg.vDiffuse=m_pMaterial[i].Kd;//�f�B�t���[�Y�J���[���V�F�[�_�[�ɓn��
			sg.vSpecular=m_pMaterial[i].Ks;//�X�y�L�����[���V�F�[�_�[�ɓn��
			memcpy_s( pData.pData, pData.RowPitch, (void*)&sg, sizeof( SHADER_GLOBAL1 ) );
			m_pDeviceContext->Unmap( m_pConstantBuffer1, 0 );
		}
		m_pDeviceContext->VSSetConstantBuffers(1,1,&m_pConstantBuffer1 );
		m_pDeviceContext->PSSetConstantBuffers(1,1,&m_pConstantBuffer1 );
		//�e�N�X�`���[���V�F�[�_�[�ɓn��
		if(m_pMaterial[i].szTextureName[0] != NULL)
		{
			m_pDeviceContext->PSSetSamplers(0,1,&m_pSampleLinear);
			m_pDeviceContext->PSSetShaderResources(0,1,&m_pMaterial[i].pTexture);
		}
		//Draw
		m_pDeviceContext->DrawIndexed(m_pMaterial[i].dwNumFace*3 , 0 ,0);
	}
}