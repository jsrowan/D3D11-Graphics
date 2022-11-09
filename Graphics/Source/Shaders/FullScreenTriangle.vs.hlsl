#include "Common.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

PSInput main(uint id : SV_VertexID)
{
    PSInput output;
    output.texcoord.x = (id == 1) ? 2.0 : 0.0;
    output.texcoord.y = (id == 2) ? 2.0 : 0.0;
    output.position.x = (id == 1) ? 3.0 : -1.0;
    output.position.y = (id == 2) ? -3.0 : 1.0;
    output.position.zw = float2(1.0, 1.0);
    return output;
}