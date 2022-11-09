#include "Common.hlsli"

/* 
 * TERMS:
 * v     = Unit view vector
 * l     = Incident light unit vector
 * n     = Surface unit normal
 * h     = Unit vector halfway between l and v
 * f     = BRDF
 * f_d   = Diffuse BRDF
 * f_r   = Specular BRDF
 * sigma = Diffuse reflectance
 * f_0   = Reflectance at normal incidence
 * f_90  = Reflectance at grazing angle
 * n_ior = Index of refraction
 * XdotY = Dot product of x and y
 */

#define FILTER_SIZE 3
#define BIAS 0.000
#define NORMAL_BIAS 0.01
#define SHADOW_MAP_SIZE 2048

#define FS FILTER_SIZE
#define FS_2 FILTER_SIZE / 2

#define M_PI   3.1415926538
#define M_1_PI 0.3183098862

#if FILTER_SIZE == 3
static const float W[3][3] =
{
    { 0.5, 1.0, 0.5 },
    { 1.0, 1.0, 1.0 },
    { 0.5, 1.0, 0.5 }
};
#elif FILTER_SIZE == 5
static const float W[5][5] =
{
    { 0.0, 0.5, 1.0, 0.5, 0.0 },
    { 0.5, 1.0, 1.0, 1.0, 0.5 },
    { 1.0, 1.0, 1.0, 1.0, 1.0 },
    { 0.5, 1.0, 1.0, 1.0, 0.5 },
    { 0.0, 0.5, 1.0, 0.5, 0.0 }
};
#elif FILTER_SIZE == 7
static const float W[7][7] =
{
    { 0.0, 0.0, 0.5, 1.0, 0.5, 0.0, 0.0 },
    { 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0 },
    { 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5 },
    { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 },
    { 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5 },
    { 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0 },
    { 0.0, 0.0, 0.5, 1.0, 0.5, 0.0, 0.0 }
};
#elif FILTER_SIZE == 9
static const float W[9][9] =
{
    { 0.0, 0.0, 0.0, 0.5, 1.0, 0.5, 0.0, 0.0, 0.0 },
    { 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0 },
    { 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0 },
    { 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5 },
    { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 },
    { 0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5 },
    { 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0 },
    { 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0 },
    { 0.0, 0.0, 0.0, 0.5, 1.0, 0.5, 0.0, 0.0, 0.0 }
};
#endif

