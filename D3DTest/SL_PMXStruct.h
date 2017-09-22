#pragma once
#include <SL_Vec4.h>
#include <SL_Vec3.h>
#include <SL_Vec2.h>

#include <vector>
#include <string>

namespace ShunLib
{
	namespace PMX
	{
		typedef unsigned char PMXByte;

		//�e�L�X�g�o�b�t�@
		struct PMXTextBuf
		{
			int byteSize;	//������
			std::string str;	//������
		};

		//PMX�w�b�_
		struct PMXHeader
		{
			PMXByte magicNum[4];		 // �}�W�b�N�i���o�[�i�t�@�C�����ʗp�@Pmd�j
			float version;				 // �o�[�W����
			PMXByte dataSize;			 // �㑱����f�[�^��̃o�C�g�T�C�Y  PMX2.0�� 8 �ŌŒ�
			std::vector<PMXByte> data;   // �㑱����f�[�^��
			//PMXByte encodeType;        // �����R�[�h�̃G���R�[�h�^�C�v 0:UTF16 1:UTF8
			//PMXByte addUVCount;        // �ǉ��t�u�� ( 0�`4 )
			//PMXByte vertexIndexSize;   // ���_Index�T�C�Y ( 1 or 2 or 4 )
			//PMXByte textureIndexSize;  // �e�N�X�`��Index�T�C�Y ( 1 or 2 or 4 )
			//PMXByte materialIndexSize; // �}�e���A��Index�T�C�Y ( 1 or 2 or 4 )
			//PMXByte boneIndexSize;     // �{�[��Index�T�C�Y ( 1 or 2 or 4 )
			//PMXByte morphIndexSize;    // ���[�tIndex�T�C�Y ( 1 or 2 or 4 )
			//PMXByte rigidIndexSize;    // ����Index�T�C�Y ( 1 or 2 or 4 )
		};

		//���f�����
		struct PMXModelInfo
		{
			PMXTextBuf name;     // ���f����
			PMXTextBuf nameE;    // ���f�����i�p��j
			PMXTextBuf comment;  // �R�����g
			PMXTextBuf commentE; // �R�����g�i�p��j
		};


		//�{�[���̃E�F�C�g
		struct  PMXBoneWeight
		{
			int bone1ReferenceIndex; // �{�[��1�̎Q��Index
			int bone2ReferenceIndex; // �{�[��2�̎Q��Index
			int bone3ReferenceIndex; // �{�[��3�̎Q��Index
			int bone4ReferenceIndex; // �{�[��4�̎Q��Index
			float bone1Weight;       // �{�[��1�̃E�F�C�g�l(0�`1.0)
			float bone2Weight;       // �{�[��2�̃E�F�C�g�l(0�`1.0)
			float bone3Weight;       // �{�[��3�̃E�F�C�g�l(0�`1.0)
			float bone4Weight;       // �{�[��4�̃E�F�C�g�l(0�`1.0)
			Vec3 SDEF_C;	         // SDEF-C�l(x,y,z)
			Vec3 SDEF_R0;			 // SDEF-R0�l(x,y,z)
			Vec3 SDEF_R1;		     // SDEF-R1�l(x,y,z) ���C���l��v�v�Z
		};

		//���_���
		struct PMXVertexInfo
		{
			Vec3 pos;                //�ʒu
			Vec3 normal;             //�@��
			Vec2 uv;                 //UV
			std::vector<Vec4> addUv; //�ǉ�UV
			PMXByte weightType;      //�E�F�C�g�ό`���� 
			PMXBoneWeight boneWeight;//�{�[���̃E�F�C�g
			float edgeScale;         //�G�b�W�{��
		};

		//�ʏ��
		struct PMXFaceInfo
		{
			int a;
			int b;
			int c;
		};

