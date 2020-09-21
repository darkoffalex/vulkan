#version 450

layout (location = 0) out VS_OUT
{
    vec2 uv;
} vs_out;

out gl_PerVertex
{
    vec4 gl_Position;
};

// Основная функция вершинного шейдера
// Шейдеры, вызываеме 3 раза, образуют треугольник без вершинного буфера, за счет индекса gl_VertexIndex
void main()
{
    // Данные UV координаты позволят отобразить на треугольнике квадратную область
    vs_out.uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    // Выходящие за пределы экрана части треугольника будут отброшены culling'ом
    gl_Position = vec4(vs_out.uv * 2.0f - 1.0f, 0.0f, 1.0f);
}