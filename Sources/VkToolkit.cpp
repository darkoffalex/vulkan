/*
* Набор вспомогательных инструментов для работы с Vulkan и рендерером
*
* Copyright (C) 2018 by Alex "DarkWolf" Nem - https://github.com/darkoffalex
*/

#include "VkToolkit.h"

/**
* Метод обратного вызова для обработки сообщений об ошибках и предупреждениях в ходе работы Vulkan
* @note - функция указывается при создании callback'а (параметр pfnCallback) при инициализации экземпляра Vulkan
*/
VKAPI_ATTR VkBool32 VKAPI_CALL vktoolkit::DebugVulkanCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData)
{
	std::string message;
	message.append("Vulkan: validation layer - ");
	message.append(msg);
	toolkit::LogMessage(message);
	return VK_FALSE;
}

/**
* Проверка поддержки расширений instance'а
* @param instanceExtensionsNames - масив c-строчек содержащих имена расширений
* @return bool - состояние наличия поддержки
* @note - некоторый функционал vulkan'а есть только в расширениях, если они нужны - нужно убедится что они поддерживаются
*/
bool vktoolkit::CheckInstanceExtensionsSupported(std::vector<const char*> instanceExtensionsNames)
{
	// Доступные расширения instance'а (массив структур)
	std::vector<VkExtensionProperties> availableExtensions;

	// Получение кол-ва доступных расширений
	unsigned int instanceExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);

	// Если кол-во нулевое - однозначно не поддерживается
	if (instanceExtensionCount == 0) {
		return false;
	}

	// Получение самих расширений, заполнение массива структур
	availableExtensions.resize(instanceExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, availableExtensions.data());

	// Пройти по всем запрашиваемым расширениям, и проверить есть ли каждое в списке доступных
	// Если какое-то не найдено - вернуть false
	for (const char* requiredExtName : instanceExtensionsNames) {
		bool found = false;
		for (const VkExtensionProperties &extProperties : availableExtensions) {
			if (strcmp(requiredExtName, extProperties.extensionName) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			return false;
		}
	}

	// Поддерживаются все расширения
	return true;
}

/**
* Проверка поддержки расширений устройства
* @param deviceExtensionsNames - масив c-строчек содержащих имена расширений
* @return bool - состояние наличия поддержки
* @note - аналагично, как и с расширениями instance'а, некоторый функционал vulkan'а есть только в расширениях
*/
bool vktoolkit::CheckDeviceExtensionSupported(VkPhysicalDevice physicalDevice, std::vector<const char*> deviceExtensionsNames)
{
	// Доступные расширения устройства (массив структур)
	std::vector<VkExtensionProperties> availableExtensions;

	// Получение кол-ва доступных расширений
	unsigned int deviceExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, nullptr);

	// Если кол-во нулевое - однозначно не поддерживается
	if (deviceExtensionCount == 0) {
		return false;
	}

	// Получение самих расширений, заполнение массива
	availableExtensions.resize(deviceExtensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, availableExtensions.data());

	// Пройти по всем запрашиваемым расширениям, и проверить есть ли каждое в списке доступных
	// Если какое-то не найдено - вернуть false
	for (const char* requiredExtName : deviceExtensionsNames) {
		bool found = false;
		for (const VkExtensionProperties &extProperties : availableExtensions) {
			if (strcmp(requiredExtName, extProperties.extensionName) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			return false;
		}
	}

	// Поддерживаются все расширения
	return true;
}

/**
* Проверка поддержки слоев валидации
* @param validationLayersNames - масив c-строчек содержащих имена слоев валидации
* @return bool - состояние наличия поддержки
* @note - слои предоставляют возможность отлаживать программу и проверять данные, если это надо - нужно убедится что они поддерживаются
*/
bool vktoolkit::CheckValidationsLayersSupported(std::vector<const char*> validationLayersNames)
{
	// Доступные слои валидации
	std::vector<VkLayerProperties> availableLayers;

	// Получение кол-ва доступных слоев
	unsigned int layersCount = 0;
	vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

	// Если кол-во нулевое - однозначно не поддерживается
	if (layersCount == 0) {
		return false;
	}

	// Получение самих слоев, заполнение массива
	availableLayers.resize(layersCount);
	vkEnumerateInstanceLayerProperties(&layersCount, availableLayers.data());

	// Пройти по всем запрашиваемым слоям, и проверить есть ли каждый в списке доступных
	// Если какой-то не найден - вернуть false
	for (const char* requiredName : validationLayersNames) {
		bool found = false;
		for (const VkLayerProperties &properties : availableLayers) {
			if (strcmp(requiredName, properties.layerName) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			return false;
		}
	}

	// Поддерживаются все
	return true;
}

/**
* Метод получения информации о семействах очередей (ID'ы нужных семейств очередей и т.п.)
* @param physicalDevice - хендл физического устройства информацю о семействах очередей которого нужно получить
* @param surface - хендл поверхности для которой осуществляется проверка поддержки тех или иных семейств
* @param uniqueStrict - нужно ли заправшивать уникальные семейства для команд рисования и представления (семейство может быть одно)
* @return QueueFamilyInfo - объект с ID'ами семейств очередей команд рисования (graphics) и представления (present)
*/
vktoolkit::QueueFamilyInfo vktoolkit::GetQueueFamilyInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, bool uniqueStrict)
{
	// Объект структуры QueueFamilyInfo с информацией о семействах, который будет возвращен функцией
	vktoolkit::QueueFamilyInfo qfi;

	// Получить кол-во семейств очередей поддерживаемых физическим устройством (видео-картой)
	unsigned int queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	// Инициализировать массив семейств очередей и получить эти семейства
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	// Найти то семейство, которое поддерживает грфические команды, и записать его индекс в объект qfi (как индекс граф. семейства)
	for (unsigned int i = 0; i < queueFamilies.size(); i++) {
		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			qfi.graphics = i;
			break;
		}
	}

	// Найти то семейство, которое поддерживает представление и записать его индекс в объект qfi (как индекс семейств представления)
	for (unsigned int i = 0; i < queueFamilies.size(); i++) {

		// Если необходимо чтобы ID'ы были уникальными (одно и то же семейство может поддерживать и те и другие команды, и ID'ы могут совпадать)
		// Пропускаем итерацию в случае совпадения текущего ID'а с ID'ом графической очереди
		if (i == qfi.graphics && uniqueStrict) {
			continue;
		}

		// Поддержка представления (по умолчанию - нет)
		unsigned int presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

		if (queueFamilies[i].queueCount > 0 && presentSupport) {
			qfi.present = i;
			break;
		}
	}

	return qfi;
}

