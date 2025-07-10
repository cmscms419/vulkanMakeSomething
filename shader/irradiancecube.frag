// Generates an irradiance cube from an environment map using convolution

#version 450

layout (location = 0) in vec3 inPos;
layout (location = 0) out vec4 outColor;
layout (binding = 0) uniform samplerCube samplerEnv;

layout(std140, push_constant) uniform PushConsts {
    layout (offset = 64) float deltaPhi;
    layout (offset = 68) float deltaTheta;
} consts;

#define PI 3.1415926535897932384626433832795

void main()
{
    vec3 N = normalize(inPos); // normal vector
    vec3 up = vec3(0.0, 1.0, 0.0); 
    vec3 right = normalize(cross(up, N)); // right vector
    up = cross(N, right); // up vector

    const float TWO_PI = PI * 2.0; // 수평 방향
    const float HALF_PI = PI * 0.5; // 수직 방향

    vec3 color = vec3(0.0);
    uint sampleCount = 0u;

    // 구면 좌표계를 사용하여 샘플링
    // 각 샘플링 각도에 대해 구면 좌표계에서 방향 벡터를 계산
    for (float phi = 0.0; phi < TWO_PI; phi += consts.deltaPhi) {// 반구의 수평 방향을 모두 탐색 0 ~ 360
        for (float theta = 0.0; theta < HALF_PI; theta += consts.deltaTheta) { // 반구의 수직 방향을 모두 탐색 0 ~ 90
            // 구면 좌표를 직교 좌표로 변환
            vec3 tempVec = cos(phi) * right + sin(phi) * up; // 수평 방향
            vec3 sampleVector = cos(theta) * N + sin(theta) * tempVec; // tempVec에서 찾은 반구의 수평 방향의 벡터에서 수직 방향에서의 빛의 방향을 찾는다.
            
            // 컨볼루션 계산
            color += texture(samplerEnv, sampleVector).rgb * cos(theta) * sin(theta); // 해당 방향으로 들어오는 빛의 색상을 샘플링하고, 각도에 따라 가중치를 부여한다.
            sampleCount++;
        }
    }
    outColor = vec4(PI * color / float(sampleCount), 1.0);
}
