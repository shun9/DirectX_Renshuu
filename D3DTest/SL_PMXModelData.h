#pragma once
#include "SL_PMXStruct.h"
#include "SL_PMXConstantNumber.h"
#include <cstdio>

namespace ShunLib
{
	namespace PMX
	{
		class PMXModelData
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

			std::string m_tetureFolder;

		public:
			//コンストラクタ
			PMXModelData() {}

			//デストラクタ
			~PMXModelData();

			bool LoadModel(char* file);

			std::string FolderPath(int num) {
				return m_tetureFolder + m_texture.fileName[num].str;
			}
			void FolderPath(const std::string& path) {
				m_tetureFolder= path;
			}

			//情報取得関連
			PMXHeader* Header()             { return &m_header; }
			PMXModelInfo* ModelInfo()       { return &m_modelInfo; }
			PMXVertex* Vertex()             { return &m_vertex;}
			PMXFace* Face()                 { return &m_face;}
			PMXTexture* Texture()           { return &m_texture; }
			PmxMaterial* Material()         { return &m_material;}
			PMXBone* Bone()                 { return &m_bone;}
			PMXMorph* Morph()               { return &m_morph;}
			PMXDisplayFrame* Displayframe() { return &m_displayframe;}
			PMXRigidBody* RigidBody()       { return &m_rigidBody;}
			PMXJoint* Joint()               { return &m_joint;}
			PMXSoftBody* SoftBody()         { return &m_softBody;}

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
			bool ReadGroupMorphOffset(std::vector<GroupMorphOffset>* buf, int count, FILE* file);
			bool ReadVertexMorphOffset(std::vector<VertexMorphOffset>* buf, int count, FILE* file);
			bool ReadBoneMorphOffset(std::vector<BoneMoptOffset>* buf, int count, FILE* file);
			bool ReadUVMorphOffset(std::vector<UVMorphOffset>* buf, int count, FILE* file);
			bool ReadMaterialMorphOffset(std::vector<MaterialMorphOffset>* buf, int count, FILE* file);

			bool MakeMorphOffset(PMXMorphInfo* buf, PMXByte type, int count, FILE* file);
		};
	}
}