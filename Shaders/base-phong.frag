#version 450
#extension GL_ARB_separate_shader_objects : enable

// Набор констант
#define LIGHT_TYPE_POINT 0        // Тип источника - точеченый
#define LIGHT_TYPE_SPOT 1         // Тип источника - прожектор
#define LIGHT_TYPE_DIRECTIONAL 2  // Тип источника - направленный
#define MAX_LIGHTS 100            // Максимальное число источников

#define TEXTURE_COLOR 0           // Тип текстуры - дифузная (цвет)
#define TEXTURE_NORMAL 1          // Тип текстуры - карта нормалей
#define TEXTURE_SPECULAR 2        // Тип текстуры - бликовая карта
#define TEXTURE_DISPLACE 3        // Тип текстуры - карта высот

/*Схема входа-выхода*/

layout (location = 0) out vec4 outColor;

layout (location = 0) in GS_OUT
{
    vec3 color;            // Цвет фрагмента
    vec3 position;         // Положение фрагмента в мировых координатах
    vec3 positionLocal;    // Положение фрагмента в локальных координатах
    vec3 normal;           // Вектор нормали фрагмента
    vec2 uv;               // Текстурные координаты фрагмента
    mat3 tbnMatrix;        // Матрица для преобразования из касательного пространства треугольника в мировое
} fs_in;

/*Вспомогательные типы*/

// Параметры источника света
struct LightSource
{
    vec3 position;
    float radius;
    vec3 color;
    vec3 orientation;
    float attenuationQuadratic;
    float attenuationLinear;
    float cutOffAngleCos;
    float cutOffOuterAngleCos;
    uint type;
};

// Параметры текущего фрагмента (Blinn-Phong)
struct Fragment
{
    vec3 position;      // Положение
    vec3 color;         // Цвет
    vec3 normal;        // Нормаль
    vec3 toView;        // Вектор в сторону камеры
    float specularity;  // Бликовая интенсивность
};

// Параметры матриала (Blinn-Phong)
struct Material
{
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
};

/*Uniform*/

layout(set = 0, binding = 0) uniform UniformCamera {
    mat4 _view;
    mat4 _proj;
    vec3 _camPosition;
};

layout(set = 2, binding = 2, std140) uniform UniformModel {
    Material _materialSettings;
};

layout(set = 2, binding = 3) uniform sampler2D _textures[4];

layout(set = 1, binding = 0) uniform UniformLightCount {
    uint _lightCount;
};

layout(set = 1, binding = 1, std140) uniform UniformLightArray {
    LightSource _lightSources[MAX_LIGHTS];
};

/*Функции*/

// Подсчет дифузной компоненты
vec3 calculateDiffuseComponent(Fragment f, LightSource l, Material m, vec3 fragmentToLight)
{
    // Освещенность - скалярное произведение (косинус угла) векторов нормали и направления к источнику (больше угол - меньше свет)
    float brightness = max(dot(f.normal, fragmentToLight), 0.0);
    // Итоговый цвет получаем перемножением цвета источника, цвета материала и цвета фрагмента (подразумевается текстура)
    return brightness * l.color * m.diffuseColor * f.color;
}

// Подсчет бликовой компоненты
vec3 calculateSpecularComponent(Fragment f, LightSource l,  Material m, vec3 fragmentToLight, bool blinnPhong)
{
    // Использовать метод Блинна-Фонга
    if(blinnPhong)
    {
        // Половинный вектор (между векторами к зрителю и к источнику света)
        vec3 halfwayDir = normalize(fragmentToLight + f.toView);
        // Яркость блика - угол между половинным вектором и нормалью (меньше угол - ярче), степень (pow) - размер пятна
        float brightness = pow(max(dot(f.normal, halfwayDir), 0.0), m.shininess);
        // Итоговый цвет получаем перемножением цвета источника, цвета материала и интенсивности блика фрагмента
        return brightness * l.color * m.specularColor * f.specularity;
    }
    // Использовать метод Фонга
    else
    {
        // Отраженный вектор от источника к фрагменту относительно нормали фрагмента
        vec3 reflectedLightDir = reflect(-fragmentToLight, f.normal);
        // Яркость блика - угол между векторами к зрителю и отраженным (меньше угол - ярче), степень (pow) - размер пятна
        float brightness = pow(max(dot(f.toView, reflectedLightDir), 0.0), m.shininess);
        // Итоговый цвет получаем перемножением цвета источника, цвета материала и интенсивности блика фрагмента
        return brightness * l.color * m.specularColor * f.specularity;
    }
}

