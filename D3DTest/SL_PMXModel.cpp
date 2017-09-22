#include "SL_PMXModel.h"
#include <Windows.h>
#include <locale.h>
#include <SL_MacroConstants.h>

using namespace std;
using namespace ShunLib;
using namespace ShunLib::PMX;

ShunLib::PMX::PMXModel::~PMXModel()
{
	//モーフのオフセットを削除
	//MakeMorphOffsetにて生成
	for (int i = 0; i < m_morph.count; i++)
	{
		SAFE_DELETE_ARRAY(m_morph.info[i].morphOffsetList);
	}
}

/// <summary>
/// PMXモデルを読み込む
/// </summary>
/// <param name="fileName">ファイルのパス</param>
/// <returns>成功したらtrue</returns>
bool PMXModel::LoadModel(char* fileName)
{
	//ファイルを開く
	FILE* file;
	auto hFP = fopen_s(&file, fileName,"rb");

	//ファイルのオープンに失敗
	if (hFP != 0)return false;
	
	//データ読み込み
	bool result = Load(file);
	
	//ファイルを閉じる
	fclose(file);

	return result;
}

/// <summary>
/// PMXのデータを読み込む
/// </summary>
bool PMXModel::Load(FILE * file)
{
	//pmxの読み込み
	if (!(LoadHeader		(file))) { return false; }
	if (!(LoadModelInfo		(file))) { return false; }
	if (!(LoadVertex		(file))) { return false; }
	if (!(LoadFace			(file))) { return false; }
	if (!(LoadTexture		(file))) { return false; }
	if (!(LoadMaterial		(file))) { return false; }
	if (!(LoadBone			(file))) { return false; }
	if (!(LoadMorph			(file))) { return false; }
	if (!(LoadDisplayFrame	(file))) { return false; }
	if (!(LoadRigidBody		(file))) { return false; }
	if (!(LoadJoint			(file))) { return false; }
	if (!(LoadSoftBody		(file))) { return false; }
	return true;
}

/// <summary>
/// PMXのヘッダー部分を読み込む
/// </summary>
/// <param name="file">オープンしたファイルポインタ</param>
/// <returns>成功でtrue</returns>
bool PMXModel::LoadHeader(FILE* file)
{
	if (file == NULL)
	{
		return false;
	}
	//マジックナンバー読み込み
	for (int i = 0; i < 4; i++)
	{
		fread_s(&m_header.magicNum[i], sizeof(PMXByte), sizeof(PMXByte), 1, file);
	}
	m_header.magicNum[3] = '\0';

	//バージョン読み込み
	fread_s(&m_header.version, sizeof(float), sizeof(float), 1, file);

	//後続のデータ列のバイトサイズ
	fread_s(&m_header.dataSize, sizeof(PMXByte), sizeof(PMXByte), 1, file);

	//データ列読み込み
	m_header.data.resize(m_header.dataSize);
	for (int i = 0; i < m_header.dataSize; i++)
	{
		fread_s(&m_header.data[i], sizeof(PMXByte), sizeof(PMXByte), 1, file);
	}

	return true;
}

/// <summary>
/// PMXのモデル情報を読み込む
/// </summary>
/// <param name="file">オープンしたファイルポインタ</param>
/// <returns>成功でtrue</returns>
bool PMXModel::LoadModelInfo(FILE * file)
{
	if (file == NULL)
	{
		return false;
	}

	// モデル名
	ReadName(&m_modelInfo.name, file);

	// モデル名（英語）
	ReadName(&m_modelInfo.nameE, file);


	// コメント
	ReadName(&m_modelInfo.comment, file);
	
	// コメント（英語）
	ReadName(&m_modelInfo.commentE, file);

	return true;
}

