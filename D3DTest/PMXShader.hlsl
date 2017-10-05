//************************************************/
//* @file  :PMXShader.hlsl
//* @brief :PMXファイル用のシェーダー
//* @date  :2017/10/05
//* @author:S.Katou
//************************************************/

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
    float3 lightDirection = { 0, 1, -0.5 };

    // ハーフベクトルを求める
    float3 halfVector = normalize(input.eye + lightDirection);
   
    // 拡散反射色を求める
    float dif = dot(lightDirection, input.normal);
  
    // 鏡面反射色を求める
    float3 spe = pow(max(0, dot(halfVector, normalize(input.normal))), specularPower) * spe.rgb;
   
    // 法線を正規化
    input.normal = normalize(input.normal);
   
    // 環境色を追加
    float4 color = ambient;
  
    // 拡散反射色を追加
    color += diffuse * max(0, dif);
 
    // 鏡面反射色を追加
    //color.rgb  += spe;
    color = saturate(color);
 
    return color;
}