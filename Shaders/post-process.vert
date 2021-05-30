#version 450
#extension GL_KHR_vulkan_glsl:enable

/*Схема входа-выхода*/

layout (location = 0) out VS_OUT
{
    vec2 uv;
} vs_out;

out gl_PerVertex
{
    vec4 gl_Position;
};

/*Вспомогательные типы*/

struct Vertex
{
    vec3 position;
    vec2 uv;
};

/*Функции*/

// Основная функция вершинного шейдера
// Шейдеры, вызываеме 6 раз, образуют квадрат, за счет индекса gl_VertexIndex
void main()
{
    // 2 треугольника
    Vertex vertices[6] = Vertex[6]
    (
        Vertex(vec3(-1.0f,-1.0f,0.0f),vec2(0.0f,1.0f)),
        Vertex(vec3(-1.0f,1.0f,0.0f),vec2(0.0f,0.0f)),
        Vertex(vec3(1.0f,1.0f,0.0f),vec2(1.0f,0.0f)),

        Vertex(vec3(1.0f,1.0f,0.0f),vec2(1.0f,0.0f)),
        Vertex(vec3(1.0f,-1.0f,0.0f),vec2(1.0f,1.0f)),
        Vertex(vec3(-1.0f,-1.0f,0.0f),vec2(0.0f,1.0f))
    );

    // UV координаты для выборки из кадрового буфера
    vs_out.uv = vertices[gl_VertexIndex].uv;
    // Положение (в NDC пространстве, без преобразований)
    gl_Position = vec4(vertices[gl_VertexIndex].position,1.0f);
}