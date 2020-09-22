#version 450
#extension GL_ARB_separate_shader_objects : enable

/*Схема входа-выхода*/

layout (location = 0) out vec4 outColor;

layout (location = 0) in VS_OUT
{
    vec2 uv;
} fs_in;

/*Uniform*/

layout(set = 0, binding = 0) uniform sampler2D _frameBuffer;

// Основная функция фрагментного шейдера
void main()
{
    outColor = vec4(texture(_frameBuffer,fs_in.uv).rgb,1.0f);
}