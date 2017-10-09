//************************************************/
//* @file  :SL_VMDStruct.h
//* @brief :VMD�t�@�C���̃f�[�^�\��
//* @date  :2017/10/08
//* @author:S.Katou
//************************************************/
#pragma once
#include <vector>
#include <SL_Vec3.h>
#include <SL_Vec4.h>
#include "SL_VMDConstantNumber.h"

namespace ShunLib{
	namespace VMD {
		typedef unsigned char VMDByte;

		struct VMDHeader
		{
			char signature[VMD_SIGNATURE_SIZE]; //�V�O�l�`��
			char modelName[VMD_MODEL_NAME_SIZE];//���f����
		};
		
		//���[�V�����f�[�^
		struct VMDMotionData
		{
			char boneName[VMD_BONE_NAME_SIZE];            //�{�[���̖��O
			int flameNum;                                 //�t���[���i���o�[
			Vec3 pos;                                     //�{�[���̈ʒu
			Vec4 rot;                                     //�{�[���̉�]�@�N�I�[�^�j�I��
			VMDByte interpolation[VMD_INTERPOLATION_SIZE];//��ԃf�[�^
		};
			
		struct VMDMotion
		{
			int recordCount;                //�f�[�^��
			std::vector<VMDMotionData> info;//���[�V�����f�[�^
		};

		//�\��f�[�^
		struct VMDSkinData
		{
			char skinName[VMD_SKIN_NAME_SIZE]; // �\�
			int flameNum;                      // �t���[���ԍ�
			float weight;                      // �\��̐ݒ�l(�\��X���C�_�[�̒l)
		};

		struct VMDSkin
		{
			int recordCount;             //�f�[�^��
			std::vector<VMDSkinData>info;//�\��f�[�^
		};
	}
}
