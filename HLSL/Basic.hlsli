#include "LightHelper.hlsli"

// Texture2D类型保存了2D纹理的信息，在这是全局变量。而register(t0)对应起始槽索引0.
Texture2D g_Tex : register(t0);
// SamplerState类型确定采样器应如何进行采样，同样也是全局变量，register(s0)对应起始槽索引0.
SamplerState g_SamLinear : register(s0);

cbuffer CBChangesEveryDrawing : register(b0)
{
    matrix g_World;
    matrix g_WorldInvTranspose;
    Material g_Material;
}

cbuffer CBDrawingStates : register(b1)
{
    int g_IsReflection;
    int g_IsShadow;
}

cbuffer CBChangesEveryFrame : register(b2)
{
    matrix g_View;
    float3 g_EyePosW;
}

cbuffer CBChangesOnResize : register(b3)
{
    matrix g_Proj;
}

cbuffer CBChangesRarely : register(b4)
{
    matrix g_Reflection;
    matrix g_Shadow;
    matrix g_RefShadow;
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

struct VertexPosTex
{
    float3 PosL : POSITION;
    float2 Tex : TEXCOORD;
};

struct VertexPosHWNormalTex
{
	float4 PosH : SV_POSITION;
    float3 PosW : POSITION;     // 在世界中的位置
    float3 NormalW : NORMAL;    // 法向量在世界中的方向
	float2 Tex : TEXCOORD;
};

struct VertexPosHTex
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD;
};










