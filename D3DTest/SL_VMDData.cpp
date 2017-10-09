//************************************************/
//* @file  :SL_VMDData.cpp
//* @brief :VMD�t�@�C���̃f�[�^�̓ǂݍ��݁��ێ�
//* @date  :2017/10/08
//* @author:S.Katou
//************************************************/
#include "SL_VMDData.h"

/// <summary>
/// ���[�V�����f�[�^�̓ǂݍ���
/// </summary>
/// <param name="path">�t�@�C����</param>
/// <returns>����������true</returns>
bool ShunLib::VMD::VMDData::LoadVMD(wchar_t * path)
{
	using namespace std;
	ifstream file;
	file.open(path, ios::in | ios::binary);

	//�t�@�C����������Γǂݍ��܂Ȃ�
	if (!file)return false;

	//�e��f�[�^�̓ǂݍ���
	if (!(LoadHeader(&file))) { return false; }
	if (!(LoadMotion(&file))) { return false; }
	if (!(LoadSkin	(&file))) { return false; }
	
	return true;
}


/// <summary>
/// �w�b�_�[�����̓ǂݍ���
/// </summary>
bool ShunLib::VMD::VMDData::LoadHeader(std::ifstream * file)
{
	file->read(m_header.signature, VMD_SIGNATURE_SIZE);
	file->read(m_header.modelName, VMD_MODEL_NAME_SIZE);

	return true;
}


/// <summary>
/// ���[�V���������̓ǂݍ���
/// </summary>
bool ShunLib::VMD::VMDData::LoadMotion(std::ifstream * file)
{
	//�f�[�^���ǂݍ���
	file->read((char*)&m_motion.recordCount, sizeof(int));

	//�f�[�^�i�[�g�쐬
	m_motion.info.resize(m_motion.recordCount);

	for (int i = 0; i < m_motion.recordCount; i++)
	{
		//�{�[�����ǂݍ���
		file->read(m_motion.info[i].boneName, VMD_BONE_NAME_SIZE);

		//�t���[�����ǂݍ���
		file->read((char*)&m_motion.info[i].flameNum, sizeof(int));

		//�ʒu�ǂݍ���
		file->read((char*)&m_motion.info[i].pos, sizeof(Vec3));
		
		//��]�ǂݍ���
		file->read((char*)&m_motion.info[i].rot, sizeof(Vec4));

		//��]�ǂݍ���
		file->read((char*)m_motion.info[i].interpolation, VMD_INTERPOLATION_SIZE);

	}
	return true;
}


/// <summary>
/// �\����̓ǂݍ���
/// </summary>
bool ShunLib::VMD::VMDData::LoadSkin(std::ifstream * file)
{
	//�f�[�^���ǂݍ���
	file->read((char*)&m_skin.recordCount, sizeof(int));

	//�f�[�^�i�[�g�쐬
	m_skin.info.resize(m_skin.recordCount);

	for (int i = 0; i < m_skin.recordCount; i++)
	{
		//�\��ǂݍ���
		file->read(m_skin.info[i].skinName, VMD_SKIN_NAME_SIZE);

		//�t���[�����ǂݍ���
		file->read((char*)&m_skin.info[i].flameNum, sizeof(int));

		//�\��̐ݒ�l�ǂݍ���
		file->read((char*)&m_skin.info[i].weight, sizeof(float));
	}
	return true;
}
