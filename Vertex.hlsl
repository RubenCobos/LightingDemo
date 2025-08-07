#include "Data.hlsl"

// VERTEX SHADER

VertexOut VS(in VertexIn v1)
{
    VertexOut v2;
    float4x4 NetTransform = mul(mul(WorldTransform, ViewTransform), ProjectionTransform);
    v2.position = mul(v1.position, NetTransform);
    v2.positionW = mul(v1.position, WorldTransform);
    v2.normal = mul((float3) v1.normal, (float3x3) WorldTransform);
    return v2;
}
