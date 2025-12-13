#version 450 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

layout(push_constant) uniform uPushConstant 
{ 
	vec2 uScale; 
	vec2 uTranslate; 
}
pushConstants;

layout(location = 0) out vec2 UV;
layout(location = 1) out vec4 Color;

out gl_PerVertex 
{ 
	vec4 gl_Position; 
};

void main()
{
    UV = aUV;
    Color = aColor;
    gl_Position = vec4(aPos * pushConstants.uScale + pushConstants.uTranslate, 0.0, 1.0);
}