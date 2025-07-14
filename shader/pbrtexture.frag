#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 inverseTranspose;
    mat4 view;
    mat4 proj;
} ubo;

layout(std140, binding = 1) uniform MaterialBufferObject {
    layout(offset = 0) float metallic;
    layout(offset = 4) float roughness;
    layout(offset = 8) float r;
    layout(offset = 12) float g;
    layout(offset = 16) float b;
    layout(offset = 20) float a;
    layout(offset = 24) bool isAlbedoMap;
    layout(offset = 28) bool isNormalMap;
    layout(offset = 32) bool isMetallic;
    layout(offset = 36) bool isRoughness;
} material;

layout(std140 ,binding = 2) uniform subUinforms {
    // camPos를 0바이트 위치에 배치
    layout(offset = 0) vec3 camPos;

    // lights 배열을 16바이트 위치부터 배치
    // vec4는 16바이트 크기이므로, lights[0]은 16, 
    layout(offset = 16) vec4 lights[4];

    // lights[3]이 끝나는 위치는 16 + (16*3) + 16 = 80
    // exposure를 80바이트 위치에 배치
    layout(offset = 80) float exposure;


    // gamma를 84바이트 위치에 배치
    layout(offset = 84) float gamma;

    // bool은 4바이트 크기로 간주하여 배치
    layout(offset = 88) bool useTexture;
    layout(offset = 92) bool brdfLUTTexture;
    layout(offset = 96) bool prefilteredCubeTexture;
    layout(offset = 100) bool irradianceCubeTexture;
} subUinform;

layout(location = 0) in vec3 Normal;
layout(location = 1) in vec3 fragTexCoord;
layout(location = 2) in vec3 WorldPos;
layout(location = 3) in vec4 inTangent; // Tangent vector

layout (binding = 3) uniform samplerCube samplerIrradiance;
layout (binding = 4) uniform sampler2D samplerBRDFLUT;
layout (binding = 5) uniform samplerCube prefilteredMap;

layout (binding = 6) uniform sampler2D albedoMap;
layout (binding = 7) uniform sampler2D aoMap;
layout (binding = 8) uniform sampler2D heightMap;
layout (binding = 9) uniform sampler2D metallicMap;
layout (binding = 10) uniform sampler2D normalMap;
layout (binding = 11) uniform sampler2D roughnessMap;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;
#define ALBEDO pow(texture(albedoMap, fragTexCoord.xy).rgb, vec3(2.2))

// From http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 Uncharted2Tonemap(vec3 color)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
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
vec3 F_Schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// Specular specularContribution --------------------------------------------

vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness)
{
    // Precalculate vectors and dot products	
    vec3 H = normalize (V + L);
    float dotNH = clamp(dot(N, H), 0.0, 1.0);
    float dotNV = clamp(dot(N, V), 0.0, 1.0);
    float dotNL = clamp(dot(N, L), 0.0, 1.0);

    // Light color fixed
    vec3 lightColor = vec3(1.0);

    vec3 color = vec3(0.0);

    if (dotNL > 0.0) {
        // D = Normal distribution (Distribution of the microfacets)
        float D = D_GGX(dotNH, roughness); 
        // G = Geometric shadowing term (Microfacets shadowing)
        float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
        // F = Fresnel factor (Reflectance depending on angle of incidence)
        vec3 F = F_Schlick(dotNV, F0);		
        vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);		
        vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);			
        color += (kD * ALBEDO / PI + spec) * dotNL;
    }

    return color;
}

vec3 prefilteredReflection(vec3 R, float roughness)
{
    const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
    float lod = roughness * MAX_REFLECTION_LOD;
    float lodf = floor(lod);
    float lodc = ceil(lod);
    vec3 a = textureLod(prefilteredMap, R, lodf).rgb;
    vec3 b = textureLod(prefilteredMap, R, lodc).rgb;
    return mix(a, b, lod - lodf);
}


vec3 calculateNormal()
{
    vec3 tangentNormal = texture(normalMap, fragTexCoord.xy).xyz * 2.0;

    vec3 N = normalize(Normal);
    vec3 T = normalize(inTangent.xyz);
    vec3 B = normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangentNormal);
}

void main() {

    vec3 N = calculateNormal();
    vec3 CameraPos = subUinform.camPos;
    vec3 V = normalize(CameraPos - WorldPos);
    vec3 R = reflect(-V, N); 


    float metallic = 1.0;
    
    if (material.isMetallic){
        metallic = texture(metallicMap, fragTexCoord.xy).r;
    }
    
    float roughness = 1.0;
    if (material.isRoughness){
        roughness = texture(roughnessMap, fragTexCoord.xy).r;
    }
    
    vec3 F0 = vec3(0.04); 
	F0 = mix(F0, ALBEDO, metallic);

    // Specular contribution
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < subUinform.lights.length(); i++) {
        vec3 L = normalize(subUinform.lights[i].xyz - WorldPos);
        Lo += specularContribution(L, V, N, F0, metallic, roughness);
    }

    vec2 brdf = vec2(1.0, 1.0);

    if (subUinform.brdfLUTTexture) {
        brdf = texture(samplerBRDFLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    }

    vec3 reflection = vec3(1.0, 1.0, 1.0);

    if (subUinform.prefilteredCubeTexture) {
        reflection = prefilteredReflection(R, roughness).rgb;
    }
    
    vec3 irradiance = vec3(1.0, 1.0, 1.0);
    
    if (subUinform.irradianceCubeTexture){
        // Sample the irradiance map
        irradiance = texture(samplerIrradiance, N).rgb;
    }
    
    // Diffuse based on irradiance
    vec3 diffuse = irradiance * ALBEDO;

    vec3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);

    // Specular reflectance
    vec3 specular = reflection * (F * brdf.x + brdf.y);

    // Ambient part
    vec3 kD = 1.0 - F;
    kD *= 1.0 - metallic;	  
    vec3 ambient = (kD * diffuse + specular) * texture(aoMap, fragTexCoord.xy).rrr;
    
    vec3 color = ambient + Lo;

    // Tone mapping
    color = Uncharted2Tonemap(color * subUinform.exposure);
    color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	

    // Gamma correction
    color = pow(color, vec3(1.0f / subUinform.gamma));

    outColor = vec4(color, 1.0);
}
