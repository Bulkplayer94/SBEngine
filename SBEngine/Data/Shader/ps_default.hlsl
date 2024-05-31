#include "default.hlsli"

float4 main(PixelShaderInput input) : SV_Position
{
    return volume.Sample(samplerState, input.uv);
}