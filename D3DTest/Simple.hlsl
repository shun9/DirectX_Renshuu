//************************************************/
//* @file  :Simple.hlsl
//* @brief :シェーダーの練習
//* @date  :2017/08/27
//* @author:S.Katou
//************************************************/

//グローバル
Texture2D g_texDecal : register(t0);
SamplerState g_samLinear : register(s0);

//グローバル
cbuffer global_0 : register(b0)
{
    float4 g_vLightDir;  //ライトの方向ベクトル
    float4 g_vEye;       //カメラ位置
};

cbuffer global_1 : register(b1)
{
    matrix g_world;                         //ワールド行列
    matrix g_wvp;                           //ワールドから射影までの変換行列
    float4 g_ambient  = float4(0, 0, 0, 0); //アンビエント光
    float4 g_diffuse  = float4(1, 0, 0, 0); //拡散反射(色）
    float4 g_specular = float4(1, 1, 1, 1); //鏡面反射	
};

//バーテックスバッファー出力構造体
struct VS_OUTPUT
{
    float4 pos    : SV_POSITION;
    float4 color  : COLOR0;
    float3 light  : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 eye    : TEXCOORD2;
    float2 tex    : TEXCOORD3;
};


//
//バーテックスシェーダー
//

VS_OUTPUT VS(float4 pos : POSITION,float4 normal : NORMAL, float2 tex : TEXCOORD)// : SV_POSITION
{
    //初期化
    VS_OUTPUT output = (VS_OUTPUT) 0;

	//射影変換（ワールド→ビュー→プロジェクション）
	//法線をワールド空間に
    normal.w        = 0; //w=0で移動成分を反映させない
    output.normal   = mul(normal, g_world);
    output.pos      = mul(pos, g_wvp);

	//ライト方向
    output.light    = g_vLightDir;
	
    //視線ベクトル
    float3 PosWorld = mul(pos, g_world);
    output.eye      = g_vEye - PosWorld;

	//テクスチャー座標
    output.tex = tex;

    return output;
}

//
//ピクセルシェーダー
//
float4 PS(VS_OUTPUT input) : SV_Target
{

    float3 Normal = normalize(input.normal);
    float3 LightDir = normalize(input.light);
    float3 ViewDir = normalize(input.eye);

    //ライトの強さ
    float lightPower = saturate(dot(Normal, LightDir));

    //   拡散反射する光の色を算出
    float3 Reflect = normalize(2 * lightPower * Normal - LightDir);
    
    float4 specular = pow(saturate(dot(Reflect, ViewDir)), 4);


    float4 color = g_texDecal.Sample(g_samLinear, input.tex);
    color += g_diffuse * lightPower + specular * g_specular;
    //color = g_diffuse * lightPower + specular * g_specular;

    return color;
}