#include <iostream>
#include <Windows.h>
#include <iomanip>
#include <math.h>
#include <glm\glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb\stb_image.h>

#include "Toolkit.h"
#include "VkRenderer.h"

using std::chrono::time_point;
using std::chrono::high_resolution_clock;

// Объявление оконной процедуры основного окна
// Оконная процедура обрабатывает сообщения системы посылаемые окну (клики, нажатия, закрытие и тд)
LRESULT CALLBACK MainWindowProcedure(HWND, UINT, WPARAM, LPARAM);

// Загрузка текстуры
// Метод вернет структуру с хендлами текстуры и дескриптора
vktoolkit::Texture LoadTextureVk(VkRenderer * renderer, std::string path);

// Хендл основного окна
HWND hMainWindow = nullptr;

// Указатель на рендерер
VkRenderer * vkRenderer = nullptr;

// Время последнего кадра (точнее последней итерации)
time_point<high_resolution_clock> lastFrameTime;

// Структура описывающая модель камеры
struct{
	// Положение на сцеене и поворот камеры
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);

	// Множитель движения (значения от -1 до 1)
	glm::i8vec3 movement = glm::i8vec3(0, 0, 0);

	// Скорость перемещения и чувствительность мыши
	float movementSpeed = 1.0f;
	float mouseSensitivity = 0.5f;

	// Движется ли камера
	bool IsMoving() const {
		return this->movement.x != 0 || this->movement.y != 0 || this->movement.z != 0;
	}

	// Обновление положения
	// float deltaMs - время одного кадра (итерации цикла) за которое будет выполнено перемещение
	void UpdatePositions(float deltaMs) {
		if (this->IsMoving()) {

			// Локальный вектор движения (после нажатия на кнопки управления)
			glm::vec3 movementVectorLocal = glm::vec3(
				(float)this->movement.x * ((this->movementSpeed / 1000.0f) * deltaMs),
				(float)this->movement.y * ((this->movementSpeed / 1000.0f) * deltaMs),
				(float)this->movement.z * ((this->movementSpeed / 1000.0f) * deltaMs)
			);

			// Глобальный вектор движения (с учетом поворота камеры)
			glm::vec3 movementVectorGlobal = glm::vec3(
				(cos(glm::radians(this->rotation.y)) * movementVectorLocal.x) - (sin(glm::radians(this->rotation.y)) * movementVectorLocal.z),
				(cos(glm::radians(this->rotation.x)) * movementVectorLocal.y) + (sin(glm::radians(this->rotation.x)) * movementVectorLocal.z),
				(sin(glm::radians(this->rotation.y)) * movementVectorLocal.x) + (cos(glm::radians(this->rotation.y)) * movementVectorLocal.z)
			);

			// Приращение позиции
			this->position += movementVectorGlobal;
		}
	}
} camera;

