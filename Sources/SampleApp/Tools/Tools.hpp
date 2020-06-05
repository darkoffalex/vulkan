#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <shlwapi.h>
#include <fstream>

#include "Camera.hpp"
#include "Timer.hpp"

namespace tools
{
    /**
     * Обработчик оконных сообщений (должна быть реализована в .cpp)
     * @param hWnd Дескриптор окна
     * @param message Сообщение
     * @param wParam Параметр сообщения
     * @param lParam Параметр сообщения
     * @return Код выполнения
     */
    LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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
}