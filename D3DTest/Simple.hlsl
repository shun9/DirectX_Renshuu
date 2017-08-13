//************************************************/
//* @file  :Simple.hlsl
//* @brief :�V�F�[�_�[�̗��K
//* @date  :2017/08/13
//* @author:S.Katou
//************************************************/

//�O���[�o��
cbuffer global
{
    matrix g_world;
    matrix g_mWVP; //���[���h����ˉe�܂ł̕ϊ��s��
    float4 g_lightDir; //���C�g�̕���
    float4 g_diffuse=float4(1,0,0,0); //�g�U����
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
};

//
//�o�[�e�b�N�X�V�F�[�_�[
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
//�s�N�Z���V�F�[�_�[
//
float4 PS( VS_OUTPUT input ) : SV_Target
{
    return input.Color;
}