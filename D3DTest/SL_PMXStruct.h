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

		//テキストバッファ
		struct PMXTextBuf
		{
			int byteSize;	//文字数
			std::string str;	//文字列
		};

		//PMXヘッダ
		struct PMXHeader
		{
			PMXByte magicNum[4];		 // マジックナンバー（ファイル識別用　Pmd）
			float version;				 // バージョン
			PMXByte dataSize;			 // 後続するデータ列のバイトサイズ  PMX2.0は 8 で固定
			std::vector<PMXByte> data;   // 後続するデータ列
			//PMXByte encodeType;        // 文字コードのエンコードタイプ 0:UTF16 1:UTF8
			//PMXByte addUVCount;        // 追加ＵＶ数 ( 0～4 )
			//PMXByte vertexIndexSize;   // 頂点Indexサイズ ( 1 or 2 or 4 )
			//PMXByte textureIndexSize;  // テクスチャIndexサイズ ( 1 or 2 or 4 )
			//PMXByte materialIndexSize; // マテリアルIndexサイズ ( 1 or 2 or 4 )
			//PMXByte boneIndexSize;     // ボーンIndexサイズ ( 1 or 2 or 4 )
			//PMXByte morphIndexSize;    // モーフIndexサイズ ( 1 or 2 or 4 )
			//PMXByte rigidIndexSize;    // 剛体Indexサイズ ( 1 or 2 or 4 )
		};

		//モデル情報
		struct PMXModelInfo
		{
			PMXTextBuf name;     // モデル名
			PMXTextBuf nameE;    // モデル名（英語）
			PMXTextBuf comment;  // コメント
			PMXTextBuf commentE; // コメント（英語）
		};


		//ボーンのウェイト
		struct  PMXBoneWeight
		{
			int bone1ReferenceIndex; // ボーン1の参照Index
			int bone2ReferenceIndex; // ボーン2の参照Index
			int bone3ReferenceIndex; // ボーン3の参照Index
			int bone4ReferenceIndex; // ボーン4の参照Index
			float bone1Weight;       // ボーン1のウェイト値(0～1.0)
			float bone2Weight;       // ボーン2のウェイト値(0～1.0)
			float bone3Weight;       // ボーン3のウェイト値(0～1.0)
			float bone4Weight;       // ボーン4のウェイト値(0～1.0)
			Vec3 SDEF_C;	         // SDEF-C値(x,y,z)
			Vec3 SDEF_R0;			 // SDEF-R0値(x,y,z)
			Vec3 SDEF_R1;		     // SDEF-R1値(x,y,z) ※修正値を要計算
		};

		//頂点情報
		struct PMXVertexInfo
		{
			Vec3 pos;                //位置
			Vec3 normal;             //法線
			Vec2 uv;                 //UV
			std::vector<Vec4> addUv; //追加UV
			PMXByte weightType;      //ウェイト変形方式 
			PMXBoneWeight boneWeight;//ボーンのウェイト
			float edgeScale;         //エッジ倍率
		};

		//面情報
		struct PMXFaceInfo
		{
			int a;
			int b;
			int c;
		};

		//マテリアル情報
		struct PmxMaterialInfo
		{
			PMXTextBuf materialName;              // 材質名
			PMXTextBuf materialNameE;            // 材質名（英語）
			Vec3 diffuse;                         // 拡散色 (R,G,B,A)
			Vec3 specular;                        // 反射色 (R,G,B)
			float shininess;                      // 反射色係数
			Vec3 ambient;                         // 環境色 (R,G,B)
			PMXByte renderFlag;                   // 描画フラグ(8bit) - 各bit 0:OFF 1:ON 0x01:両面描画, 0x02:地面影, 0x04:セルフシャドウマップへの描画, 0x08:セルフシャドウの描画, 0x10:エッジ描画, 0x20:頂点カラー(※2.1拡張), 0x40:Point描画(※2.1拡張), 0x80:Line描画(※2.1拡張)
			Vec4 edgeColor;                       // エッジ色 (R,G,B,A)
			float edgeSize;                       // エッジサイズ
			int textureTableReferenceIndex;       // 通常テクスチャ, テクスチャテーブルの参照Index
			int sphereTextureTableReferenceIndex; // スフィアテクスチャ, テクスチャテーブルの参照Index  ※テクスチャ拡張子の制限なし
			PMXByte sphereMode;                   // スフィアモード 0:無効 1:乗算(sph) 2:加算(spa) 3:サブテクスチャ(追加UV1のx,yをUV参照して通常テクスチャ描画を行う)
			PMXByte shareToonFlag;                // 共有Toonフラグ 0:継続値は個別Toon 1:継続値は共有Toon
			int textureIndex;                     // 共有Toonフラグ:0 の場合 Toonテクスチャ, テクスチャテーブルの参照Index 共有Toonフラグ:1 の場合 共有Toonテクスチャ[0～9] -> それぞれ toon01.bmp～toon10.bmp に対応どちらか一方なので注意
			PMXTextBuf memo;                      // メモ : 自由欄／スクリプト記述／エフェクトへのパラメータ配置など
			int faceVertexCount;                  // 材質に対応する面(頂点)数 (必ず3の倍数になる)
		};

		// IKリンク
		struct PMXIKLink
		{
			int linkBoneIndex;      // リンクボーンのボーンIndex
			PMXByte isRotateLimited;// 角度制限 0:OFF 1:ON
			Vec3 minimumRadian;     // 下限 (x,y,z) -> ラジアン角
			Vec3 maximumRadian;     // 上限 (x,y,z) -> ラジアン角
		};

		//ボーン
		struct PmxBoneInfo
		{
			PMXTextBuf boneName;            // ボーン名
			PMXTextBuf boneNameE;          // ボーン名（英語）
			Vec3 position;                  // 位置
			int parentBoneIndex;            // 親ボーンのボーンIndex
			int deformationLevel;           // 変形階層
			unsigned short boneFlag;        // ボーンフラグ(16bit) 各bit 0:OFF 1:ON
			Vec3 positionOffset;            // 座標オフセット, ボーン位置からの相対分
			int connectedBoneIndex;         // 接続先ボーンのボーンIndex
			int providedParentBoneIndex;    // 付与親ボーンのボーンIndex
			float providedRatio;            // 付与率
			Vec3 axisDirectionVector;       // 軸の方向ベクトル
			Vec3 dimentionXDirectionVector; // X軸の方向ベクトル
			Vec3 dimentionZDirectionVector; // Z軸の方向ベクトル
			int keyValue;                   // Key値
			int ikTargetBoneIndex;          // IKターゲットボーンのボーンIndex
			int ikLoopCount;                // IKループ回数 (PMD及びMMD環境では255回が最大になるようです)
			float ikLimitedRadian;          // IKループ計算時の1回あたりの制限角度 -> ラジアン角 | PMDのIK値とは4倍異なるので注意
			int ikLinkCount;                // IKリンク数 : 後続の要素数
			std::vector<PMXIKLink> ikLinkList;// IKリンクリスト
		};


		// 頂点モーフオフセット
		struct VertexMorphOffset
		{
			int vertexIndex;           // 頂点Index
			Vec3 positionOffset;     // 座標オフセット量(x,y,z)
		};

		// UVモーフオフセット
		struct UVMorphOffset
		{
			int vertexIndex;           // 頂点Index
			Vec4 uvOffset;           // UVオフセット量(x,y,z,w) ※通常UVはz,wが不要項目になるがモーフとしてのデータ値は記録しておく
		};

		// ボーンモーフオフセット
		struct BoneMoptOffset
		{
			int boneIndex;             // ボーンIndex
			Vec3 quantityOfMoving;   // 移動量(x,y,z)
			Vec4 quantityOfRotating; // 回転量-クォータニオン(x,y,z,w)
		};

		// 材質モーフオフセット
		struct MaterialMorphOffset
		{
			int materialIndex;         // 材質Index -> -1:全材質対象
			PMXByte offsetCalclationType; // オフセット演算形式 | 0:乗算, 1:加算 - 詳細は後述
			Vec4 diffuse;              // 拡散色 (R,G,B,A) - 乗算:1.0／加算:0.0 が初期値となる
			Vec3 specular;              // 反射色(R,G,B)- 乗算:1.0／加算:0.0 が初期値となる
			float specularCoefficient; // 反射色係数- 乗算:1.0／加算:0.0 が初期値となる
			Vec3 ambient;               // 環境色 (R,G,B)- 乗算:1.0／加算:0.0 が初期値となる
			Vec4 edgeColor;            // エッジ色 (R,G,B,A)- 乗算:1.0／加算:0.0 が初期値となる
			float edgeSize;            // エッジサイズ- 乗算:1.0／加算:0.0 が初期値となる
			Vec4 textureCoefficient;   // テクスチャ係数 (R,G,B,A)- 乗算:1.0／加算:0.0 が初期値となる
			Vec4 sphereTextureCoefficient; // スフィアテクスチャ係数 (R,G,B,A)- 乗算:1.0／加算:0.0 が初期値となる
			Vec4 toonTextureCoefficient; // Toonテクスチャ係数 (R,G,B,A)- 乗算:1.0／加算:0.0 が初期値となる
		};

		// グループモーフオフセット
		struct GroupMorphOffset
		{
			int morphIndex;             // モーフIndex  ※仕様上グループモーフのグループ化は非対応とする
			float morphRatio;           // モーフ率 : グループモーフのモーフ値 * モーフ率 = 対象モーフのモーフ値
		};

		// フリップモーフオフセット
		struct FlipMorphOffset
		{
			int morphIndex;             // モーフIndex
			float morphValue;           // モーフ値
		};

		// インパルスモーフオフセット
		struct ImpulseMorphOffset
		{
			int rigidBodyIndex;         // 剛体Index
			PMXByte localFlag;             // ローカルフラグ   | 0:OFF 1:ON
			Vec3 velocityOfMoving;    // 移動速度 (x,y,z)
			Vec3 torqueOfRotating;    // 回転トルク (x,y,z)   ※ 移動速度／回転トルク すべて 0 の場合は"停止制御"として特殊化
		};

		// モーフ
		// ○モーフ種類
		// 格納可能なモーフは大別して、頂点モーフ、UVモーフ、ボーンモーフ、材質モーフ、
		// グループモーフ、フリップモーフ(※2.1拡張) インパルスモーフ(※2.1拡張)の７種類。
		// さらにUVモーフは、UV／追加UV1～4の計5種類に分類される。
		// ※追加UV数によっては不要なUVモーフが格納されることがあるが、
		// モーフ側は特に削除などは行わないので注意。
		struct PMXMorphInfo
		{
			PMXTextBuf morphName;      // モーフ名
			PMXTextBuf morphNameE;     // モーフ名（英語）
			PMXByte operationPanel;    // 操作パネル (PMD:カテゴリ) 1:眉(左下) 2:目(左上) 3:口(右上) 4:その他(右下)  | 0:システム予約
			PMXByte type;              // モーフ種類 - 0:グループ, 1:頂点, 2:ボーン, 3:UV, 4:追加UV1, 5:追加UV2, 6:追加UV3, 7:追加UV4, 8:材質,  9:フリップ(※2.1拡張) 10:インパルス(※2.1拡張)
			int morphOffsetCount;      // モーフのオフセット数 : 後続の要素数
			void* morphOffsetList;     // モーフ種類に従ってオフセットデータを格納 ※異なる種類の混合は不可
		};


		// 枠内要素リスト
		struct PMXFrameElement
		{
			PMXByte isMorph;   // 要素対象 0:ボーン 1:モーフ
			int index;         // ボーンIndexまたはモーフIndex
		};

		// 表示枠
		struct PMXDisplayFrameInfo
		{
			PMXTextBuf frameName;              // 枠名
			PMXTextBuf frameNameE;            // 枠名（英語）
			PMXByte isSpecialFrame;            // 特殊枠フラグ - 0:通常枠 1:特殊枠
			int elementCount;		           // 枠内要素数 : 後続の要素数
			PMXFrameElement *frameElementList; // 枠内要素リスト
		};

		// 剛体
		struct PMXRigidBodyInfo
		{
			PMXTextBuf rigidBodyName;            // 剛体名
			PMXTextBuf rigidBodyNameE;          // 剛体名（英語）
			int boneIndex;                       // 関連ボーンIndex - 関連なしの場合は-1
			PMXByte rigidBodyGroup;              // グループ
			unsigned short unCollisionGroupFlag; // 非衝突グループフラグ
			PMXByte shape;                       // 形状 - 0:球 1:箱 2:カプセル
			Vec3 size;                           // サイズ(x,y,z)
			Vec3 position;                       // 位置(x,y,z)
			Vec3 rotation;                       // 回転(x,y,z) -> ラジアン角
			float mass;                          // 質量
			float moveAttenuation;               // 移動減衰
			float rotationAttenuation;           // 回転減衰
			float repulsion;                     // 反発力
			float friction;                      // 摩擦力
			PMXByte physicsCalcType;             // 剛体の物理演算 - 0:ボーン追従(static) 1:物理演算(dynamic) 2:物理演算 + Bone位置合わせ
		};

		// ジョイント
		struct PMXJointInfo
		{
			PMXTextBuf jointName;             // ジョイント名
			PMXTextBuf jointNameE;           // ジョイント名（英語）
			PMXByte jointType;                // ジョイント種類 - 0:ﾊﾞﾈ付6DOF, 1:6DOF, 2:P2P, 3:ConeTwist, 4:Slider, 5:Hinge
			int rigidBodyAIndex;              // 関連剛体AのIndex - 関連なしの場合は-1
			int rigidBodyBIndex;              // 関連剛体BのIndex - 関連なしの場合は-1
			Vec3 position;                    // 位置(x,y,z)
			Vec3 rotation;                    // 回転(x,y,z) -> ラジアン角
			Vec3 moveLimitationMin;           // 移動制限-下限(x,y,z)
			Vec3 moveLimitationMax;           // 移動制限-上限(x,y,z)
			Vec3 rotationLimitationMin;       // 回転制限-下限(x,y,z) -> ラジアン角
			Vec3 rotationLimitationMax;       // 回転制限-上限(x,y,z) -> ラジアン角
			Vec3 springMoveCoefficient;       // バネ定数-移動(x,y,z)
			Vec3 springRotationCoefficient;   // バネ定数-回転(x,y,z)
		};

		// アンカー剛体       
		struct AnchorRigidBody
		{
			int RigidBodyIndex;                 // 関連剛体Index
			int VertexIndex;                    // 関連頂点Index
			PMXByte    NearMode;                // Near モード  0:OFF 1:ON
		};

		// Pin頂点        
		struct PinVertex
		{
			int VertexIndex;                    // 関連頂点Index
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

		// 軟体
		struct PMXSoftBodyInfo
		{
			PMXTextBuf softBodyName;            // 軟体名
			PMXTextBuf softBodyNameE;          // 軟体名（英語）
			PMXByte shape;                      // 形状 - 0:TriMesh 1:Rope
			int materialIndex;                  // 関連材質Index
			PMXByte softBodyGroup;              // グループ
			unsigned short unCollisionGroupFlag;// 非衝突グループフラグ
			PMXByte linkFlag;                   // フラグ(8bit) - 各bit 0:OFF 1:ON
			int blinkRange;                     // B-Link 作成距離
			int clusterCount;                   // クラスタ数
			float grossMass;                    // 総質量
			float collisionMargin;              // 衝突マージン
			int aeroModel;                      // AeroModel - 0:V_Point, 1:V_TwoSided, 2:V_OneSided, 3:F_TwoSided, 4:F_OneSided
			Config config;                      // Config
			Cluster cluster;                    // Cluster
			Iteration iteration;                // Iteration
			Material material;                  // Material
			int AnchorRigidBodyNumber;          // アンカー剛体数
			AnchorRigidBody *anchorRigidBody;   // アンカー剛体   
			int PinVertexCount;                 // Pin頂点数
			PinVertex *pinVertex;               // Pin頂点
		};

		//頂点
		struct PMXVertex
		{
			int count;
			std::vector<PMXVertexInfo> info;
		};

		//面
		struct PMXFace
		{
			int count;
			std::vector<PMXFaceInfo> info;
		};

		//テクスチャ
		struct PMXTexture
		{
			int count;
			std::vector<PMXTextBuf> fileName;
		};

		//マテリアル
		struct PmxMaterial
		{
			int count;
			std::vector<PmxMaterialInfo> info;
		};

		//ボーン
		struct PMXBone
		{
			int count;
			std::vector<PmxBoneInfo> info;
		};

		// モーフ
		struct PMXMorph
		{
			int count;				        // モーフの数
			std::vector<PMXMorphInfo> info; // モーフ
		};

		// 表示枠
		struct PMXDisplayFrame
		{
			int count;								// 表示枠数
			std::vector<PMXDisplayFrameInfo> info;	// 表示枠
		};

		// 剛体
		struct PMXRigidBody
		{
			int count;							// 剛体数
			std::vector<PMXRigidBodyInfo> info;	// 剛体
		};

		// ジョイント
		struct PMXJoint
		{
			int count;						// ジョイント数
			std::vector<PMXJointInfo> info;	// ジョイント
		};

		// 軟体
		struct PMXSoftBody
		{
			int count;							// 軟体数
			std::vector<PMXSoftBodyInfo> info;	// 軟体
		};
	}
}