/// <summary>
/// PMXの頂点情報を読み込む
/// </summary>
/// <param name="file">オープンしたファイルポインタ</param>
/// <returns>成功でtrue</returns>
bool PMXModel::LoadVertex(FILE * file)
{
	if (file == NULL)
	{
		return false;
	}

	// 頂点数
	fread_s(&m_vertex.count, sizeof(int), sizeof(int), 1, file);

	m_vertex.info.resize(m_vertex.count);
	for (int i = 0; i < m_vertex.count; i++)
	{
		//位置　法線　UV読み込み
		fread_s(&m_vertex.info[i].pos, sizeof(Vec3), sizeof(Vec3), 1, file);
		fread_s(&m_vertex.info[i].normal, sizeof(Vec3), sizeof(Vec3), 1, file);
		fread_s(&m_vertex.info[i].uv, sizeof(Vec2), sizeof(Vec2), 1, file);
	
		//追加UV読み込み
		m_vertex.info[i].addUv.resize(m_header.data[HEADER_DATA_2_0::ADD_UV]);
		for (int j = 0; j < (int)(m_vertex.info[i].addUv.size()); j++)
		{
			fread_s(&m_vertex.info[i].addUv[i], sizeof(Vec4), sizeof(Vec4), 1, file);
		}

		//ウェイト変形方式読み込み
		fread_s(&m_vertex.info[i].weightType, sizeof(PMXByte), sizeof(PMXByte), 1, file);

		//ウェイトの読み込み
		if (!ReadBoneWeight(&m_vertex.info[i].boneWeight, m_vertex.info[i].weightType, file))
			return false;
		
		//エッジ倍率
		fread_s(&m_vertex.info[i].weightType, sizeof(float), sizeof(float), 1, file);
	}
	return true;
}

/// <summary>
/// PMXの面情報を読み込む
/// </summary>
/// <param name="file">オープンしたファイルポインタ</param>
/// <returns>成功でtrue</returns>
bool PMXModel::LoadFace(FILE * file)
{
	if (file == NULL)
	{
		return false;
	}
	// 面数
	fread_s(&m_face.count, sizeof(int), sizeof(int), 1, file);
	m_face.info.resize(m_face.count);

	//頂点の参照Index
	PMXByte size = m_header.data[HEADER_DATA_2_0::BONE_INDEX];
	for (int i = 0; i < m_face.count; i++)
	{
		fread_s(&m_face.info[i].a, sizeof(int), size, 1, file);
	}
	return true;
}

/// <summary>
/// PMXのテクスチャ情報を読み込む
/// </summary>
/// <param name="file">オープンしたファイルポインタ</param>
/// <returns>成功でtrue</returns>
bool PMXModel::LoadTexture(FILE * file)
{
	if (file == NULL)
	{
		return false;
	}

	// テクスチャ数
	fread_s(&m_texture.count, sizeof(int), sizeof(int), 1, file);
	m_texture.fileName.resize(m_texture.count);

	//テクスチャパス読み込み
	for (int i = 0; i < m_texture.count; i++)
	{
		ReadName(&m_texture.fileName[i], file);
	}

	return true;
}

