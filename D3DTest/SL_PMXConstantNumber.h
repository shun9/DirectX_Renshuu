#pragma once

#define PMX_UTF16 ((PMXByte)0)
#define PMX_UTF8 ((PMXByte)1)

namespace ShunLib
{
	namespace PMX
	{
		//�w�b�_�[���
		enum HEADER_DATA_2_0
		{
			ENCODE_TYPE = 0,
			ADD_UV,
			VERTEX_INDEX,
			TEXTURE_INDEX,
			MATERIAL_INDEX,
			BONE_INDEX,
			MORPH_INDEX,
			RIGID_INDEX,
		};

		//�{�[���E�F�C�g�ό`����
		enum BONE_WEIGHT_TYPE
		{
			BDEF_1 = 0,
			BDEF_2,
			BDEF_4,
			SDEF,
		};

		enum MATERIAL_RENDER_TYPE
		{
			DUPLEX           = 0x01, // ���ʕ`��
			GROUND_SHADOW    = 0x02, // �n�ʉe
			SELF_SHADOW_MAP  = 0x04, // �Z���t�V���h�E�}�b�v�ւ̕`��
			DRAW_SELF_SHADOW = 0x08, // �Z���t�V���h�E�̕`��,
			EDGE             = 0x10, // �G�b�W�`��		
		};

		//�{�[���t���O
		enum BONE_FLAG
		{
			CONNECTED                      = 0x0001, //�ڑ���
			ROTATABLE                      = 0x0002, //��]�\
			MOVEABLE                       = 0x0004, //�ړ��\
			DISPLAY                        = 0x0008, //�\��
			OPERABLE                       = 0x0010, //�����
			IK                             = 0x0020, //IK
			LOCAL_GRANT                    = 0x0080, //���[�J���t�^
			ROTATION_GRANT                 = 0x0100, //��]�t�^
			MOVEMENT_GRANT                 = 0x0200, //�ړ��t�^
			LOCK_AXIS					   = 0x0400, //���Œ�
			LOCAL_AXIS                     = 0x0800, //���[�J����
			POST_PHYSICAL_DEFORMATION      = 0x1000, //������ό`
			EXTERNAL_PARENT_TRANSFORMATION = 0x2000, //�O���e�ό`
		};

		//���[�t�̎��
		enum MOTPH_TYPE
		{
			GROUP = 0,
			VERTEX,
			BONE,
			UV,
			ADD_UV1,
			ADD_UV2,
			ADD_UV3,
			ADD_UV4,
			MATERIAL,
		};

		//�g���v�f�̑Ώ�
		enum FLAME_TARGET
		{
			TERGET_BONE  = 0,
			TERGET_MORPH = 1,
		};
	}
}
