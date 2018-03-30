#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UniformBufferObjectWorld {
    mat4 world;
    mat4 view;
    mat4 proj;
} uboWorld;

layout(set = 0, binding = 1) uniform UniformBufferObjectModel {
    mat4 model;
} uboModel;


layout(location = 0) in vec3 inputPosition;
layout(location = 1) in vec3 inputColor;
layout(location = 2) in vec2 inputTexCoord;

layout(location = 0) out vec3 fragmentColor;
layout(location = 1) out vec2 fragmentTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	gl_Position = uboWorld.proj * uboWorld.view * uboWorld.world * uboModel.model * vec4(inputPosition, 1.0);
	fragmentColor = inputColor;
	fragmentTexCoord = inputTexCoord;
}