		//�}�e���A�����
		struct PmxMaterialInfo
		{
			PMXTextBuf materialName;              // �ގ���
			PMXTextBuf materialNameE;            // �ގ����i�p��j
			Vec3 diffuse;                         // �g�U�F (R,G,B,A)
			Vec3 specular;                        // ���ːF (R,G,B)
			float shininess;                      // ���ːF�W��
			Vec3 ambient;                         // ���F (R,G,B)
			PMXByte renderFlag;                   // �`��t���O(8bit) - �ebit 0:OFF 1:ON 0x01:���ʕ`��, 0x02:�n�ʉe, 0x04:�Z���t�V���h�E�}�b�v�ւ̕`��, 0x08:�Z���t�V���h�E�̕`��, 0x10:�G�b�W�`��, 0x20:���_�J���[(��2.1�g��), 0x40:Point�`��(��2.1�g��), 0x80:Line�`��(��2.1�g��)
			Vec4 edgeColor;                       // �G�b�W�F (R,G,B,A)
			float edgeSize;                       // �G�b�W�T�C�Y
			int textureTableReferenceIndex;       // �ʏ�e�N�X�`��, �e�N�X�`���e�[�u���̎Q��Index
			int sphereTextureTableReferenceIndex; // �X�t�B�A�e�N�X�`��, �e�N�X�`���e�[�u���̎Q��Index  ���e�N�X�`���g���q�̐����Ȃ�
			PMXByte sphereMode;                   // �X�t�B�A���[�h 0:���� 1:��Z(sph) 2:���Z(spa) 3:�T�u�e�N�X�`��(�ǉ�UV1��x,y��UV�Q�Ƃ��Ēʏ�e�N�X�`���`����s��)
			PMXByte shareToonFlag;                // ���LToon�t���O 0:�p���l�͌�Toon 1:�p���l�͋��LToon
			int textureIndex;                     // ���LToon�t���O:0 �̏ꍇ Toon�e�N�X�`��, �e�N�X�`���e�[�u���̎Q��Index ���LToon�t���O:1 �̏ꍇ ���LToon�e�N�X�`��[0�`9] -> ���ꂼ�� toon01.bmp�`toon10.bmp �ɑΉ��ǂ��炩����Ȃ̂Œ���
			PMXTextBuf memo;                      // ���� : ���R���^�X�N���v�g�L�q�^�G�t�F�N�g�ւ̃p�����[�^�z�u�Ȃ�
			int faceVertexCount;                  // �ގ��ɑΉ������(���_)�� (�K��3�̔{���ɂȂ�)
		};

		// IK�����N
		struct PMXIKLink
		{
			int linkBoneIndex;      // �����N�{�[���̃{�[��Index
			PMXByte isRotateLimited;// �p�x���� 0:OFF 1:ON
			Vec3 minimumRadian;     // ���� (x,y,z) -> ���W�A���p
			Vec3 maximumRadian;     // ��� (x,y,z) -> ���W�A���p
		};

		//�{�[��
		struct PmxBoneInfo
		{
			PMXTextBuf boneName;            // �{�[����
			PMXTextBuf boneNameE;          // �{�[�����i�p��j
			Vec3 position;                  // �ʒu
			int parentBoneIndex;            // �e�{�[���̃{�[��Index
			int deformationLevel;           // �ό`�K�w
			unsigned short boneFlag;        // �{�[���t���O(16bit) �ebit 0:OFF 1:ON
			Vec3 positionOffset;            // ���W�I�t�Z�b�g, �{�[���ʒu����̑��Ε�
			int connectedBoneIndex;         // �ڑ���{�[���̃{�[��Index
			int providedParentBoneIndex;    // �t�^�e�{�[���̃{�[��Index
			float providedRatio;            // �t�^��
			Vec3 axisDirectionVector;       // ���̕����x�N�g��
			Vec3 dimentionXDirectionVector; // X���̕����x�N�g��
			Vec3 dimentionZDirectionVector; // Z���̕����x�N�g��
			int keyValue;                   // Key�l
			int ikTargetBoneIndex;          // IK�^�[�Q�b�g�{�[���̃{�[��Index
			int ikLoopCount;                // IK���[�v�� (PMD�y��MMD���ł�255�񂪍ő�ɂȂ�悤�ł�)
			float ikLimitedRadian;          // IK���[�v�v�Z����1�񂠂���̐����p�x -> ���W�A���p | PMD��IK�l�Ƃ�4�{�قȂ�̂Œ���
			int ikLinkCount;                // IK�����N�� : �㑱�̗v�f��
			std::vector<PMXIKLink> ikLinkList;// IK�����N���X�g
		};


		// ���_���[�t�I�t�Z�b�g
		struct VertexMorphOffset
		{
			int vertexIndex;           // ���_Index
			Vec3 positionOffset;     // ���W�I�t�Z�b�g��(x,y,z)
		};

