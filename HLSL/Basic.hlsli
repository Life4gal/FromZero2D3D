#include "LightHelper.hlsli"

// Texture2D类型保存了2D纹理的信息，在这是全局变量。而register(t0)对应起始槽索引0.
Texture2D g_DiffuseMap : register(t0);
// SamplerState类型确定采样器应如何进行采样，同样也是全局变量，register(s0)对应起始槽索引0.
SamplerState g_Sam : register(s0);

cbuffer CBChangesEveryInstanceDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
}

cbuffer CBChangesEveryObjectDrawing : register(b1)
{
    Material g_Material;
}

cbuffer CBDrawingStates : register(b2)
{
    int g_TextureUsed;
    float3 g_Pad;
}

cbuffer CBChangesEveryFrame : register(b3)
{
    matrix g_View;
    float3 g_EyePosW;
    float g_Pad2;
}

cbuffer CBChangesOnResize : register(b4)
{
    matrix g_Proj;
}

cbuffer CBChangesRarely : register(b5)
{
    DirectionalLight g_DirLight[5];
    PointLight g_PointLight[5];
    SpotLight g_SpotLight[5];
}

struct VertexPosNormalTex
{
	float3 PosL : POSITION;
    float3 NormalL : NORMAL;
	float2 Tex : TEXCOORD;
};

struct VertexPosHWNormalTex
{
	float4 PosH : SV_POSITION;
    float3 PosW : POSITION;     // 在世界中的位置
    float3 NormalW : NORMAL;    // 法向量在世界中的方向
	float2 Tex : TEXCOORD;
};

struct InstancePosNormalTex
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
    matrix World : World;
    matrix WorldInvTranspose : WorldInvTranspose;
};










