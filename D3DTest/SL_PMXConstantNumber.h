#pragma once

#define PMX_UTF16 ((PMXByte)0)
#define PMX_UTF8 ((PMXByte)1)

namespace ShunLib
{
	namespace PMX
	{
		//ヘッダー情報
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

		//ボーンウェイト変形方式
		enum BONE_WEIGHT_TYPE
		{
			BDEF_1 = 0,
			BDEF_2,
			BDEF_4,
			SDEF,
		};

		enum MATERIAL_RENDER_TYPE
		{
			DUPLEX           = 0x01, // 両面描画
			GROUND_SHADOW    = 0x02, // 地面影
			SELF_SHADOW_MAP  = 0x04, // セルフシャドウマップへの描画
			DRAW_SELF_SHADOW = 0x08, // セルフシャドウの描画,
			EDGE             = 0x10, // エッジ描画		
		};

		//ボーンフラグ
		enum BONE_FLAG
		{
			CONNECTED                      = 0x0001, //接続先
			ROTATABLE                      = 0x0002, //回転可能
			MOVEABLE                       = 0x0004, //移動可能
			DISPLAY                        = 0x0008, //表示
			OPERABLE                       = 0x0010, //操作可
			IK                             = 0x0020, //IK
			LOCAL_GRANT                    = 0x0080, //ローカル付与
			ROTATION_GRANT                 = 0x0100, //回転付与
			MOVEMENT_GRANT                 = 0x0200, //移動付与
			LOCK_AXIS					   = 0x0400, //軸固定
			LOCAL_AXIS                     = 0x0800, //ローカル軸
			POST_PHYSICAL_DEFORMATION      = 0x1000, //物理後変形
			EXTERNAL_PARENT_TRANSFORMATION = 0x2000, //外部親変形
		};

		//モーフの種類
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

		//枠内要素の対象
		enum FLAME_TARGET
		{
			TERGET_BONE  = 0,
			TERGET_MORPH = 1,
		};
	}
}
