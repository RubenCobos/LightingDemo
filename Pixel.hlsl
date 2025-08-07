#include "Data.hlsl"

static const int PARALLEL = 0;
static const int POINT = 1;
static const int SPOT = 2;

// LIGHTING CALCULATIONS

float4 CalculateLighting(in VertexOut v, in int LightType)
{
    float4 ambientReflection = { 0.0f, 0.0f, 0.0f, 1.0f };
    float4 totalReflection = { 0.0f, 0.0f, 0.0f, 1.0f };
    float3 lightVector = { 0.0f, 0.0f, 0.0f };
    float attenuation = 1.0f;
    float spotFade = 1.0;
    
    // directional light
    
    if (LightType == PARALLEL)
    {
        lightVector = (float3) -directLight;
        ambientReflection = ambient * DiffuseAlbedo;
    }
    
    // point light
    
    if (LightType == POINT)
    {
        lightVector = (float3) (pointLight - v.positionW);
        float distance = length(lightVector);
        
        if (distance >= 50.0f)
        {
            return totalReflection;
        }
        else
        {
            attenuation = clamp((50.0f - distance) / (50.0f - 1.0f), 0.0f, 1.0f);
        }
        
        lightVector = normalize(lightVector);
    }
    
    // spot light
    
    if (LightType == SPOT)
    {
        lightVector = (float3) (spotLight - v.positionW);
        float distance = length(lightVector);
        
        if (distance >= 30.0f)
        {
            return totalReflection;
        }
        else
        {
            attenuation = clamp((30.0f - distance) / (30.0f - 1.0f), 0.0f, 1.0f);
        }
        
        lightVector = normalize(lightVector);
        spotFade = dot(-lightVector, (float3) spotAim);
        
        if (spotFade < 0.0f)
        {
            return totalReflection;
        }
        else
        {
            spotFade = pow(spotFade, 8.0f);
        }
    }
    
    v.normal = normalize(v.normal);
    float3 viewVector = normalize((float3) (camera - v.positionW));
    float3 halfVector = normalize(lightVector + viewVector);
    float LambCos = clamp(dot(lightVector, v.normal), 0.0f, 1.0f);
    float toEyeReflection = clamp(dot(lightVector, halfVector), 0.0f, 1.0f);
    float MicroNormals = clamp(dot(halfVector, v.normal), 0.0f, 1.0f);
    float4 diffuseReflection = (spotFade * attenuation * LambCos * color) * DiffuseAlbedo;
    float4 Fresnel = SpecProperty + (1.0f - SpecProperty) * pow((1.0f - toEyeReflection), 5.0f);
    float4 specularReflection = (spotFade * attenuation * LambCos * color) * (Fresnel * pow(MicroNormals, roughness));
    totalReflection = ambientReflection + diffuseReflection + specularReflection;
    return totalReflection;
}

// PIXEL SHADER

float4 PS(in VertexOut v) : SV_Target
{
    float4 color = { 0.0f, 0.0f, 0.0f, 0.0f };
    color += CalculateLighting(v, PARALLEL);
    color += CalculateLighting(v, POINT);
    color += CalculateLighting(v, SPOT);
    color.w = DiffuseAlbedo.w;
    return color;
}
