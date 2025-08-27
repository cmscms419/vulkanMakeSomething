#version 450

layout (binding = 1) uniform samplerCube samplerCubeMap;

layout (binding = 2) uniform UBOParams {
    vec4 lights[4];
    float exposure;
    float gamma;
} uboParams;

layout (location = 0) in vec3 inUVW;

layout (location = 0) out vec4 outFragColor;

// From http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 Uncharted2Tonemap(vec3 color)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

void main() 
{
    // Sample the environment map
    vec3 tex = inUVW;
    tex.y = -tex.y; // Flip the y axis  

    vec3 color = texture(samplerCubeMap, tex).rgb;

    // Tone mapping
    color = Uncharted2Tonemap(color * uboParams.exposure);
    color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
    // Gamma correction
    color = pow(color, vec3(1.0f / uboParams.gamma));
    
    outFragColor = vec4(color, 1.0);

}