#ifndef COMMON_HLSL
#define COMMON_HLSL

cbuffer PerObject : register(b0)
{
    float4x4 g_model;
};

cbuffer PerFrame : register(b1)
{
    int g_nlights;
    float3 g_eye;
    float4x4 g_viewProj;
    float4x4 g_view;
    float4x4 g_lightViewProj[3];
    float3 cascadeSplits;
    float pad;
};

SamplerState g_linearWrap : register(s0);
SamplerState g_linearClamp : register(s1);
SamplerState g_anisotropicWrap : register(s2);
SamplerState g_anisotropicClamp : register(s3);
SamplerState g_pointWrap : register(s4);
SamplerState g_pointClamp : register(s5);
SamplerComparisonState g_pointComp : register(s6);
SamplerComparisonState g_linearComp : register(s7);

#endif