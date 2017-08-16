//************************************************/
//* @file  :Simple.hlsl
//* @brief :シェーダーの練習
//* @date  :2017/08/16
//* @author:S.Katou
//************************************************/

Texture2D g_texDecal : register(t0); //テクスチャーは レジスターt(n)
SamplerState g_samLinear : register(s0); //サンプラーはレジスターs(n)

//グローバル
cbuffer global
{
    //matrix g_world;                        //ワールド行列
    matrix g_mWVP;                         //ワールドから射影までの変換行列
    //float4 g_lightDir;                     //ライトの方向
    //float4 g_diffuse = float4(1, 0, 0, 0); //拡散反射
                                           //float4 g_eye;                     //カメラ位置
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    //float2 Tex : TEXCOORD;
    //float3 Light : TEXCOORD;
    //float3 Normal : TEXCOORD1;
    //float3 Eye : TEXCOORD2;
};

//
//バーテックスシェーダー
//
VS_OUTPUT VS(float4 pos : POSITION)// : SV_POSITION
{
    VS_OUTPUT output = (VS_OUTPUT) 0;

    output.Pos = mul(pos, g_mWVP);
    //output.Tex = tex;
    //output.Normal    = mul(normal, (float3x3)g_world);
    //output.Light     = g_lightDir;

    //float3 posWorld = mul(pos, g_world);
    //output.Eye       = g_eye - posWorld;

    return output;
}
//
//ピクセルシェーダー
//
float4 PS(VS_OUTPUT input) : SV_Target
{
    //float3 normal   = normalize(input.Normal);
    //float3 lightDir = normalize(input.Light);
    //float3 viewDir  = normalize(input.Eye);
    //float4 nl       = saturate(dot(normal, lightDir));

    //float3 reflect  = normalize(2 * nl * normal - lightDir);
    //float4 specular = 2 * pow(saturate(dot(reflect, viewDir)), 2);

    //return g_texDecal.Sample(g_samLinear, input.Tex);
    return float4(1.0f, 1.0f, 0.0f, 1.0f);
}