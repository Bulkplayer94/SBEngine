cbuffer Common : register(b0)
{
    float4x4 worldMat;
    float4x4 perspectiveMat;
    float4x4 projectionMat;
}

cbuffer Animation : register(b1)
{
    float deltaTime;
}

cbuffer ModelMat : register(b2)
{
    float4x4 modelMat;
}

struct VertexShaderInput
{
    float3 pos : POS;
    float2 uv : TEX;
    float3 norm : NORM;
};

SamplerState samplerState : register(s0); // Texture Sampler
Texture2D volume : register(t0); // Volume Map
Texture2D diffuse : register(t1); // Diffuse Map
Texture2D normal : register(t2); // Normal Map