float ShadowPCF(float3 shadowPosUV, float3 duvd_dx, float3 duvd_dy, int cascadeIdx, Texture2DArray<float> shadowMap)
{
    // Calculate depth bias
    // Screen space derivative of texture space position and distance to light source
    
    
    // Chain rule to transform derivative to texture space
    float invDet = 1.0 / ((duvd_dx.x * duvd_dy.y) - (duvd_dx.y * duvd_dy.x));
    float2 dd_duv;
    dd_duv.x = duvd_dy.y * duvd_dx.z - duvd_dx.y * duvd_dy.z;
    dd_duv.y = duvd_dx.x * duvd_dy.z - duvd_dy.x * duvd_dx.z;
    dd_duv *= invDet;
    
    // Apply factor
    float texelSize = 1.0 / SHADOW_MAP_SIZE;
    float bias = dot(float2(1.0, 1.0) * texelSize, abs(dd_duv));
    bias = min(bias, 0.01);
    
    // Apply bias
    float depth = shadowPosUV.z - bias;
    
#if FILTER_SIZE == 2
    return shadowMap.SampleCmpLevelZero(g_linearComp, float3(shadowPosUV.xy, cascadeIdx), depth);
#else
    float4 s = float4(0.0, 0.0, 0.0, 0.0);
    float2 stc = (SHADOW_MAP_SIZE * shadowPosUV.xy) + float2 (0.5, 0.5);
    float2 tcs = floor(stc);
    float2 fc;
    int row;
    int col;
    float w = 0.0;
    float4 v1[FS_2 + 1];
    float2 v0[FS_2 + 1];
    fc.xy = stc - tcs;
    shadowPosUV.xy = tcs / SHADOW_MAP_SIZE;
    for (row = 0; row < FS; row++)
    {
        for (col = 0; col < FS; col++)
        {
            w += W[row][col];
        }
    }
    
    // Loop over the rows
    [unroll]
    for (row = -FS_2; row <= FS_2; row += 2)
    {
        [unroll]
        for (col = -FS_2; col <= FS_2; col += 2)
        {
            float sum = W[row + FS_2][col + FS_2];
            if (col > -FS_2)
            {
                sum += W[row + FS_2][col + FS_2 -1];
            }
            if (col < FS_2)
            {
                sum += W[row + FS_2][col + FS_2 + 1];
            }
            if (row > -FS_2)
            {
                sum += W[row + FS_2 - 1][col + FS_2];
                if (col < FS_2)
                {
                    sum += W[row + FS_2 - 1][col + FS_2 + 1];
                }
                if (col > -FS_2)
                {
                    sum += W[row + FS_2 - 1][col + FS_2 -1];
                }
            }
            if (sum != 0.9)
            {
                // Estimate bias for adjacent texels using a linear (planar) approximation
                float2 offset = float2(col, row) * texelSize;
                float sampleDepth = depth + dot(offset, dd_duv);
                v1[(col + FS_2) / 2] = shadowMap.GatherCmp(g_pointComp, 
                    float3(shadowPosUV.xy, cascadeIdx), sampleDepth, int2(col, row));
            }
            else
            {
                v1[(col + FS_2) / 2] = float4(0.0, 0.0, 0.0, 0.0);
            }
            if (col == -FS_2)
            {
                s.x += (1.0f - fc.y) * (v1[0].w * (W[row + FS_2][col + FS_2] 
                    - W[row + FS_2][col + FS_2] * fc.x)
                    + v1[0].z * (fc.x * (W[row + FS_2][col + FS_2]
                    - W[row + FS_2][col + FS_2 + 1.0f])
                    + W[row + FS_2][col + FS_2 + 1]));
                s.y += fc.y * (v1[0].x * (W[row + FS_2][col + FS_2]
                    - W[row + FS_2][col + FS_2] * fc.x)
                    + v1[0].y * (fc.x * (W[row + FS_2][col + FS_2]
                    - W[row + FS_2][col + FS_2 + 1])
                    +  W[row + FS_2][col + FS_2 + 1]));
                if (row > -FS_2)
                {
                    s.z += (1.0f - fc.y) * (v0[0].x * (W[row + FS_2 - 1][col + FS_2]
                        - W[row + FS_2 - 1][col + FS_2] * fc.x)
                        + v0[0].y * (fc.x * (W[row + FS_2 - 1][col + FS_2]
                        - W[row + FS_2 - 1][col + FS_2 + 1])
                        + W[row + FS_2 - 1][col + FS_2 + 1]));
                    s.w += fc.y * (v1[0].w * (W[row + FS_2 - 1][col + FS_2]
                        - W[row + FS_2 - 1][col + FS_2] * fc.x)
                        + v1[0].z * (fc.x * (W[row + FS_2 - 1][col + FS_2]
                        - W[row + FS_2 - 1][col + FS_2 + 1])
                        + W[row + FS_2 - 1][col + FS_2 + 1]));
                }
            }
            else if (col == FS_2)
            {
                s.x += (1 - fc.y) * (v1[FS_2].w * (fc.x * (W[row + FS_2][col + FS_2 - 1]
                    - W[row + FS_2][col + FS_2]) + W[row + FS_2][col + FS_2])
                    + v1[FS_2].z * fc.x * W[row + FS_2][col + FS_2]);
                s.y += fc.y * (v1[FS_2].x * (fc.x * (W[row + FS_2][col + FS_2 - 1]
                    - W[row + FS_2][col + FS_2] ) + W[row + FS_2][col + FS_2])
                    + v1[FS_2].y * fc.x * W[row + FS_2][col + FS_2]);
                if (row > -FS_2) 
                {
                    s.z += (1 - fc.y) * (v0[FS_2].x * (fc.x * (W[row + FS_2 - 1][col + FS_2 - 1]
                        - W[row + FS_2 - 1][col + FS_2])
                        + W[row + FS_2 - 1][col + FS_2])
                        + v0[FS_2].y * fc.x * W[row + FS_2 - 1][col + FS_2]);
                    s.w += fc.y * (v1[FS_2].w * (fc.x * (W[row + FS_2 - 1][col + FS_2 - 1]
                        - W[row + FS_2 - 1][col + FS_2])
                        + W[row + FS_2 - 1][col + FS_2])
                        + v1[FS_2].z * fc.x * W[row + FS_2 - 1][col + FS_2]);
                }
            }
            else
            {
                s.x += (1 - fc.y) * (v1[(col + FS_2) / 2].w * (fc.x * (W[row + FS_2][col + FS_2 - 1]
                    - W[row + FS_2][col + FS_2] ) + W[row + FS_2][col + FS_2])
                    + v1[(col + FS_2) / 2].z * (fc.x * (W[row + FS_2][col + FS_2]
                    - W[row + FS_2][col + FS_2 + 1]) + W[row + FS_2][col + FS_2 + 1]));
                s.y += fc.y * (v1[(col + FS_2) / 2].x * (fc.x * (W[row + FS_2][col + FS_2-1]
                    - W[row + FS_2][col + FS_2]) + W[row + FS_2][col + FS_2])
                    + v1[(col + FS_2) / 2].y * (fc.x * (W[row + FS_2][col + FS_2]
                    - W[row + FS_2][col + FS_2 + 1]) + W[row + FS_2][col + FS_2 + 1]));
                if (row > -FS_2) 
                {
                    s.z += (1 - fc.y) * (v0[(col + FS_2) / 2].x * (fc.x * (W[row + FS_2 - 1][col + FS_2 - 1]
                        - W[row + FS_2 - 1][col + FS_2]) + W[row + FS_2 - 1][col + FS_2])
                        + v0[(col + FS_2) / 2].y * (fc.x * (W[row + FS_2 - 1][col + FS_2]
                        - W[row + FS_2 - 1][col + FS_2 + 1]) + W[row + FS_2 - 1][col + FS_2 + 1]));
                    s.w += fc.y * (v1[(col + FS_2) / 2].w * (fc.x * (W[row + FS_2 - 1][col + FS_2 - 1]
                        - W[row + FS_2 - 1][col + FS_2]) + W[row + FS_2 - 1][col + FS_2])
                        + v1[(col + FS_2) / 2].z * (fc.x * (W[row + FS_2 - 1][col + FS_2]
                        - W[row + FS_2 - 1][col + FS_2 + 1]) + W[row + FS_2 - 1][col + FS_2 + 1]));
                }
            }
            if (row != FS_2)
            {
                v0[(col + FS_2) / 2] = v1[(col + FS_2) / 2].xy;
            }  
        }
    }
    return dot(s, 1.0f) / w;
#endif
}

