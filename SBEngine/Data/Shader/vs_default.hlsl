#include "default.hlsli"

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    output.pos = mul(modelMat, float4(input.pos, 1.0F));
    output.uv = input.uv;
    output.norm = input.norm;
    
    return output;
}