// Подсчет освещенности фрагмента точеченым источником
vec3 pointLight(Fragment f, LightSource l, Material m)
{
    // Направление в сторону источника света
    vec3 fragmentToLight = normalize(l.position - f.position);

    // Коэффициент затухания (использует расстояние до источника, а так же спец-коэффициенты)
    float dist = length(l.position - f.position);
    float attenuation = 1.0f / (1.0f + l.attenuationLinear * dist + l.attenuationQuadratic * (dist * dist));

    // Компоненты освещенности
    vec3 diffuse = calculateDiffuseComponent(f,l,m,fragmentToLight);
    vec3 specular = calculateSpecularComponent(f,l,m,fragmentToLight,false);
    vec3 ambient = l.color * m.ambientColor;

    // Вернуть суммарную интенсивность
    return attenuation * (diffuse + specular + ambient);
}

// Подсчет освещенности фрагмента источником-прожектором
vec3 spotLight(Fragment f, LightSource l, Material m)
{
    // Направление в сторону источника света
    vec3 fragmentToLight = normalize(l.position - f.position);

    // Коэффициент затухания (использует расстояние до источника, а так же спец-коэффициенты)
    float dist = length(l.position - f.position);
    float attenuation = 1.0f / (1.0f + l.attenuationLinear * dist + l.attenuationQuadratic * (dist * dist));

    // Косинус угла между вектором падения света и вектором направления источника
    float thetaCos = dot(fragmentToLight, normalize(-l.orientation));

    // Разница косинусов между углом внутреннего конуса и углом внешнего
    float epsilon = l.cutOffAngleCos - l.cutOffOuterAngleCos;

    // Свет наиболее интенсивен в центре (где thetaCos - 1, угол между лучем и направлением фонарика - 0)
    // К краям интенсивность спадает. Благодаря коэффициенту epsilon есть так же яркое пятно внутри (внутр. конус)
    float intensity = clamp((thetaCos - l.cutOffOuterAngleCos) / epsilon, 0.0, 1.0);

    // Компоненты освещенности
    vec3 diffuse = calculateDiffuseComponent(f,l,m,fragmentToLight);
    vec3 specular = calculateSpecularComponent(f,l,m,fragmentToLight,false);
    vec3 ambient = l.color * m.ambientColor;

    // Вернуть суммарную интенсивность
    return attenuation * intensity * (diffuse + specular + ambient);
}

// Подсчет освещенности фрагмента направленным источником
vec3 directionalLight(Fragment f, LightSource l, Material m)
{
    // Компоненты освещенности
    vec3 diffuse = calculateDiffuseComponent(f,l,m,-l.orientation);
    vec3 specular = calculateSpecularComponent(f,l,m,-l.orientation,false);
    vec3 ambient = l.color * m.ambientColor;

    // Вернуть суммарную интенсивность
    return diffuse + specular + ambient;
}

// Получить нормаль из карты нормалей произведя необходимые преобразования
vec3 getNormalFromMap(vec2 uv)
{
    vec3 normal = normalize(texture(_textures[TEXTURE_NORMAL],uv).rgb * 2.0 - 1.0);
    return fs_in.tbnMatrix * normal;
}

// Доступна ли текстур
bool isTextureAvailable(uint textureType)
{
    ivec2 sizes =  textureSize(_textures[textureType],0);
    return sizes.x > 1 && sizes.y > 1;
}

// Основная функция вершинного шейдера
// Вычисление итогового цвета фрагмента (пикселя)
void main()
{
    // Структура описывающая текущий фрагмент
    Fragment f;
    f.position = fs_in.position;
    f.color = isTextureAvailable(TEXTURE_COLOR) ? texture(_textures[TEXTURE_COLOR],fs_in.uv).rgb : vec3(1.0f);
    f.normal = isTextureAvailable(TEXTURE_NORMAL) ? getNormalFromMap(fs_in.uv) : normalize(fs_in.normal);
    f.toView = normalize(_camPosition - fs_in.position);
    f.specularity = isTextureAvailable(TEXTURE_SPECULAR) ? texture(_textures[TEXTURE_SPECULAR],fs_in.uv).r : 0.0f;

    // Итоговый цвет
    vec3 result = vec3(0.0f);

    // Пройтись по всем источникам света
    for(uint i = 0; i < _lightCount; i++)
    {
        switch(_lightSources[i].type)
        {
            case LIGHT_TYPE_POINT:
            result += pointLight(f, _lightSources[i], _materialSettings);
            break;

            case LIGHT_TYPE_SPOT:
            result += spotLight(f, _lightSources[i], _materialSettings);
            break;

            case LIGHT_TYPE_DIRECTIONAL:
            result += directionalLight(f, _lightSources[i], _materialSettings);
            break;
        }
    }

    // Вывод цвета во вложение
    outColor = vec4(result,1.0f);
}