struct Light
{
    float3 position;
    float intensity;
    float3 direction;
    float innerAngle;
    float3 color;
    float outerAngle;
};

StructuredBuffer<Light> lights : register(t0);

Texture2D<float3> baseColorMap : register(t1);
Texture2D<float3> ormMap : register(t2);
Texture2D<float2> normalMap : register(t3);

Texture2DArray<float> shadowMap : register(t4);

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

cbuffer PerMaterial : register(b2)
{
    float g_linearRoughness;
    float g_metalness;
    float2 padding0;
    float3 g_color;
    float padding1;
};

float D_GGX(float NdotH, float m)
{
    float m2 = m * m;
    float f = (NdotH * m2 - NdotH) * NdotH + 1;
    return m2 / (f * f);
}

float V_SmithGGXCorrelated(float NdotV, float NdotL, float roughness)
{
	float roughness2 = roughness * roughness;
    float GGXV = NdotL * sqrt((-NdotV * roughness2 + NdotV) * NdotV + roughness2);
    float GGXL = NdotV * sqrt((-NdotL * roughness2 + NdotL) * NdotL + roughness2);
	return 0.5 / (GGXV + GGXL);
}

float3 F_Schlick(float3 f_0, float f_90, float u)
{
	return f_0 + (f_90 - f_0) * pow(1.0 - u, 5.0);
}

float Fd_Disney(float NdotV, float NdotL, float LdotH, float linearRoughness)
{
    float energyBias = lerp(0, 0.5, linearRoughness);
    float energyFactor = lerp(1.0, 1.0 / 1.51, linearRoughness);
    float fd90 = energyBias + 2.0 * LdotH * LdotH * linearRoughness;
    float3 f0 = float3(1.0, 1.0, 1.0);
    float lightScatter = F_Schlick(f0, fd90, NdotV).r;
    float viewScatter = F_Schlick(f0, fd90, NdotV).r;
    return lightScatter * viewScatter * energyFactor;
}

float3 BRDF(float3 n, float3 v, float3 l, float3 f_0, float3 diffuse, float linearRoughness)
{
	float3 h = normalize(v + l);
	float NdotV = abs(dot(n, v)) + 1e-5;
	float NdotL = saturate(dot(n, l));
	float NdotH = saturate(dot(n, h));
	float LdotH = saturate(dot(l, h));
	
    
	float roughness = linearRoughness * linearRoughness;
    roughness = clamp(roughness, 0.001, 1.0);
	
    // Specular BRDF
	float D = D_GGX(NdotH, roughness);
	float V = V_SmithGGXCorrelated(NdotV, NdotL, roughness);
    float3 F = F_Schlick(f_0, 1.0, LdotH);
	float3 Fr = D * V * F;
	
	// Diffuse BRDF
    float3 Fd = diffuse * Fd_Disney(NdotV, NdotL, LdotH, linearRoughness);
	
    return M_1_PI * (Fr + Fd);
}