// Положение курсора мыши в системе координат окна
struct {
	int32_t x = 0;
	int32_t y = 0;
} mousePos;

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

		// Загрузка данных текстуры
		int width; int height; int channels;
		std::string texDir = toolkit::ExeDir() + "..\\Textures\\texture.jpg";
		unsigned char* pixels = stbi_load(texDir.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		// Инициализация рендерера
		vkRenderer = new VkRenderer(hInstance, hMainWindow, 100);

		// Загрузка текстур
		vktoolkit::Texture groundTexture = LoadTextureVk(vkRenderer, "ground.jpg");
		vktoolkit::Texture cubeTexture = LoadTextureVk(vkRenderer, "cube.jpg");

		// Пол
		vkRenderer->AddPrimitive({
			{ { -5.0f, 0.0f,  5.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { -5.0f, 0.0f,  -5.0f },{ 1.0f, 1.0f, 1.0f },{ 10.0f, 0.0f } },
			{ { 5.0f,  0.0f,  -5.0f },{ 1.0f, 1.0f, 1.0f },{ 10.0f, 10.0f } },
			{ { 5.0f,  0.0f,  5.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 10.0f } },

		}, { 0,1,2,2,3,0 }, &groundTexture, { 0.0f,-0.5f,0.0f }, { 0.0f,0.0f,0.0f });

		// Куб
		vkRenderer->AddPrimitive({
			{ { -0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { -0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { 0.2f,  0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { 0.2f,  -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

			{ { 0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { 0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { 0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { 0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

			{ { -0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { -0.2f, 0.2f, -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { 0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { 0.2f,  0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

			{ { -0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { -0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { -0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { -0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

			{ { 0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { 0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { -0.2f, 0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { -0.2f, -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

			{ { 0.2f,  -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { 0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { -0.2f, -0.2f, -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { -0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

		}, { 0,1,2,2,3,0, 4,5,6,6,7,4, 8,9,10,10,11,8, 12,13,14,14,15,12, 16,17,18,18,19,16, 20,21,22,22,23,20 }, &cubeTexture, { 0.0f,-0.3f,-2.0f }, { 0.0f,45.0f,0.0f });

		// Куб
		vkRenderer->AddPrimitive({
			{ { -0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { -0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { 0.2f,  0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { 0.2f,  -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

			{ { 0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { 0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { 0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { 0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

			{ { -0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { -0.2f, 0.2f, -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { 0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { 0.2f,  0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

			{ { -0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { -0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { -0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { -0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

			{ { 0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { 0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { -0.2f, 0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { -0.2f, -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

			{ { 0.2f,  -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
			{ { 0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
			{ { -0.2f, -0.2f, -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
			{ { -0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

		}, { 0,1,2,2,3,0, 4,5,6,6,7,4, 8,9,10,10,11,8, 12,13,14,14,15,12, 16,17,18,18,19,16, 20,21,22,22,23,20 }, &cubeTexture, { 1.0f,-0.3f,-3.0f }, { 0.0f,0.0f,0.0f });

		// Цельный индексированный куб
		/*
		vkRenderer->AddPrimitive({
			{ { -0.2f,  0.2f,  0.2f },{ 0.0f, 0.0f, 0.0f } },
			{ { 0.2f,  0.2f,  0.2f },{ 0.0f, 0.0f, 1.0f } },
			{ { 0.2f, -0.2f,  0.2f },{ 0.0f, 1.0f, 0.0f } },
			{ { -0.2f, -0.2f,  0.2f },{ 1.0f, 0.0f, 0.0f } },
			
			{ { -0.2f,  0.2f, -0.2f },{ 0.0f, 0.0f, 0.0f } },
			{ { 0.2f,  0.2f, -0.2f },{ 0.0f, 0.0f, 1.0f } },
			{ { 0.2f, -0.2f, -0.2f },{ 0.0f, 1.0f, 0.0f } },
			{ { -0.2f, -0.2f, -0.2f },{ 1.0f, 0.0f, 0.0f } },
		}, { 0,1,2, 2,3,0, 1,5,6, 6,2,1, 7,6,5, 5,4,7, 4,0,3, 3,7,4, 4,5,1, 1,0,4, 3,2,6, 6,7,3, }, nullptr, { 0.0f,0.0f,-2.0f }, { 0.0f,0.0f,0.0f });
		*/

		// Конфигурация перспективы
		vkRenderer->SetCameraPerspectiveSettings(60.0f, 0.1f, 256.0f);

		// Время последнего кадра - время начала цикла
		lastFrameTime = high_resolution_clock::now();

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
			}

			// Если хендл окна не пуст (он может стать пустым при закрытии окна)
			if (hMainWindow) {

				// Время текущего кадра (текущей итерации)
				time_point<high_resolution_clock> currentFrameTime = high_resolution_clock::now();

				// Сколько микросекунд прошло с последней итерации
				// 1 миллисекунда = 1000 микросекунд = 1000000 наносекунд
				int64_t delta = std::chrono::duration_cast<std::chrono::microseconds>(currentFrameTime - lastFrameTime).count();
				
				// Перевести в миллисекунды
				float deltaMs = (float)delta / 1000;

				// Обновить время последней итерации
				lastFrameTime = currentFrameTime;

				// Обновление перемещений камеры с учетом времени кадра
				camera.UpdatePositions(deltaMs);

				// Обновить рендерер и отрисовать кадр
				vkRenderer->SetCameraPosition(camera.position.x, camera.position.y, camera.position.z);
				vkRenderer->SetCameraRotation(camera.rotation.x, camera.rotation.y, camera.rotation.z);
				vkRenderer->Update();
				vkRenderer->Draw();
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
		// При нажатии кнопок (WASD)
		switch (wParam)
		{
		case 0x57:
			camera.movement.z = 1;
			break;
		case 0x41:
			camera.movement.x = 1;
			break;
		case 0x53:
			camera.movement.z = -1;
			break;
		case 0x44:
			camera.movement.x = -1;
			break;
		}
		break;
	case WM_KEYUP:
		// При отжатии кнопок (WASD)
		switch (wParam)
		{
		case 0x57:
			camera.movement.z = 0;
			break;
		case 0x41:
			camera.movement.x = 0;
			break;
		case 0x53:
			camera.movement.z = 0;
			break;
		case 0x44:
			camera.movement.x = 0;
			break;
		}
		break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
		// При нажатии любой кнопки мыши
		mousePos.x = (int32_t)LOWORD(lParam);
		mousePos.y = (int32_t)HIWORD(lParam);
		break;
	case WM_MOUSEMOVE:
		// При движении мыши
		// Если зажата левая кнопка мыши
		if (wParam & MK_LBUTTON){
			int32_t posx = (int32_t)LOWORD(lParam);
			int32_t posy = (int32_t)HIWORD(lParam);
			camera.rotation.x -= (mousePos.y - (float)posy) * camera.mouseSensitivity;
			camera.rotation.y -= (mousePos.x - (float)posx) * camera.mouseSensitivity;
			mousePos.x = posx;
			mousePos.y = posy;
		}
		break;
	default:
		//В случае остальных сообщений - перенаправлять их в функцию DefWindowProc (функция обработки по умолчанию)
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

// Загрузка текстуры
// Метод вернет структуру с хендлами текстуры и дескриптора
vktoolkit::Texture LoadTextureVk(VkRenderer * renderer, std::string path)
{
	int width;        // Ширина загруженного изображения
	int height;       // Высота загруженного изображения
	int channels;     // Кол-во каналов
	int bpp = 4;      // Байт на пиксель

	// Путь к файлу
	std::string filename = toolkit::ExeDir() + "..\\Textures\\" + path;

	// Получить пиксели (массив байт)
	unsigned char* pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	
	// Создать текстуру (загрузить пиксели в память устройства)
	vktoolkit::Texture result = renderer->CreateTexture(
		pixels, (uint32_t)width, 
		(uint32_t)height, 
		(uint32_t)channels, 
		(uint32_t)bpp);

	// Очистить массив байт
	stbi_image_free(pixels);

	return result;
}