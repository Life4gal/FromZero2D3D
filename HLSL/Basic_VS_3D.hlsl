#include "Basic.hlsli"

// 顶点着色器(3D)
VertexPosHWNormalTex VS_3D(VertexPosNormalTex vIn)
{
    VertexPosHWNormalTex vOut;                  //定义传出顶点属性
    
    matrix viewProj = mul(g_View, g_Proj);      //视图投影矩阵
    float4 posW = mul(float4(vIn.PosL, 1.0f), g_World); //计算顶点的世界坐标
    float3 normalW = mul(vIn.NormalL, (float3x3) g_WorldInvTranspose); //法线变换
    
    // 若当前在绘制反射物体，先进行反射操作
    [flatten]
    if (g_IsReflection)
    {
        posW = mul(posW, g_Reflection);
        normalW = mul(normalW, (float3x3) g_Reflection);
    }
    
    vOut.PosH = mul(posW, viewProj);            //计算顶点MVP变换后的齐次坐标
    vOut.PosW = posW.xyz;                       //保存世界坐标
    vOut.NormalW = normalW;
    vOut.Tex = vIn.Tex;                         // 纹理不需要变换，原样赋值
    return vOut;
}