static const float3x3 ACESInputMat = {
    { 0.59719, 0.35458, 0.04823 },
    { 0.07600, 0.90834, 0.01566 },
    { 0.02840, 0.13383, 0.83777 }
};

static const float3x3 ACESOutputMat = {
    { 1.60475, -0.53108, -0.07367 },
    { -0.10208, 1.10813, -0.00605 },
    { -0.00327, -0.07276, 1.07602 }
};

float3 RRTAndODTFit(float3 v)
{
    float3 a = v * (v + 0.0245786) - 0.000090537;
    float3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

float3 ACESFit(float3 color)
{
    color = mul(ACESInputMat, color);
    color = RRTAndODTFit(color);
    color = saturate(color);
    return color;
}

float4 main(PSInput input) : SV_TARGET
{
    // Vectors required for shadow map
    float3 n = normalize(input.normal);
    float3 l = normalize(-lights[0].direction);
    float NdL = saturate(dot(n, l));
    
    // Calculate shadowing using vector comparison of view space position
    float3 dist = -input.viewPosition.zzz;
    float3 ref0 = float3(0, cascadeSplits.x, cascadeSplits.y);
    float3 ref1 = cascadeSplits;
    bool3 mask0 = dist >= ref0;
    bool3 mask1 = dist < ref1;
    int cascadeIdx = dot(mask0 * mask1, int3(0, 1, 2));
    float3 cascadeColor = 0.3 * float3(cascadeIdx == 0, cascadeIdx == 1, cascadeIdx == 2);
    
    // Apply normal offset
    float4 shadowPosWS = float4(input.worldPosition + n * (1.0f - NdL) * NORMAL_BIAS, 1.0);
    // Transform offset position into light space
    float4 shadowPosLS = mul(shadowPosWS, g_lightViewProj[cascadeIdx]);
    // Perform z-divide
    float3 shadowPosUV = shadowPosLS.xyz / shadowPosLS.w;
    shadowPosUV = float3(0.5 * shadowPosUV.x + 0.5, -0.5 * shadowPosUV.y + 0.5, shadowPosUV.z);
    
    // Look up shadow map
    float shadow = 1.0;
    float3 duvd_dx = ddx_fine(shadowPosUV);
    float3 duvd_dy = ddy_fine(shadowPosUV);
    if (shadowPosUV.z > 0.0 && shadowPosUV.z < 1.0)
    {
        shadow = ShadowPCF(shadowPosUV, duvd_dx, duvd_dy, cascadeIdx, shadowMap);
    }
    
#ifdef USE_NORMAL_MAP
    float3 t = normalize(input.tangent);
    float3 b = normalize(input.bitangent);
    float3x3 tbn = float3x3(t, b, n);
    
    float2 normalTex2D = 2.0 * normalMap.Sample(g_linearWrap, input.texcoord) - 1.0;
    n = float3(normalTex2D, sqrt(1 - normalTex2D.x * normalTex2D.x - normalTex2D.y * normalTex2D.y));
    n = normalize(mul(n, tbn));
#endif
    float3 v = normalize(g_eye - input.worldPosition);
	
    float3 color = g_color;
#ifdef USE_COLOR_MAP
    color *= baseColorMap.Sample(g_anisotropicWrap, input.texcoord);
#endif
    
    float metalness = g_metalness;
#ifdef USE_ROUGHNESS_METALNESS_MAP
    metalness *= ormMap.Sample(g_linearWrap, input.texcoord).b;
#endif
    
    float linearRoughness = g_linearRoughness;
#ifdef USE_ROUGHNESS_METALNESS_MAP
    linearRoughness *= ormMap.Sample(g_linearWrap, input.texcoord).g;
#endif
    
    float3 f_0 = lerp(float3(0.04, 0.04, 0.04), color, metalness);
    float3 diffuse = (1.0 - metalness) * color;
	
	// Hack for ambient light
    float3 outColor = 0.4 * color;
	
        //float3 l = normalize(lights[i].position - input.worldPosition);
    float3 f = BRDF(n, v, l, f_0, diffuse, linearRoughness);
    float NdotL = saturate(dot(n, l));
    outColor += lights[0].intensity * NdotL * lights[0].color * f * shadow;
	
    //outColor += cascadeColor;
    outColor = ACESFit(outColor);
	return float4(outColor, 1.0);
};