		// UV���[�t�I�t�Z�b�g
		struct UVMorphOffset
		{
			int vertexIndex;           // ���_Index
			Vec4 uvOffset;           // UV�I�t�Z�b�g��(x,y,z,w) ���ʏ�UV��z,w���s�v���ڂɂȂ邪���[�t�Ƃ��Ẵf�[�^�l�͋L�^���Ă���
		};

		// �{�[�����[�t�I�t�Z�b�g
		struct BoneMoptOffset
		{
			int boneIndex;             // �{�[��Index
			Vec3 quantityOfMoving;   // �ړ���(x,y,z)
			Vec4 quantityOfRotating; // ��]��-�N�H�[�^�j�I��(x,y,z,w)
		};

		// �ގ����[�t�I�t�Z�b�g
		struct MaterialMorphOffset
		{
			int materialIndex;         // �ގ�Index -> -1:�S�ގ��Ώ�
			PMXByte offsetCalclationType; // �I�t�Z�b�g���Z�`�� | 0:��Z, 1:���Z - �ڍׂ͌�q
			Vec4 diffuse;              // �g�U�F (R,G,B,A) - ��Z:1.0�^���Z:0.0 �������l�ƂȂ�
			Vec3 specular;              // ���ːF(R,G,B)- ��Z:1.0�^���Z:0.0 �������l�ƂȂ�
			float specularCoefficient; // ���ːF�W��- ��Z:1.0�^���Z:0.0 �������l�ƂȂ�
			Vec3 ambient;               // ���F (R,G,B)- ��Z:1.0�^���Z:0.0 �������l�ƂȂ�
			Vec4 edgeColor;            // �G�b�W�F (R,G,B,A)- ��Z:1.0�^���Z:0.0 �������l�ƂȂ�
			float edgeSize;            // �G�b�W�T�C�Y- ��Z:1.0�^���Z:0.0 �������l�ƂȂ�
			Vec4 textureCoefficient;   // �e�N�X�`���W�� (R,G,B,A)- ��Z:1.0�^���Z:0.0 �������l�ƂȂ�
			Vec4 sphereTextureCoefficient; // �X�t�B�A�e�N�X�`���W�� (R,G,B,A)- ��Z:1.0�^���Z:0.0 �������l�ƂȂ�
			Vec4 toonTextureCoefficient; // Toon�e�N�X�`���W�� (R,G,B,A)- ��Z:1.0�^���Z:0.0 �������l�ƂȂ�
		};

		// �O���[�v���[�t�I�t�Z�b�g
		struct GroupMorphOffset
		{
			int morphIndex;             // ���[�tIndex  ���d�l��O���[�v���[�t�̃O���[�v���͔�Ή��Ƃ���
			float morphRatio;           // ���[�t�� : �O���[�v���[�t�̃��[�t�l * ���[�t�� = �Ώۃ��[�t�̃��[�t�l
		};

		// �t���b�v���[�t�I�t�Z�b�g
		struct FlipMorphOffset
		{
			int morphIndex;             // ���[�tIndex
			float morphValue;           // ���[�t�l
		};

		// �C���p���X���[�t�I�t�Z�b�g
		struct ImpulseMorphOffset
		{
			int rigidBodyIndex;         // ����Index
			PMXByte localFlag;             // ���[�J���t���O   | 0:OFF 1:ON
			Vec3 velocityOfMoving;    // �ړ����x (x,y,z)
			Vec3 torqueOfRotating;    // ��]�g���N (x,y,z)   �� �ړ����x�^��]�g���N ���ׂ� 0 �̏ꍇ��"��~����"�Ƃ��ē��ꉻ
		};

