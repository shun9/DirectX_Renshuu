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
		MessageBox(0, L"���b�V���p�V�F�[�_�[�쐬���s", NULL, MB_OK);
		return E_FAIL;
	}
	if (FAILED(CreateFromFBX(FileName)))
	{
		MessageBox(0, L"���b�V���쐬���s", NULL, MB_OK);
		return E_FAIL;
	}
	//�e�N�X�`���[�p�T���v���[�쐬
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
/// �V�F�[�_�[�쐬
/// </summary>
HRESULT FbxSkinMesh::InitShader()
{
	auto graphic = Graphics::GetInstance();
	auto device = Window::GetInstance()->Device();

	//���_�C���v�b�g���C�A�E�g���`	
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION"   , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL"     , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD"   , 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_INDEX" , 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	//�o�[�e�b�N�X�V�F�[�_�[�쐬
	if (FAILED(graphic->CreateVertexShader(L"SKIN_MESH.hlsl", "VSSkin", "vs_5_0", &m_vertexShader, layout, numElements, &m_vertexLayout)))
	{
		MessageBox(0, L"�o�[�e�b�N�X�V�F�[�_�[�쐬���s", NULL, MB_OK);
		return E_FAIL;
	}
	//�s�N�Z���V�F�[�_�[�쐬
	if (FAILED(graphic->CreatePixleShader(L"SKIN_MESH.hlsl", "PSSkin", "ps_5_0", &m_pixelShader)))
	{
		MessageBox(0, L"�s�N�Z���V�F�[�_�[�쐬���s", NULL, MB_OK);
		return E_FAIL;
	}

	//�R���X�^���g�o�b�t�@�[�쐬�@�ϊ��s��n���p
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

	//�R���X�^���g�o�b�t�@�[�쐬  �}�e���A���n���p
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth = sizeof(SHADER_GLOBAL1);
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(device->CreateBuffer(&cb, NULL, &m_constantBuffer1)))
	{
		return E_FAIL;
	}

	//�R���X�^���g�o�b�t�@�[�{�[���p�@�쐬
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
//�w�肵���t�@�C����FBX������������
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
//FBX����X�L���֘A�̏���ǂݏo���@
HRESULT FbxSkinMesh::ReadSkinInfo(FbxMesh* pFbxMesh, MY_VERTEX* pvVB, POLY_TABLE* PolyTable)
{
	//FBX���璊�o���ׂ����́A���_���Ƃ̃{�[���C���f�b�N�X�A���_���Ƃ̃{�[���E�F�C�g�A�o�C���h�s��A�|�[�Y�s��@��4����

	int i, k;
	int iNumBone = 0;//�{�[����

	//�f�t�H�[�}�[�𓾂� 
	FbxDeformer* pDeformer = pFbxMesh->GetDeformer(0);
	FbxSkin* pSkinInfo = static_cast<FbxSkin*>(pDeformer);
	//
	//�{�[���𓾂�
	iNumBone = pSkinInfo->GetClusterCount();
	m_ppCluster = new fbxsdk::FbxCluster*[iNumBone];
	for (i = 0; i < iNumBone; i++)
	{
		m_ppCluster[i] = pSkinInfo->GetCluster(i);
	}

	//�ʏ�̏ꍇ�@�i���_����=UV���@pvVB�����_�C���f�b�N�X�x�[�X�̏ꍇ�j
	if (m_dwNumVert >= m_dwNumUV)
	{
		//���ꂼ��̃{�[���ɉe�����󂯂钸�_�𒲂ׂ�@��������t�ɁA���_�x�[�X�Ń{�[���C���f�b�N�X�E�d�݂𐮓ڂ���
		for (i = 0; i < iNumBone; i++)
		{
			int iNumIndex = m_ppCluster[i]->GetControlPointIndicesCount();//���̃{�[���ɉe�����󂯂钸�_��
			int* piIndex = m_ppCluster[i]->GetControlPointIndices();
			double* pdWeight = m_ppCluster[i]->GetControlPointWeights();
			//���_������C���f�b�N�X�����ǂ��āA���_�T�C�h�Ő�������
			for (k = 0; k < iNumIndex; k++)
			{
				for (int m = 0; m < 4; m++)//FBX��CG�\�t�g���{�[��4�{�ȓ��Ƃ͌���Ȃ��B5�{�ȏ�̏ꍇ�́A�d�݂̑傫������4�{�ɍi�� 
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
	//UV�C���f�b�N�X�x�[�X�̏ꍇ�@�i���_��<UV���j
	else
	{
		int PolyIndex = 0;
		int UVIndex = 0;
		//���ꂼ��̃{�[���ɉe�����󂯂钸�_�𒲂ׂ�@��������t�ɁA���_�x�[�X�Ń{�[���C���f�b�N�X�E�d�݂𐮓ڂ���
		for (i = 0; i < iNumBone; i++)
		{
			int iNumIndex = m_ppCluster[i]->GetControlPointIndicesCount();//���̃{�[���ɉe�����󂯂钸�_��
			int* piIndex = m_ppCluster[i]->GetControlPointIndices();
			double* pdWeight = m_ppCluster[i]->GetControlPointWeights();
			//���_������C���f�b�N�X�����ǂ��āA���_�T�C�h�Ő�������
			for (k = 0; k < iNumIndex; k++)
			{
				//���̒��_���܂�ł���|���S�����ׂĂɁA���̃{�[���ƃE�F�C�g��K�p
				for (int p = 0; p < PolyTable[piIndex[k]].NumRef; p++)
				{
					//���_�������|���S�������̃|���S����UV�C���f�b�N�X�@�Ƌt����
					PolyIndex = PolyTable[piIndex[k]].PolyIndex[p];
					UVIndex = pFbxMesh->GetTextureUVIndex(PolyIndex, PolyTable[piIndex[k]].Index123[p], FbxLayerElement::eTextureDiffuse);

					for (int m = 0; m < 4; m++)//FBX��CG�\�t�g���{�[��4�{�ȓ��Ƃ͌���Ȃ��B5�{�ȏ�̏ꍇ�́A�d�݂̑傫������4�{�ɍi�� 
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
	//�{�[���𐶐�
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
//FBX����X�L�����b�V�����쐬����@�@���Ӂj�f�ށiFBX)�̂ق��́A�O�p�|���S���ɂ��邱��
HRESULT FbxSkinMesh::CreateFromFBX(CHAR* szFileName)
{
	auto graphic = Graphics::GetInstance();
	auto device = Window::GetInstance()->Device();

	//FBX���[�_�[��������
	if (FAILED(InitFBX(szFileName)))
	{
		MessageBox(0, L"FBX���[�_�[���������s", NULL, MB_OK);
		return E_FAIL;
	}

	FbxNode* pNode = m_scene->GetRootNode();

	int index = 0;
	FbxNodeAttribute* pAttr = pNode->GetNodeAttribute();
	while (!pAttr || pAttr->GetAttributeType() != FbxNodeAttribute::eMesh)//1���b�V����z�肵�Ă���̂ŁAeMESH�������������_�ŁA������g��
	{
		pNode = pNode->GetChild(index);
		index++;
		pAttr = pNode->GetNodeAttribute();
	}

	FbxMesh* pFbxMesh = pNode->GetMesh();

	//���O�ɒ��_���A�|���S�������𒲂ׂ�
	m_dwNumVert = pFbxMesh->GetControlPointsCount();
	m_dwNumUV = pFbxMesh->GetTextureUVCount();
	m_dwNumFace = pFbxMesh->GetPolygonCount();
	//�ꎞ�I�ȃ������m�ہi���_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@�j
	MY_VERTEX* pvVB = NULL;
	m_dwNumVert < m_dwNumUV ? pvVB = new MY_VERTEX[m_dwNumUV] : pvVB = new MY_VERTEX[m_dwNumVert];

	////�|���S�����ƂɁ@���_�ǂݍ��� �A�@���ǂݍ��݁AUV�ǂݍ���
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
		int* piIndex = pFbxMesh->GetPolygonVertices();//�i���_�C���f�b�N�X�j�ǂݍ���
		iIndex0 = piIndex[iStartIndex];
		iIndex1 = piIndex[iStartIndex + 1];
		iIndex2 = piIndex[iStartIndex + 2];

		if (m_dwNumVert < m_dwNumUV)//UV���̂ق��������ꍇ�͖{���́i���_�x�[�X�j�C���f�b�N�X�𗘗p���Ȃ��BUV�C���f�b�N�X����ɂ���
		{
			iIndex0 = pFbxMesh->GetTextureUVIndex(i, 0, FbxLayerElement::eTextureDiffuse);
			iIndex1 = pFbxMesh->GetTextureUVIndex(i, 1, FbxLayerElement::eTextureDiffuse);
			iIndex2 = pFbxMesh->GetTextureUVIndex(i, 2, FbxLayerElement::eTextureDiffuse);
		}

		//���_
		int index = pFbxMesh->GetPolygonVertex(i, 0);
		FbxVector4 Coord = pFbxMesh->GetControlPointAt(index);
		SetIndexOrNormal(&(pvVB[iIndex0].pos), Coord.mData);

		index = pFbxMesh->GetPolygonVertex(i, 1);
		Coord = pFbxMesh->GetControlPointAt(index);
		SetIndexOrNormal(&(pvVB[iIndex1].pos), Coord.mData); 

		index = pFbxMesh->GetPolygonVertex(i, 2);
		Coord = pFbxMesh->GetControlPointAt(index);
		SetIndexOrNormal(&(pvVB[iIndex2].pos), Coord.mData); 

		//�@��		
		pFbxMesh->GetPolygonVertexNormal(i, 0, Normal);
		SetIndexOrNormal(&(pvVB[iIndex0].pos), Normal);

		pFbxMesh->GetPolygonVertexNormal(i, 1, Normal);
		SetIndexOrNormal(&(pvVB[iIndex1].pos), Normal);

		pFbxMesh->GetPolygonVertexNormal(i, 2, Normal);
		SetIndexOrNormal(&(pvVB[iIndex2].pos), Normal);

		//�e�N�X�`���[���W
		int UVindex = pFbxMesh->GetTextureUVIndex(i, 0, FbxLayerElement::eTextureDiffuse);
		FbxLayerElementUV* pUV = 0;

		FbxLayerElementUV* uv = pFbxMesh->GetLayer(0)->GetUVs();

		if (m_dwNumUV && uv->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
		{
			//���̎擾�̎d���́AUV�}�b�s���O���[�h��eBY_POLYGON_VERTEX�̎�����Ȃ��Ƃł��Ȃ�
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

	//�}�e���A���ǂݍ���
	pNode = pFbxMesh->GetNode();
	m_dwNumMaterial = pNode->GetMaterialCount();

	m_material = new MY_MATERIAL[m_dwNumMaterial];

	//�}�e���A���̐������C���f�b�N�X�o�b�t�@�[���쐬
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
		//�t�H�����f����z��
		FbxSurfaceMaterial* pMaterial = pNode->GetMaterial(i);
		FbxSurfacePhong* pPhong = (FbxSurfacePhong*)pMaterial;

		//����
		fbxsdk::FbxDouble3 d3Ambient = pPhong->Ambient;
		auto a = d3Ambient.mData;
		SetMaterial(&m_material[i].Ka, a);

		//�g�U���ˌ�
		fbxsdk::FbxDouble3 d3Diffuse = pPhong->Diffuse;
		auto d = d3Diffuse.mData;
		SetMaterial(&m_material[i].Kd, d);


		//���ʔ��ˌ�
		fbxsdk::FbxDouble3 d3Specular = pPhong->Specular;
		auto s = d3Specular.mData;
		SetMaterial(&m_material[i].Ks, s);

		//�e�N�X�`���[�i�f�B�t���[�Y�e�N�X�`���[�̂݁j
		fbxsdk::FbxProperty lProperty;
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		FbxTexture* texture = FbxCast<FbxTexture>(lProperty.GetSrcObject(0));

		if (texture)
		{
			auto name = texture->GetName();
			strcpy_s(m_material[i].szTextureName, name);

			//�ϊ�������i�[�o�b�t�@
			wchar_t	wStrW[256];
			//���P�[���w��
			setlocale(LC_ALL, "japanese");
			//�ϊ� wchar_t -> char
			errno_t err = mbstowcs_s(nullptr, wStrW, 120, m_material[i].szTextureName, _TRUNCATE);

			if (err != 0)
			{
				MessageBox(0, L"������̕ϊ��Ɏ��s", NULL, MB_OK);
				return E_FAIL;
			}

			//�e�N�X�`���[���쐬
			if (FAILED(graphic->CreateShaderResourceView(wStrW, &m_material[i].pTexture)))
			{
				//�t�@�C����+�g���q�݂̂ɂ���
				char ext[10];
				auto path = m_material[i].szTextureName;

				_splitpath_s(path, nullptr, 0, nullptr, 0, m_material[i].szTextureName, sizeof(m_material[i].szTextureName), ext, sizeof(ext));

				strcat_s(m_material[i].szTextureName, ext);

				//�ϊ�������i�[�o�b�t�@
				wchar_t	wStrW[50];
				//���P�[���w��
				setlocale(LC_ALL, "japanese");
				//�ϊ�
				errno_t err = mbstowcs_s(nullptr, wStrW, 20, m_material[i].szTextureName, _TRUNCATE);

				if (err != 0)
				{
					MessageBox(0, L"�p�X�̕ϊ��Ɏ��s", NULL, MB_OK);
					return E_FAIL;
				}
				if (FAILED(graphic->CreateShaderResourceView(wStrW, &m_material[i].pTexture)))
				{
					MessageBox(0, L"�e�N�X�`���[�ǂݍ��ݎ��s", NULL, MB_OK);
					return E_FAIL;
				}
			}
		}
		//�}�e���A���̐������C���f�b�N�X�o�b�t�@�[���쐬
		int iCount = 0;
		int* pIndex = new int[m_dwNumFace * 3];//�Ƃ肠�����A���b�V�����̃|���S�����Ń������m�ہi�X�̃|���S���O���[�v�͂��Ȃ炸����ȉ��ɂȂ邪�j

		//���̃}�e���A���ł���C���f�b�N�X�z����̊J�n�C���f�b�N�X�𒲂ׂ�@����ɃC���f�b�N�X�̌������ׂ�		
		iCount = 0;
		for (DWORD k = 0; k < m_dwNumFace; k++)
		{
			FbxLayerElementMaterial* mat = pFbxMesh->GetLayer(0)->GetMaterials();//���C���[��1��������z��
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
		m_material[i].dwNumFace = iCount / 3;//���̃}�e���A�����̃|���S����		
		delete[] pIndex;
	}

	//���_����|���S�����t�����������̂ŁA�t�����e�[�u������� 
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

	//�X�L�����i�W���C���g�A�E�F�C�g�@�j�ǂݍ���
	ReadSkinInfo(pFbxMesh, pvVB, PolyTable);

	//�o�[�e�b�N�X�o�b�t�@�[���쐬
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

	//�ꎞ�I�ȓ��ꕨ�́A���͂�s�v
	delete[] pvVB;

	return S_OK;
}

//HRESULT FBX_SKINMESH::CreateIndexBuffer(DWORD dwSize,int* pIndex,ID3D11Buffer** ppIndexBuffer)
//Direct3D�̃C���f�b�N�X�o�b�t�@�[�쐬
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
//�{�[�������̃|�[�Y�ʒu�ɃZ�b�g����
void FbxSkinMesh::SetNewPoseMatrices(int frame)
{
	Matrix m;
	m.m_value[0][0] = -1.0f;

	int i;
	fbxsdk::FbxTime time;
	time.SetTime(0, 0, 0, frame, 0, 0, fbxsdk::FbxTime::eFrames30);//30�t���[��/�b�@�Ɛ���@�����ɂ͏󋵂��Ƃɒ��ׂ�K�v����

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
		m_bone[i].newPose = m * m_bone[i].newPose;//FBX�E����W�n�Ȃ̂ł����t�@�␳����	
	}

	//
	//�t���[����i�߂����Ƃɂ��ω������|�[�Y�i�{�[���̍s��j���V�F�[�_�[�ɓn��
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
//���́i���݂́j�|�[�Y�s���Ԃ�
Matrix FbxSkinMesh::GetCurrentPoseMatrix(int index)
{
	Matrix inv;
	inv = Matrix::Inverse(m_bone[index].bindPose);
	Matrix ret = inv*m_bone[index].newPose;//�o�C���h�|�[�Y�̋t�s��ƃt���[���p���s���������B�Ȃ��A�o�C���h�|�[�Y���̂����ɋt�s��ł���Ƃ���l��������B�iFBX�̏ꍇ�͈Ⴄ���j

	return ret;
}



//�����_�����O
void FbxSkinMesh::Render(const Matrix& world, const Matrix& view, const Matrix& proj,
	const Vec3& vLight, const Vec3& vEye)
{
	auto context = Window::GetInstance()->DeviceContext();


	D3D11_MAPPED_SUBRESOURCE pData;
	if (SUCCEEDED(context->Map(m_constantBuffer0, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		SHADER_GLOBAL0 sg;
		//���C�g�̕�����n��
		sg.lightDir = Vec4(vLight.m_x, vLight.m_y, vLight.m_z, 0.0f);
		//���_�ʒu��n��
		sg.eyePos = Vec4(vEye.m_x, vEye.m_y, vEye.m_z, 0.0f);

		memcpy_s(pData.pData, pData.RowPitch, (void*)&sg, sizeof(SHADER_GLOBAL0));
		context->Unmap(m_constantBuffer0, 0);
	}
	
	//�g�p����V�F�[�_�[�̓o�^	
	context->VSSetShader(m_vertexShader, NULL, 0);
	context->PSSetShader(m_pixelShader, NULL, 0);



	//���̃R���X�^���g�o�b�t�@�[���g���V�F�[�_�[�̓o�^
	context->VSSetConstantBuffers(0, 1, &m_constantBuffer0);
	context->PSSetConstantBuffers(0, 1, &m_constantBuffer0);
	
	//���_�C���v�b�g���C�A�E�g���Z�b�g
	context->IASetInputLayout(m_vertexLayout);

	//�o�[�e�b�N�X�o�b�t�@�[���Z�b�g�i�o�[�e�b�N�X�o�b�t�@�[�͈�ł����j
	UINT stride = sizeof(MY_VERTEX);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	//�}�e���A���̐������A���ꂼ��̃}�e���A���̃C���f�b�N�X�o�b�t�@�|��`��
	for (DWORD i = 0; i < m_dwNumMaterial; i++)
	{
		//�g�p����Ă��Ȃ��}�e���A���΍�
		if (m_material[i].dwNumFace == 0)
		{
			continue;
		}
		//�C���f�b�N�X�o�b�t�@�[���Z�b�g
		stride = sizeof(int);
		offset = 0;
		context->IASetIndexBuffer(m_ppIndexBuffer[i], DXGI_FORMAT_R32_UINT, 0);

		//�v���~�e�B�u�E�g�|���W�[���Z�b�g
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		//�}�e���A���̊e�v�f�ƕϊ��s����V�F�[�_�[�ɓn��
		D3D11_MAPPED_SUBRESOURCE pData;
		if (SUCCEEDED(context->Map(m_constantBuffer1, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
		{
			SHADER_GLOBAL1 sg;
			sg.world=Matrix::Transpose(world);
			sg.wvp = Matrix::Transpose((world * view * proj));
			
			sg.ambient = m_material[i].Ka;//�A���r�G���g�����V�F�[�_�[�ɓn��
			sg.diffuse = m_material[i].Kd;//�f�B�t���[�Y�J���[���V�F�[�_�[�ɓn��
			sg.specular = m_material[i].Ks;//�X�y�L�����[���V�F�[�_�[�ɓn��

			memcpy_s(pData.pData, pData.RowPitch, (void*)&sg, sizeof(SHADER_GLOBAL1));
			context->Unmap(m_constantBuffer1, 0);
		}
	
		context->VSSetConstantBuffers(1, 1, &m_constantBuffer1);
		context->PSSetConstantBuffers(1, 1, &m_constantBuffer1);

		//�e�N�X�`���[���V�F�[�_�[�ɓn��
		if (m_material[i].szTextureName[0] != NULL)
		{
			context->PSSetSamplers(0, 1, &m_sampleLinear);
			context->PSSetShaderResources(0, 1, &m_material[i].pTexture);
		}

		//Draw
		context->DrawIndexed(m_material[i].dwNumFace * 3, 0, 0);
	}
}