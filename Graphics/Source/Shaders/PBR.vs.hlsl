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
#endif
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldPosition : WORLD_POSITION;
    float3 viewPosition : VIEW_POSITION;
    float3 normal : NORMAL;
#ifdef HAS_TEXCOORDS
    float2 texcoord : TEXCOORD;
#endif
#ifdef HAS_TANGENTS
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
#endif
};

PSInput main(VSInput input)
{
	PSInput output;

    float4 worldPosition = mul(float4(input.position, 1.0), g_model);
    output.worldPosition = worldPosition.xyz;
    output.position = mul(worldPosition, g_viewProj);
    output.viewPosition = mul(worldPosition, g_view).xyz;
    output.normal = mul(float4(input.normal, 0.0), g_model).xyz;
#ifdef HAS_TEXCOORDS
    output.texcoord = input.texcoord;
#endif
#ifdef HAS_TANGENTS
    output.tangent = mul(float4(input.tangent, 0.0), g_model).xyz;
    output.bitangent = cross(output.tangent, output.normal);
#endif
    return output;
}