//************************************************/
//* @file  :PMXShader.hlsl
//* @brief :PMX�t�@�C���p�̃V�F�[�_�[
//* @date  :2017/10/05
//* @author:S.Katou
//************************************************/

//�e�N�X�`���\�ƃT���v���[
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

//�R���X�^���g�o�b�t�@
cbuffer CONSTANT_BUFFER : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
};

//�R���X�^���g�o�b�t�@ �}�e���A��
cbuffer MATERIAL_BUFFER : register(b1)
{
    float4 diffuse;       //�g�U�F
    float3 specular;      //���ːF
    float  specularPower; //���ːF�W��
    float4 ambient;       //���F
};


//�o�[�e�b�N�X�V�F�[�_�[
PS_INPUT VS(VS_INPUT input)
{
      // �J�����̈ʒu
    float4 Eye = { 0.0f, 10.0f, -15.0f, 0.0f };
 
    PS_INPUT output = (PS_INPUT) 0;
    output.pos      = mul(input.pos, world);
    output.pos      = mul(output.pos, view);
    output.pos      = mul(output.pos, proj);
    output.normal   = mul(input.normal, world);

    //�e�N�X�`�����W
    output.uv       = input.uv;

    // �J�����̈ʒu - ���_�ʒu
    output.eye      = normalize(Eye - mul(input.pos, world));

    return output;
}


//�s�N�Z���V�F�[�_�[
float4 PS(PS_INPUT input) : SV_Target
{
    // ���C�g����
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