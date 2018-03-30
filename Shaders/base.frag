#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragmentColor;
layout(location = 1) in vec2 fragmentTexCoord;

layout(location = 0) out vec4 outputColor;

void main() 
{
	outputColor = vec4(fragmentColor * texture(texSampler, fragmentTexCoord).rgb, 1.0);
	//outputColor = vec4(fragmentColor, 1.0);
}