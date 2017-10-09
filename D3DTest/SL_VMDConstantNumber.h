//************************************************/
//* @file  :SL_VMDConstantNumber.h
//* @brief :VMDで使用する定数
//* @date  :2017/10/09
//* @author:S.Katou
//************************************************/
#pragma once

namespace ShunLib {
	namespace VMD {

		//VMDのシグネチャのバイト数
		const int VMD_SIGNATURE_SIZE = 30;

		//VMDのモデル名のバイト数
		const int VMD_MODEL_NAME_SIZE = 20;

		//VMDのボーン名のバイト数
		const int VMD_BONE_NAME_SIZE = 15;

		//VMDの補間データのバイト数
		const int VMD_INTERPOLATION_SIZE = 64;

		//VMDの表情名のバイト数
		const int VMD_SKIN_NAME_SIZE = 15;
	}
}
