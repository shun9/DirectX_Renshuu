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
	//�}�l�[�W���[�̍쐬
	m_manager = fbxsdk::FbxManager::Create();

	//�C���|�[�^�[�̍쐬
	m_importer = FbxImporter::Create(m_manager, "importer");

	//�t�@�C�����J��
	m_importer->Initialize(szFileName);

	//�V�[���̍쐬
	m_pmyScene = FbxScene::Create(m_manager, "scene");

	//�t�@�C������V�[����ǂݍ���
	m_importer->Import(m_pmyScene);

	//�s�v�ɂȂ����C���|�[�^�[��j��
	m_importer->Destroy();
	return S_OK;
}


//�f�ށiFBX)�́A�O�p�|���S���ɂ���
HRESULT FBXMesh::CreateFromFBX(CHAR* FileName)
{
	//�w��̃t�@�C����FBX������
	if (FAILED(InitFBX(FileName)))
	{
		MessageBox(0, L"FBX���[�_�[���������s", NULL, MB_OK);
		return E_FAIL;
	}

	//�V�[���̃��[�g�m�[�h���擾
	FbxNode* rootNode = m_pmyScene->GetRootNode();
	FbxMesh* fbxMesh = 0;
	FbxNodeAttribute::EType type;

	FbxNodeAttribute *pAttr = rootNode->GetNodeAttribute();
	if (pAttr != NULL)
	{
		type = pAttr->GetAttributeType();
		//�^�C�v�����b�V���̏ꍇ�@���ʂ̓��[�g�����b�V�����̂��̂ł��邱�Ƃ͏��Ȃ�
		if (type == FbxNodeAttribute::EType::eMesh)
		{
			fbxMesh = rootNode->GetMesh();
		}
	}
	else//�q�����b�V���̉\������
	{
		fbxMesh = SearchMesh(rootNode);
		if (fbxMesh == nullptr)
		{
			MessageBox(0, L"���b�V���̓ǂݍ��ݎ��s", NULL, MB_OK);
			return E_FAIL;
		}
	}

	//���O�ɒ��_���A�|���S�������𒲂ׂ�
	m_dwNumVert = fbxMesh->GetControlPointsCount();
	m_dwNumUV = fbxMesh->GetTextureUVCount();
	m_dwNumFace = fbxMesh->GetPolygonCount();
	//�ꎞ�I�ȃ������m�ہi���_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@�j
	FBX_VERTEX* pvVB = NULL;
	m_dwNumVert < m_dwNumUV ? pvVB = new FBX_VERTEX[m_dwNumUV] : pvVB = new FBX_VERTEX[m_dwNumVert];
	
	/// ////�|���S�����ƂɁ@���_�ǂݍ��� �A�@���ǂݍ��݁AUV�ǂݍ���
	FbxVector4 Normal;
	for (DWORD i = 0; i < m_dwNumFace; i++)
	{
		int iIndex0 = 0;
		int iIndex1 = 0;
		int iIndex2 = 0;

		int iStartIndex = fbxMesh->GetPolygonVertexIndex(i);
		int* piIndex = fbxMesh->GetPolygonVertices();//�i���_�C���f�b�N�X�j�ǂݍ���
		iIndex0 = piIndex[iStartIndex];
		iIndex1 = piIndex[iStartIndex + 1];
		iIndex2 = piIndex[iStartIndex + 2];
		if (m_dwNumVert < m_dwNumUV)//UV���̂ق��������ꍇ�͖{���́i���_�x�[�X�j�C���f�b�N�X�𗘗p���Ȃ��BUV�C���f�b�N�X����ɂ���
		{
			iIndex0 = fbxMesh->GetTextureUVIndex(i, 0, FbxLayerElement::EType::eTextureDiffuse);
			iIndex1 = fbxMesh->GetTextureUVIndex(i, 1, FbxLayerElement::EType::eTextureDiffuse);
			iIndex2 = fbxMesh->GetTextureUVIndex(i, 2, FbxLayerElement::EType::eTextureDiffuse);
		}
		//���_
		int index = fbxMesh->GetPolygonVertex(i, 0);
		FbxVector4 Coord = fbxMesh->GetControlPointAt(index);
		pvVB[iIndex0].pos.m_x = -Coord.mData[0];//FBX�͉E����W�n�Ȃ̂�x���邢�͂��𔽓]
		pvVB[iIndex0].pos.m_y = Coord.mData[1];
		pvVB[iIndex0].pos.m_z = Coord.mData[2];

		index = fbxMesh->GetPolygonVertex(i, 1);
		Coord = fbxMesh->GetControlPointAt(index);
		pvVB[iIndex1].pos.m_x = -Coord.mData[0];//FBX�͉E����W�n�Ȃ̂�x���邢�͂��𔽓]
		pvVB[iIndex1].pos.m_y = Coord.mData[1];
		pvVB[iIndex1].pos.m_z = Coord.mData[2];

		index = fbxMesh->GetPolygonVertex(i, 2);
		Coord = fbxMesh->GetControlPointAt(index);
		pvVB[iIndex2].pos.m_x = -Coord.mData[0];//FBX�͉E����W�n�Ȃ̂�x���邢�͂��𔽓]
		pvVB[iIndex2].pos.m_y = Coord.mData[1];
		pvVB[iIndex2].pos.m_z = Coord.mData[2];

		//�@��		
		fbxMesh->GetPolygonVertexNormal(i, 0, Normal);
		pvVB[iIndex0].norm.m_x = -Normal[0];//FBX�͉E����W�n�Ȃ̂�x���邢�͂��𔽓]
		pvVB[iIndex0].norm.m_y = Normal[1];
		pvVB[iIndex0].norm.m_z = Normal[2];

		fbxMesh->GetPolygonVertexNormal(i, 1, Normal);
		pvVB[iIndex1].norm.m_x = -Normal[0];//FBX�͉E����W�n�Ȃ̂�x���邢�͂��𔽓]
		pvVB[iIndex1].norm.m_y = Normal[1];
		pvVB[iIndex1].norm.m_z = Normal[2];

		fbxMesh->GetPolygonVertexNormal(i, 2, Normal);
		pvVB[iIndex2].norm.m_x = -Normal[0];//FBX�͉E����W�n�Ȃ̂�x���邢�͂��𔽓]
		pvVB[iIndex2].norm.m_y = Normal[1];
		pvVB[iIndex2].norm.m_z = Normal[2];

		//�e�N�X�`���[���W
		int UVindex = fbxMesh->GetTextureUVIndex(i, 0, FbxLayerElement::EType::eTextureDiffuse);
		FbxLayerElementUV* pUV = 0;

		FbxLayerElementUV* uv = fbxMesh->GetLayer(0)->GetUVs();

		if (m_dwNumUV && uv->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
		{
			//���̎擾�̎d���́AUV�}�b�s���O���[�h��eBY_POLYGON_VERTEX�̎�����Ȃ��Ƃł��Ȃ�
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

	//�}�e���A���ǂݍ���
	FbxNode* pNode = fbxMesh->GetNode();
	m_dwNumMaterial = pNode->GetMaterialCount();

	m_material = new FBX_MATERIAL[m_dwNumMaterial];

	//�}�e���A���̐������C���f�b�N�X�o�b�t�@�[���쐬
	m_indexBuffer = new ID3D11Buffer*[m_dwNumMaterial];

	for (DWORD i = 0; i < m_dwNumMaterial; i++)
	{
		//�t�H�����f����z��
		FbxSurfaceMaterial* pMaterial = pNode->GetMaterial(i);
		FbxSurfacePhong* pPhong = (FbxSurfacePhong*)pMaterial;

		//����
		FbxDouble3 d3Ambient = pPhong->Ambient;
		m_material[i].Ka.m_x = (float)d3Ambient.mData[0];
		m_material[i].Ka.m_y = (float)d3Ambient.mData[1];
		m_material[i].Ka.m_z = (float)d3Ambient.mData[2];
		//�g�U���ˌ�
		FbxDouble3 d3Diffuse = pPhong->Diffuse;
		auto d = d3Diffuse.mData;
		m_material[i].Kd.m_x = (float)d[0];
		m_material[i].Kd.m_y = (float)d[1];
		m_material[i].Kd.m_z = (float)d[2];
		//���ʔ��ˌ�
		FbxDouble3 d3Specular = pPhong->Specular;
		auto s = d3Specular.mData;
		m_material[i].Ks.m_x = (float)s[0];
		m_material[i].Ks.m_y = (float)s[1];
		m_material[i].Ks.m_z = (float)s[2];

		//�e�N�X�`���[�i�f�B�t���[�Y�e�N�X�`���[�̂݁j
		FbxProperty lProperty;
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
			auto graphics = ShunLib::Graphics::GetInstance();
			if (FAILED(graphics->CreateShaderResourceView(wStrW, &m_material[i].pTexture)))
			{

				//�t�@�C����+�g���q�݂̂ɂ���
				char ext[10];
				auto path = m_material[i].szTextureName;

				_splitpath_s(path, nullptr, 0, nullptr, 0, m_material[i].szTextureName,sizeof(m_material[i].szTextureName),ext,sizeof(ext));

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

				//�t�@�C�����݂̂Ńg���C
				if (FAILED(graphics->CreateShaderResourceView(wStrW, &m_material[i].pTexture)))
				{
					MessageBox(0, L"�e�N�X�`���[�ǂݍ��ݎ��s", NULL, MB_OK);
					return E_FAIL;
				}
			}
		}

		//�}�e���A���̐������C���f�b�N�X�o�b�t�@�[���쐬
		int iCount = 0;
		int* index = new int[m_dwNumFace * 3];//���b�V�����̃|���S�����Ń������m�ہi�X�̃|���S���O���[�v�͂��Ȃ炸����ȉ��ɂȂ�j

		//���̃}�e���A���ł���C���f�b�N�X�z����̊J�n�C���f�b�N�X�𒲂ׂ�@����ɃC���f�b�N�X�̌������ׂ�		
		iCount = 0;
		for (DWORD k = 0; k < m_dwNumFace; k++)
		{
			FbxLayerElementMaterial* mat = fbxMesh->GetLayer(0)->GetMaterials();//���C���[��1��������z��
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
		m_material[i].dwNumFace = iCount / 3;//���̃}�e���A�����̃|���S����

		delete[] index;
	}

	auto device = ShunLib::Window::GetInstance()->Device();

	//�o�[�e�b�N�X�o�b�t�@�[���쐬
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

	//�ꎞ�I�ȓ��ꕨ�́A���͂�s�v
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
/// ���b�V���̕`��
/// </summary>
void FBXMesh::RenderMesh(const ShunLib::Matrix& world, const ShunLib::Matrix& view, const ShunLib::Matrix& proj)
{
	auto context = ShunLib::Window::GetInstance()->DeviceContext();
 
	//�o�[�e�b�N�X�o�b�t�@�[���Z�b�g�i�o�[�e�b�N�X�o�b�t�@�[�͈�j
	UINT stride = sizeof(FBX_VERTEX);
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
		context->IASetIndexBuffer(m_indexBuffer[i], DXGI_FORMAT_R32_UINT, 0);

		//�v���~�e�B�u�E�g�|���W�[���Z�b�g
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		//�V�F�[�_�[�ɏ���n��
		D3D11_MAPPED_SUBRESOURCE pData;
		if (SUCCEEDED(context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
		{
			SHADER_GLOBAL1 sg;
			sg.world = ShunLib::Matrix::Transpose(world);             //���[���h�s����V�F�[�_�[�ɓn��
			sg.wvp = ShunLib::Matrix::Transpose((sg.world*view*proj));//�s����V�F�[�_�[�ɓn���@���[���h�~�r���[�~�v���W�F�N�V�����@�s��
			sg.ambient = m_material[i].Ka;                            //�A���r�G���g���V�F�[�_�[�ɓn��
			sg.diffuse = m_material[i].Kd;                            //�f�B�t���[�Y�J���[���V�F�[�_�[�ɓn��
			sg.specular = m_material[i].Ks;                           //�X�y�L�����[���V�F�[�_�[�ɓn��

			sg.diffuse = ShunLib::Vec4(1.0f,1.0f,1.0f,1.0f);                            //�f�B�t���[�Y�J���[���V�F�[�_�[�ɓn��

			memcpy_s(pData.pData, pData.RowPitch, (void*)&sg, sizeof(SHADER_GLOBAL1));
			context->Unmap(m_constantBuffer, 0);
		}

		//�V�F�[�_�[�Ƀo�b�t�@�[���Z�b�g
		context->VSSetConstantBuffers(1, 1, &m_constantBuffer);
		context->PSSetConstantBuffers(1, 1, &m_constantBuffer);

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

/// <summary>
/// ���b�V���m�[�h��T��
/// </summary>
FbxMesh* FBXMesh::SearchMesh(FbxNode* rootNode)
{
	//�q�̐�����
	int iNumChild = rootNode->GetChildCount();
	FbxNodeAttribute* attr;
	FbxNodeAttribute::EType type;
	for (int i = 0; i < iNumChild; i++)
	{
		FbxNode* childNode = rootNode->GetChild(i);
		if (!childNode) continue;
		attr = childNode->GetNodeAttribute();
		type = attr->GetAttributeType();

		//�^�C�v�����b�V���̏ꍇ
		if (type == FbxNodeAttribute::EType::eMesh)
		{
			auto mesh = childNode->GetMesh();
			return mesh;
		}
	}

	return nullptr;
}
