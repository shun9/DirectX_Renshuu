//************************************************/
//* @file  :SL_VMDData.h
//* @brief :VMDファイルのデータの読み込み＆保持
//* @date  :2017/10/08
//* @author:S.Katou
//************************************************/
#pragma once
#include "SL_VMDStruct.h"
#include <fstream>

namespace ShunLib {
	namespace VMD{
		class VMDData
		{
		private:
			VMDHeader m_header;
			VMDMotion m_motion;
			VMDSkin m_skin;
		public:
			VMDData() {}
			~VMDData() {}

			bool LoadVMD(wchar_t* path);
		private:
			bool LoadHeader(std::ifstream* file);
			bool LoadMotion(std::ifstream* file);
			bool LoadSkin  (std::ifstream* file);
		};
	}
}

