#include "default.hlsli"

float4 main(PixelShaderInput input) : SV_TARGET
{
    return volume.Sample(samplerState, input.uv);
}