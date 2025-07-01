#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 inverseTranspose;
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform PushConsts {
    float scale;
} pushConsts;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTexCoord;

layout(location = 0) out vec3 normalWorld;
layout(location = 1) out vec3 posWorld;
layout(location = 2) out vec3 texcoord;
layout(location = 3) out vec4 color;
layout(location = 4) out vec4 posProj;

void main() {
    // ��� ��ȯ (invTranspose�� �ݵ�� Uniform�� ���ԵǾ�� ��)
    vec3 nWorld = normalize(mat3(ubo.inverseTranspose) * inNormal);
    //vec3 nWorld = mat3(ubo.model) * inNormal;
    //vec3 nWorld = inNormal;

    // ��ġ ��ȯ
    vec4 pos = vec4(inPosition, 1.0);
    pos = ubo.model * pos;

    float t = inTexCoord.x;

    // ��� �������� ���� �̵�
    pos.xyz += nWorld * t * pushConsts.scale;

    posWorld = pos.xyz;

    // ��/�������� ��ȯ
    posProj = ubo.proj * ubo.view * pos;
    gl_Position = posProj;
    // gl_Position = vec4(posWorld, 1.0);

    texcoord = inTexCoord;

    color = vec4(mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), t), 1.0);
    normalWorld = nWorld;
}
