#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <shlwapi.h>
#include <fstream>

#include "VkRenderer.h"
#include "VkResources/TextureBuffer.hpp"

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
    inline vk::resources::TextureBufferPtr LoadVulkanTexture(VkRenderer* pRenderer, const std::string &filename)
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
                false);

        // Очистить память
        stbi_image_free(bytes);

        // Отдать smart-pointer объекта ресурса текстуры
        return texture;
    }
}