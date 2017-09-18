//************************************************/
//* @file  :Simple.hlsl
//* @brief :�V�F�[�_�[�̗��K
//* @date  :2017/08/27
//* @author:S.Katou
//************************************************/

//�O���[�o��
Texture2D g_texDecal : register(t0);
SamplerState g_samLinear : register(s0);

//�O���[�o��
cbuffer global_0 : register(b0)
{
    float4 g_vLightDir;  //���C�g�̕����x�N�g��
    float4 g_vEye;       //�J�����ʒu
};

cbuffer global_1 : register(b1)
{
    matrix g_world;                         //���[���h�s��
    matrix g_wvp;                           //���[���h����ˉe�܂ł̕ϊ��s��
    float4 g_ambient  = float4(0, 0, 0, 0); //�A���r�G���g��
    float4 g_diffuse  = float4(1, 0, 0, 0); //�g�U����(�F�j
    float4 g_specular = float4(1, 1, 1, 1); //���ʔ���	
};

//�o�[�e�b�N�X�o�b�t�@�[�o�͍\����
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
//�o�[�e�b�N�X�V�F�[�_�[
//

VS_OUTPUT VS(float4 pos : POSITION,float4 normal : NORMAL, float2 tex : TEXCOORD)// : SV_POSITION
{
    //������
    VS_OUTPUT output = (VS_OUTPUT) 0;

	//�ˉe�ϊ��i���[���h���r���[���v���W�F�N�V�����j
	//�@�������[���h��Ԃ�
    normal.w        = 0; //w=0�ňړ������𔽉f�����Ȃ�
    output.normal   = mul(normal, g_world);
    output.pos      = mul(pos, g_wvp);

	//���C�g����
    output.light    = g_vLightDir;
	
    //�����x�N�g��
    float3 PosWorld = mul(pos, g_world);
    output.eye      = g_vEye - PosWorld;

	//�e�N�X�`���[���W
    output.tex = tex;

    return output;
}

//
//�s�N�Z���V�F�[�_�[
//
float4 PS(VS_OUTPUT input) : SV_Target
{

    float3 Normal = normalize(input.normal);
    float3 LightDir = normalize(input.light);
    float3 ViewDir = normalize(input.eye);

    //���C�g�̋���
    float lightPower = saturate(dot(Normal, LightDir));

    //   �g�U���˂�����̐F���Z�o
    float3 Reflect = normalize(2 * lightPower * Normal - LightDir);
    
    float4 specular = pow(saturate(dot(Reflect, ViewDir)), 4);


    float4 color = g_texDecal.Sample(g_samLinear, input.tex);
    color += g_diffuse * lightPower + specular * g_specular;
    //color = g_diffuse * lightPower + specular * g_specular;

    return color;
}