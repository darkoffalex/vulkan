#version 450
#extension GL_ARB_separate_shader_objects : enable

/*Схема входа-выхода*/

layout (location = 0) out vec4 outColor;

layout (location = 0) in VS_OUT
{
    vec2 uv;
} fs_in;

// Основная функция фрагментного шейдера
void main()
{
    outColor = vec4(1.0f,0.0f,0.0f,1.0f);
}