/// <summary>
/// PMXのマテリアル情報を読み込む
/// </summary>
/// <param name="file">オープンしたファイルポインタ</param>
/// <returns>成功でtrue</returns>
bool PMXModel::LoadMaterial(FILE * file)
{
	if (file == NULL)
	{
		return false;
	}

	// マテリアル数
	fread_s(&m_material.count, sizeof(int), sizeof(int), 1, file);
	m_material.info.resize(m_material.count);

	//マテリアル情報読み込み
	for (int i = 0; i < m_material.count; i++)
	{
		//マテリアル名
		ReadName(&m_material.info[i].materialName, file);
	
		//マテリアル名(英語)
		ReadName(&m_material.info[i].materialNameE, file);
	
		//拡散色　反射色　反射色係数　環境色　読み込み
		fread_s(&m_material.info[i].diffuse, sizeof(Vec4), sizeof(Vec4), 1, file);
		fread_s(&m_material.info[i].specular, sizeof(Vec3), sizeof(Vec3), 1, file);
		fread_s(&m_material.info[i].shininess, sizeof(float), sizeof(float), 1, file);
		fread_s(&m_material.info[i].ambient, sizeof(Vec3), sizeof(Vec3), 1, file);

		//描画フラグ
		fread_s(&m_material.info[i].renderFlag, sizeof(PMXByte), sizeof(PMXByte), 1, file);
		
		//エッジ色　エッジサイズ　読み込み
		fread_s(&m_material.info[i].edgeColor, sizeof(Vec4), sizeof(Vec4), 1, file);
		fread_s(&m_material.info[i].edgeSize, sizeof(float), sizeof(float), 1, file);

		//通常テクスチャ　スフィアテクスチャ　読み込み
		PMXByte textureIndexSize = m_header.data[HEADER_DATA_2_0::TEXTURE_INDEX];
		fread_s(&m_material.info[i].textureTableReferenceIndex, textureIndexSize, textureIndexSize, 1, file);
		fread_s(&m_material.info[i].sphereTextureTableReferenceIndex, textureIndexSize, textureIndexSize, 1, file);

		//スフィアモード 
		fread_s(&m_material.info[i].sphereMode, sizeof(PMXByte), sizeof(PMXByte), 1, file);
		
		// 共有Toonフラグ  0:継続値は個別Toon  1:継続値は共有Toon
		fread_s(&m_material.info[i].shareToonFlag, sizeof(PMXByte), sizeof(PMXByte), 1, file);
		
		//テクスチャIndex
		if (m_material.info[i].shareToonFlag == 0){
			fread_s(&m_material.info[i].textureIndex, textureIndexSize, textureIndexSize, 1, file);
		}
		else if (m_material.info[i].shareToonFlag == 1){
			fread_s(&m_material.info[i].textureIndex, sizeof(PMXByte), sizeof(PMXByte), 1, file);
		}

		//メモ
		fread_s(&m_material.info[i].memo.byteSize, sizeof(int), sizeof(int), 1, file);
		ReadText(&m_material.info[i].memo.str, m_material.info[i].memo.byteSize, m_header.data[HEADER_DATA_2_0::ENCODE_TYPE], file);
		
		// 材質に対応する面(頂点)数 (必ず3の倍数になる)
		fread_s(&m_material.info[i].faceVertexCount, sizeof(int), sizeof(int), 1, file);
	}

	return true;
}


/// <summary>
/// PMXのボーン情報を読み込む
/// </summary>
/// <param name="file">オープンしたファイルポインタ</param>
/// <returns>成功でtrue</returns>
bool PMXModel::LoadBone(FILE * file)
{
	if (file == NULL)
	{
		return false;
	}
	PMXByte boneIndexSize = m_header.data[HEADER_DATA_2_0::BONE_INDEX];

	// ボーン数
	fread_s(&m_bone.count, sizeof(int), sizeof(int), 1, file);
	m_bone.info.resize(m_bone.count);

	//ボーン情報読み込み
	for (int i = 0; i < m_bone.count; i++)
	{
		//ボーン名
		ReadName(&m_bone.info[i].boneName, file);

		//ボーン名(英語)
		ReadName(&m_bone.info[i].boneNameE, file);

		//位置
		fread_s(&m_bone.info[i].position, sizeof(Vec3), sizeof(Vec3), 1, file);

		//親ボーンのIndex
		fread_s(&m_bone.info[i].parentBoneIndex, boneIndexSize, boneIndexSize, 1, file);

		//変形階層
		fread_s(&m_bone.info[i].deformationLevel, sizeof(int), sizeof(int), 1, file);

		//ボーンフラグ(16bit) 各bit  0:OFF  1:ON
		fread_s(&m_bone.info[i].boneFlag, sizeof(unsigned short), sizeof(unsigned short), 1, file);

		ReadBoneFlag(&m_bone.info[i], m_bone.info[i].boneFlag, file);
	}
	return true;
}