		// ���[�t
		// �����[�t���
		// �i�[�\�ȃ��[�t�͑�ʂ��āA���_���[�t�AUV���[�t�A�{�[�����[�t�A�ގ����[�t�A
		// �O���[�v���[�t�A�t���b�v���[�t(��2.1�g��) �C���p���X���[�t(��2.1�g��)�̂V��ށB
		// �����UV���[�t�́AUV�^�ǉ�UV1�`4�̌v5��ނɕ��ނ����B
		// ���ǉ�UV���ɂ���Ă͕s�v��UV���[�t���i�[����邱�Ƃ����邪�A
		// ���[�t���͓��ɍ폜�Ȃǂ͍s��Ȃ��̂Œ��ӁB
		struct PMXMorphInfo
		{
			PMXTextBuf morphName;      // ���[�t��
			PMXTextBuf morphNameE;     // ���[�t���i�p��j
			PMXByte operationPanel;    // ����p�l�� (PMD:�J�e�S��) 1:��(����) 2:��(����) 3:��(�E��) 4:���̑�(�E��)  | 0:�V�X�e���\��
			PMXByte type;              // ���[�t��� - 0:�O���[�v, 1:���_, 2:�{�[��, 3:UV, 4:�ǉ�UV1, 5:�ǉ�UV2, 6:�ǉ�UV3, 7:�ǉ�UV4, 8:�ގ�,  9:�t���b�v(��2.1�g��) 10:�C���p���X(��2.1�g��)
			int morphOffsetCount;      // ���[�t�̃I�t�Z�b�g�� : �㑱�̗v�f��
			void* morphOffsetList;     // ���[�t��ނɏ]���ăI�t�Z�b�g�f�[�^���i�[ ���قȂ��ނ̍����͕s��
		};


		// �g���v�f���X�g
		struct PMXFrameElement
		{
			PMXByte isMorph;   // �v�f�Ώ� 0:�{�[�� 1:���[�t
			int index;         // �{�[��Index�܂��̓��[�tIndex
		};

		// �\���g
		struct PMXDisplayFrameInfo
		{
			PMXTextBuf frameName;              // �g��
			PMXTextBuf frameNameE;            // �g���i�p��j
			PMXByte isSpecialFrame;            // ����g�t���O - 0:�ʏ�g 1:����g
			int elementCount;		           // �g���v�f�� : �㑱�̗v�f��
			PMXFrameElement *frameElementList; // �g���v�f���X�g
		};

		// ����
		struct PMXRigidBodyInfo
		{
			PMXTextBuf rigidBodyName;            // ���̖�
			PMXTextBuf rigidBodyNameE;          // ���̖��i�p��j
			int boneIndex;                       // �֘A�{�[��Index - �֘A�Ȃ��̏ꍇ��-1
			PMXByte rigidBodyGroup;              // �O���[�v
			unsigned short unCollisionGroupFlag; // ��Փ˃O���[�v�t���O
			PMXByte shape;                       // �`�� - 0:�� 1:�� 2:�J�v�Z��
			Vec3 size;                           // �T�C�Y(x,y,z)
			Vec3 position;                       // �ʒu(x,y,z)
			Vec3 rotation;                       // ��](x,y,z) -> ���W�A���p
			float mass;                          // ����
			float moveAttenuation;               // �ړ�����
			float rotationAttenuation;           // ��]����
			float repulsion;                     // ������
			float friction;                      // ���C��
			PMXByte physicsCalcType;             // ���̂̕������Z - 0:�{�[���Ǐ](static) 1:�������Z(dynamic) 2:�������Z + Bone�ʒu���킹
		};

		// �W���C���g
		struct PMXJointInfo
		{
			PMXTextBuf jointName;             // �W���C���g��
			PMXTextBuf jointNameE;           // �W���C���g���i�p��j
			PMXByte jointType;                // �W���C���g��� - 0:��ȕt6DOF, 1:6DOF, 2:P2P, 3:ConeTwist, 4:Slider, 5:Hinge
			int rigidBodyAIndex;              // �֘A����A��Index - �֘A�Ȃ��̏ꍇ��-1
			int rigidBodyBIndex;              // �֘A����B��Index - �֘A�Ȃ��̏ꍇ��-1
			Vec3 position;                    // �ʒu(x,y,z)
			Vec3 rotation;                    // ��](x,y,z) -> ���W�A���p
			Vec3 moveLimitationMin;           // �ړ�����-����(x,y,z)
			Vec3 moveLimitationMax;           // �ړ�����-���(x,y,z)
			Vec3 rotationLimitationMin;       // ��]����-����(x,y,z) -> ���W�A���p
			Vec3 rotationLimitationMax;       // ��]����-���(x,y,z) -> ���W�A���p
			Vec3 springMoveCoefficient;       // �o�l�萔-�ړ�(x,y,z)
			Vec3 springRotationCoefficient;   // �o�l�萔-��](x,y,z)
		};

