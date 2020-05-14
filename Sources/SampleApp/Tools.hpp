#pragma once
#include <windows.h>

namespace tools
{
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
}