/// <summary>
/// PMXのモーフ情報を読み込む
/// </summary>
/// <param name="file">オープンしたファイルポインタ</param>
/// <returns>成功でtrue</returns>
bool PMXModel::LoadMorph(FILE * file)
{
	if (file == NULL)
	{
		return false;
	}
	PMXByte morphIndexSize = m_header.data[HEADER_DATA_2_0::MORPH_INDEX];

	// モーフ数
	fread_s(&m_morph.count, sizeof(int), sizeof(int), 1, file);
	m_morph.info.resize(m_morph.count);

	//モーフ情報読み込み
	for (int i = 0; i < m_morph.count; i++)
	{
		//モーフ名　モーフ名(英語)
		ReadName(&m_morph.info[i].morphName, file);
		ReadName(&m_morph.info[i].morphNameE, file);
	
		//操作パネル
		fread_s(&m_morph.info[i].operationPanel, sizeof(PMXByte), sizeof(PMXByte), 1, file);

		//モーフ種類
		fread_s(&m_morph.info[i].type, sizeof(PMXByte), sizeof(PMXByte), 1, file);

		//モーフのオフセット数
		fread_s(&m_morph.info[i].morphOffsetCount, sizeof(int), sizeof(int), 1, file);
		
		//モーフのオフセットを作成
		MakeMorphOffset(&m_morph.info[i], m_morph.info[i].type, m_morph.info[i].morphOffsetCount, file);
	}
	return true;
}

/// <summary>
/// PMXの表示枠情報を読み込む
/// </summary>
/// <param name="file">オープンしたファイルポインタ</param>
/// <returns>成功でtrue</returns>
bool PMXModel::LoadDisplayFrame(FILE * file)
{
	if (file == NULL)
	{
		return false;
	}

	// 表示枠数
	fread_s(&m_displayframe.count, sizeof(int), sizeof(int), 1, file);
	m_displayframe.info.resize(m_displayframe.count);

	//表示枠情報読み込み
	for (int i = 0; i < m_displayframe.count; i++)
	{
		//表示枠名　表示枠名(英語)
		ReadName(&m_displayframe.info[i].frameName, file);
		ReadName(&m_displayframe.info[i].frameNameE, file);

		//特殊枠フラグ
		fread_s(&m_displayframe.info[i].isSpecialFrame, sizeof(PMXByte), sizeof(PMXByte), 1, file);

		//枠内要素数
		fread_s(&m_displayframe.info[i].elementCount, sizeof(int), sizeof(int), 1, file);
		m_displayframe.info[i].frameElementList.resize(m_displayframe.info[i].elementCount);

		//枠内要素
		for (int j = 0; j < m_displayframe.info[i].elementCount; j++)
		{
			//要素対象
			fread_s(&m_displayframe.info[i].frameElementList[j].isMorph, sizeof(PMXByte), sizeof(PMXByte), 1, file);
			if (m_displayframe.info[i].frameElementList[j].isMorph == FLAME_TARGET::TERGET_MORPH){
				//モーフ
				PMXByte morphIndexSize = m_header.data[HEADER_DATA_2_0::MORPH_INDEX];
				fread_s(&m_displayframe.info[i].frameElementList[j].index, morphIndexSize, morphIndexSize, 1, file);
			}
			else {
				//ボーン
				PMXByte boneIndexSize = m_header.data[HEADER_DATA_2_0::BONE_INDEX];
				fread_s(&m_displayframe.info[i].frameElementList[j].index, boneIndexSize, boneIndexSize, 1, file);
			}
		}
	}
	return true;
}

/// <summary>
/// PMXの剛体情報を読み込む
/// </summary>
/// <param name="file">オープンしたファイルポインタ</param>
/// <returns>成功でtrue</returns>
bool PMXModel::LoadRigidBody(FILE * file)
{
	if (file == NULL)
	{
		return false;
	}
	// 剛体数
	fread_s(&m_rigidBody.count, sizeof(int), sizeof(int), 1, file);
	m_rigidBody.info.resize(m_rigidBody.count);

	//表示枠情報読み込み
	for (int i = 0; i < m_rigidBody.count; i++)
	{
		//表示枠名　表示枠名(英語)
		ReadName(&m_rigidBody.info[i].rigidBodyName, file);
		ReadName(&m_rigidBody.info[i].rigidBodyNameE, file);
	}
	return true;
}

