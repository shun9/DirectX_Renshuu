//************************************************/
//* @file  :SL_VMDData.cpp
//* @brief :VMDファイルのデータの読み込み＆保持
//* @date  :2017/10/08
//* @author:S.Katou
//************************************************/
#include "SL_VMDData.h"

/// <summary>
/// モーションデータの読み込み
/// </summary>
/// <param name="path">ファイル名</param>
/// <returns>成功したらtrue</returns>
bool ShunLib::VMD::VMDData::LoadVMD(wchar_t * path)
{
	using namespace std;
	ifstream file;
	file.open(path, ios::in | ios::binary);

	//ファイルが無ければ読み込まない
	if (!file)return false;

	//各種データの読み込み
	if (!(LoadHeader(&file))) { return false; }
	if (!(LoadMotion(&file))) { return false; }
	if (!(LoadSkin	(&file))) { return false; }
	
	return true;
}


/// <summary>
/// ヘッダー部分の読み込み
/// </summary>
bool ShunLib::VMD::VMDData::LoadHeader(std::ifstream * file)
{
	file->read(m_header.signature, VMD_SIGNATURE_SIZE);
	file->read(m_header.modelName, VMD_MODEL_NAME_SIZE);

	return true;
}


/// <summary>
/// モーション部分の読み込み
/// </summary>
bool ShunLib::VMD::VMDData::LoadMotion(std::ifstream * file)
{
	//データ数読み込み
	file->read((char*)&m_motion.recordCount, sizeof(int));

	//データ格納枠作成
	m_motion.info.resize(m_motion.recordCount);

	for (int i = 0; i < m_motion.recordCount; i++)
	{
		//ボーン名読み込み
		file->read(m_motion.info[i].boneName, VMD_BONE_NAME_SIZE);

		//フレーム数読み込み
		file->read((char*)&m_motion.info[i].flameNum, sizeof(int));

		//位置読み込み
		file->read((char*)&m_motion.info[i].pos, sizeof(Vec3));
		
		//回転読み込み
		file->read((char*)&m_motion.info[i].rot, sizeof(Vec4));

		//回転読み込み
		file->read((char*)m_motion.info[i].interpolation, VMD_INTERPOLATION_SIZE);

	}
	return true;
}


/// <summary>
/// 表情部分の読み込み
/// </summary>
bool ShunLib::VMD::VMDData::LoadSkin(std::ifstream * file)
{
	//データ数読み込み
	file->read((char*)&m_skin.recordCount, sizeof(int));

	//データ格納枠作成
	m_skin.info.resize(m_skin.recordCount);

	for (int i = 0; i < m_skin.recordCount; i++)
	{
		//表情名読み込み
		file->read(m_skin.info[i].skinName, VMD_SKIN_NAME_SIZE);

		//フレーム数読み込み
		file->read((char*)&m_skin.info[i].flameNum, sizeof(int));

		//表情の設定値読み込み
		file->read((char*)&m_skin.info[i].weight, sizeof(float));
	}
	return true;
}