/**
* Метод получения информации об особенностях поверхности
* @param VkPhysicalDevice physicalDevice - хендл физического устройства которое предоставляет поддержку тех или иных форматов конкретной поверхностью
* @param VkSurfaceKHR surface - хендл поверхности о которой нужно получить информацию
* @return SurfaceInfo - объект у которого есть массивы форматов (formats), режимов (presentModes) и набор возможностей (capabilities)
*/
vktoolkit::SurfaceInfo vktoolkit::GetSurfaceInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	// Получить информацию о возможностях поверхности
	SurfaceInfo si;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &(si.capabilities));

	// Получить кол-во поддерживаемых форматов
	unsigned int formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

	// Если какие-либо форматы поддерживаются - получить их
	if (formatCount > 0) {
		si.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, si.formats.data());
	}

	// Получить кол-во режимов представления
	unsigned int presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

	// Если какие-либо режимы представления поддерживаются - получить их
	if (presentModeCount > 0) {
		si.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, si.presentModes.data());
	}

	return si;
}

/**
* Получить индекс типа памяти, которая поддерживает конкретные особенности
* @param physicalDevice - хендл физического устройства информацю о возможных типах памяти которого нужно получить
* @param unsigned int typeFlags - побитовая маска с флагами типов запрашиваемой памяти
* @param VkMemoryPropertyFlags properties - параметры запрашиваемой памяти
* @return int - возвращает индекс типа памяти, который соответствует всем условиям
* @note - данный метод используется, например, при создании буферов
*/
int vktoolkit::GetMemoryTypeIndex(VkPhysicalDevice device, unsigned int typeFlags, VkMemoryPropertyFlags properties)
{
	// Получить настройки памяти физического устройства
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(device, &deviceMemoryProperties);

	// Пройтись по всем типам и найти подходящий
	// Для опредения нужного индекса типа памяти использются побитовые операции, подробнее о побитовых операциях - https://ravesli.com/urok-45-pobitovye-operatory/
	for (unsigned int i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
		if ((typeFlags & (1 << i)) && (deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	// Отрицательное значение в случае отсутствия необходимого индекса
	return -1;
}

/**
* Создание буфера
* @param vktoolkit::Device &device - устройство в памяти которого, либо с доступном для которого, будет создаваться буфер
* @param VkDeviceSize size - размер создаваемого буфера
* @param VkBufferUsageFlags usage - как буфер будет использован (например, как вершинный - VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
* @param VkMemoryPropertyFlags properties - свойства памяти буфера (память устройства, память хоста, для "кого" память видима и т.д.)
* @param VkSharingMode sharingMode - настройка доступа к памяти буфера для очередей (VK_SHARING_MODE_EXCLUSIVE - с буфером работает одна очередь)
* @return vktoolkit::Buffer - структура содержающая хендл буфера, хендл памяти а так же размер буфера
*/
vktoolkit::Buffer vktoolkit::CreateBuffer(const vktoolkit::Device &device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkSharingMode sharingMode)
{
	// Объект буфера что будет отдан функцией
	vktoolkit::Buffer resultBuffer;

	// Установить размер
	resultBuffer.size = size;

	// Настройка создания vk-буфера
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = sharingMode;
	bufferInfo.flags = 0;

	// Попытка создания буфера
	if (vkCreateBuffer(device.logicalDevice, &bufferInfo, nullptr, &(resultBuffer.vkBuffer)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating buffer. Can't create!");
	}

	// Получить требования буфера к памяти
	VkMemoryRequirements memRequirements = {};
	vkGetBufferMemoryRequirements(device.logicalDevice, resultBuffer.vkBuffer, &memRequirements);

	// Получить индекс типа памяти соответствующего требованиям буфера
	int memoryTypeIndex = vktoolkit::GetMemoryTypeIndex(device.physicalDevice, memRequirements.memoryTypeBits, properties);
	if (memoryTypeIndex < 0) {
		throw std::runtime_error("Vulkan: Error while creating buffer. Can't find suitable memory type!");
	}

	// Настрйока выделения памяти (учитывая требования и полученный индекс)
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = (unsigned int)memoryTypeIndex;

	// Выделение памяти для буфера
	if (vkAllocateMemory(device.logicalDevice, &memoryAllocateInfo, nullptr, &(resultBuffer.vkDeviceMemory)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while allocating buffer memory!");
	}

	// Привязать память к буферу
	vkBindBufferMemory(device.logicalDevice, resultBuffer.vkBuffer, resultBuffer.vkDeviceMemory, 0);

	// Вернуть буфер
	return resultBuffer;
}

/**
* Получить описание привязок вершинных данных к конвейеру
* @param unsigned int bindingIndex - индекс привязки буфера вершин к конвейеру
* @return std::vector<VkVertexInputBindingDescription> - массив описаний привязок
*
* @note - при привязывании буфера вершин к конвейеру, указывается индекс привязки. Нужно получить информацию
* для конкретной привязки, о том как конвейер будет интерпретировать привязываемый буфер, какого размера
* один элемент (вершина) в буфере, как переходить к следующему и тд. Вся информация в этой структуре
*/
std::vector<VkVertexInputBindingDescription> vktoolkit::GetVertexInputBindingDescriptions(unsigned int bindingIndex)
{
	return
	{
		{
			bindingIndex,                   // Индекс привязки вершинных буферов
			sizeof(vktoolkit::Vertex),      // Размерность шага
			VK_VERTEX_INPUT_RATE_VERTEX     // Правила перехода к следующим
		}
	};
}

/**
* Получить описание аттрибутов привязываемых к конвейеру вершин
* @param unsigned int bindingIndex - индекс привязки буфера вершин к конвейеру
* @return std::vector<VkVertexInputAttributeDescription> - массив описаний атрибутов передаваемых вершин
*
* @note - конвейеру нужно знать как интерпретировать данные о вершинах. Какие у каждой вершины, в привязываемом буфере,
* есть параметры (аттрибуты). В какой последовательности они идут, какого типа каждый аттрибут.
*/
std::vector<VkVertexInputAttributeDescription> vktoolkit::GetVertexInputAttributeDescriptions(unsigned int bindingIndex)
{
	return
	{
		{
			0,                                      // Индекс аттрибута (location в шейдере)
			bindingIndex,                           // Индекс привязки вершинных буферов
			VK_FORMAT_R32G32B32_SFLOAT,             // Тип аттрибута (соответствует vec3 у шейдера)
			offsetof(vktoolkit::Vertex, position)   // Cдвиг в структуре
		},
		{
			1,
			bindingIndex,
			VK_FORMAT_R32G32B32_SFLOAT,
			offsetof(vktoolkit::Vertex, color)
		},
		{
			2,
			bindingIndex,
			VK_FORMAT_R32G32_SFLOAT,               // Тип аттрибута (соответствует vec2 у шейдера)
			offsetof(vktoolkit::Vertex, texCoord)
		},
	};
}

/**
* Загрузка шейдерного модуля из файла
* @param std::string filename - наименование файла шейдера, поиск по умолчанию в папке shaders
* @param VkDevice logicalDevice - хендл логичекского устройства, нужен для созданния шейдерного модуля
* @return VkShaderModule - хендл шейдерного модуля созданного из загруженного файла
*/
VkShaderModule vktoolkit::LoadSPIRVShader(std::string filename, VkDevice logicalDevice)
{
	// Размер
	size_t shaderSize;

	// Содержимое файла (код шейдера)
	char* shaderCode = nullptr;

	// Загрузить код шейдера
	bool loaded = toolkit::LoadBytesFromFile(toolkit::ExeDir() + "/../shaders/" + filename, &shaderCode, &shaderSize);

	// Если не удалось загрузить или файл пуст
	if (!loaded || shaderSize == 0){
		std::string msg = "Vulkan: Error while loading shader code from file " + filename;
		throw std::runtime_error(msg);
	}

	// Конфигурация шейдерного модуля
	VkShaderModuleCreateInfo moduleCreateInfo{};
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.codeSize = shaderSize;
	moduleCreateInfo.pCode = (unsigned int*)shaderCode;

	// Создать шейдерный модуль
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logicalDevice, &moduleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		std::string msg = "Vulkan: Error whiler creating shader module from file " + filename;
		throw std::runtime_error(msg);
	}

	// Удаляем код шейдера (хранить его более не нужно, поскольку он был передан в шейдерный модуль)
	delete[] shaderCode;

	// Вернуть хендл шейдерного модуля
	return shaderModule;
}

