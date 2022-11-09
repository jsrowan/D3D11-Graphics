#include "Common.hlsli"

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
#ifdef HAS_TEXCOORDS
    float2 texcoord : TEXCOORD;
#endif
#ifdef HAS_TANGENTS
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
#endif
    uint instance : SV_InstanceID;
};

struct VSOutput
{
    float4 position : SV_Position;
    uint renderTarget : SV_RenderTargetArrayIndex;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.position = mul(mul(float4(input.position, 1.0), g_model), g_lightViewProj[input.instance]);
    output.position.z = max(output.position.z, 0.0);
    output.renderTarget = input.instance;
    return output;
}