/// <summary>
/// PMXのジョイント情報を読み込む
/// </summary>
/// <param name="file">オープンしたファイルポインタ</param>
/// <returns>成功でtrue</returns>
bool PMXModel::LoadJoint(FILE * file)
{
	if (file == NULL)
	{
		return false;
	}
	return true;
}

/// <summary>
/// PMXの軟体情報を読み込む
/// </summary>
/// <param name="file">オープンしたファイルポインタ</param>
/// <returns>成功でtrue</returns>
bool PMXModel::LoadSoftBody(FILE * file)
{
	if (file == NULL)
	{
		return false;
	}
	return true;
}


/// <summary>
/// 名前を読み込む
/// </summary>
/// <param name="textBuf">テキストを入れるバッファー</param>
/// <param name="file">読み込むファイル</param>
bool PMXModel::ReadName(PMXTextBuf * textBuf, FILE * file)
{
	fread_s(&textBuf->byteSize, sizeof(int), sizeof(int), 1, file);
	ReadText(&textBuf->str, textBuf->byteSize, m_header.data[HEADER_DATA_2_0::ENCODE_TYPE], file);

	return false;
}

/// <summary>
/// テキストを読み込む
/// </summary>
/// <param name="buf">テキストを入れるバッファー</param>
/// <param name="byte">読み込むバイト数</param>
/// <param name="encode">エンコードタイプ</param>
/// <param name="file">読み込むファイル</param>
bool PMXModel::ReadText(std::string* buf, int byte, PMXByte encode, FILE * file)
{
	if (encode == PMX_UTF16)
	{
		int size = byte / 2;
		wchar_t* strBuf = new wchar_t[size+1];

		for (int i = 0; i < size; i++)
		{
			fread_s(&strBuf[i], sizeof(wchar_t), sizeof(wchar_t), 1, file);
		}
		strBuf[size] = '\0';

		setlocale(LC_ALL, "japanese");

		size_t tmp;
		char* tmpStr = new char[byte + 1];
		wcstombs_s(&tmp, tmpStr, byte +1, strBuf, byte + 1);

		*buf = (tmpStr);

		delete[] tmpStr;
		delete[] strBuf;
	}
	return true;
}


/// <summary>
/// ボーンウェイトを読み込む
/// </summary>
/// <param name="buf">情報を入れるバッファー</param>
/// <param name="type">ウェイトタイプ</param>
/// <param name="file">読み込むファイル</param>
bool PMXModel::ReadBoneWeight(PMXBoneWeight* buf, PMXByte type, FILE * file)
{
	PMXByte size = m_header.data[HEADER_DATA_2_0::BONE_INDEX];
	switch (type)
	{
	case ShunLib::PMX::BDEF_1:
		fread_s(&buf->bone1ReferenceIndex, sizeof(int), size, 1, file);
		break;

	case ShunLib::PMX::BDEF_2:
		fread_s(&buf->bone1ReferenceIndex, sizeof(int), size, 1, file);
		fread_s(&buf->bone2ReferenceIndex, sizeof(int), size, 1, file);
		fread_s(&buf->bone1Weight, sizeof(float), sizeof(float), 1, file);
		break;

	case ShunLib::PMX::BDEF_4:
		fread_s(&buf->bone1ReferenceIndex, sizeof(int), size, 1, file);
		fread_s(&buf->bone2ReferenceIndex, sizeof(int), size, 1, file);
		fread_s(&buf->bone3ReferenceIndex, sizeof(int), size, 1, file);
		fread_s(&buf->bone4ReferenceIndex, sizeof(int), size, 1, file);
		fread_s(&buf->bone1Weight, sizeof(float), sizeof(float), 1, file);
		fread_s(&buf->bone2Weight, sizeof(float), sizeof(float), 1, file);
		fread_s(&buf->bone3Weight, sizeof(float), sizeof(float), 1, file);
		fread_s(&buf->bone4Weight, sizeof(float), sizeof(float), 1, file);
		break;

	case ShunLib::PMX::SDEF:
		fread_s(&buf->bone1ReferenceIndex, sizeof(int), size, 1, file);
		fread_s(&buf->bone2ReferenceIndex, sizeof(int), size, 1, file);
		fread_s(&buf->bone1Weight, sizeof(float), sizeof(float), 1, file);
		fread_s(&buf->SDEF_C, sizeof(Vec3), sizeof(Vec3), 1, file);
		fread_s(&buf->SDEF_R0, sizeof(Vec3), sizeof(Vec3), 1, file);
		fread_s(&buf->SDEF_R1, sizeof(Vec3), sizeof(Vec3), 1, file);
		break;

	default:
		return false;
		break;
	}
	return true;
}

