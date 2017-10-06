//************************************************/
//* @file  :PMXShader.hlsl
//* @brief :PMXファイル用のシェーダー
//* @date  :2017/10/05
//* @author:S.Katou
//************************************************/

//テクスチャ―とサンプラー
Texture2D g_texture    : register(t0);
SamplerState g_sampler : register(s0);

struct VS_INPUT
{
    float4 pos    : POSITION;
    float3 normal : NORMAL;
    float2 uv     : TEXCOORD;
};

struct PS_INPUT
{
    float4 pos    : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv     : TEXCOORD1;
    float4 eye    : TEXCOORD2;
};

//コンスタントバッファ
cbuffer CONSTANT_BUFFER : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
};

//コンスタントバッファ マテリアル
cbuffer MATERIAL_BUFFER : register(b1)
{
    float4 diffuse;       //拡散色
    float3 specular;      //反射色
    float  specularPower; //反射色係数
    float4 ambient;       //環境色
};


//バーテックスシェーダー
PS_INPUT VS(VS_INPUT input)
{
      // カメラの位置
    float4 Eye = { 0.0f, 10.0f, -15.0f, 0.0f };
 
    PS_INPUT output = (PS_INPUT) 0;
    output.pos      = mul(input.pos, world);
    output.pos      = mul(output.pos, view);
    output.pos      = mul(output.pos, proj);
    output.normal   = mul(input.normal, world);

    //テクスチャ座標
    output.uv       = input.uv;

    // カメラの位置 - 頂点位置
    output.eye      = normalize(Eye - mul(input.pos, world));

    return output;
}


//ピクセルシェーダー
float4 PS(PS_INPUT input) : SV_Target
{
    // ライト方向
    float3 lightDirection = { 0, 2, -0.5 };
    	
    float3 LightDir = normalize(lightDirection);
    float3 ViewDir = normalize(input.eye);
    float4 NL = saturate(dot(input.normal, LightDir))/2;
	
    float3 Reflect = normalize(NL * input.normal - LightDir);
    float4 specular = pow(saturate(dot(Reflect, ViewDir)), 4);

    float4 color;
    color = g_texture.Sample(g_sampler, input.uv);
    color += diffuse * NL + specular * specular;
	
    //return g_texture.Sample(g_sampler, input.uv);
    return color;
}