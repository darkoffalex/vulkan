#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <shlwapi.h>
#include <fstream>

#include "VkRenderer.h"
#include "VkResources/TextureBuffer.hpp"
#include "VkResources/GeometryBuffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

namespace tools
{
    /**
     * Вызывается перед закрытием окна (должна быть реализована в .cpp)
     */
    void BeforeWindowClose();

    /**
     * Вызывается после изменения размеров окна (должна быть реализована в .cpp)
     */
    void OnWindowResized();

    /**
     * Обработчик оконных сообщений
     * @param hWnd Дескриптор окна
     * @param message Сообщение
     * @param wParam Параметр сообщения
     * @param lParam Параметр сообщения
     * @return Код выполнения
     */
    inline LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
            // Закрытие окна
            case WM_DESTROY:
            {
                PostQuitMessage(0);
                break;
            }

            case WM_CLOSE:
            {
                BeforeWindowClose();
                return DefWindowProc(hWnd, message, wParam, lParam);
            }

            // Нажатие кнопок клавиатуры
            case WM_KEYDOWN:
            {
                break;
            }

            // Отпускание кнопок клавиатуры
            case WM_KEYUP:
            {
                break;
            }

            // ЛКМ нажата
            case WM_LBUTTONDOWN:
            {
                break;
            }

            // ЛКМ отпущена
            case WM_LBUTTONUP:
            {
                break;
            }

            // Движение курсора
            case WM_MOUSEMOVE:
            {
                break;
            }