/// <summary>
/// ボーンフラグをもとに情報を読み込む
/// </summary>
/// <param name="buf">情報を入れるバッファー</param>
/// <param name="flag">ボーンフラグ</param>
/// <param name="file">読み込むファイル</param>
bool PMXModel::ReadBoneFlag(PmxBoneInfo * buf, unsigned short flag, FILE * file)
{
	PMXByte boneIndexSize = m_header.data[HEADER_DATA_2_0::BONE_INDEX];

	//接続先
	if (!(flag & BONE_FLAG::CONNECTED)){
		fread_s(&buf->positionOffset, sizeof(Vec3), sizeof(Vec3), 1, file);		
	}
	else {
		fread_s(&buf->connectedBoneIndex, boneIndexSize, boneIndexSize, 1, file);
	}

	//回転付与:1 または 移動付与:1
	if ((flag & BONE_FLAG::MOVEMENT_GRANT) || (flag & BONE_FLAG::ROTATION_GRANT))
	{
		fread_s(&buf->providedParentBoneIndex, boneIndexSize, boneIndexSize, 1, file);
		fread_s(&buf->providedRatio, sizeof(float), sizeof(float), 1, file);
	}

	//固定軸
	if ((flag & BONE_FLAG::LOCK_AXIS))
	{
		fread_s(&buf->axisDirectionVector, sizeof(Vec3), sizeof(Vec3), 1, file);
	}

	//ローカル軸
	if ((flag & BONE_FLAG::LOCAL_AXIS))
	{
		fread_s(&buf->dimentionXDirectionVector, sizeof(Vec3), sizeof(Vec3), 1, file);
		fread_s(&buf->dimentionZDirectionVector, sizeof(Vec3), sizeof(Vec3), 1, file);
	}

	//外部親変形
	if ((flag & BONE_FLAG::EXTERNAL_PARENT_TRANSFORMATION))
	{
		fread_s(&buf->keyValue, sizeof(int), sizeof(int), 1, file);
	}


	//IK
	if ((flag & BONE_FLAG::IK))
	{
		fread_s(&buf->ikTargetBoneIndex, boneIndexSize, boneIndexSize, 1, file);
		fread_s(&buf->ikLoopCount, sizeof(int), sizeof(int), 1, file);
		fread_s(&buf->ikLimitedRadian, sizeof(float), sizeof(float), 1, file);
		fread_s(&buf->ikLinkCount, sizeof(int), sizeof(int), 1, file);
	
		buf->ikLinkList.resize(buf->ikLinkCount);
	
		//IKリンク
		for (int i = 0; i < buf->ikLinkCount; i++)
		{
			fread_s(&buf->ikLinkList[i].linkBoneIndex, boneIndexSize, boneIndexSize, 1, file);
			fread_s(&buf->ikLinkList[i].isRotateLimited, sizeof(PMXByte), sizeof(PMXByte), 1, file);
			if (buf->ikLinkList[i].isRotateLimited == 1)
			{
				fread_s(&buf->ikLinkList[i].minimumRadian, sizeof(Vec3), sizeof(Vec3), 1, file);
				fread_s(&buf->ikLinkList[i].maximumRadian, sizeof(Vec3), sizeof(Vec3), 1, file);
			}
		}
	}

	return true;
}



