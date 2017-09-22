#pragma once
#include "SL_PMXStruct.h"
#include "SL_PMXConstantNumber.h"
#include <cstdio>

namespace ShunLib
{
	namespace PMX
	{
		class PMXModel
		{
		private:
			PMXHeader m_header;
			PMXModelInfo m_modelInfo;
			PMXVertex m_vertex;
			PMXFace m_face;
			PMXTexture m_texture;
			PmxMaterial m_material;
			PMXBone m_bone;
			PMXMorph m_morph;
			PMXDisplayFrame m_displayframe;
			PMXRigidBody m_rigidBody;
			PMXJoint m_joint;
			PMXSoftBody m_softBody;

		public:
			//コンストラクタ
			PMXModel() {}

			//デストラクタ
			~PMXModel();

			bool LoadModel(char* file);

		private:
			bool Load(FILE* file);
			bool LoadHeader(FILE* file);
			bool LoadModelInfo(FILE* file);
			bool LoadVertex(FILE* file);
			bool LoadFace(FILE* file);
			bool LoadTexture(FILE* file);
			bool LoadMaterial(FILE* file);
			bool LoadBone(FILE* file);
			bool LoadMorph(FILE* file);
			bool LoadDisplayFrame(FILE* file);
			bool LoadRigidBody(FILE* file);
			bool LoadJoint(FILE* file);
			bool LoadSoftBody(FILE* file);

			bool ReadName(PMXTextBuf* textBuf,FILE* file);
			bool ReadText(std::string* buf, int byte, PMXByte encode, FILE* file);
			bool ReadBoneWeight(PMXBoneWeight* buf, PMXByte type, FILE* file);
			bool ReadBoneFlag(PmxBoneInfo* buf,unsigned short flag, FILE* file);
			bool ReadGroupMorphOffset(GroupMorphOffset* buf, int count, FILE* file);
			bool ReadVertexMorphOffset(VertexMorphOffset* buf, int count, FILE* file);
			bool ReadBoneMorphOffset(BoneMoptOffset* buf, int count, FILE* file);
			bool ReadUVMorphOffset(UVMorphOffset* buf, int count, FILE* file);
			bool ReadMaterialMorphOffset(MaterialMorphOffset* buf, int count, FILE* file);

			bool MakeMorphOffset(PMXMorphInfo* buf, PMXByte type, int count, FILE* file);
		};
	}
}