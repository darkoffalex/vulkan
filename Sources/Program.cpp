#include <iostream>
#include <Windows.h>

#include "Toolkit.h"
#include "VkRenderer.h"

// Объявление оконной процедуры основного окна
// Оконная процедура обрабатывает сообщения системы посылаемые окну (клики, нажатия, закрытие и тд)
LRESULT CALLBACK MainWindowProcedure(HWND, UINT, WPARAM, LPARAM);

// Хендл основного окна
HWND hMainWindow = nullptr;

// Указатель на рендерер
VkRenderer * vkRenderer = nullptr;

// Точка входа
int main(int argc, char* argv[]) 
{
	toolkit::LogMessage("Startup");

	try {
		// Идентификатор модуля (по сути выполняемого приложения)
		// Если бы использовалась функция WinMain (и конфигурация оконного приложения соответственно),
		// то данный идентификатор можно было бы получить из аргументов WinMain
		HINSTANCE hInstance = GetModuleHandleA(nullptr);

		// Структура описывающая новый регистрируемый класс окна
		WNDCLASSEX wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEX);                                           // Размер структуры
		wcex.style = CS_HREDRAW | CS_VREDRAW;                                       // Перерисовывать при изменении горизонтальных или вертикальных размеров
		wcex.lpfnWndProc = MainWindowProcedure;                                     // Оконная процедура
		wcex.hInstance = hInstance;                                                 // Хендл модуля
		wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);                          // Иконка
		wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);                   // Мини-иконка
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);                                 // Курсор
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);                            // Цвет фона
		wcex.lpszMenuName = NULL;                                                   // Название меню
		wcex.lpszClassName = L"VulkanWindowClass";                                  // Наименования класса (строка wchar_t символов)

		// Попытка зарегистрировать класс
		if (!RegisterClassEx(&wcex)) {
			throw std::runtime_error("Can't register new windows class");
		}

		toolkit::LogMessage("Window class registered sucessfully");

		// Создание основного окна
		hMainWindow = CreateWindow(
			wcex.lpszClassName,                                                    // Наименование зарегистрированного класса 
			L"Sample Vulkan Renderer",                                             // Заголовок окна
			WS_OVERLAPPEDWINDOW,                                                   // Стиль окна
			CW_USEDEFAULT,                                                         // Положение по X 
			CW_USEDEFAULT,                                                         // Положение по Y
			640,                                                                   // Ширина 
			480,                                                                   // Высота
			NULL,                                                                  // Указатель на меню
			NULL,                                                                  // Хендл родительского окна
			hInstance,                                                             // Хендл экземпляра (instance) приложения
			NULL);                                                                 // Передаваемый в оконную процедуру параметр (сообщ. WM_CREATE)

		// Если хендл окна все еще пуст - генерируем исключение
		if (!hMainWindow) {
			throw std::runtime_error("Can't create new window");
		}

		toolkit::LogMessage("Window created sucessfully");

		// Показ окна
		ShowWindow(hMainWindow, SW_SHOWNORMAL);

		// Инициализация рендерера
		vkRenderer = new VkRenderer(hInstance, hMainWindow, 100);

		// Тестовая геометрия (индексированный куб)
		vkRenderer->AddPrimitive({
			{ { -0.2f, -0.2f,  0.2f },{ 1.0f, 0.0f, 0.0f } },
			{ { 0.2f, -0.2f,  0.2f },{ 0.0f, 1.0f, 0.0f } },
			{ { 0.2f,  0.2f,  0.2f },{ 0.0f, 0.0f, 1.0f } },
			{ { -0.2f,  0.2f,  0.2f },{ 0.0f, 0.0f, 0.0f } },
			{ { -0.2f, -0.2f, -0.2f },{ 1.0f, 0.0f, 0.0f } },
			{ { 0.2f, -0.2f, -0.2f },{ 0.0f, 1.0f, 0.0f } },
			{ { 0.2f,  0.2f, -0.2f },{ 0.0f, 0.0f, 1.0f } },
			{ { -0.2f,  0.2f, -0.2f },{ 0.0f, 0.0f, 0.0f } },
		}, { 0,1,2, 2,3,0, 1,5,6, 6,2,1, 7,6,5, 5,4,7, 4,0,3, 3,7,4, 4,5,1, 1,0,4, 3,2,6, 6,7,3, }, { 0.0f,0.0f,0.0f }, { 45.0f,45.0f,0.0f });

		// Основной цикл приложения
		// В нем происходит:
		// - Взаимодействие с пользователем (посредством обработки оконных сообщений)
		// - Обновление данных (положение объектов на сцене и прочее)
		// - Отрисовка объектов согласно обновленным данным
		while (true)
		{
			// Структура оконного (системного) сообщения
			MSG msg = {};

			// Если получено какое-то сообщение системы
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				// Подготовить (трансляция символов) и перенаправить сообщение в оконную процедуру класса окна
				TranslateMessage(&msg);
				DispatchMessage(&msg);

				// Если пришло сообщение WM_QUIT - нужно закрыть прогрумму (оборвать цикл)
				if (msg.message == WM_QUIT) {
					break;
				}

				// Если хендл окна не пуст (он может стать пустым при закрытии окна)
				if (hMainWindow) {
					vkRenderer->Update();
					vkRenderer->Draw();
				}
			}
		}

		// Уничтожить рендерер
		delete vkRenderer;
	}
	catch (const std::exception &ex) {
		toolkit::LogError(ex.what());
		system("pause");
	}

	// Выход с кодом 0
	toolkit::LogMessage("Application closed successfully\n");
	return 0;
}

// Определение оконной процедуры основного окна
// Оконная процедура обрабатывает сообщения системы посылаемые окну (клики, нажатия, закрытие и тд)
LRESULT CALLBACK MainWindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_DESTROY:
		// При получении сообщения об уничтожении окна (оно присылается обычно при закрытии, если WM_CLOSE не переопределено)
		// очищаем хендл (это остановит запросы к рендереру) и отправляем сообщение WM_QUIT (при помощи PostQuitMessage)
		hMainWindow = nullptr;
		PostQuitMessage(0);
		break;
	case WM_EXITSIZEMOVE:
		// При изменении размеров окна (после смены размеров и отжатия кнопки мыши)
		if (vkRenderer != nullptr) {
			vkRenderer->VideoSettingsChanged();
		}
		break;
	case WM_KEYDOWN:
		// При нажатии каких-либо клавиш на клавиатуре
		break;
	case WM_MBUTTONDOWN:
		// При нажатии кнопки мыши
		break;
	case WM_MOUSEMOVE:
		// При движении мыши
		break;
	default:
		//В случае остальных сообщений - перенаправлять их в функцию DefWindowProc (функция обработки по умолчанию)
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}