/// <summary>
/// モーフのオフセットに情報を読み込む
/// </summary>
/// <param name="buf">情報を入れるバッファー</param>
/// <param name="count">作成数</param>
/// <param name="file">読み込むファイル</param>
bool PMXModel::ReadGroupMorphOffset(GroupMorphOffset* buf,int count, FILE * file)
{
	PMXByte morphIndexSize = m_header.data[HEADER_DATA_2_0::MORPH_INDEX];
	for (int i = 0; i < count; i++)
	{
		fread_s(&buf[i].morphIndex, morphIndexSize, morphIndexSize, 1, file);
		fread_s(&buf[i].morphRatio, sizeof(float), sizeof(float), 1, file);
	}

	return true;
}

/// <summary>
/// モーフのオフセットに情報を読み込む
/// </summary>
/// <param name="buf">情報を入れるバッファー</param>
/// <param name="count">作成数</param>
/// <param name="file">読み込むファイル</param>
bool ShunLib::PMX::PMXModel::ReadVertexMorphOffset(VertexMorphOffset * buf, int count, FILE * file)
{
	PMXByte vertexIndexSize = m_header.data[HEADER_DATA_2_0::VERTEX_INDEX];
	for (int i = 0; i < count; i++)
	{
		fread_s(&buf[i].vertexIndex, vertexIndexSize, vertexIndexSize, 1, file);
		fread_s(&buf[i].positionOffset, sizeof(Vec3), sizeof(Vec3), 1, file);
	}
	return true;
}

/// <summary>
/// モーフのオフセットに情報を読み込む
/// </summary>
/// <param name="buf">情報を入れるバッファー</param>
/// <param name="count">作成数</param>
/// <param name="file">読み込むファイル</param>
bool ShunLib::PMX::PMXModel::ReadBoneMorphOffset(BoneMoptOffset * buf, int count, FILE * file)
{
	PMXByte boneIndexSize = m_header.data[HEADER_DATA_2_0::BONE_INDEX];
	for (int i = 0; i < count; i++)
	{
		fread_s(&buf[i].boneIndex, boneIndexSize, boneIndexSize, 1, file);
		fread_s(&buf[i].quantityOfMoving, sizeof(Vec3), sizeof(Vec3), 1, file);
		fread_s(&buf[i].quantityOfRotating, sizeof(Vec4), sizeof(Vec4), 1, file);
	}
	return true;
}

/// <summary>
/// モーフのオフセットに情報を読み込む
/// </summary>
/// <param name="buf">情報を入れるバッファー</param>
/// <param name="count">作成数</param>
/// <param name="file">読み込むファイル</param>
bool ShunLib::PMX::PMXModel::ReadUVMorphOffset(UVMorphOffset * buf, int count, FILE * file)
{
	PMXByte vertexIndexSize = m_header.data[HEADER_DATA_2_0::VERTEX_INDEX];
	for (int i = 0; i < count; i++)
	{
		fread_s(&buf[i].vertexIndex, vertexIndexSize, vertexIndexSize, 1, file);
		fread_s(&buf[i].uvOffset, sizeof(Vec4), sizeof(Vec4), 1, file);
	}
	return true;
}

