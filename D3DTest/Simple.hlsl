//************************************************/
//* @file  :Simple.hlsl
//* @brief :�V�F�[�_�[�̗��K
//* @date  :2017/08/16
//* @author:S.Katou
//************************************************/

Texture2D g_texDecal : register(t0); //�e�N�X�`���[�� ���W�X�^�[t(n)
SamplerState g_samLinear : register(s0); //�T���v���[�̓��W�X�^�[s(n)

//�O���[�o��
cbuffer global
{
    //matrix g_world;                        //���[���h�s��
    matrix g_mWVP;                         //���[���h����ˉe�܂ł̕ϊ��s��
    //float4 g_lightDir;                     //���C�g�̕���
    //float4 g_diffuse = float4(1, 0, 0, 0); //�g�U����
                                           //float4 g_eye;                     //�J�����ʒu
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
//�o�[�e�b�N�X�V�F�[�_�[
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
//�s�N�Z���V�F�[�_�[
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