            // Завершение изменения размера окна
            case WM_EXITSIZEMOVE:
            {
                OnWindowResized();
                return DefWindowProc(hWnd, message, wParam, lParam);
            }

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
        }

        return 0;
    }

    /**
     * Регистрация оконного класса
     * @param strClassName Наименование класса
     * @param pfnWindowProcedure Указатель на оконную процедуру
     * @return Состояние регистрации класса
     */
    inline bool RegisterWindowClass(HINSTANCE hInstance, const char* strClassName, WNDPROC pfnWindowProcedure = WindowProcedure)
    {
        // Информация о классе
        WNDCLASSEX classInfo;
        classInfo.cbSize = sizeof(WNDCLASSEX);
        classInfo.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        classInfo.cbClsExtra = 0;
        classInfo.cbWndExtra = 0;
        classInfo.hInstance = hInstance;
        classInfo.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
        classInfo.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);
        classInfo.hCursor = LoadCursor(nullptr, IDC_ARROW);
        classInfo.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));
        classInfo.lpszMenuName = nullptr;
        classInfo.lpszClassName = strClassName;
        classInfo.lpfnWndProc = pfnWindowProcedure;

        // Вернуть состояние регистрации класса
        return RegisterClassEx(&classInfo) != 0;
    }

    /**
     * Путь к рабочему каталогу
     * @return Строка содержащая путь к директории
     */
    inline std::string WorkingDir()
    {
        char path[MAX_PATH] = {};
        GetCurrentDirectoryA(MAX_PATH, path);
        PathAddBackslashA(path);
        return std::string(path);
    }

    /**
     * Путь к каталогу с исполняемым файлом (директория содержащая запущенный .exe)
     * @return Строка содержащая путь к директории
     */
    inline std::string ExeDir()
    {
        char path[MAX_PATH] = {};
        GetModuleFileNameA(nullptr, path, MAX_PATH);
        PathRemoveFileSpecA(path);
        PathAddBackslashA(path);
        return std::string(path);
    }

    /**
     * Абсолютный путь к папке с шейдерами
     * @return Строка содержащая путь к директории
     */
    inline std::string ShaderDir()
    {
        std::string exeDir = ExeDir();
        return exeDir.append("..\\Shaders\\");
    }

    /**
     * Загрузить байты из файлы
     * @param path Путь к файлу
     * @return Массив байт
     */
    inline std::vector<unsigned char> LoadBytesFromFile(const std::string &path)
    {
        // Открытие файла в режиме бинарного чтения
        std::ifstream is(path.c_str(), std::ios::binary | std::ios::in | std::ios::ate);

        if (is.is_open())
        {
            auto size = is.tellg();
            auto pData = new char[size];
            is.seekg(0, std::ios::beg);
            is.read(pData,size);
            is.close();

            return std::vector<unsigned char>(pData, pData + size);
        }

        return {};
    }

    /**
     * Загрузка текстуры Vulkan из файла
     * @param pRenderer Указатель на рендерер
     * @param filename Имя файла в папке Textures
     * @return Smart pointer объекта буфера текстуры
     */
    inline vk::resources::TextureBufferPtr LoadVulkanTexture(
            VkRenderer* pRenderer,
            const std::string &filename,
            bool mip = false,
            bool sRgb = false)
    {
        // Полный путь к файлу
        auto path = tools::ExeDir().append("..\\Textures\\").append(filename);

        // Параметры изображения
        int width, height, channels;
        // Включить вертикальный flip
        stbi_set_flip_vertically_on_load(true);
        // Загрузить
        unsigned char* bytes = stbi_load(path.c_str(),&width,&height,&channels,STBI_rgb_alpha);

        // Если не удалось загрузит
        if(bytes == nullptr){
            throw std::runtime_error(std::string("Can't load texture (").append(path).append(")").c_str());
        }

        // Создать ресурс текстуры
        // Для простоты пока будем считать что кол-вао байт на пиксель равно кол-ву каналов
        auto texture = pRenderer->createTextureBuffer(
                bytes,
                static_cast<size_t>(width),
                static_cast<size_t>(height),
                static_cast<size_t>(channels),
                mip,
                sRgb);

        // Очистить память
        stbi_image_free(bytes);

        // Отдать smart-pointer объекта ресурса текстуры
        return texture;
    }

    /**
     * Генерация геометрии квадрата
     * @param pRenderer Указатель на рендерер
     * @param size Размер стороны квадрата
     * @return Smart pointer объекта геометрического буфера
     */
    inline vk::resources::GeometryBufferPtr GenerateQuadGeometry(VkRenderer* pRenderer, float size)
    {
        // Вершины
        std::vector<vk::tools::Vertex> vertices = {
                { { (size / 2),(size / 2),0.0f },{ 1.0f,1.0f,1.0f },{ 1.0f,1.0f }, {0.0f,0.0f,1.0f} },
                { { (size / 2),-(size / 2),0.0f },{ 1.0f,1.0f,1.0f },{ 1.0f,0.0f }, {0.0f,0.0f,1.0f} },
                { { -(size / 2),-(size / 2),0.0f },{ 1.0f,1.0f,1.0f },{ 0.0f,0.0f }, {0.0f,0.0f,1.0f} },
                { { -(size / 2),(size / 2),0.0f },{ 1.0f,1.0f,1.0f },{ 0.0f,1.0f }, {0.0f,0.0f,1.0f} },
        };

        // Индексы
        std::vector<unsigned> indices = {
                0,1,2, 0,2,3,
        };

        // Отдать smart-pointer объекта ресурса геометрического буфера
        return pRenderer->createGeometryBuffer(vertices,indices);
    }

    /**
     * Генерация геометрии куба
     * @param pRenderer Указатель на рендерер
     * @param size Размер стороны куба
     * @return Smart pointer объекта геометрического буфера
     */
    inline vk::resources::GeometryBufferPtr GenerateCubeGeometry(VkRenderer* pRenderer, float size)
    {
        // Вершины
        std::vector<vk::tools::Vertex> vertices = {
                { { (size / 2),(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,1.0f }, {0.0f,0.0f,1.0f} },
                { { (size / 2),-(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,0.0f }, {0.0f,0.0f,1.0f} },
                { { -(size / 2),-(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,0.0f }, {0.0f,0.0f,1.0f} },
                { { -(size / 2),(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,1.0f }, {0.0f,0.0f,1.0f} },

                { { (size / 2),(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,1.0f }, {1.0f,0.0f,0.0f} },
                { { (size / 2),-(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,0.0f }, {1.0f,0.0f,0.0f} },
                { { (size / 2),-(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,0.0f }, {1.0f,0.0f,0.0f} },
                { { (size / 2),(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,1.0f }, {1.0f,0.0f,0.0f} },

                { { (size / 2),  (size / 2), -(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,1.0f },{0.0f,1.0f,0.0f} },
                { { (size / 2), (size / 2), (size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,0.0f },{0.0f,1.0f,0.0f} },
                { { -(size / 2), (size / 2), (size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,0.0f }, {0.0f,1.0f,0.0f} },
                { { -(size / 2),  (size / 2), -(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,1.0f }, {0.0f,1.0f,0.0f} },

                { { -(size / 2),(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,1.0f }, {0.0f,0.0f,-1.0f}  },
                { { -(size / 2),-(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,0.0f }, {0.0f,0.0f,-1.0f}  },
                { { (size / 2),-(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,0.0f }, {0.0f,0.0f,-1.0f}  },
                { { (size / 2),(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,1.0f }, {0.0f,0.0f,-1.0f}  },

                { { -(size / 2),(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,1.0f }, {-1.0f,0.0f,0.0f} },
                { { -(size / 2),-(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,0.0f }, {-1.0f,0.0f,0.0f} },
                { { -(size / 2),-(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,0.0f }, {-1.0f,0.0f,0.0f} },
                { { -(size / 2),(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,1.0f }, {-1.0f,0.0f,0.0f} },

                { { -(size / 2),  -(size / 2), -(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,1.0f }, {0.0f,-1.0f,0.0f} },
                { { -(size / 2), -(size / 2), (size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,0.0f }, {0.0f,-1.0f,0.0f}  },
                { { (size / 2), -(size / 2), (size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,0.0f }, {0.0f,-1.0f,0.0f}  },
                { { (size / 2),  -(size / 2), -(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,1.0f }, {0.0f,-1.0f,0.0f}  },
        };

        // Индексы
        std::vector<unsigned> indices = {
                0,1,2, 0,2,3,
                4,5,6, 4,6,7,
                8,9,10, 8,10,11,
                12,13,14, 12,14,15,
                16,17,18, 16,18,19,
                20,21,22, 20,22,23,
        };

        // Отдать smart-pointer объекта ресурса геометрического буфера
        return pRenderer->createGeometryBuffer(vertices,indices);
    }

    /**
     * Генерация геометрии сферы
     * @param pRenderer Указатель на рендерер
     * @param segments Кол-во сегментов
     * @param radius Радиус
     * @return Smart pointer объекта геометрического буфера
     */
    inline vk::resources::GeometryBufferPtr GenerateSphereGeometry(VkRenderer* pRenderer, unsigned segments, float radius)
    {
        const float pi = 3.14159265359f;

        std::vector<vk::tools::Vertex> vertices = {};
        std::vector<unsigned> indices = {};

        // Вершины
        for (unsigned int y = 0; y <= segments; ++y)
        {
            for (unsigned int x = 0; x <= segments; ++x)
            {
                // В какой части окружности (по 2 осям) мы находимся на конкретной итерации
                const float xSegment = static_cast<float>(x) / static_cast<float>(segments);
                const float ySegment = static_cast<float>(y) / static_cast<float>(segments);

                // Вычисляем положение вершины
                const float xPos = std::cos(xSegment * 2.0f * pi) * std::sin(ySegment * pi);
                const float yPos = std::cos(ySegment * pi);
                const float zPos = std::sin(xSegment * 2.0f * pi) * std::sin(ySegment * pi);

                // Добавляем вершину
                vertices.push_back({
                        {xPos * radius,yPos * radius,zPos * radius},
                        {1.0f,1.0f,1.0f},
                        {xSegment,ySegment},
                        {xPos,yPos,zPos}
                });
            }
        }

        // Индексы
        for (unsigned latNumber = 0; latNumber < segments; latNumber++) {
            for (unsigned int longNumber = 0; longNumber < segments; longNumber++) {
                const unsigned first = latNumber * (segments + 1) + longNumber;
                const unsigned second = first + segments + 1;

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        // Отдать smart-pointer объекта ресурса геометрического буфера
        return pRenderer->createGeometryBuffer(vertices,indices);
    }
}