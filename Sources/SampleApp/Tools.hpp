#pragma once

#include <windows.h>
#include <vector>
#include <string>
#include <shlwapi.h>
#include <fstream>

namespace tools
{
    /**
     * Вызывается перед закрытием окна (должна быть реализована в .cpp)
     */
    void beforeWindowClose();

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
                beforeWindowClose();
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
                break;
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
    std::string WorkingDir()
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
    std::string ExeDir()
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
    std::string ShaderDir()
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