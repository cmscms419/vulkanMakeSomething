#ifndef BRDF_GLSL
#define BRDF_GLSL

#define PI 3.14159265359

// 직접광 한 개의 결과 (UE 의 FDirectLighting)
struct FDirectLighting
{
    vec3 Diffuse;
    vec3 Specular;
};

float Pow4(float X)
{
    float XX = X * X;
    return XX * XX;
}

vec3 Diffuse_Lambert(vec3 DiffuseColor)
{
    return DiffuseColor * (1.0 / PI);
}

float D_GGX(float a2, float NoH)
{
    float d = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (PI * d * d);
}

// Schlick-GGX (Geometry Term)
float Vis_Schlick(float a2, float NoV, float NoL)
{
    float k = sqrt(a2) * 0.5;
    float Vis_SchlickV = NoV * (1.0 - k) + k;
    float Vis_SchlickL = NoL * (1.0 - k) + k;
    return 0.25 / (Vis_SchlickV * Vis_SchlickL);
}

// Schlick 프레넬, Karis 2013 의 spherical gaussian 근사
vec3 F_Schlick(vec3 SpecularColor, float VoH)
{
    float Fc = exp2((-5.55473 * VoH - 6.98316) * VoH);
    return SpecularColor + (vec3(1.0) - SpecularColor) * Fc;
}

// D * Vis * F = (D * G * F) / 4 * (N*L)*(N*V)
vec3 SpecularGGX(float Roughness, vec3 SpecularColor, float NoV, float NoL, float NoH, float VoH)
{
    float a2 = Pow4(Roughness);

    float VisA2 = Pow4((Roughness + 1.0) * 0.5);

    float D = D_GGX(a2, NoH);
    float Vis = Vis_Schlick(VisA2, NoV, NoL);
    vec3 F = F_Schlick(SpecularColor, VoH);

    return (D * Vis) * F;
}

// 기본 셰이딩 모델 (UE 의 DefaultLitBxDF).
// 빛 색과 그림자는 곱하지 않은 값이다 — 호출하는 쪽에서 LightColor * Shadow 를 곱한다.
//   L: 표면 -> 빛 방향 (정규화됨), V: 표면 -> 카메라 방향 (정규화됨)
FDirectLighting DefaultLitBxDF(vec3 DiffuseColor, vec3 SpecularColor, float Roughness, vec3 N, vec3 V, vec3 L)
{
    FDirectLighting Lighting;
    Lighting.Diffuse = vec3(0.0);
    Lighting.Specular = vec3(0.0);

    float NoL = clamp(dot(N, L), 0.0, 1.0);
    if (NoL <= 0.0)
    {
        return Lighting;
    }

    vec3 H = normalize(V + L);
    float NoV = clamp(abs(dot(N, V)) + 1e-5, 0.0, 1.0); // 0 으로 나누지 않도록 (UE 와 같은 처리)
    float NoH = clamp(dot(N, H), 0.0, 1.0);
    float VoH = clamp(dot(V, H), 0.0, 1.0);

    Lighting.Diffuse = NoL * Diffuse_Lambert(DiffuseColor);
    Lighting.Specular = NoL * SpecularGGX(Roughness, SpecularColor, NoV, NoL, NoH, VoH);
    return Lighting;
}

// IBL 스페큘러, split-sum 근사 (Karis 2013 의 ApproximateSpecularIBL)
//   PrefilteredColor * (SpecularColor * A + B)
//   A, B 는 BRDF LUT (genbrdflut.frag 의 IntegrateBRDF 결과), 좌표 = (NoV, Roughness)
// 밉 선택은 선형이다 (prefiltered 큐브맵이 밉마다 거칠기를 선형으로 나눠 만들어졌다고 가정).
vec3 ApproximateSpecularIBL(samplerCube PrefilteredMap, sampler2D BRDFLut,
                            vec3 SpecularColor, float Roughness, vec3 N, vec3 V)
{
    float NoV = clamp(dot(N, V), 0.0, 1.0);
    vec3 R = 2.0 * dot(V, N) * N - V;

    float MipLevel = Roughness * float(textureQueryLevels(PrefilteredMap) - 1);
    vec3 PrefilteredColor = textureLod(PrefilteredMap, R, MipLevel).rgb;
    vec2 EnvBRDF = texture(BRDFLut, vec2(NoV, Roughness)).rg;

    return PrefilteredColor * (SpecularColor * EnvBRDF.x + EnvBRDF.y);
}

#endif // BRDF_GLSL
