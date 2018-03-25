/*
* Набор вспомогательных инструментов для работы с Vulkan и рендерером
*
* Copyright (C) 2018 by Alex "DarkWolf" Nem - https://github.com/darkoffalex
*/

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include <vulkan\vulkan.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Toolkit.h"

#define VK_CHECK(result, errMsg) {if((result) != VK_SUCCESS) {throw std::runtime_error((errMsg));}}

namespace vktoolkit
{
	/* В С П О М О Г А Т Е Л Ь Н Ы Е  С Т Р У К Т У Р Ы  /  К Л А С С Ы */

	/**
	* Стурктура описывающая объект с информацией о семействах очередей (их индексы, проверка - доступен ли рендеринг)
	* 
	* Команды, которые программа будет отдавать устройству, разделены на семейства, например:
	* - Семейство команд рисования (графические команды)
	* - Семейство команд прдеставления (показ того что нарисовано)
	* - Семейство команд вычисления (подсчеты) и прочие
	*
	* Семейство может поддерживать команды сразу нескольких типов, потому индекс семейства, например, для граифческих
	* команд и команд представления, может совпадать. Объект данной структуры можно будет получить при помощи функции GetQueueFamilyInfo
	* которая вернет информацию о семействах поддерживаемых конкретным устройством для конкретной поверхности
	*/
	struct QueueFamilyInfo {

		// Индексты семейств очередей
		uint32_t graphics = -1;
		uint32_t present = -1;
		uint32_t compute = -1;
		uint32_t transfer = -1;

		// Совместим ли набор доступных семейств с рендерингом
		bool IsRenderingCompatible() const {
			return graphics >= 0 && present >= 0;
		}
	};

	/**
	* Структура объекта с информацией о возможностях поверхности, о ее форматах и доступных режимах отображения
	* Пояснение: То, где будет показываться отрендереная картинка, называется поверхностью. Например,
	* поверхность окна Windows или какая либо иная поверхность (если, например, показ происходит вне окна, сразу на экран).
	* Каждая поверхность обладает своими ограничениями и свойствами (цвета, частота обновления, и прочее).
	*/
	struct SurfaceInfo {

		// Возможности поверхности, форматы, режимы представления
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;

		// Поддерживается ли конкретный формат поверхности (состоит из формата и цветового пространства)
		bool IsSurfaceFormatSupported(VkSurfaceFormatKHR surfaceFormat) const {
			return this->IsFormatSupported(surfaceFormat.format) && this->IsColorSpaceSupported(surfaceFormat.colorSpace);
		}

		// Поддержимвается ли конкретное цветовое пространство
		bool IsColorSpaceSupported(VkColorSpaceKHR colorSpace) const {
			if (this->formats.size() == 1 && this->formats[0].format == VK_FORMAT_UNDEFINED) {
				return true;
			}
			else if (this->formats.size() > 1) {
				for (const VkSurfaceFormatKHR& formatEntry : this->formats) {
					if (formatEntry.colorSpace == colorSpace) {
						return true;
					}
				}
			}
			return false;
		}

		// Поддерживается ли конкретный формат
		bool IsFormatSupported(VkFormat format) const {
			if (this->formats.size() == 1 && this->formats[0].format == VK_FORMAT_UNDEFINED) {
				return true;
			}
			else if (this->formats.size() > 1) {
				for (const VkSurfaceFormatKHR& formatEntry : this->formats) {
					if (formatEntry.format == format) {
						return true;
					}
				}
			}
			return false;
		}
	};

	/**
	* Структура описывает объект устройства. Данный объект содержит хендлы физического и логического устройства,
	* информацию о доступных семействах очередей, хендлы самих очередей, позволяет получить необходимую
	* информацию об устройстве
	*/
	struct Device
	{
		// Физическое и логическое устройство
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice logicalDevice = VK_NULL_HANDLE;

		// Информация о доступных семействах очередей
		// Эта информация записывается сюда при инициализации устройства (при поиске подходящего)
		vktoolkit::QueueFamilyInfo queueFamilies = {};

		// Хендлы используемых очередей
		// Заполняются во время инициализации устройства
		struct {
			VkQueue graphics = VK_NULL_HANDLE;
			VkQueue present = VK_NULL_HANDLE;
		} queues;

		// Получение информации о физическом устройстве
		// Дает возможность получить такие данные как название, разлиные аппаратные ограничения и прочее
		VkPhysicalDeviceProperties GetProperties() const {
			VkPhysicalDeviceProperties properties = {};
			if (this->physicalDevice != VK_NULL_HANDLE) {
				vkGetPhysicalDeviceProperties(this->physicalDevice, &properties);
			}
			return properties;
		}

