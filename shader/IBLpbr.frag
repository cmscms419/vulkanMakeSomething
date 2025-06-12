#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 1) uniform sampler2D Sampler;

layout(binding = 2) uniform MaterialBufferObject {
    float metallic;
    float roughness;
    float r;
    float g;
    float b;
    float a;
} material;

layout(binding = 3) uniform subUinform {
    vec4 camPos;
    vec4 lightPos;
    vec4 objectPos;
} another;


layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragTexCoord;
layout(location = 2) in vec3 WorldPos;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

//#define ROUGHNESS_PATTERN 1

vec3 materialcolor()
{
    return vec3(material.r, material.g, material.b);
}

// Normal Distribution function --------------------------------------
float D_GGX(float dotNH, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
    return (alpha2)/(PI * denom*denom); 
}

// Geometric Shadowing function --------------------------------------
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float GL = dotNL / (dotNL * (1.0 - k) + k);
    float GV = dotNV / (dotNV * (1.0 - k) + k);
    return GL * GV;
}

// Fresnel function ----------------------------------------------------
vec3 F_Schlick(float cosTheta, float metallic)
{
    vec3 F0 = mix(vec3(0.04), materialcolor(), metallic); // * material.specular
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0); 
    return F;    
}

// Specular BRDF composition --------------------------------------------

vec3 BRDF(vec3 L, vec3 V, vec3 N, float metallic, float roughness)
{
    // Precalculate vectors and dot products	
    vec3 H = normalize (V + L);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);
    float dotLH = clamp(dot(L, H), 0.0, 1.0);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);

    // Light color fixed
    vec3 lightColor = vec3(1.0);

    vec3 color = vec3(0.0);

    if (dotNL > 0.0)
    {
        float rroughness = max(0.05, roughness);
        // D = Normal distribution (Distribution of the microfacets)
        float D = D_GGX(dotNH, roughness); 
        // G = Geometric shadowing term (Microfacets shadowing)
        float G = G_SchlicksmithGGX(dotNL, dotNV, rroughness);
        // F = Fresnel factor (Reflectance depending on angle of incidence)
        vec3 F = F_Schlick(dotNV, metallic);

        vec3 spec = D * F * G / (4.0 * dotNL * dotNV);

        color += spec * dotNL * lightColor;
    }

    return color;
}

void main() {

    vec3 N = normalize(fragNormal);
    vec3 V = normalize(another.camPos.xyz - WorldPos);

    float roughness = material.roughness;

        // Add striped pattern to roughness based on vertex position
    #ifdef ROUGHNESS_PATTERN
        roughness = max(roughness, step(fract(WorldPos.y * 2.02), 0.5));
    #endif

    // Specular contribution
    vec3 Lo = vec3(0.0);
    vec3 L = normalize(another.lightPos.xyz - WorldPos);
    
    Lo = BRDF(L, V, N, material.metallic, roughness);

    // Combine with ambient
    vec3 color = materialcolor() * 0.02;
    color += Lo;

    // Gamma correct
    color = pow(color, vec3(0.4545));

    outColor = vec4(Lo, 1.0) + texture(Sampler, fragTexCoord.xy);;
}
