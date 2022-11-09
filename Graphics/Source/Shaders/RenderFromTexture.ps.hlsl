#include "Common.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

Texture2D<float3> tex : register(t0);

float4 main(PSInput input) : SV_TARGET
{
    return float4(tex.Sample(g_pointClamp, input.texcoord), 1.0);
}