/// <summary>
/// モーフのオフセットに情報を読み込む
/// </summary>
/// <param name="buf">情報を入れるバッファー</param>
/// <param name="count">作成数</param>
/// <param name="file">読み込むファイル</param>
bool ShunLib::PMX::PMXModel::ReadMaterialMorphOffset(MaterialMorphOffset * buf, int count, FILE * file)
{
	PMXByte materialIndexSize = m_header.data[HEADER_DATA_2_0::MATERIAL_INDEX];
	for (int i = 0; i < count; i++)
	{
		fread_s(&buf[i].materialIndex, materialIndexSize, materialIndexSize, 1, file);
		fread_s(&buf[i].offsetCalclationType, sizeof(PMXByte), sizeof(PMXByte), 1, file);
		fread_s(&buf[i].diffuse, sizeof(Vec4), sizeof(Vec4), 1, file);
		fread_s(&buf[i].specular, sizeof(Vec3), sizeof(Vec3), 1, file);
		fread_s(&buf[i].specularCoefficient, sizeof(float), sizeof(float), 1, file);
		fread_s(&buf[i].ambient, sizeof(Vec3), sizeof(Vec3), 1, file);
		fread_s(&buf[i].edgeColor, sizeof(Vec4), sizeof(Vec4), 1, file);
		fread_s(&buf[i].edgeSize, sizeof(float), sizeof(float), 1, file);
		fread_s(&buf[i].textureCoefficient, sizeof(Vec4), sizeof(Vec4), 1, file);
		fread_s(&buf[i].sphereTextureCoefficient, sizeof(Vec4), sizeof(Vec4), 1, file);
		fread_s(&buf[i].toonTextureCoefficient, sizeof(Vec4), sizeof(Vec4), 1, file);
	}
	return true;
}


/// <summary>
/// モーフのオフセットを作成
/// ※動的に確保するためdeleteが必要
/// </summary>
/// <param name="buf">情報を入れるバッファー</param>
/// <param name="type">モーフの種類</param>
/// <param name="count">作成数</param>
/// <param name="file">読み込むファイル</param>
bool PMXModel::MakeMorphOffset(PMXMorphInfo* buf, PMXByte type, int count, FILE * file)
{
	switch (type)
	{
	case MOTPH_TYPE::GROUP:
		buf->morphOffsetList = new GroupMorphOffset[count];
		ReadGroupMorphOffset(static_cast<GroupMorphOffset*>(buf->morphOffsetList), count, file);
		break;

	case MOTPH_TYPE::VERTEX:
		buf->morphOffsetList = new VertexMorphOffset[count];
		ReadVertexMorphOffset(static_cast<VertexMorphOffset*>(buf->morphOffsetList), count, file);
		break;

	case MOTPH_TYPE::BONE:
		buf->morphOffsetList = new BoneMoptOffset[count];
		ReadBoneMorphOffset(static_cast<BoneMoptOffset*>(buf->morphOffsetList), count, file);
		break;

	case MOTPH_TYPE::UV:
		buf->morphOffsetList = new UVMorphOffset[count];
		ReadUVMorphOffset(static_cast<UVMorphOffset*>(buf->morphOffsetList), count, file);
		break;

	case MOTPH_TYPE::ADD_UV1:
		buf->morphOffsetList = new UVMorphOffset[count];
		ReadUVMorphOffset(static_cast<UVMorphOffset*>(buf->morphOffsetList), count, file);
		break;

	case MOTPH_TYPE::ADD_UV2:
		buf->morphOffsetList = new UVMorphOffset[count];
		ReadUVMorphOffset(static_cast<UVMorphOffset*>(buf->morphOffsetList), count, file);
		break;

	case MOTPH_TYPE::ADD_UV3:
		buf->morphOffsetList = new UVMorphOffset[count];
		ReadUVMorphOffset(static_cast<UVMorphOffset*>(buf->morphOffsetList), count, file);
		break;

	case MOTPH_TYPE::ADD_UV4:
		buf->morphOffsetList = new UVMorphOffset[count];
		ReadUVMorphOffset(static_cast<UVMorphOffset*>(buf->morphOffsetList), count, file);
		break;

	case MOTPH_TYPE::MATERIAL:
		buf->morphOffsetList = new MaterialMorphOffset[count];
		ReadMaterialMorphOffset(static_cast<MaterialMorphOffset*>(buf->morphOffsetList), count, file);
		break;

	default:
		return false;
	}

	return true;
}

