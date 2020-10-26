#version 450
#extension GL_ARB_separate_shader_objects : enable

/*Схема входа-выхода*/

// На входе треугольники
layout (triangles) in;
// На выходе треугольники
layout (triangle_strip, max_vertices = 3) out;

// Значения принятые на вход с предыдущих этапов
layout (location = 0) in VS_OUT
{
    vec3 color;            // Цвет вершины
    vec3 position;         // Положение вершины в мировых координатах
    vec3 positionLocal;    // Положение вершины в локальных координатах
    vec3 normal;           // Вектор нормали вершины
    vec2 uv;               // Текстурные координаты вершины
    mat3 normalMatrix;     // Матрица преобразования нормалей
} gs_in[];

// Выходные значения шейдера
// Эти значения далее будут интерполироваться для каждого фрагмента
layout (location = 0) out GS_OUT
{
    vec3 color;            // Цвет фрагмента
    vec3 position;         // Положение фрагмента в мировых координатах
    vec3 positionLocal;    // Положение фрагмента в локальных координатах
    vec3 normal;           // Вектор нормали фрагмента
    vec2 uv;               // Текстурные координаты фрагмента
    mat3 tbnMatrix;        // Матрица для преобразования из касательного пространства треугольника в мировое
    mat3 tbnMatrixInverse; // Матрица для преобразования из мирового пространства в касательное
} gs_out;

/*Функции*/

// Функция для подсчета тангента
// Тангент - вектор перпендикулярный нормали треугольника и учитывающий UV координаты
vec3 calcTangent(vec3 v0, vec3 v1, vec3 v2, vec2 uv0, vec2 uv1, vec2 uv2)
{
    // Грани полигона в виде векторов
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;

    // Дельта UV для каждой грани
    vec2 deltaUV1 = uv1 - uv0;
    vec2 deltaUV2 = uv2 - uv0;

    // Коэффициент для подсчета тангента
    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    // Подсчитать и вернуть тангент
    return normalize(vec3(
    f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
    f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
    f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)));
}

// Функция ориентирует тангент по нормали вершины
// Нормаль полигона, по которому построен тангент, не всегда совпадает с нормалью вершины, что может вызывать искажения
vec3 orthogonalizeTangent(vec3 tangent, vec3 vertexNormal)
{
    vec3 bitangent = cross(vertexNormal, tangent);
    return cross(bitangent, vertexNormal);
}

// Основная функция геометрического шейдера
// Вычисление TBN матрицы для карт нормалей и предача остальных данных в следующий этап
void main()
{
    // Тангент полигона
    vec3 polygonTangent = calcTangent(
    gs_in[0].positionLocal,
    gs_in[1].positionLocal,
    gs_in[2].positionLocal,
    gs_in[0].uv,
    gs_in[1].uv,
    gs_in[2].uv);

    // Пройтись по всем вершинам
    for(int i = 0; i < gl_in.length(); i++)
    {
        // Передача основных параметров в следующий шейдер как есть
        gl_Position = gl_in[i].gl_Position;
        gs_out.color = gs_in[i].color;
        gs_out.position = gs_in[i].position;
        gs_out.positionLocal = gs_in[i].positionLocal;
        gs_out.normal = gs_in[i].normal;
        gs_out.uv = gs_in[i].uv;

        // Собрать TBN матрицу (касательного-мирового пространства)
        vec3 T = normalize(orthogonalizeTangent(gs_in[i].normalMatrix * polygonTangent, gs_in[i].normal));
        vec3 B = normalize(cross(gs_in[i].normal, T));
        vec3 N = normalize(gs_in[i].normal);
        gs_out.tbnMatrix = mat3(T,B,N);
        gs_out.tbnMatrixInverse = inverse(gs_out.tbnMatrix);

        // Добавить вершину
        EmitVertex();
    }

    // Завершить построение примитива
    EndPrimitive();
}