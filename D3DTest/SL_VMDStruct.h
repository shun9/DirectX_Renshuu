//************************************************/
//* @file  :SL_VMDStruct.h
//* @brief :VMDファイルのデータ構造
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
			char signature[VMD_SIGNATURE_SIZE]; //シグネチャ
			char modelName[VMD_MODEL_NAME_SIZE];//モデル名
		};
		
		//モーションデータ
		struct VMDMotionData
		{
			char boneName[VMD_BONE_NAME_SIZE];            //ボーンの名前
			int flameNum;                                 //フレームナンバー
			Vec3 pos;                                     //ボーンの位置
			Vec4 rot;                                     //ボーンの回転　クオータニオン
			VMDByte interpolation[VMD_INTERPOLATION_SIZE];//補間データ
		};
			
		struct VMDMotion
		{
			int recordCount;                //データ数
			std::vector<VMDMotionData> info;//モーションデータ
		};

		//表情データ
		struct VMDSkinData
		{
			char skinName[VMD_SKIN_NAME_SIZE]; // 表情名
			int flameNum;                      // フレーム番号
			float weight;                      // 表情の設定値(表情スライダーの値)
		};

		struct VMDSkin
		{
			int recordCount;             //データ数
			std::vector<VMDSkinData>info;//表情データ
		};
	}
}