		// Проинициализировано ли устройство
		bool IsReady() const {
			return this->physicalDevice != VK_NULL_HANDLE &&
				this->logicalDevice != VK_NULL_HANDLE &&
				this->queues.graphics != VK_NULL_HANDLE &&
				this->queues.present != VK_NULL_HANDLE &&
				this->queueFamilies.IsRenderingCompatible();
		}

		// Деинициализация устройства
		void Deinit() {
			if (this->logicalDevice != VK_NULL_HANDLE) {
				vkDestroyDevice(this->logicalDevice, nullptr);
				this->logicalDevice = VK_NULL_HANDLE;
			}
			this->physicalDevice = VK_NULL_HANDLE;
			this->queues.graphics = VK_NULL_HANDLE;
			this->queues.present = VK_NULL_HANDLE;
			this->queueFamilies = {};
		}

		// Получить выравнивание памяти для конкретного типа даныз учитывая аппаратные лимиты физического устройства
		// Иногда необходимо аллоцировать именно выравненную память, этот метод возвращает размер одного фрагмента
		// выравненной памяти с учетом размера типа и лимитов устройства
		template <typename T>
		VkDeviceSize GetDynamicAlignment() const {
			VkDeviceSize minUboAlignment = this->GetProperties().limits.minUniformBufferOffsetAlignment;
			VkDeviceSize dynamicAlignment = (VkDeviceSize)sizeof(T);

			if (minUboAlignment > 0) {
				dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
			}

			return dynamicAlignment;
		}
	};

	/**
	* Структура описывающая своп-чейн (список показа). Список показа представляет из себя набор сменяемых изображений.
	* В одно изображение может производится запись (рендеринг), в то время как другое будет показываться (презентация).
	* Как правило изображений в списке от 1 до 3. Если изображение одно - рендеринг будет происходить в показанное изображение
	* Структура содержит :
	* - Хендл своп-чейна (списка показа)
	* - Массив изоражений своп-чейна (хендлы изображений в которые будет производится показ)
	* - Массив видов изображений (своеобразные ссылки на изображения предостовляющие необходимый доступ к ресурсу)
	* - Массив буферов кадров
	*/
	struct Swapchain {
		VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		std::vector<VkFramebuffer> framebuffers;
		VkFormat imageFormat = {};
		VkExtent2D imageExtent = {};
	};

	/**
	* Структура описывающая простейший буфер vulkan
	* Содержит хендл буфера и хендл памяти выделенной под него
	*/
	struct Buffer {
		VkBuffer vkBuffer = VK_NULL_HANDLE;
		VkDeviceMemory vkDeviceMemory = VK_NULL_HANDLE;
		VkDeviceSize size = 0;
	};

	/**
	* Буфер вершин - (отличается наличием кол-ва вершин)
	*/
	struct VertexBuffer : Buffer 
	{
		uint32_t count = 0;
	};

	/**
	* Буфер индексов (отличается наличием кол-ва индексов)
	*/
	struct IndexBuffer : Buffer
	{
		uint32_t count = 0;
	};

	/**
	* Буфер-uniform. Содержит информацию о матрицах преобразования координат вершин сцены
	* Может быть как статическим (при создании учитывается изветный рзамер объекта UBO, что будет передаваться)
	* либо динамическим, для множенства UBO объектов (учитывается выравнивание аллоцируемой памяти и кол-во элементов массива)
	*/
	struct UniformBuffer : Buffer
	{
		// Информация для дескрипторного набора (используется при инициализации набора)
		VkDescriptorBufferInfo descriptorBufferInfo = {};

		// Указатель на размеченную память буфера
		void * pMapped = nullptr;

		// Разметить память (после этого указатель pMapped будет указывать на нее)
		VkResult map(VkDevice device, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0){
			return vkMapMemory(device, this->vkDeviceMemory, offset, size, 0, &(this->pMapped));
		}

		// Отменить разметку (отвязать указатель от памяти)
		void unmap(VkDevice device){
			if (this->pMapped){
				vkUnmapMemory(device, this->vkDeviceMemory);
			}
		}

		// Конфигурация дескриптора
		// Указываем какая именно память буфера будет доступна дескриптору (доступна для шейдера)
		void configDescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
		{
			this->descriptorBufferInfo.offset = offset;
			this->descriptorBufferInfo.buffer = this->vkBuffer;
			this->descriptorBufferInfo.range = size;
		}
	};

	/**
	* Объект передаваемый в статический uniform-буфер, описывающий матрицы глобальной сцены
	* Структура с набором ОБЩИХ ДЛЯ ВСЕХ объектов матриц преобразований
	*/
	struct UboWorld
	{
		glm::mat4 worldMatrix = {};
		glm::mat4 viewMatrix = {};
		glm::mat4 projectionMatrix = {};
	};

