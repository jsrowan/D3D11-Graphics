#include "Common.hlsli"

#define FLT_MAX 1E+37
#define FLT_MIN 1E-38

#define NUMTHREADS_1D 8
#define NUMTHREADS (NUMTHREADS_1D * NUMTHREADS_1D)

[numthreads(1, 1, 1)]
void main(uint3 globalID : SV_DispatchThreadID, uint3 localID : SV_GroupThreadID, uint3 groupID : SV_GroupID)
{
    
}