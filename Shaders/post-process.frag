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

/*Константы*/

// Величина смещения в стороны от текущего фрагмента
const float offset = 1.0 / 300.0;

// Смещения сверточного ядра
const vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), // верх-лево
    vec2( 0.0f,    offset), // верх-центр
    vec2( offset,  offset), // верх-право

    vec2(-offset,  0.0f),   // центр-лево
    vec2( 0.0f,    0.0f),   // центр-центр
    vec2( offset,  0.0f),   // центр-право

    vec2(-offset, -offset), // низ-лево
    vec2( 0.0f,   -offset), // низ-центр
    vec2( offset, -offset)  // низ-право
);

/*Функции*/

// Подсчитать сверточное ядро
vec3 calcKernel(float kernel[9])
{
    vec3 resultColor = vec3(0.0f);

    for(int i = 0; i < 9; i++)
        resultColor += kernel[i] * texture(_frameBuffer, fs_in.uv + offsets[i]).rgb;

    return resultColor;
}

// Эффект blur
vec3 blur()
{
    float kernel[9] = float[](
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16
    );

    return calcKernel(kernel);
}

// Эффект выделения границ
vec3 bounds()
{
    float kernel[9] = float[](
        1.0, 1.0, 1.0,
        1.0, -8.0, 1.0,
        1.0, 1.0, 1.0
    );

    return calcKernel(kernel);
}

// Основная функция фрагментного шейдера
void main()
{
    // Итоговый цвет
    vec3 result = texture(_frameBuffer, fs_in.uv).rgb;

    // Тональная компрессия
    vec3 mapped = result / (result + vec3(1.0f));

    // Гамма коррекция
    float gamma = 1.4f;
    outColor = vec4(pow(mapped,vec3(1.0f/gamma)),1.0f);
}