		// �A���J�[����       
		struct AnchorRigidBody
		{
			int RigidBodyIndex;                 // �֘A����Index
			int VertexIndex;                    // �֘A���_Index
			PMXByte    NearMode;                // Near ���[�h  0:OFF 1:ON
		};

		// Pin���_        
		struct PinVertex
		{
			int VertexIndex;                    // �֘A���_Index
		};

		// Config       
		struct Config
		{
			float   VCF;                        // VCF
			float   DP;                         // DP
			float   DG;                         // DG
			float   LF;                         // LF
			float   PR;                         // PR
			float   VC;                         // VC
			float   DF;                         // DF
			float   MT;                         // MT
			float   CHR;                        // CHR
			float   KHR;                        // KHR
			float   SHR;                        // SHR
			float   AHR;                        // AHR
		};

		// Cluster
		struct Cluster
		{
			float   SRHR_CL;
			float   SKHR_CL;
			float   SSHR_CL;
			float   SR_SPLT_CL;
			float   SK_SPLT_CL;
			float   SS_SPLT_CL;
		};

		// Iteration
		struct Iteration
		{
			int V_IT;                           // V_IT
			int P_IT;                           // P_IT
			int D_IT;                           // D_IT
			int C_IT;                           // C_IT
		};

		// Material
		struct Material
		{
			float   LST;
			float   AST;
			float   VST;
		};

		// ���
		struct PMXSoftBodyInfo
		{
			PMXTextBuf softBodyName;            // ��̖�
			PMXTextBuf softBodyNameE;          // ��̖��i�p��j
			PMXByte shape;                      // �`�� - 0:TriMesh 1:Rope
			int materialIndex;                  // �֘A�ގ�Index
			PMXByte softBodyGroup;              // �O���[�v
			unsigned short unCollisionGroupFlag;// ��Փ˃O���[�v�t���O
			PMXByte linkFlag;                   // �t���O(8bit) - �ebit 0:OFF 1:ON
			int blinkRange;                     // B-Link �쐬����
			int clusterCount;                   // �N���X�^��
			float grossMass;                    // ������
			float collisionMargin;              // �Փ˃}�[�W��
			int aeroModel;                      // AeroModel - 0:V_Point, 1:V_TwoSided, 2:V_OneSided, 3:F_TwoSided, 4:F_OneSided
			Config config;                      // Config
			Cluster cluster;                    // Cluster
			Iteration iteration;                // Iteration
			Material material;                  // Material
			int AnchorRigidBodyNumber;          // �A���J�[���̐�
			AnchorRigidBody *anchorRigidBody;   // �A���J�[����   
			int PinVertexCount;                 // Pin���_��
			PinVertex *pinVertex;               // Pin���_
		};

		//���_
		struct PMXVertex
		{
			int count;
			std::vector<PMXVertexInfo> info;
		};

		//��
		struct PMXFace
		{
			int count;
			std::vector<PMXFaceInfo> info;
		};

		//�e�N�X�`��
		struct PMXTexture
		{
			int count;
			std::vector<PMXTextBuf> fileName;
		};

		//�}�e���A��
		struct PmxMaterial
		{
			int count;
			std::vector<PmxMaterialInfo> info;
		};

		//�{�[��
		struct PMXBone
		{
			int count;
			std::vector<PmxBoneInfo> info;
		};

		// ���[�t
		struct PMXMorph
		{
			int count;				        // ���[�t�̐�
			std::vector<PMXMorphInfo> info; // ���[�t
		};

		// �\���g
		struct PMXDisplayFrame
		{
			int count;								// �\���g��
			std::vector<PMXDisplayFrameInfo> info;	// �\���g
		};

		// ����
		struct PMXRigidBody
		{
			int count;							// ���̐�
			std::vector<PMXRigidBodyInfo> info;	// ����
		};

		// �W���C���g
		struct PMXJoint
		{
			int count;						// �W���C���g��
			std::vector<PMXJointInfo> info;	// �W���C���g
		};

		// ���
		struct PMXSoftBody
		{
			int count;							// ��̐�
			std::vector<PMXSoftBodyInfo> info;	// ���
		};
	}
}
