// INPUT REGISTER DATA

cbuffer Transformation : register(b0)
{
    float4x4 WorldTransform;
    float4x4 ViewTransform;
    float4x4 ProjectionTransform;
}

cbuffer Material : register(b1)
{
    float4 DiffuseAlbedo;
    float4 SpecProperty;
    int roughness;
}

cbuffer Light : register(b2)
{
    float4 directLight;
    float4 pointLight;
    float4 spotLight;
    float4 spotAim;
    float4 color;
    float4 ambient;
    float4 camera;
}

struct VertexIn
{
    float4 position : POSITION;
    float4 normal : NORMAL;
};

struct VertexOut
{
    float4 position : SV_POSITION;
    float4 positionW : POSITION;
    float3 normal : NORMAL;
};
