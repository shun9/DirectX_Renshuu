//************************************************/
//* @file  :Simple.hlsl
//* @brief :シェーダーの練習
//* @date  :2017/08/13
//* @author:S.Katou
//************************************************/

//グローバル
cbuffer global
{
    matrix g_world;
    matrix g_mWVP; //ワールドから射影までの変換行列
    float4 g_lightDir; //ライトの方向
    float4 g_diffuse=float4(1,0,0,0); //拡散反射
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
};

//
//バーテックスシェーダー
//
VS_OUTPUT VS(float4 pos : POSITION, float4 normal : NORMAL)// : SV_POSITION
{ 
    VS_OUTPUT output=(VS_OUTPUT)0;
    output.Pos = mul(pos, g_mWVP);
    normal.w = 0;
    normal = mul(normal, g_world);
    normal = normalize(normal);

    output.Color = 1.0f*g_diffuse*dot(normal,g_lightDir);

    return output;
}
//
//ピクセルシェーダー
//
float4 PS( VS_OUTPUT input ) : SV_Target
{
    return input.Color;
}