	/**
	* Объект передаваемый в динамический uniform-буфер, описывающий матрицы для каждого отдельного объекта
	* По сути является указателем на массив матриц, при его аллокации используется выравнивание
	*/
	typedef glm::mat4 * UboModelArray;

	/**
	* Структура описывающая вершину
	* Содержит координаты вершины в 3 измерениях, цвет (RGB) и текстурные координаты (на плоскости)
	*/
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 texCoord;
	};

	/**
	* Структура с набором примтивов синхронизации (семафоры)
	* Используется для синхронизации команд рендеринга и запросов показа изображения
	*/
	struct Synchronization
	{
		VkSemaphore readyToRender = VK_NULL_HANDLE;
		VkSemaphore readyToPresent = VK_NULL_HANDLE;
	};

	/**
	* Стурктура описывающая примитив (набор вершин)
	* Содержит хендлы буферов вершин и индексов, а так же параметры положеняи примитива
	* - Позиция в глобальном пространстве
	* - Повторот относительно локального (своего) центра
	* - Масштаб (размер)
	*/
	struct Primitive
	{
		bool drawIndexed = true;
		vktoolkit::VertexBuffer vertexBuffer;
		vktoolkit::IndexBuffer indexBuffer;
		glm::vec3 position = {};
		glm::vec3 rotation = {};
		glm::vec3 scale = {};
	};

	/**
	* Стурктура описывает параметры камеры
	* Содержит параметры используемые для подготовки матриц проекции и вида
	*
	* Матрица проекции используется для проецирования 3-мерных точек на двумерную
	* плоскость. Для ее построения используются такие параметры как:
	* - Угол обзора
	* - Пропорции
	* - Ближняя граница отсечения (ближе которой ничего не будет отображаться)
	* - Дальняя граница отсечения (дальше которой все будет отбрасываться)
	*
	* Матрица вида отвечает за положение и поворот камеры в пространстве. Матрица
	* вида по своей сути - матрица перехода из одной системы коородинат в другую. То есть
	* веришины начинают проецироваться так, будто бы они сместились относительно некоего 
	* глобального центра, с учетом положения локальной системы координат (камеры) относительного 
	* глобальной, что и дает эффект перемещения наблюдателя
	*/
	struct CameraSettings
	{
		// Положение и поворот камеры
		glm::vec3 position = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 roation = glm::vec3(0.0f, 0.0f, 0.0f);

		// Угол обзора, пропорции, ближняя и дальняя грань отсечения
		float fFOV = 60.0f;
		float aspectRatio = 1.0f;
		float fNear = 0.1f;
		float fFar = 256.0f;

		// Подготовить матрицу проекии
		glm::mat4 MakeProjectionMatrix() const {
			glm::mat4 result = glm::perspective(glm::radians(this->fFOV), aspectRatio, fNear, fFar);
			result[1][1] *= -1; // Хотфикс glm для вулканом (glm разработан для opengl, а там ось Y инвертирована)
			return result;
		};

		// Подготовить матрицу вида
		glm::mat4 MakeViewMatrix() const {
			glm::mat4 rotationMatrix = glm::rotate(glm::mat4(), glm::radians(this->roation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			rotationMatrix = glm::rotate(rotationMatrix, glm::radians(this->roation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			rotationMatrix = glm::rotate(rotationMatrix, glm::radians(this->roation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			glm::mat4 translationMatrix = glm::translate(glm::mat4(), this->position);

			return rotationMatrix * translationMatrix;
		};
	};

	/* В С П О М О Г А Т Е Л Ь Н Ы Е  М Е Т О Д Ы */

	/**
	* Метод обратного вызова для обработки сообщений об ошибках и предупреждениях в ходе работы Vulkan
	* @note - функция указывается при создании callback'а (параметр pfnCallback) при инициализации экземпляра Vulkan
	*/
	VKAPI_ATTR VkBool32 VKAPI_CALL DebugVulkanCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData);

	/**
	* Проверка поддержки расширений instance'а
	* @param instanceExtensionsNames - масив c-строчек содержащих имена расширений
	* @return bool - состояние наличия поддержки
	* @note - некоторый функционал vulkan'а есть только в расширениях, если они нужны - нужно убедится что они поддерживаются
	*/
	bool CheckInstanceExtensionsSupported(std::vector<const char*> instanceExtensionsNames);

	/**
	* Проверка поддержки расширений устройства
	* @param deviceExtensionsNames - масив c-строчек содержащих имена расширений
	* @return bool - состояние наличия поддержки
	* @note - аналагично, как и с расширениями instance'а, некоторый функционал vulkan'а есть только в расширениях
	*/
	bool CheckDeviceExtensionSupported(VkPhysicalDevice physicalDevice, std::vector<const char*> deviceExtensionsNames);

	/**
	* Проверка поддержки слоев валидации
	* @param validationLayersNames - масив c-строчек содержащих имена слоев валидации
	* @return bool - состояние наличия поддержки
	* @note - слои предоставляют возможность отлаживать программу и проверять данные, если это надо - нужно убедится что они поддерживаются
	*/
	bool CheckValidationsLayersSupported(std::vector<const char*> validationLayersNames);

	/**
	* Метод получения информации о семействах очередей (ID'ы нужных семейств очередей и т.п.)
	* @param physicalDevice - хендл физического устройства информацю о семействах очередей которого нужно получить
	* @param surface - хендл поверхности для которой осуществляется проверка поддержки тех или иных семейств
	* @param uniqueStrict - нужно ли заправшивать уникальные семейства для команд рисования и представления (семейство может быть одно)
	* @return QueueFamilyInfo - объект с ID'ами семейств очередей команд рисования (graphics) и представления (present)
	*/
	QueueFamilyInfo GetQueueFamilyInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, bool uniqueStrict = false);

	/**
	* Метод получения информации об особенностях поверхности
	* @param VkPhysicalDevice physicalDevice - хендл физического устройства которое предоставляет поддержку тех или иных форматов конкретной поверхностью
	* @param VkSurfaceKHR surface - хендл поверхности о которой нужно получить информацию
	* @return SurfaceInfo - объект у которого есть массивы форматов (formats), режимов (presentModes) и набор возможностей (capabilities)
	*/
	SurfaceInfo GetSurfaceInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	/**
	* Получить индекс типа памяти, которая поддерживает конкретные особенности
	* @param physicalDevice - хендл физического устройства информацю о возможных типах памяти которого нужно получить
	* @param unsigned int typeFlags - побитовая маска с флагами типов запрашиваемой памяти
	* @param VkMemoryPropertyFlags properties - параметры запрашиваемой памяти
	* @return int - возвращает индекс типа памяти, который соответствует всем условиям
	* @note - данный метод используется, например, при создании буферов
	*/
	int GetMemoryTypeIndex(VkPhysicalDevice physicalDevice, unsigned int typeFlags, VkMemoryPropertyFlags properties);

	/**
	* Создание буфера
	* @param vktoolkit::Device &device - устройство в памяти которого, либо с доступном для которого, будет создаваться буфер
	* @param VkDeviceSize size - размер создаваемого буфера
	* @param VkBufferUsageFlags usage - как буфер будет использован (например, как вершинный - VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
	* @param VkMemoryPropertyFlags properties - свойства памяти буфера (память устройства, память хоста, для "кого" память видима и т.д.)
	* @param VkSharingMode sharingMode - настройка доступа к памяти буфера для очередей (VK_SHARING_MODE_EXCLUSIVE - с буфером работает одна очередь)
	* @return vktoolkit::Buffer - структура содержающая хендл буфера, хендл памяти а так же размер буфера
	*/
	Buffer CreateBuffer(const vktoolkit::Device &device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);

	/**
	* Получить описание привязок вершинных данных к конвейеру
	* @param unsigned int bindingIndex - индекс привязки буфера вершин к конвейеру
	* @return std::vector<VkVertexInputBindingDescription> - массив описаний привязок
	*
	* @note - при привязывании буфера вершин к конвейеру, указывается индекс привязки. Нужно получить информацию
	* для конкретной привязки, о том как конвейер будет интерпретировать привязываемый буфер, какого размера
	* один элемент (вершина) в буфере, как переходить к следующему и тд. Вся информация в этой структуре
	*/
	std::vector<VkVertexInputBindingDescription> GetVertexInputBindingDescriptions(unsigned int bindingIndex);

	/**
	* Получить описание аттрибутов привязываемых к конвейеру вершин
	* @param unsigned int bindingIndex - индекс привязки буфера вершин к конвейеру
	* @return std::vector<VkVertexInputAttributeDescription> - массив описаний атрибутов передаваемых вершин
	*
	* @note - конвейеру нужно знать как интерпретировать данные о вершинах. Какие у каждой вершины, в привязываемом буфере,
	* есть параметры (аттрибуты). В какой последовательности они идут, какого типа каждый аттрибут.
	*/
	std::vector<VkVertexInputAttributeDescription> GetVertexInputAttributeDescriptions(unsigned int bindingIndex);

	/**
	* Загрузка шейдерного модуля из файла
	* @param std::string filename - наименование файла шейдера, поиск по умолчанию в папке shaders
	* @param VkDevice logicalDevice - хендл логичекского устройства, нужен для созданния шейдерного модуля
	* @return VkShaderModule - хендл шейдерного модуля созданного из загруженного файла
	*/
	VkShaderModule LoadSPIRVShader(std::string filename, VkDevice logicalDevice);
};

