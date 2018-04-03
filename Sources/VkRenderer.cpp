#include "VkRenderer.h"

/* К О Н С Т Р У К Т О Р / Д Е С Т Р У К Т О Р */

/**
* Конструктор рендерера
* @param HINSTANCE hInstance - хендл Windows приложения
* @param HWND hWnd хендл - Windows окна
* @param int primitivesMaxCount - максимальное кол-во отдельных объектов для отрисовки
* @note - конструктор запистит инициализацию всех необходимых компоненстов Vulkan
*/
VkRenderer::VkRenderer(HINSTANCE hInstance, HWND hWnd, unsigned int primitivesMaxCount):
	isReady_(false),
	isRendering_(true),
	instance_(VK_NULL_HANDLE),
	validationReportCallback_(VK_NULL_HANDLE),
	surface_(VK_NULL_HANDLE),
	renderPass_(VK_NULL_HANDLE),
	commandPoolDraw_(VK_NULL_HANDLE),
	descriptorSetLayoutMain_(VK_NULL_HANDLE),
	descriptorSetLayoutTextures_(VK_NULL_HANDLE),
	descriptorSetMain_(VK_NULL_HANDLE),
	pipelineLayout_(VK_NULL_HANDLE),
	pipeline_(VK_NULL_HANDLE),
	primitivesMaxCount_(primitivesMaxCount),
	uboModels_(nullptr)
{
	// Присвоить параметры камеры по умолчанию
	this->camera_.fFar = DEFAULT_FOV;
	this->camera_.fFar = DEFAULT_FAR;
	this->camera_.fNear = DEFAULT_NEAR;

	// Списки расширений и слоев запрашиваемых по умолчанию
	std::vector<const char*> instanceExtensionsRequired = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
	std::vector<const char*> deviceExtensionsRequired = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	std::vector<const char*> validationLayersRequired = {};

	// Если это DEBUG конфигурация - запросить еще расширения и слои для валидации
	if (IS_VK_DEBUG) {
		instanceExtensionsRequired.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		validationLayersRequired.push_back("VK_LAYER_LUNARG_standard_validation");
	}

	// Инициализация экземпляра
	this->instance_ = this->InitInstance("Sample Vulkan App", "Sample Engine", instanceExtensionsRequired, validationLayersRequired);

	// Инициализация поверхности отображения
	this->surface_ = this->InitWindowSurface(this->instance_, hInstance, hWnd);

	// Инициализация устройства
	this->device_ = this->InitDevice(this->instance_, this->surface_, deviceExtensionsRequired, validationLayersRequired, false);

	// Инициализация прохода рендеринга
	this->renderPass_ = this->InitRenderPass(this->device_, this->surface_, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT);

	// Инициализация swap-chain
	this->swapchain_ = this->InitSwapChain(this->device_, this->surface_, { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, VK_FORMAT_D32_SFLOAT_S8_UINT, this->renderPass_, 3, nullptr);

	// Инициализация командного пула
	this->commandPoolDraw_ = this->InitCommandPool(this->device_, this->device_.queueFamilies.graphics);

	// Аллокация командных буферов (получение хендлов)
	this->commandBuffersDraw_ = this->InitCommandBuffers(this->device_, this->commandPoolDraw_, (unsigned int)(this->swapchain_.framebuffers.size()));

	// Аллокация глобального uniform-буфера
	this->uniformBufferWorld_ = this->InitUnformBufferWorld(this->device_);

	// Аллокация uniform-буфера отдельных объектов (динамический буфер)
	this->uniformBufferModels_ = this->InitUniformBufferModels(this->device_, this->primitivesMaxCount_);

	// Создание дескрипторного пула для выделения основного набора (для unform-буфера)
	this->descriptorPoolMain_ = this->InitDescriptorPoolMain(this->device_);

	// Создание дескрипторного пула для выделения текстурного набора (текстурные семплеры)
	this->descriptorPoolTextures_ = this->InitDescriptorPoolTextures(this->device_);

	// Инициализация размещения основного дескрипторного набора
	this->descriptorSetLayoutMain_ = this->InitDescriptorSetLayoutMain(this->device_);

	// Инициализация размещения теккстурного набора
	this->descriptorSetLayoutTextures_ = this->InitDescriptorSetLayoutTextures(this->device_);

	// Инициализация текстурного семплера
	this->textureSampler_ = this->InitTextureSampler(this->device_);

	// Инициализация дескрипторного набора
	this->descriptorSetMain_ = this->InitDescriptorSetMain(
		this->device_,
		this->descriptorPoolMain_,
		this->descriptorSetLayoutMain_,
		this->uniformBufferWorld_,
		this->uniformBufferModels_);

	// Аллокация памяти массива ubo-объектов отдельных примитивов
	this->uboModels_ = this->AllocateUboModels(this->device_, this->primitivesMaxCount_);

	// Инициализация размещения графического конвейера
	this->pipelineLayout_ = this->InitPipelineLayout(this->device_, { this->descriptorSetLayoutMain_, this->descriptorSetLayoutTextures_});

	// Инициализация графического конвейера
	this->pipeline_ = this->InitGraphicsPipeline(this->device_, this->pipelineLayout_, this->swapchain_, this->renderPass_);

	// Примитивы синхронизации
	this->sync_ = this->InitSynchronization(this->device_);

	// Подготовка базовых комманд
	this->PrepareDrawCommands(
		this->commandBuffersDraw_,
		this->renderPass_,
		this->pipelineLayout_,
		this->descriptorSetMain_,
		this->pipeline_,
		this->swapchain_,
		this->primitives_);

	// Готово к рендерингу
	this->isReady_ = true;

	// Обновить
	this->Update();
}

/**
* Деструктор рендерера
* @note Деструктор уничтожит все используемые компоненты и освободит память
*/
VkRenderer::~VkRenderer()
{
	this->Pause();
	this->isReady_ = false;

	// Сброс буферов команд
	this->ResetCommandBuffers(this->device_, this->commandBuffersDraw_);

	// Деинициализация примитивов синхронизации
	this->DeinitSynchronization(this->device_, &(this->sync_));

	// Деинициализация графического конвейера
	this->DeinitGraphicsPipeline(this->device_, &(this->pipeline_));

	// Деинициализация графического конвейера
	this->DeinitPipelineLayout(this->device_, &(this->pipelineLayout_));

	// Очистка памяти массива ubo-объектов отдельных примитивов
	this->FreeUboModels(&(this->uboModels_));

	// Деинициализация набора дескрипторов
	this->DeinitDescriptorSet(this->device_, this->descriptorPoolMain_, &(this->descriptorSetMain_));

	// Деинициализация текстурного семплера
	this->DeinitTextureSampler(this->device_, &(this->textureSampler_));

	// Деинициализация размещения текстурного дескрипторного набора
	this->DeinitDescriporSetLayout(this->device_, &(this->descriptorSetLayoutTextures_));

	// Деинициализация размещения основоного дескрипторного набора
	this->DeinitDescriporSetLayout(this->device_, &(this->descriptorSetLayoutMain_));

	// Уничтожение ntrcnehyjuj дескрипторного пула
	this->DeinitDescriptorPool(this->device_, &(this->descriptorPoolTextures_));

	// Уничтожение основного дескрипторного пула
	this->DeinitDescriptorPool(this->device_, &(this->descriptorPoolMain_));

	// Деинициализация uniform-буффера объектов
	this->DeinitUniformBuffer(this->device_, &(this->uniformBufferModels_));

	// Деинициализация глобального uniform-буффера
	this->DeinitUniformBuffer(this->device_, &(this->uniformBufferWorld_));

	// Деинициализация командных буферов
	this->DeinitCommandBuffers(this->device_, this->commandPoolDraw_, &(this->commandBuffersDraw_));

	// Деинициализация командного пула
	this->DeinitCommandPool(this->device_, &(this->commandPoolDraw_));

	// Деинициализация swap-chain'а
	this->DeinitSwapchain(this->device_, &(this->swapchain_));

	// Деинциализация прохода рендеринга
	this->DeinitRenderPass(this->device_, &(this->renderPass_));

	// Динициализация устройства
	this->DeinitDevice(&(this->device_));

	// Деинициализация поверзности
	this->DeinitWindowSurface(this->instance_, &(this->surface_));

	// Деинициализация экземпляра Vulkan
	this->DeinitInstance(&(this->instance_));
}

/* I N S T A N C E */

/**
* Метод инициализации instance'а vulkan'а
* @param std::string applicationName - наименование приложения (информация может быть полезна разработчикам драйверов)
* @param std::string engineName - наименование движка (информация может быть полезна разработчикам драйверов)
* @param std::vector<const char*> extensionsRequired - запрашиваемые расширения экземпляра
* @param std::vector<const char*>validationLayersRequired - запрашиваемые слои валидации
* @return VkInstance - хендл экземпляра Vulkan
*/
VkInstance VkRenderer::InitInstance(
	std::string applicationName,
	std::string engineName,
	std::vector<const char*> extensionsRequired,
	std::vector<const char*> validationLayersRequired)
{
	// Структура с информацией о создаваемом приложении
	// Здесь содержиться информация о названии, версии приложения и движка. Эта информация может быть полезна разработчикам драйверов
	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = applicationName.c_str();
	applicationInfo.pEngineName = engineName.c_str();
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	// Структура с информацией о создаваемом экземпляре vulkan
	// Здесь можно указать информацию о приложении (ссылка на структуру выше) а так же указать используемые расширения
	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;

	// Запрашивается ли валидация
	bool validationQueried = false;

	// Если запрашиваются расширения
	if (!extensionsRequired.empty())
	{
		// Если не все запрашиваемые расширения доступны - ошибка
		if (!vktoolkit::CheckInstanceExtensionsSupported(extensionsRequired)) {
			throw std::runtime_error("Vulkan: Not all required instance extensions supported. Can't initialize renderer");
		}

		// Указать запрашиваемые расширения и их кол-во
		instanceCreateInfo.ppEnabledExtensionNames = extensionsRequired.data();
		instanceCreateInfo.enabledExtensionCount = (uint32_t)extensionsRequired.size();

		// Запрашивается расширение для обработки ошибок (по умолчанию нет)
		bool debugReportExtensionQueried = false;

		// Если среди запрашиваемых расш. (которые точно поддерживаются, ппрверка была выше) есть EXT_DEBUG_REPORT
		// значит расширение для обработки ошибок запрошено (и точно поддерживается)
		for (const char* extensionName : extensionsRequired) {
			if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, extensionName) == 0) {
				debugReportExtensionQueried = true;
				break;
			}
		}

		// Если запрашивается расширение для обработки ошибок и предупреждений,
		// есть ссмысл проверить поддержку запрашиваемых слоев валидации
		if (debugReportExtensionQueried && !validationLayersRequired.empty()) {

			// Если какие-то слои валидации не поддерживаются - ошибка
			if (!vktoolkit::CheckValidationsLayersSupported(validationLayersRequired)) {
				throw std::runtime_error("Vulkan: Not all required validation layers supported. Can't initialize renderer");
			}

			// Указать слои валидации и их кол-во
			instanceCreateInfo.enabledLayerCount = (uint32_t)validationLayersRequired.size();
			instanceCreateInfo.ppEnabledLayerNames = validationLayersRequired.data();

			// Валидация запрашивается (поскольку есть и расширение и необходимые слои)
			validationQueried = true;
			toolkit::LogMessage("Vulkan: Validation quiried. Report callback object will be created");
		}
	}

	// Хендл экземпляра
	VkInstance vkInstance;

	// Создание экземпляра
	// Передается указатель на структуру CreateInfo, которую заполнили выше, и указатель на переменную хендла 
	// instance'а, куда и будет помещен сам хендл. Если функция не вернула VK_SUCCESS - ошибка
	if (vkCreateInstance(&instanceCreateInfo, nullptr, &(vkInstance)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in the 'vkCreateInstance' function. Can't initialize renderer");
	}

	toolkit::LogMessage("Vulkan: Instance sucessfully created");

	// Если запрошена валидация - создать callback метод для обработки исключений
	if (validationQueried) {

		// Конфигурация callback'а
		VkDebugReportCallbackCreateInfoEXT debugReportCallbackcreateInfo = {};
		debugReportCallbackcreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debugReportCallbackcreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		debugReportCallbackcreateInfo.pfnCallback = vktoolkit::DebugVulkanCallback;

		// Получить адрес функции создания callback'а (поскольку это функция расширения)
		PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
			(PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");

		// Создать debug report callback для вывода сообщений об ошибках
		if (vkCreateDebugReportCallbackEXT(vkInstance, &debugReportCallbackcreateInfo, nullptr, &(this->validationReportCallback_)) != VK_SUCCESS) {
			throw std::runtime_error("Vulkan: Error in the 'vkCreateDebugReportCallbackEXT' function. Can't initialize renderer");
		}

		toolkit::LogMessage("Vulkan: Report callback sucessfully created");
	}

	// Вернуть хендл instance'а vulkan'а
	return vkInstance;
}

/**
* Метод деинициализации Instance'а
* @param VkInstance * vkInstance - указатель на хендл Instance'а который нужно уничтожить
*/
void VkRenderer::DeinitInstance(VkInstance * vkInstance)
{
	// Если хендл debug callback'а не пуст - очистить
	if (this->validationReportCallback_ != VK_NULL_HANDLE) {

		// Получить функцию для его уничтожения (поскольку это функция расширения)
		PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
			(PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(*vkInstance, "vkDestroyDebugReportCallbackEXT");

		// Уничтожить
		vkDestroyDebugReportCallbackEXT(*vkInstance, this->validationReportCallback_, nullptr);
		this->validationReportCallback_ = VK_NULL_HANDLE;
		toolkit::LogMessage("Vulkan: Report callback sucessfully destroyed");
	}

	// Если instance не пуст - очистить
	if (*vkInstance != VK_NULL_HANDLE) {
		vkDestroyInstance(*vkInstance, nullptr);
		*vkInstance = VK_NULL_HANDLE;
		toolkit::LogMessage("Vulkan: Instance sucessfully destroyed");
	}
}

/* П О В Е Р Х Н О С Т Ь */

/**
* Инициализация поверхности отображения (поверхность окна)
* @param VkInstance vkInstance - хендл экземпляра Vulkan
* @param HINSTANCE hInstance - хендл Windows приложения
* @param HWND hWnd - хендл Windows окна
* @return VkSurfaceKHR - хендл созданной поверхности
*/
VkSurfaceKHR VkRenderer::InitWindowSurface(VkInstance vkInstance, HINSTANCE hInstance, HWND hWnd)
{
	// Конфигурация поверхности
	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKhr;
	win32SurfaceCreateInfoKhr.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfoKhr.hwnd = hWnd;
	win32SurfaceCreateInfoKhr.hinstance = hInstance;
	win32SurfaceCreateInfoKhr.flags = 0;
	win32SurfaceCreateInfoKhr.pNext = nullptr;

	// Получить адрес функции создания поверхности для окна Windows (поскольку это функция расширения)
	PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR =
		(PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(vkInstance, "vkCreateWin32SurfaceKHR");

	// Хендл поверхности
	VkSurfaceKHR surface;

	// Инициализация (создание) поверхности
	if (vkCreateWin32SurfaceKHR(vkInstance, &win32SurfaceCreateInfoKhr, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in the 'vkCreateWin32SurfaceKHR' function!");
	}

	toolkit::LogMessage("Vulkan: Window surface initilized");
	return surface;
}

/**
* Деинициализация поверхности отображения (поверхность окна)
* @param VkInstance vkInstance - хендл экземпляра Vulkan
* @param VkSurfaceKHR * surface - указатель на хендл поверхности которую нужно уничтожить
*/
void VkRenderer::DeinitWindowSurface(VkInstance vkInstance, VkSurfaceKHR * surface)
{
	// Если поверхность инициализирована - уничтожить
	if (surface != nullptr && *surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(vkInstance, *surface, nullptr);
		*surface = VK_NULL_HANDLE;
		toolkit::LogMessage("Vulkan: Window surface destroyed");
	}
}

/* У С Т Р О Й С Т В О */

/**
* Инициализации устройства. Поиск подходящего физ. устройства, создание логического на его основе.
* @param VkInstance vkInstance - хендл экземпляра Vulkan
* @param VkSurfaceKHR surface - хендл поверхности для которой будет проверяться поддержка необходимых очередей устройства
* @param std::vector<const char*> extensionsRequired - запрашиваемые расширения устройства
* @param std::vector<const char*> validationLayersRequired - запрашиваемые слои валидации
* @param bool uniqueQueueFamilies - нужны ли уникальные семейства очередей (разные) для показа и представления
* @return vktoolkit::Device - структура с хендлами устройства (физического и логического) и прочей информацией
*/
vktoolkit::Device VkRenderer::InitDevice(
	VkInstance vkInstance,
	VkSurfaceKHR surface,
	std::vector<const char*> extensionsRequired,
	std::vector<const char*> validationLayersRequired,
	bool uniqueQueueFamilies)
{
	// Устройство которое будет отдано (пустое)
	vktoolkit::Device resultDevice = {};

	// Получить кол-во видео-карт в системе
	unsigned int deviceCount = 0;
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

	// Если не нашлось видео-карт работающих с vulkan - ошибка
	if (deviceCount == 0) {
		throw std::runtime_error("Vulkan: Can't detect divice with Vulkan suppurt");
	}

	// Получить доступные видео-карты (их идентификаторы)
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, physicalDevices.data());


	// Пройтись по всем видео-картам и проверить каждую на соответствие минимальным требованиям, устройство должно поддерживать:
	// - Запрашиваемые расширения vulkan
	// - Семейтсва очередей поддерживающие отрисовку и представление
	// - Совместимость swap-chain с поверхностью отображения
	for (const VkPhysicalDevice& physicalDevice : physicalDevices)
	{
		// Получить информацию об очередях поддерживаемых устройством
		resultDevice.queueFamilies = vktoolkit::GetQueueFamilyInfo(physicalDevice, surface, uniqueQueueFamilies);

		// Если очереди данного устройства не совместимы с рендерингом - переходим к следующему
		if (!(resultDevice.queueFamilies.IsRenderingCompatible())) {
			continue;
		}

		// Если данное устройство не поддерживает запрашиваемые расширения - переходим к следующему
		if (extensionsRequired.size() > 0 && !vktoolkit::CheckDeviceExtensionSupported(physicalDevice, extensionsRequired)) {
			continue;
		}

		// Получить информацию о том как устройство может работать с поверхностью
		// Если для поверхности нет форматов и режимов показа (представления) - переходим к след. устройству
		vktoolkit::SurfaceInfo si = vktoolkit::GetSurfaceInfo(physicalDevice, surface);
		if (si.formats.empty() || si.presentModes.empty()) {
			continue;
		}

		// Записать хендл физического устройства, которое прошло все проверки
		resultDevice.physicalDevice = physicalDevice;
	}

	// Если не нашлось видео-карт которые удовлетворяют всем требованиям - ошибка
	if (resultDevice.physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("Vulkan: Error in the 'InitDevice' function! Can't detect suitable device");
	}

	// Массив объектов структуры VkDeviceQueueCreateInfo содержащих информацию для инициализации очередей
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	// Массив инедксов семейств (2 индекса - графич. семейство и семейство представления. 
	// Индексы могут совпадать, семейство у обеих очередей может быть одно и то же)
	uint32_t queueFamilies[2] = { (uint32_t)resultDevice.queueFamilies.graphics, (uint32_t)resultDevice.queueFamilies.present };

	// Если графич. семейство и семейство представления - одно и то же (тот же индекс),
	// нет смысла создавать две очереди одного и того же семейства, можно обойтись одной
	for (int i = 0; i < (uniqueQueueFamilies ? 2 : 1); i++) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilies[i];
		queueCreateInfo.queueCount = 1;                                      // Выделяем одну очередь для каждого семейства
		queueCreateInfo.pQueuePriorities = nullptr;                          // Массив пр-тетов очередей в плане выделения ресурсов (одинаковые пр-теты, не используем)
		queueCreateInfos.push_back(queueCreateInfo);                         // Помещаем структуру в массив
	}


	// Информация о создаваемом логическом устройстве
	// Указываем структуры для создания очередей (queueCreateInfos) и используемые устройством расширения (deviceExtensionsRequired_)
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = (unsigned int)queueCreateInfos.size();

	// Проверка запрашиваемых расширений, указать если есть (если не доступны - ошибка)
	if (!extensionsRequired.empty()) {
		if (!vktoolkit::CheckDeviceExtensionSupported(resultDevice.physicalDevice, extensionsRequired)) {
			throw std::runtime_error("Vulkan: Not all required device extensions supported. Can't initialize renderer");
		}

		deviceCreateInfo.enabledExtensionCount = (uint32_t)extensionsRequired.size();
		deviceCreateInfo.ppEnabledExtensionNames = extensionsRequired.data();
	}

	// Проверка запрашиваемых слоев валидации, указать если есть (если не доступны - ошибка)
	if (!validationLayersRequired.empty()) {
		if (!vktoolkit::CheckValidationsLayersSupported(validationLayersRequired)) {
			throw std::runtime_error("Vulkan: Not all required validation layers supported. Can't initialize renderer");
		}

		deviceCreateInfo.enabledLayerCount = (uint32_t)validationLayersRequired.size();
		deviceCreateInfo.ppEnabledLayerNames = validationLayersRequired.data();
	}

	// Особенности устройства (пока-что пустая структура)
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// Создание логического устройства
	if (vkCreateDevice(resultDevice.physicalDevice, &deviceCreateInfo, nullptr, &(resultDevice.logicalDevice)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Failed to create logical device. Can't initialize renderer");
	}

	// Получить хендлы очередей устройства (графической очереди и очереди представления)
	// Если использовано одно семейство, то индексы первых (нулевых) очередей для Graphics и Present будут одинаковы
	vkGetDeviceQueue(resultDevice.logicalDevice, resultDevice.queueFamilies.graphics, 0, &(resultDevice.queues.graphics));
	vkGetDeviceQueue(resultDevice.logicalDevice, resultDevice.queueFamilies.present, 0, &(resultDevice.queues.present));

	// Если в итоге устройство не готово - ошибка
	if (!resultDevice.IsReady()) {
		throw std::runtime_error("Vulkan: Failed to initialize device and queues. Can't initialize renderer");
	}

	// Сообщение об успешной инициализации устройства
	std::string deviceName = std::string(resultDevice.GetProperties().deviceName);
	std::string message = "Vulkan: Device successfully initialized (" + deviceName + ")";
	toolkit::LogMessage(message);

	// Вернуть устройство
	return resultDevice;
}

/**
* Деинициализация поверхности отображения (поверхность окна)
* @param vktoolkit::Device * device - указатель на структуру с хендлами устройства
*/
void VkRenderer::DeinitDevice(vktoolkit::Device * device)
{
	device->Deinit();
	toolkit::LogMessage("Vulkan: Device successfully destroyed");
}

/* П Р О Х О Д  Р Е Н Д Е Р И Н Г А */

/**
* Инициализация прохода рендеринга.
* @param const vktoolkit::Device &device - устройство
* @param VkSurfaceKHR surface - хендо поверхности, передается лишь для проверки поддержки запрашиваемого формата
* @param VkFormat colorAttachmentFormat - формат цветовых вложений/изображений, должен поддерживаться поверхностью
* @param VkFormat depthStencilFormat - формат вложений глубины, должен поддерживаться устройством
* @return VkRenderPass - хендл прохода рендеринга
*
* @note - Проход рендеринга можно понимать как некую стадию на которой выполняются все команды рендерига и происходит цикл конвейера
* Проход состоит из под-проходов, и у каждого под-прохода может быть своя конфигурация конвейера. Конфигурация же прохода
* определяет в каком состоянии (размещении памяти) будет вложение (цветовое, глубины и тд)
*/
VkRenderPass VkRenderer::InitRenderPass(const vktoolkit::Device &device, VkSurfaceKHR surface, VkFormat colorAttachmentFormat, VkFormat depthStencilFormat)
{
	// Проверка доступности формата вложений (изображений)
	vktoolkit::SurfaceInfo si = vktoolkit::GetSurfaceInfo(device.physicalDevice, surface);
	if (!si.IsFormatSupported(colorAttachmentFormat)) {
		throw std::runtime_error("Vulkan: Required surface format is not supported. Can't initialize render-pass");
	}

	// Проверка доступности формата глубины
	if (!device.IsDepthFormatSupported(depthStencilFormat)) {
		throw std::runtime_error("Vulkan: Required depth-stencil format is not supported. Can't initialize render-pass");
	}

	// Массив описаний вложений
	// Пояснение: изображения в которые происходит рендеринг либо которые поступают на вход прохода называются вложениями.
	// Это может быть цветовое вложение (по сути обычное изображение) либо вложение глубины, трафарета и т.д.
	std::vector<VkAttachmentDescription> attachments;

	// Описаниие цветового вложения (изображение)
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = colorAttachmentFormat;                                       // Формат цвета должен соответствовать тому что будет использован при создании своп-чейна
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;                                      // Не использовать мультисемплинг (один семпл на пиксел)
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                                 // На этапе начала прохода - очищать вложение
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;                               // На этапе конца прохода - хранить вложение (для дальнешей презентации)
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                      // Подресурс трафарета (начало прохода) - не используется
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                    // Подресурс трафарета (конце прохода) - не исрользуется
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                            // Размещение памяти в начале (не имеет значения, любое)
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;                        // Размещение памяти к которому вложение будет приведено после окончания прохода (для представления)
	attachments.push_back(colorAttachment);

	// Описание вложения глубины трафарета (z-буфер)
	VkAttachmentDescription depthStencilAttachment = {};
	depthStencilAttachment.format = depthStencilFormat;
	depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                           // На этапе начала прохода - очищать вложение
	depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                     // На этапе конца прохода - не имеет значение (память не используется для презентации, можно не хранить)
	depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                // Подресурс трафарета (начало прохода) - не используется
	depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;              // Подресурс трафарета (конце прохода) - не исрользуется
	depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                      // Размещение памяти в начале (не имеет значения, любое)
	depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Размещение памяти к которому вложение будет приведено после окончания прохода (глубина-трафарет)
	attachments.push_back(depthStencilAttachment);


	// Массив ссылок на цветовые вложения
	// Для каждой ссылки указывается индекс цветового вложения в массиве вложений и ожидаемое размещение
	std::vector<VkAttachmentReference> colorAttachmentReferences = {
		{
			0,                                                       // Первый элемент массива вложений 
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL                 // Ожидается что размещение будет VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		}
	};

	// Ссылка на вложение глубины-трафарета
	VkAttachmentReference depthStencilAttachemntReference = {
		1,                                                           // Второй элемент массива вложений 
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL             // Ожидается что размещение будет VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	// Описание единственного под-прохода
	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();  // Кол-во цветовых вложений
	subpassDescription.pColorAttachments = colorAttachmentReferences.data();               // Цветовые вложения (вложения для записи)
	subpassDescription.pDepthStencilAttachment = &depthStencilAttachemntReference;         // Глубина-трафарет (не используется)
	subpassDescription.inputAttachmentCount = 0;                                           // Кол-во входных вложений (не используются)
	subpassDescription.pInputAttachments = nullptr;                                        // Входные вложения (вложения для чтения, напр. того что было записано в пред-щем под-проходе)
	subpassDescription.preserveAttachmentCount = 0;                                        // Кол-во хранимых вложений (не используется)
	subpassDescription.pPreserveAttachments = nullptr;                                     // Хранимые вложения могут быть использованы для много-кратного использования в разных под-проходах
	subpassDescription.pResolveAttachments = nullptr;                                      // Resolve-вложения (полезны при работе с мульти-семплингом, не используется)

	// Настройка зависимостей под-проходов
	// Они добавят неявный шаблон перехода вложений
	// Каждая зависимость под-прохода подготовит память во время переходов вложения между под-проходами
	// Пояснение: VK_SUBPASS_EXTERNAL ссылается на внешний, неявный под-проход
	std::vector<VkSubpassDependency> dependencies = {
		// Первая зависимость, начало конвейера
		// Перевод размещения от заключительного (final) к первоначальному (initial)
		{
			VK_SUBPASS_EXTERNAL,                                                       // Передыдущий под-проход - внешний, неявный
			0,                                                                         // Следующий под-проход - первый (и единсвтенный)
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,                                      // Подразумевается что предыдыщий под-проход (внешний) завершен на этапе завершения конвейера
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                             // Целевой подпроход (первый) начнется на этапе вывода цветовой информации из конвейера
			VK_ACCESS_MEMORY_READ_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		},
		// Вторая зависимость, выход - конец конвейера
		// Перевод размещения от первоначального (initial) к заключительному (final)
		{
			0,                                                                         // Предыдщий под-проход - первый (и единсвтенный)
			VK_SUBPASS_EXTERNAL,                                                       // Следующий - внешний, неявный
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                             // Предыдущий под-проход (первый) завершен на этапе вывода цветовой информации
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,                                      // Следующий под-проход (внешний) начат на этапе завершения конвейера
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		},
	};

	// Создать проход
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = (unsigned int)attachments.size();              //Кол-во описаний вложений
	renderPassInfo.pAttachments = attachments.data();                               //Описания вложений
	renderPassInfo.subpassCount = 1;                                                //Кол-вл под-проходов
	renderPassInfo.pSubpasses = &subpassDescription;                                //Описание под-проходов
	renderPassInfo.dependencyCount = (unsigned int)dependencies.size();             //Кол-во зависимсотей
	renderPassInfo.pDependencies = dependencies.data();                             //Зависимости

	//Создание прохода
	VkRenderPass renderPass;
	if (vkCreateRenderPass(device.logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Failed to create render pass!");
	}

	toolkit::LogMessage("Vulkan: Render pass successfully initialized");

	return renderPass;
}

/**
* Деинициализация прохода рендеринга
* @param const vktoolkit::Device &device - устройство которой принимало участие в создании прохода
* @param VkRenderPass * renderPass - указатель на хендл прохода
*/
void VkRenderer::DeinitRenderPass(const vktoolkit::Device &device, VkRenderPass * renderPass)
{
	// Если создан проход рендера - уничтожить
	if (*renderPass != VK_NULL_HANDLE) {
		vkDestroyRenderPass(device.logicalDevice, *renderPass, nullptr);
		*renderPass = VK_NULL_HANDLE;
		toolkit::LogMessage("Vulkan: Render pass successfully deinitialized");
	}
}

/* S W A P C H A I N */

/**
* Swap-chain (список показа, цепочка свопинга) - представляет из себя набор сменяемых изображений
* @param const vktoolkit::Device &device - устройство, необходимо для создания
* @param VkSurfaceKHR surface - хкндл поверхоности, необходимо для создания объекта swap-chain и для проверки поддержки формата
* @param VkSurfaceFormatKHR surfaceFormat - формат изображений и цветовое пространство (должен поддерживаться поверхностью)
* @param VkFormat depthStencilFormat - формат вложений глубины (должен поддерживаться устройством)
* @param VkRenderPass renderPass - хендл прохода рендеринга, нужен для создания фрейм-буферов swap-chain
* @param unsigned int bufferCount - кол-во буферов кадра (напр. для тройной буферизации - 3)
* @param vktoolkit::Swapchain * oldSwapchain - передыдуший swap-chain (полезно в случае пересоздания свап-чейна, например, сменив размеро поверхности)
* @return vktoolkit::Swapchain структура описывающая swap-chain cодержащая необходимые хендлы
* @note - в одно изображение может происходить запись (рендеринг) в то время как другое будет показываться (презентация)
*/
vktoolkit::Swapchain VkRenderer::InitSwapChain(
	const vktoolkit::Device &device,
	VkSurfaceKHR surface,
	VkSurfaceFormatKHR surfaceFormat,
	VkFormat depthStencilFormat,
	VkRenderPass renderPass,
	unsigned int bufferCount,
	vktoolkit::Swapchain * oldSwapchain)
{

	// Возвращаемый результат (структура содержит хендлы свопчейна, изображений, фрейм-буферов и тд)
	vktoolkit::Swapchain resultSwapchain;

	// Информация о поверхности
	vktoolkit::SurfaceInfo si = vktoolkit::GetSurfaceInfo(device.physicalDevice, surface);

	// Проверка доступности формата и цветового пространства изображений
	if (!si.IsSurfaceFormatSupported(surfaceFormat)) {
		throw std::runtime_error("Vulkan: Required surface format is not supported. Can't initialize swap-chain");
	}

	// Проверка доступности формата глубины
	if (!device.IsDepthFormatSupported(depthStencilFormat)) {
		throw std::runtime_error("Vulkan: Required depth-stencil format is not supported. Can't initialize render-pass");
	}

	// Если кол-во буферов задано
	if (bufferCount > 0) {
		// Проверить - возможно ли использовать запрашиваемое кол-во буферов (и изоображений соответственно)
		if (bufferCount < si.capabilities.minImageCount || bufferCount > si.capabilities.maxImageCount) {
			std::string message = "Vulkan: Surface don't support " + std::to_string(bufferCount) + " images/buffers in swap-chain";
			throw std::runtime_error(message);
		}
	}
	// В противном случае попытаться найти оптимальное кол-во
	else {
		bufferCount = (si.capabilities.minImageCount + 1) > si.capabilities.maxImageCount ? si.capabilities.maxImageCount : (si.capabilities.minImageCount + 1);
	}

	// Выбор режима представления (FIFO_KHR по умолчнию, самый простой)
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	// Если запрашиваемое кол-во буферов больше единицы - есть смысл выбрать более сложный режим,
	// но перед этим необходимо убедиться что поверхность его поддерживает
	if (bufferCount > 1) {
		for (const VkPresentModeKHR& availablePresentMode : si.presentModes) {
			//Если возможно - использовать VK_PRESENT_MODE_MAILBOX_KHR (вертикальная синхронизация)
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				presentMode = availablePresentMode;
				break;
			}
		}
	}

	// Информация о создаваемом swap-chain
	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = this->surface_;
	swapchainCreateInfo.minImageCount = bufferCount;                        // Минимальное кол-во изображений
	swapchainCreateInfo.imageFormat = surfaceFormat.format;					// Формат изображения
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;			// Цветовое пространство
	swapchainCreateInfo.imageExtent = si.capabilities.currentExtent;        // Резрешение (расширение) изображений
	swapchainCreateInfo.imageArrayLayers = 1;                               // Слои (1 слой, не стереоскопический рендер)
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;   // Как используются изображения (как цветовые вложения)

	// Добавить информацию о формате и расширении в результирующий объект swap-chain'а (он будет отдан функцией)
	resultSwapchain.imageFormat = swapchainCreateInfo.imageFormat;
	resultSwapchain.imageExtent = swapchainCreateInfo.imageExtent;

	// Если старый swap-chain был передан - очистить его информацию о формате и расширении
	if (oldSwapchain != nullptr) {
		oldSwapchain->imageExtent = {};
		oldSwapchain->imageFormat = {};
	}

	// Индексы семейств
	std::vector<unsigned int> queueFamilyIndices = {
		(unsigned int)device.queueFamilies.graphics,
		(unsigned int)device.queueFamilies.present
	};

	// Если для команд графики и представления используются разные семейства, значит доступ к ресурсу (в данном случае к буферам изображений)
	// должен быть распараллелен (следует использовать VK_SHARING_MODE_CONCURRENT, указав при этом кол-во семейств и их индексы)
	if (device.queueFamilies.graphics != device.queueFamilies.present) {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}
	// В противном случае подходящим будет VK_SHARING_MODE_EXCLUSIVE (с ресурсом работают команды одного семейства)
	else {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	swapchainCreateInfo.preTransform = si.capabilities.currentTransform;                                        // Не используем трансформацмию изображения (трансформация поверхности по умолчнию) (???)
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;                                     // Смешивание альфа канала с другими окнами в системе (нет смешивания)
	swapchainCreateInfo.presentMode = presentMode;                                                              // Установка режима представления (тот что выбрали ранее)
	swapchainCreateInfo.clipped = VK_TRUE;                                                                      // Не рендерить перекрываемые другими окнами пиксели
	swapchainCreateInfo.oldSwapchain = (oldSwapchain != nullptr ? oldSwapchain->vkSwapchain : VK_NULL_HANDLE);  // Старый swap-chain (для более эффективного пересоздания можно указывать старый swap-chain)

	// Создание swap-chain (записать хендл в результирующий объект)
	if (vkCreateSwapchainKHR(device.logicalDevice, &swapchainCreateInfo, nullptr, &(resultSwapchain.vkSwapchain)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateSwapchainKHR function. Failed to create swapchain");
	}

	// Уничтожение предыдущего swap-chain, если он был передан
	if (oldSwapchain != nullptr) {
		vkDestroySwapchainKHR(device.logicalDevice, oldSwapchain->vkSwapchain, nullptr);
		oldSwapchain->vkSwapchain = VK_NULL_HANDLE;
	}

	// Получить хендлы изображений swap-chain
	// Кол-во изображений по сути равно кол-ву буферов (за это отвечает bufferCount при создании swap-chain)
	unsigned int swapChainImageCount = 0;
	vkGetSwapchainImagesKHR(device.logicalDevice, resultSwapchain.vkSwapchain, &swapChainImageCount, nullptr);
	resultSwapchain.images.resize(swapChainImageCount);
	vkGetSwapchainImagesKHR(device.logicalDevice, resultSwapchain.vkSwapchain, &swapChainImageCount, resultSwapchain.images.data());

	// Теперь необходимо создать image-views для каждого изображения (своеобразный интерфейс объектов изображений предостовляющий нужные возможности)
	// Если был передан старый swap-chain - предварительно очистить все его image-views
	if (oldSwapchain != nullptr) {
		if (!oldSwapchain->imageViews.empty()) {
			for (VkImageView const &swapchainImageView : oldSwapchain->imageViews) {
				vkDestroyImageView(device.logicalDevice, swapchainImageView, nullptr);
			}
			oldSwapchain->imageViews.clear();
		}
	}

	// Для каждого изображения (image) swap-chain'а создать свой imageView объект
	for (unsigned int i = 0; i < resultSwapchain.images.size(); i++) {

		// Идентификатор imageView (будт добавлен в массив)
		VkImageView swapсhainImageView;

		// Информация для инициализации
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = resultSwapchain.images[i];                       // Связь с изображением swap-chain
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;                        // Тип изображения (2Д текстура)
		createInfo.format = surfaceFormat.format;							// Формат изображения
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;			// По умолчанию
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		// Создать и добавить в массив
		if (vkCreateImageView(device.logicalDevice, &createInfo, nullptr, &swapсhainImageView) == VK_SUCCESS) {
			resultSwapchain.imageViews.push_back(swapсhainImageView);
		}
		else {
			throw std::runtime_error("Vulkan: Error in vkCreateImageView function. Failed to create image views");
		}
	}


	// Буфер глубины (Z-буфер)
	// Буфер может быть один для всех фрейм-буферов, даже при двойной/тройной буферизации (в отличии от изображений swap-chain)
	// поскольку он не учавствует в презентации (память из него непосредственно не отображается на экране).

	// Если это пересоздание swap-chain (передан старый) следует очистить компоненты прежнего буфера глубины
	if (oldSwapchain != nullptr) {
		oldSwapchain->depthStencil.Deinit(device.logicalDevice);
	}

	// Создать буфер глубины-трафарета (обычное 2D-изображение с требуемым форматом)
	resultSwapchain.depthStencil = vktoolkit::CreateImageSingle(
		device,
		VK_IMAGE_TYPE_2D,
		depthStencilFormat,
		{ si.capabilities.currentExtent.width, si.capabilities.currentExtent.height, 1 },
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		swapchainCreateInfo.imageSharingMode);


	// Теперь необходимо создать фрейм-буферы привязанные к image-views объектам изображений и буфера глубины (изображения глубины)
	// Перед этим стоит очистить буферы старого swap-сhain (если он был передан)
	if (oldSwapchain != nullptr) {
		if (!oldSwapchain->framebuffers.empty()) {
			for (VkFramebuffer const &frameBuffer : oldSwapchain->framebuffers) {
				vkDestroyFramebuffer(device.logicalDevice, frameBuffer, nullptr);
			}
			oldSwapchain->framebuffers.clear();
		}
	}

	// Пройтись по всем image views и создать фрейм-буфер для каждого
	for (unsigned int i = 0; i < resultSwapchain.imageViews.size(); i++) {

		// Хендл нового фреймбуфера
		VkFramebuffer framebuffer;

		std::vector<VkImageView> attachments(2);
		attachments[0] = resultSwapchain.imageViews[i];                             // Цветовое вложение (на каждый фрейм-буфер свое)		
		attachments[1] = resultSwapchain.depthStencil.vkImageView;                  // Буфер глубины (один на все фрейм-буферы)

		// Описание нового фреймбуфера
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;                          // Указание прохода рендера
		framebufferInfo.attachmentCount = (uint32_t)attachments.size();   // Кол-во вложений
		framebufferInfo.pAttachments = attachments.data();                // Связь с image-views объектом изображения swap-chain'а
		framebufferInfo.width = resultSwapchain.imageExtent.width;        // Разрешение (ширина)
		framebufferInfo.height = resultSwapchain.imageExtent.height;      // Разрешение (высота)
		framebufferInfo.layers = 1;                                       // Один слой

		// В случае успешного создания - добавить в массив
		if (vkCreateFramebuffer(device.logicalDevice, &framebufferInfo, nullptr, &framebuffer) == VK_SUCCESS) {
			resultSwapchain.framebuffers.push_back(framebuffer);
		}
		else {
			throw std::runtime_error("Vulkan: Error in vkCreateFramebuffer function. Failed to create frame buffers");
		}
	}

	toolkit::LogMessage("Vulkan: Swap-chain successfully initialized");

	return resultSwapchain;
}

/**
* Деинициализация swap-chain
* @param const vktoolkit::Device &device - устройство которое принимало участие в создании swap-chain
* @param vktoolkit::Swapchain * swapchain - указатель на объект своп-чейна
*/
void VkRenderer::DeinitSwapchain(const vktoolkit::Device &device, vktoolkit::Swapchain * swapchain)
{
	// Очистить фрейм-буферы
	if (!swapchain->framebuffers.empty()) {
		for (VkFramebuffer const &frameBuffer : swapchain->framebuffers) {
			vkDestroyFramebuffer(device.logicalDevice, frameBuffer, nullptr);
		}
		swapchain->framebuffers.clear();
	}

	// Очистить image-views объекты
	if (!swapchain->imageViews.empty()) {
		for (VkImageView const &imageView : swapchain->imageViews) {
			vkDestroyImageView(device.logicalDevice, imageView, nullptr);
		}
		swapchain->imageViews.clear();
	}

	// Очиска компонентов Z-буфера
	swapchain->depthStencil.Deinit(device.logicalDevice);

	// Очистить swap-chain
	if (swapchain->vkSwapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(device.logicalDevice, swapchain->vkSwapchain, nullptr);
		swapchain->vkSwapchain = VK_NULL_HANDLE;
	}

	// Сбросить расширение и формат
	swapchain->imageExtent = {};
	swapchain->imageFormat = {};

	toolkit::LogMessage("Vulkan: Swap-chain successfully deinitialized");
}

/* К О М А Н Д Н Ы Е  П У Л Ы  И  Б У Ф Е Р Ы */

/**
* Инциализация командного пула
* @param const vktoolkit::Device &device - устройство
* @param unsigned int queueFamilyIndex - индекс семейства очередей команды которого будут передаваться в аллоцированных их пуда буферах
* @return VkCommandPool - хендл командного пула
* @note - из командных пулов осуществляется аллокация буферов команд, следует учитывать что для отедльных очередей нужны отдельные пулы
*/
VkCommandPool VkRenderer::InitCommandPool(const vktoolkit::Device &device, unsigned int queueFamilyIndex)
{
	// Результат
	VkCommandPool resultPool = VK_NULL_HANDLE;

	// Описание пула
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = (unsigned int)queueFamilyIndex;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	// Создание пула
	if (vkCreateCommandPool(device.logicalDevice, &commandPoolCreateInfo, nullptr, &resultPool) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateCommandPool function. Failed to create command pool");
	}

	toolkit::LogMessage("Vulkan: Command pool successfully initialized");

	// Вернуть pool
	return resultPool;
}

/**
* Деинциализация командного пула
* @param const vktoolkit::Device &device - устройство
* @param VkCommandPool * commandPool - указатель на хендл командного пула
*/
void VkRenderer::DeinitCommandPool(const vktoolkit::Device &device, VkCommandPool * commandPool)
{
	if (commandPool != nullptr && *commandPool != VK_NULL_HANDLE) {
		vkDestroyCommandPool(device.logicalDevice, *commandPool, nullptr);
		*commandPool = VK_NULL_HANDLE;
		toolkit::LogMessage("Vulkan: Command pool successfully deinitialized");
	}
}

/**
* Аллокация командных буферов
* @param const vktoolkit::Device &device - устройство
* @param VkCommandPool commandPool - хендл командного пула из которого будет осуществляться аллокация
* @param unsigned int count - кол-во аллоцируемых буферов
* @return std::vector<VkCommandBuffer> массив хендлов командных буферов
*/
std::vector<VkCommandBuffer> VkRenderer::InitCommandBuffers(const vktoolkit::Device &device, VkCommandPool commandPool, unsigned int count)
{
	// Результат
	std::vector<VkCommandBuffer> resultBuffers(count);

	// Конфигурация аллокации буферов
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;                               // Указание командного пула
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;                 // Передается в очередь непосредственно
	allocInfo.commandBufferCount = (unsigned int)resultBuffers.size(); // Кол-во командных буферов

	// Аллоцировать буферы команд
	if (vkAllocateCommandBuffers(device.logicalDevice, &allocInfo, resultBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkAllocateCommandBuffers function. Failed to allocate command buffers");
	}

	toolkit::LogMessage("Vulkan: Command buffers successfully allocated");

	// Вернуть массив хендлов
	return resultBuffers;
}

/**
* Деинициализация (очистка) командных буферов
* @param const vktoolkit::Device &device - устройство
* @param VkCommandPool commandPool - хендл командного пула из которого были аллоцированы буферы
* @param std::vector<VkCommandBuffer> * buffers - указатель на массив с хендлами буферов (он будет обнулен после очистки)
*/
void VkRenderer::DeinitCommandBuffers(const vktoolkit::Device &device, VkCommandPool commandPool, std::vector<VkCommandBuffer> * buffers)
{
	// Если массив индентификаторов буферов команд рисования не пуст
	if (device.logicalDevice != VK_NULL_HANDLE && buffers != nullptr && !buffers->empty()) {
		// Очистисть память
		vkFreeCommandBuffers(device.logicalDevice, commandPool, (unsigned int)(buffers->size()), buffers->data());
		// Очистить массив
		buffers->clear();

		toolkit::LogMessage("Vulkan: Command buffers successfully freed");
	}
}

/* U N I F O R M - Б У Ф Е Р Ы */

/**
* Создание мирового (глобального) unform-buffer'а
* @param const vktoolkit::Device &device - устройство
* @return vktoolkit::UniformBuffer - буфер, структура с хендлами буфера, его памяти, а так же доп. свойствами
*
* @note - unform-буфер это буфер доступный для шейдера посредством дескриптороа. В нем содержится информация о матрицах используемых
* для преобразования координат вершин сцены. В буфер помещается UBO объект содержащий необходимые матрицы. При каждом обновлении сцены
* можно отправлять объект с новыми данными (например, если сменилось положение камеры, либо угол ее поворота). Таким образом шейдер будет
* использовать для преобразования координат вершины новые данные.
*/
vktoolkit::UniformBuffer VkRenderer::InitUnformBufferWorld(const vktoolkit::Device &device)
{
	// Результирующий объект
	vktoolkit::UniformBuffer resultBuffer = {};

	// Создать буфер, выделить память, привязать память к буферу
	vktoolkit::Buffer buffer = vktoolkit::CreateBuffer(
		device,
		sizeof(vktoolkit::UboWorld),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Основная конфиуграция результирущего буфера
	resultBuffer.vkBuffer = buffer.vkBuffer;
	resultBuffer.vkDeviceMemory = buffer.vkDeviceMemory;
	resultBuffer.size = buffer.size;

	// Настройка информации для дескриптора
	resultBuffer.configDescriptorInfo(buffer.size, 0);

	// Разметить буфер (сделать его доступным для копирования информации)
	resultBuffer.map(device.logicalDevice, buffer.size, 0);

	toolkit::LogMessage("Vulkan: Uniform buffer for world scene successfully allocated");

	return resultBuffer;
}

/**
* Создание буфера для моделей (динамический uniform-bufer)
* @param const vktoolkit::Device &device - устройство
* @param unsigned int maxObjects - максимальное кол-во отдельных объектов на сцене
* @return vktoolkit::UniformBuffer - буфер, структура с хендлами буфера, его памяти, а так же доп. свойствами
*
* @note - в отличии от мирового uniform-буфера, uniform-буфер моделей содержит отдельные матрицы для кадой модели (по сути массив)
* и выделение памяти под передаваемый в такой буфер объект должно использовать выравнивание. У устройства есть определенные лимиты
* на выравнивание памяти, поэтому размер такого буфера вычисляется с учетом допустимого шага выравивания и кол-ва объектов которые
* могут быть на сцене.
*/
vktoolkit::UniformBuffer VkRenderer::InitUniformBufferModels(const vktoolkit::Device &device, unsigned int maxObjects)
{
	// Результирующий объект
	vktoolkit::UniformBuffer resultBuffer = {};

	// Вычислить размер буфера учитывая доступное вырванивание памяти (для типа glm::mat4 размером в 64 байта)
	VkDeviceSize bufferSize = device.GetDynamicAlignment<glm::mat4>() * maxObjects;

	vktoolkit::Buffer buffer = vktoolkit::CreateBuffer(
		device,
		bufferSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	// Настройка результирубщего буфера (uniform-буфер)
	resultBuffer.vkBuffer = buffer.vkBuffer;
	resultBuffer.vkDeviceMemory = buffer.vkDeviceMemory;
	resultBuffer.size = buffer.size;

	// Настройка информации для дескриптора
	resultBuffer.configDescriptorInfo(VK_WHOLE_SIZE);

	// Разметить буфер (сделать его доступным для копирования информации)
	resultBuffer.map(device.logicalDevice, VK_WHOLE_SIZE, 0);

	toolkit::LogMessage("Vulkan: Uniform buffer for models successfully allocated");

	return resultBuffer;
}

/**
* Деинициализация (очистка) командного буфера
* @param const vktoolkit::Device &device - устройство
* @param vktoolkit::UniformBuffer * uniformBuffer - указатель на структуру буфера
*/
void VkRenderer::DeinitUniformBuffer(const vktoolkit::Device &device, vktoolkit::UniformBuffer * uniformBuffer)
{
	if (uniformBuffer != nullptr) {

		uniformBuffer->unmap(device.logicalDevice);

		if (uniformBuffer->vkBuffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(device.logicalDevice, uniformBuffer->vkBuffer, nullptr);
			uniformBuffer->vkBuffer = VK_NULL_HANDLE;
		}

		if (uniformBuffer->vkDeviceMemory != VK_NULL_HANDLE) {
			vkFreeMemory(device.logicalDevice, uniformBuffer->vkDeviceMemory, nullptr);
			uniformBuffer->vkDeviceMemory = VK_NULL_HANDLE;
		}

		uniformBuffer->descriptorBufferInfo = {};
		*uniformBuffer = {};

		toolkit::LogMessage("Vulkan: Uniform buffer successfully deinitialized");
	}
}

/* Д Е С К Р И П Т О Р Н Ы Й  П У Л */

/**
* Инициализация основного декскрипторного пула
* @param const vktoolkit::Device &device - устройство
* @return VkDescriptorPool - хендл дескрипторного пула
* @note - дескрипторный пул позволяет выделять специальные наборы дескрипторов, обеспечивающие доступ к определенным буферам из шейдера
*/
VkDescriptorPool VkRenderer::InitDescriptorPoolMain(const vktoolkit::Device &device)
{
	// Хендл нового дескрипторого пула
	VkDescriptorPool descriptorPoolResult = VK_NULL_HANDLE;

	// Парамтеры размеров пула
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
	{
		// Один дескриптор для глобального uniform-буфера
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , 1 },
		// Один дескриптор для unform-буферов отдельных объектов (динамический)
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 }
	};


	// Конфигурация пула
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
	poolInfo.pPoolSizes = descriptorPoolSizes.data();
	poolInfo.maxSets = 1;

	// Создание дескрипторного пула
	if (vkCreateDescriptorPool(device.logicalDevice, &poolInfo, nullptr, &descriptorPoolResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorPool function. Cant't create descriptor pool");
	}

	toolkit::LogMessage("Vulkan: Main descriptor pool successfully initialized");

	// Отдать результат
	return descriptorPoolResult;
}

/**
* Инициализация декскрипторного пула под текстурные наборы дескрипторов
* @param const vktoolkit::Device &device - устройство
* @param uint32_t maxDescriptorSets - максимальное кол-во наборов
* @return VkDescriptorPool - хендл дескрипторного пула
* @note - дескрипторный пул позволяет выделять специальные наборы дескрипторов, обеспечивающие доступ к определенным буферам из шейдера
*/
VkDescriptorPool VkRenderer::InitDescriptorPoolTextures(const vktoolkit::Device &device, uint32_t maxDescriptorSets)
{
	// Хендл нового дескрипторого пула
	VkDescriptorPool descriptorPoolResult = VK_NULL_HANDLE;

	// Парамтеры размеров пула
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
	{
		// Один дескриптор для текстурного семплера
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 1 },
	};

	// Конфигурация пула
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
	poolInfo.pPoolSizes = descriptorPoolSizes.data();
	poolInfo.maxSets = maxDescriptorSets;

	// Создание дескрипторного пула
	if (vkCreateDescriptorPool(device.logicalDevice, &poolInfo, nullptr, &descriptorPoolResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorPool function. Cant't create descriptor pool");
	}

	toolkit::LogMessage("Vulkan: Texture descriptor pool successfully initialized");

	// Отдать результат
	return descriptorPoolResult;
}

/**
* Деинициализация дескрипторного пула
* @param const vktoolkit::Device &device - устройство
* @VkDescriptorPool * descriptorPool - указатель на хендл дескрипторного пула
*/
void VkRenderer::DeinitDescriptorPool(const vktoolkit::Device &device, VkDescriptorPool * descriptorPool)
{
	if (descriptorPool != nullptr && *descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(device.logicalDevice, *descriptorPool, nullptr);
		*descriptorPool = VK_NULL_HANDLE;
		toolkit::LogMessage("Vulkan: Descriptor pool successfully deinitialized");
	}
}

/* Р А З М Е Щ Е Н И Е  Д Е С К Р И П Т О Р Н О Г О  Н А Б О Р А */

/**
* Инициализация описания размещения дескрипторного пула (под основной дескрипторный набор)
* @param const vktoolkit::Device &device - устройство
* @return VkDescriptorSetLayout - хендл размещения дескрипторного пула
* @note - Размещение - информация о том сколько и каких именно (какого типа) дескрипторов следует ожидать на определенных этапах конвейера
*/
VkDescriptorSetLayout VkRenderer::InitDescriptorSetLayoutMain(const vktoolkit::Device &device)
{
	// Результирующий хендл
	VkDescriptorSetLayout layoutResult = VK_NULL_HANDLE;

	// Необходимо описать привязки дескрипторов к этапам конвейера
	// Каждая привязка соостветствует типу дескриптора и может относиться к определенному этапу графического конвейера
	std::vector<VkDescriptorSetLayoutBinding> bindings =
	{
		{
			0,                                            // Индекс привязки
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            // Тип дескриптора (буфер формы, обычный)
			1,                                            // Кол-во дескрипторов
			VK_SHADER_STAGE_VERTEX_BIT,                   // Этап конвейера (вершинный шейдер)
			nullptr
		},
		{
			1,                                            // Индекс привязки
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,    // Тип дескриптора (буфер формы, динамический)
			1,                                            // Кол-во дескрипторов
			VK_SHADER_STAGE_VERTEX_BIT,                   // Этап конвейера (вершинный шейдер)
			nullptr
		},
	};

	// Инициализировать размещение дескрипторного набора
	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
	descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutInfo.pNext = nullptr;
	descriptorLayoutInfo.bindingCount = (uint32_t)bindings.size();
	descriptorLayoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device.logicalDevice, &descriptorLayoutInfo, nullptr, &layoutResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorSetLayout. Can't initialize descriptor set layout");
	}

	toolkit::LogMessage("Vulkan: Main descriptor set layout successfully initialized");

	return layoutResult;
}

/**
* Инициализация описания размещения дескрипторного пула (под текстурные наборы дескрипторов)
* @param const vktoolkit::Device &device - устройство
* @return VkDescriptorSetLayout - хендл размещения дескрипторного пула
* @note - Размещение - информация о том сколько и каких именно (какого типа) дескрипторов следует ожидать на определенных этапах конвейера
*/
VkDescriptorSetLayout VkRenderer::InitDescriptorSetLayoutTextures(const vktoolkit::Device &device)
{
	// Результирующий хендл
	VkDescriptorSetLayout layoutResult = VK_NULL_HANDLE;

	// Необходимо описать привязки дескрипторов к этапам конвейера
	// Каждая привязка соостветствует типу дескриптора и может относиться к определенному этапу графического конвейера
	std::vector<VkDescriptorSetLayoutBinding> bindings =
	{
		{
			0,                                            // Индекс привязки
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,    // Тип дескриптора (семплер изображения)
			1,                                            // Кол-во дескрипторов
			VK_SHADER_STAGE_FRAGMENT_BIT,                 // Этап конвейера (вершинный шейдер)
			nullptr
		},
	};

	// Инициализировать размещение дескрипторного набора
	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
	descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutInfo.pNext = nullptr;
	descriptorLayoutInfo.bindingCount = (uint32_t)bindings.size();
	descriptorLayoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device.logicalDevice, &descriptorLayoutInfo, nullptr, &layoutResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorSetLayout. Can't initialize descriptor set layout");
	}

	toolkit::LogMessage("Vulkan: Texture descriptor set layout successfully initialized");

	return layoutResult;
}

/**
* Деинициализация размещения
* @param const vktoolkit::Device &device - устройство
* @VkDescriptorSetLayout * descriptorSetLayout - указатель на хендл размещения
*/
void VkRenderer::DeinitDescriporSetLayout(const vktoolkit::Device &device, VkDescriptorSetLayout * descriptorSetLayout)
{
	if (device.logicalDevice != VK_NULL_HANDLE && descriptorSetLayout != nullptr && *descriptorSetLayout != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(device.logicalDevice, *descriptorSetLayout, nullptr);
		*descriptorSetLayout = VK_NULL_HANDLE;

		toolkit::LogMessage("Vulkan: Descriptor set layout successfully deinitialized");
	}
}

/* Т Е К С Т У Р Н Ы Й  С Е М П Л Е Р */

/**
* Инициализация текстурного семплера
* @param const vktoolkit::Device &device - устройство
* @note - описывает как данные текстуры подаются в шейдер и как интерпретируются координаты
*/
VkSampler VkRenderer::InitTextureSampler(const vktoolkit::Device &device)
{
	// Результирующий хендл семплера
	VkSampler resultSampler;

	// Настройка семплера
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;                      // Тип интерполяции когда тексели больше фрагментов
	samplerInfo.minFilter = VK_FILTER_LINEAR;                      // Тип интерполяции когда тексели меньше фрагментов
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;     // Повторять при выходе за пределы
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;                        // Включть анизотропную фильтрацию
	samplerInfo.maxAnisotropy = 4;                                 // уровень фильтрации
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;    // Цвет грани
	samplerInfo.unnormalizedCoordinates = VK_FALSE;                // Использовать нормальзованные координаты (не пиксельные)
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	// Создание семплера
	if (vkCreateSampler(device.logicalDevice, &samplerInfo, nullptr, &resultSampler) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating texture sampler");
	}

	toolkit::LogMessage("Vulkan: Texture sampler successfully initialized");

	return resultSampler;
}

/**
* Деинициализация текстурного семплера
* @param const vktoolkit::Device &device - устройство
* @param VkSampler * sampler - деинициализация текстурного семплера
*/
void VkRenderer::DeinitTextureSampler(const vktoolkit::Device &device, VkSampler * sampler)
{
	if (sampler != nullptr && *sampler != VK_NULL_HANDLE) {
		vkDestroySampler(device.logicalDevice, *sampler, nullptr);
		*sampler = VK_NULL_HANDLE;
	}
}

/* Н А Б О Р  Д Е С К Р И П Т О Р О В */

/**
* Инициализация набор дескрипторов
* @param const vktoolkit::Device &device - устройство
* @param VkDescriptorPool descriptorPool - хендл дескрипторного пула из которого будет выделен набор
* @param VkDescriptorSetLayout descriptorSetLayout - хендл размещения дескрипторно набора
* @param const vktoolkit::UniformBuffer &uniformBufferWorld - буфер содержит необходимую для создания дескриптора информацию
* @param const vktoolkit::UniformBuffer &uniformBufferModels - буфер содержит необходимую для создания дескриптора информацию
*/
VkDescriptorSet VkRenderer::InitDescriptorSetMain(
	const vktoolkit::Device &device,
	VkDescriptorPool descriptorPool,
	VkDescriptorSetLayout descriptorSetLayout,
	const vktoolkit::UniformBuffer &uniformBufferWorld,
	const vktoolkit::UniformBuffer &uniformBufferModels)
{
	// Результирующий хендл дескриптора
	VkDescriptorSet descriptorSetResult = VK_NULL_HANDLE;

	// Получить новый набор дескрипторов из дескриптороного пула
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(device.logicalDevice, &descriptorSetAllocInfo, &descriptorSetResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkAllocateDescriptorSets. Can't allocate descriptor set");
	}

	// Конфигурация добавляемых в набор дескрипторов
	std::vector<VkWriteDescriptorSet> writes =
	{
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // Тип структуры
			nullptr,                                     // pNext
			descriptorSetResult,                         // Целевой набор дескрипторов
			0,                                           // Точка привязки (у шейдера)
			0,                                           // Элемент массив (массив не используется)
			1,                                           // Кол-во дескрипторов
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,           // Тип дескриптора
			nullptr,
			&(uniformBufferWorld.descriptorBufferInfo),  // Информация о параметрах буфера
			nullptr
		},
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // Тип структуры
			nullptr,                                     // pNext
			descriptorSetResult,                         // Целевой набор дескрипторов
			1,                                           // Точка привязки (у шейдера)
			0,                                           // Элемент массив (массив не используется)
			1,                                           // Кол-во дескрипторов
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,   // Тип дескриптора
			nullptr,
			&(uniformBufferModels.descriptorBufferInfo), // Информация о параметрах буфера
			nullptr,
		},
	};

	// Обновить наборы дескрипторов
	vkUpdateDescriptorSets(device.logicalDevice, (uint32_t)writes.size(), writes.data(), 0, nullptr);

	toolkit::LogMessage("Vulkan: Descriptor set successfully initialized");

	// Вернуть хендл набора
	return descriptorSetResult;
}

/**
* Деинициализация набор дескрипторов
* @param const vktoolkit::Device &device - устройство
* @param VkDescriptorPool descriptorPool - хендл дескрипторного пула из которого будет выделен набор
* @param VkDescriptorSet * descriptorSet - указатель на хендл набора дескрипторов
*/
void VkRenderer::DeinitDescriptorSet(const vktoolkit::Device &device, VkDescriptorPool descriptorPool, VkDescriptorSet * descriptorSet)
{
	if (device.logicalDevice != VK_NULL_HANDLE
		&& descriptorPool != VK_NULL_HANDLE
		&& descriptorSet != nullptr
		&& *descriptorSet != VK_NULL_HANDLE)
	{
		if (vkFreeDescriptorSets(device.logicalDevice, descriptorPool, 1, descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("Vulkan: Error while destroying descriptor set");
		}
		*descriptorSet = VK_NULL_HANDLE;

		toolkit::LogMessage("Vulkan: Descriptor set successfully deinitialized");
	}
}

/* О Б Ъ Е К Т Ы  Д И Н А М И Ч. Д Е С К Р И П Т О Р Н Ы Х  Б У Ф Е Р О В */

/**
* Аллокация памяти под объект динамического UBO буфера
* @param const vktoolkit::Device &device - устройство (необходимо для получения выравнивания)
* @param unsigned int maxObjects - максимальное кол-вл объектов (для выяснения размера аллоцируемой памяти)
* @return vktoolkit::UboModelArray - указатель на аллоцированный массив матриц
*/
vktoolkit::UboModelArray VkRenderer::AllocateUboModels(const vktoolkit::Device &device, unsigned int maxObjects)
{
	// Получить оптимальное выравнивание для типа glm::mat4
	std::size_t dynamicAlignment = (std::size_t)device.GetDynamicAlignment<glm::mat4>();

	// Вычислить размер буфера учитывая доступное вырванивание памяти (для типа glm::mat4 размером в 64 байта)
	std::size_t bufferSize = (std::size_t)(dynamicAlignment * maxObjects);

	// Аллоцировать память с учетом выравнивания
	vktoolkit::UboModelArray result = (vktoolkit::UboModelArray)_aligned_malloc(bufferSize, dynamicAlignment);

	toolkit::LogMessage("Vulkan: Dynamic UBO satage-buffer successfully allocated");

	return result;
}

/**
* Освобождение памяти объекта динамического UBO буфера
* @param vktoolkit::UboModelArray * uboModels - указатель на массив матриц, память которого будет очищена
*/
void VkRenderer::FreeUboModels(vktoolkit::UboModelArray * uboModels)
{
	_aligned_free(*uboModels);
	*uboModels = nullptr;
	toolkit::LogMessage("Vulkan: Dynamic UBO satage-buffer successfully freed");
}

/* Г Р А Ф И Ч Е С К И Й  К О Н В Е Й Е Р */

/**
* Инициализация размещения графического конвейера
* @param const vktoolkit::Device &device - устройство
* @param std::vector<VkDescriptorSetLayout> descriptorSetLayouts - хендлы размещениий дискрипторного набора (дает конвейеру инфу о дескрипторах)
* @return VkPipelineLayout - хендл размещения конвейера
*/
VkPipelineLayout VkRenderer::InitPipelineLayout(const vktoolkit::Device &device, std::vector<VkDescriptorSetLayout> descriptorSetLayouts)
{
	VkPipelineLayout resultLayout;

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = nullptr;
	pPipelineLayoutCreateInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
	pPipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();

	if (vkCreatePipelineLayout(device.logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &resultLayout) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating pipeline layout");
	}

	toolkit::LogMessage("Vulkan: Pipeline layout successfully initialized");

	return resultLayout;
}

/**
* Деинициализация размещения графического конвейера
* @param const vktoolkit::Device &device - устройство
* @param VkPipelineLayout * pipelineLayout - указатель на хендл размещения
*/
void VkRenderer::DeinitPipelineLayout(const vktoolkit::Device &device, VkPipelineLayout * pipelineLayout)
{
	if (device.logicalDevice != VK_NULL_HANDLE && pipelineLayout != nullptr && *pipelineLayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(device.logicalDevice, *pipelineLayout, nullptr);
		*pipelineLayout = VK_NULL_HANDLE;

		toolkit::LogMessage("Vulkan: Pipeline layout successfully deinitialized");
	}
}

/**
* Инициализация графического конвейера
* @param const vktoolkit::Device &device - устройство
* @param VkPipelineLayout pipelineLayout - хендл размещения конвейера
* @param vktoolkit::Swapchain &swapchain - swap-chain, для получения информации о разрешении
* @param VkRenderPass renderPass - хендл прохода рендеринга (на него ссылается конвейер)
*
* @note - графический конвейер производит рендериннг принимая вершинные данные на вход и выводя пиксели в
* буферы кадров. Конвейер состоит из множества стадий, некоторые из них программируемые (шейдерные). Конвейер
* относится к конкретному проходу рендеринга (и подпроходу). В методе происходит конфигурация всех стадий,
* загрузка шейдеров, настройка конвейера.
*/
VkPipeline VkRenderer::InitGraphicsPipeline(
	const vktoolkit::Device &device,
	VkPipelineLayout pipelineLayout,
	const vktoolkit::Swapchain &swapchain,
	VkRenderPass renderPass)
{
	// Результирующий хендл конвейера
	VkPipeline resultPipelineHandle = VK_NULL_HANDLE;

	// Конфигурация привязок и аттрибутов входных данных (вершинных)
	std::vector<VkVertexInputBindingDescription> bindingDescription = vktoolkit::GetVertexInputBindingDescriptions(0);
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = vktoolkit::GetVertexInputAttributeDescriptions(0);

	// Конфигурация стадии ввода вершинных данных
	VkPipelineVertexInputStateCreateInfo vertexInputStage = {};
	vertexInputStage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStage.vertexBindingDescriptionCount = (uint32_t)bindingDescription.size();
	vertexInputStage.pVertexBindingDescriptions = bindingDescription.data();
	vertexInputStage.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
	vertexInputStage.pVertexAttributeDescriptions = attributeDescriptions.data();

	// Описание этапа "сборки" входных данных
	// Конвейер будет "собирать" вершинны в набор треугольников
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStage = {};
	inputAssemblyStage.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStage.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;					// Список треугольников
	inputAssemblyStage.primitiveRestartEnable = VK_FALSE;								// Перезагрузка примитивов не используется

	// Прогамируемые (шейдерные) этапы конвейера
	// Используем 2 шейдера - вершинный (для каждой вершины) и фрагментный (пиксельный, для каждого пикселя)
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_VERTEX_BIT,
			vktoolkit::LoadSPIRVShader("base.vert.spv", device.logicalDevice),
			"main",
			nullptr
		},
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			vktoolkit::LoadSPIRVShader("base.frag.spv", device.logicalDevice),
			"main",
			nullptr
		}
	};

	// Описываем область просмотра
	// Размеры равны размерам изображений swap-chain, которые в свою очередь равны размерам поверхности отрисовки
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapchain.imageExtent.width;
	viewport.height = (float)swapchain.imageExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Настройки обрезки изображения (не обрезать, размеры совпадают с областью просмотра)
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchain.imageExtent;

	// Описываем этап вывода в область просмотра
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Описываем этап растеризации
	VkPipelineRasterizationStateCreateInfo rasterizationStage = {};
	rasterizationStage.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStage.depthClampEnable = VK_FALSE;                     // Фрагменты за ближней и дальней гранью камеры отбрасываются (VK_TRUE для противоположного эффекта)
	rasterizationStage.rasterizerDiscardEnable = VK_FALSE;              // Отключение растеризации геометрии - не нужно (VK_TRUE для противоположного эффекта)
	rasterizationStage.polygonMode = VK_POLYGON_MODE_FILL;              // Закрашенные полигоны
	rasterizationStage.lineWidth = 1.0f;                                // Ширина линии
	rasterizationStage.cullMode = VK_CULL_MODE_BACK_BIT;                // Отсечение граней (отсекаются те, что считаются задними)
	rasterizationStage.frontFace = VK_FRONT_FACE_CLOCKWISE;             // Порялок следования вершин для лицевой грани - по часовой стрелке
	rasterizationStage.depthBiasEnable = VK_FALSE;                      // Контроль значений глубины
	rasterizationStage.depthBiasConstantFactor = 0.0f;
	rasterizationStage.depthBiasClamp = 0.0f;
	rasterizationStage.depthBiasSlopeFactor = 0.0f;

	// Описываем этап z-теста (теста глубины)
	// Активировать тест глубины, использовать сравнение "меньше или равно"
	VkPipelineDepthStencilStateCreateInfo depthStencilStage = {};
	depthStencilStage.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStage.depthTestEnable = VK_TRUE;
	depthStencilStage.depthWriteEnable = VK_TRUE;
	depthStencilStage.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilStage.depthBoundsTestEnable = VK_FALSE;
	depthStencilStage.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilStage.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilStage.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depthStencilStage.stencilTestEnable = VK_FALSE;
	depthStencilStage.front = depthStencilStage.back;

	// Описываем этап мультисемплинга (сглаживание пиксельных лесенок)
	// Мултисемплинг - добавляет дополнительные ключевые точки в пиксел (семплы), для более точного вычисления его цвета
	// за счет чего возникает эффект более сглаженных линий (в данном примере не используется мульти-семплинг)
	VkPipelineMultisampleStateCreateInfo multisamplingStage = {};
	multisamplingStage.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingStage.sampleShadingEnable = VK_FALSE;
	multisamplingStage.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisamplingStage.minSampleShading = 1.0f;
	multisamplingStage.pSampleMask = nullptr;
	multisamplingStage.alphaToCoverageEnable = VK_FALSE;
	multisamplingStage.alphaToOneEnable = VK_FALSE;

	// Этап смешивания цвета (для каждого фрейм-буфера)
	// На этом этапе можно настроить как должны отображаться, например, полупрозрачные объекты
	// В начлае нужно описать состояния смешивания для цветовых вложений (используем одно состояние на вложение) (???)
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	// Глобальные настройки смешивания
	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachment;

	// Информация инициализации графического конвейера
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = (uint32_t)shaderStages.size();    // Кол-во шейдерных этапов
	pipelineInfo.pStages = shaderStages.data();                 // Шейдерные этапы (их конфигураци)
	pipelineInfo.pVertexInputState = &vertexInputStage;         // Настройки этапа ввода вершинных данных
	pipelineInfo.pInputAssemblyState = &inputAssemblyStage;     // Настройки этапа сборки примитивов из полученных вершин
	pipelineInfo.pViewportState = &viewportState;               // Настройки области видимости
	pipelineInfo.pRasterizationState = &rasterizationStage;     // Настройки этапа растеризации
	pipelineInfo.pDepthStencilState = &depthStencilStage;       // Настройка этапа z-теста
	pipelineInfo.pMultisampleState = &multisamplingStage;       // Настройки этапа мультисемплинга
	pipelineInfo.pColorBlendState = &colorBlendState;           // Настройки этапа смешивания цветов
	pipelineInfo.layout = pipelineLayout;                       // Размещение конвейера
	pipelineInfo.renderPass = renderPass;                       // Связываем конвейер с соответствующим проходом рендеринга
	pipelineInfo.subpass = 0;                                   // Связываем с под-проходом (первый под-проход)

	// Создание графического конвейера
	if (vkCreateGraphicsPipelines(device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &resultPipelineHandle) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating pipeline");
	}

	toolkit::LogMessage("Vulkan: Pipeline sucessfully initialized");

	// Шейдерные модули больше не нужны после создания конвейера
	for (VkPipelineShaderStageCreateInfo &shaderStageInfo : shaderStages) {
		vkDestroyShaderModule(device.logicalDevice, shaderStageInfo.module, nullptr);
	}

	return resultPipelineHandle;
}

/**
* Деинициализация графического конвейера
* @param const vktoolkit::Device &device - устройство
* @param VkPipeline * pipeline - указатель на хендл конвейера
*/
void VkRenderer::DeinitGraphicsPipeline(const vktoolkit::Device &device, VkPipeline * pipeline)
{
	if (device.logicalDevice != VK_NULL_HANDLE && pipeline != nullptr && *pipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(device.logicalDevice, *pipeline, nullptr);
		*pipeline = VK_NULL_HANDLE;
		toolkit::LogMessage("Vulkan: Pipeline sucessfully deinitialized");
	}
}

/* С И Н Х Р О Н И З А Ц И Я */

/**
* Инициализация примитивов синхронизации
* @param const vktoolkit::Device &device - устройство
* @return vktoolkit::Synchronization - структура с набором хендлов семафоров
* @note - семафоры синхронизации позволяют отслеживать состояние рендеринга и в нужный момент показывать изображение
*/
vktoolkit::Synchronization VkRenderer::InitSynchronization(const vktoolkit::Device &device)
{
	// Результат
	vktoolkit::Synchronization syncResult = {};

	// Информация о создаваемом семафоре (ничего не нужно указывать)
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Создать примитивы синхронизации
	if (vkCreateSemaphore(device.logicalDevice, &semaphoreInfo, nullptr, &(syncResult.readyToRender)) != VK_SUCCESS ||
		vkCreateSemaphore(device.logicalDevice, &semaphoreInfo, nullptr, &(syncResult.readyToPresent)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating synchronization primitives");
	}

	toolkit::LogMessage("Vulkan: Synchronization primitives sucessfully initialized");

	return syncResult;
}

/**
* Деинициализация примитивов синхронизации
* @param const vktoolkit::Device &device - устройство
* @param vktoolkit::Synchronization * sync - указатель на структуру с хендлами семафоров
*/
void VkRenderer::DeinitSynchronization(const vktoolkit::Device &device, vktoolkit::Synchronization * sync)
{
	if (sync != nullptr) {
		if (sync->readyToRender != VK_NULL_HANDLE) {
			vkDestroySemaphore(device.logicalDevice, sync->readyToRender, nullptr);
			sync->readyToRender = VK_NULL_HANDLE;
		}

		if (sync->readyToRender != VK_NULL_HANDLE) {
			vkDestroySemaphore(device.logicalDevice, sync->readyToRender, nullptr);
			sync->readyToRender = VK_NULL_HANDLE;
		}

		toolkit::LogMessage("Vulkan: Synchronization primitives sucessfully deinitialized");
	}
}

/* П О Д Г О Т О В К А  К О М А Н Д */

/**
* Подготовка команд, заполнение командных буферов
* @param std::vector<VkCommandBuffer> commandBuffers - массив хендлов командных буферов
* @param VkRenderPass renderPass - хендл прохода рендеринга, используется при привязке прохода
* @param VkPipelineLayout pipelineLayout - хендл размещения конвейра, исппользуется при привязке дескрипторов
* @param VkDescriptorSet descriptorSet - хендл набор дескрипторов, исппользуется при привязке дескрипторов
* @param VkPipeline pipeline - хендл конвейера, используется при привязке конвейера
* @param const vktoolkit::Swapchain &swapchain - свопчейн, используется при конфигурации начала прохода (в.ч. для указания фрейм-буфера)
* @param const std::vector<vktoolkit::Primitive> &primitives - массив примитивов (привязка буферов вершин, буферов индексов для каждого и т.д)
*
* @note - данную операцию нет нужды выполнять при каждом обновлении кадра, набор команд как правило относительно неизменный. Метод лишь заполняет
* буферы команд, а сама отправка происходть в draw. При изменении кол-ва примитивов следует сбросить командные буферы и заново их заполнить
* при помощи данного метода
*/
void VkRenderer::PrepareDrawCommands(
	std::vector<VkCommandBuffer> commandBuffers,
	VkRenderPass renderPass,
	VkPipelineLayout pipelineLayout,
	VkDescriptorSet descriptorSetMain,
	VkPipeline pipeline,
	const vktoolkit::Swapchain &swapchain,
	const std::vector<vktoolkit::Primitive> &primitives)
{

	// Информация начала командного буфера
	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	cmdBufInfo.pNext = nullptr;

	// Параметры очистки вложений в начале прохода
	std::vector<VkClearValue> clearValues(2);
	// Очистка первого вложения (цветового)
	clearValues[0].color = { 0.6f, 0.8f, 0.8f, 1.0f };
	// Очиска второго вложения (вложения глубины-трфарета)
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	// Информация о начале прохода
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = swapchain.imageExtent.width;
	renderPassBeginInfo.renderArea.extent.height = swapchain.imageExtent.height;
	renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
	renderPassBeginInfo.pClearValues = clearValues.data();


	// Пройтись по всем буферам
	for (unsigned int i = 0; i < commandBuffers.size(); ++i)
	{
		// Начать запись команд в командный буфер i
		vkBeginCommandBuffer(commandBuffers[i], &cmdBufInfo);

		// Установить целевой фрейм-буфер (поскольку их кол-во равно кол-ву командных буферов, индексы соответствуют)
		renderPassBeginInfo.framebuffer = swapchain.framebuffers[i];

		// Начать первый под-проход основного прохода, это очистит цветоые вложения
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Привязать графический конвейер
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		// Пройтись по всем примитивам
		if (!primitives.empty()) {
			for (unsigned int primitiveIndex = 0; primitiveIndex < primitives.size(); primitiveIndex++)
			{
				// Спиок динамических смещений для динамических UBO буферов в наборах дескрипторов
				// При помощи выравниваниях получаем необходимое смещение для дескрипторов, чтобы была осуществлена привязка
				// нужного буфера UBO (с матрицей модели) для конкретного примитива
				std::vector<uint32_t> dynamicOffsets = {
					primitiveIndex * static_cast<uint32_t>(this->device_.GetDynamicAlignment<glm::mat4>())
				};

				// Наборы дескрипторов (массив наборов)
				// По умолчанию в нем только основной
				std::vector<VkDescriptorSet> descriptorSets = {
					descriptorSetMain
				};

				// Если у примитива есть текстура
				// Добавить в список дескрипторов еще один (отвечающий за подачу текстуры и параметров семплинга в шейдер)
				if (primitives[primitiveIndex].texture != nullptr) {
					descriptorSets.push_back(primitives[primitiveIndex].texture->descriptorSet);
				}
				
				// Привязать наборы дескрипторов
				vkCmdBindDescriptorSets(
					commandBuffers[i], 
					VK_PIPELINE_BIND_POINT_GRAPHICS, 
					pipelineLayout, 
					0, 
					(uint32_t)descriptorSets.size(),
					descriptorSets.data(), 
					(uint32_t)dynamicOffsets.size(), 
					dynamicOffsets.data());

				// Если нужно рисовать индексированную геометрию
				if (primitives[primitiveIndex].drawIndexed && primitives[primitiveIndex].indexBuffer.count > 0) {
					// Привязать буфер вершин
					VkDeviceSize offsets[1] = { 0 };
					vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &(primitives[primitiveIndex].vertexBuffer.vkBuffer), offsets);

					// Привязать буфер индексов
					vkCmdBindIndexBuffer(commandBuffers[i], primitives[primitiveIndex].indexBuffer.vkBuffer, 0, VK_INDEX_TYPE_UINT32);

					// Отрисовка геометрии
					vkCmdDrawIndexed(commandBuffers[i], primitives[primitiveIndex].indexBuffer.count, 1, 0, 0, 0);
				}
				// Если индексация вершин не используется
				else {
					// Привязать буфер вершин
					VkDeviceSize offsets[1] = { 0 };
					vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &(primitives[primitiveIndex].vertexBuffer.vkBuffer), offsets);

					// Отрисовка
					vkCmdDraw(commandBuffers[i], primitives[primitiveIndex].vertexBuffer.count, 1, 0, 0);
				}

			}
		}

		// Завершение прохода
		vkCmdEndRenderPass(commandBuffers[i]);

		// Завершение прохода добавит неявное преобразование памяти фрейм-буфера в
		// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR для представления содержимого 

		// Завершение записи команд
		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Vulkan: Error while preparing commands");
		}
	}
}

/**
* Сброс командных буферов (для перезаписи)
* @param const vktoolkit::Device &device - устройство, для получения хендлов очередей
* @param std::vector<VkCommandBuffer> commandBuffers - массив хендлов командных буферов, которые необходимо сбросить
*/
void VkRenderer::ResetCommandBuffers(const vktoolkit::Device &device, std::vector<VkCommandBuffer> commandBuffers)
{
	// Приостановить рендеринг
	this->isReady_ = false;

	// Подождать завершения всех команд в очередях
	if (device.queues.graphics != VK_NULL_HANDLE && device.queues.present != VK_NULL_HANDLE) {
		vkQueueWaitIdle(device.queues.graphics);
		vkQueueWaitIdle(device.queues.present);
	}

	// Сбросить буферы команд
	if (!commandBuffers.empty()) {
		for (const VkCommandBuffer &buffer : commandBuffers) {
			if (vkResetCommandBuffer(buffer, 0) != VK_SUCCESS) {
				throw std::runtime_error("Vulkan: Error while resetting commad buffers");
			}
		}
	}

	// Снова готово к рендерингу
	this->isReady_ = true;
}

/* И Н Т Е Р Ф Е Й С Н Ы Е  М Е Т О Д Ы */

/**
* Приостановка рендеринга с ожиданием завершения выполнения всех команд
*/
void VkRenderer::Pause()
{
	// Ожидание завершения всех возможных процессов
	if (this->device_.logicalDevice != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(this->device_.logicalDevice);
	}

	this->isRendering_ = false;
}

/**
* Возврат к состоянию "рендерится"
*/
void VkRenderer::Continue()
{
	this->isRendering_ = true;
}

/**
* Данный метод вызывается при смене разрешения поверхности отображения
* либо (в дальнейшем) при смене каких-либо ниных настроек графики. В нем происходит
* пересоздание swap-chain'а и всего того, что зависит от измененных настроек
*/
void VkRenderer::VideoSettingsChanged()
{
	// Оставноить выполнение команд
	this->Pause();

	// В начале деинициализировать компоненты зависимые от swap-chain
	this->DeinitCommandBuffers(this->device_, this->commandPoolDraw_, &(this->commandBuffersDraw_));
	this->DeinitGraphicsPipeline(this->device_, &(this->pipeline_));

	// Render pass не зависит от swap-chain, но поскольку поверхность могла сменить свои свойства - следует пересоздать
	// по новой, проверив формат цветового вложения
	this->DeinitRenderPass(this->device_, &(this->renderPass_));
	this->renderPass_ = this->InitRenderPass(this->device_, this->surface_, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT);

	// Ре-инициализация swap-cahin. 
	// В начале получаем старый swap-chain
	vktoolkit::Swapchain oldSwapChain = this->swapchain_;
	// Инициализируем обновленный
	this->swapchain_ = this->InitSwapChain(
		this->device_,
		this->surface_,
		{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		this->renderPass_,
		3,
		&oldSwapChain);
	// Уничтожаем старый
	this->DeinitSwapchain(this->device_, &(oldSwapChain));

	// Инициализация графического конвейера
	this->pipeline_ = this->InitGraphicsPipeline(this->device_, this->pipelineLayout_, this->swapchain_, this->renderPass_);

	// Аллокация командных буферов (получение хендлов)
	this->commandBuffersDraw_ = this->InitCommandBuffers(this->device_, this->commandPoolDraw_, (unsigned int)this->swapchain_.framebuffers.size());

	// Подготовка базовых комманд
	this->PrepareDrawCommands(
		this->commandBuffersDraw_,
		this->renderPass_,
		this->pipelineLayout_,
		this->descriptorSetMain_,
		this->pipeline_,
		this->swapchain_,
		this->primitives_);

	// Снова можно рендерить
	this->Continue();

	// Обновить
	this->Update();
}

/**
* В методе отрисовки происходит отправка подготовленных команд а так-же показ
* готовых изображение на поверхности показа
*/
void VkRenderer::Draw()
{
	// Ничего не делать если не готово или приостановлено
	if (!this->isReady_ || !this->isRendering_) {
		return;
	}

	// Индекс доступного изображения
	unsigned int imageIndex;

	// Получить индекс доступного изображения из swap-chain и "включить" семафор сигнализирующий о доступности изображения для рендеринга
	VkResult acquireStatus = vkAcquireNextImageKHR(
		this->device_.logicalDevice,
		this->swapchain_.vkSwapchain,
		10000,
		this->sync_.readyToRender,
		VK_NULL_HANDLE,
		&imageIndex);

	// Если не получилось получить изображение, вероятно поверхность изменилась или swap-chain более ей не соответствует по каким-либо причинам
	// VK_SUBOPTIMAL_KHR означает что swap-chain еще может быть использован, но в полной мере поверхности не соответствует
	if (acquireStatus != VK_SUCCESS && acquireStatus != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Vulkan: Error. Can't acquire swap-chain image");
	}

	// Данные семафоры будут ожидаться на определенных стадиях ковейера
	std::vector<VkSemaphore> waitSemaphores = { this->sync_.readyToRender };

	// Данные семафоры будут "включаться" на определенных стадиях ковейера
	std::vector<VkSemaphore> signalSemaphores = { this->sync_.readyToPresent };

	// Стадии конвейера на которых будет происходить одидание семафоров (на i-ой стадии включения i-ого семафора из waitSemaphores)		
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	// Информация об отправке команд в буфер
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = (uint32_t)waitSemaphores.size();       // Кол-во семафоров ожидания
	submitInfo.pWaitSemaphores = waitSemaphores.data();                    // Семафоры велючение которых будет ожидаться
	submitInfo.pWaitDstStageMask = waitStages;                             // Стадии на которых конвейер "приостановиться" до включения семафоров
	submitInfo.commandBufferCount = 1;                                     // Число командных буферов за одну отправку
	submitInfo.pCommandBuffers = &(this->commandBuffersDraw_[imageIndex]); // Командный буфер (для текущего изображения в swap-chain)
	submitInfo.signalSemaphoreCount = (uint32_t)signalSemaphores.size();   // Кол-во семафоров сигнала (завершения стадии)
	submitInfo.pSignalSemaphores = signalSemaphores.data();                // Семафоры которые включатся при завершении

	// Инициировать отправку команд в очередь (на рендеринг)
	VkResult result = vkQueueSubmit(this->device_.queues.graphics, 1, &submitInfo, VK_NULL_HANDLE);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error. Can't submit commands");
	}

	// Настройка представления (отображение того что отдал конвейер)
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = (uint32_t)signalSemaphores.size();    // Кол-во ожидаемых семафоров
	presentInfo.pWaitSemaphores = signalSemaphores.data();                 // Cемафоры "включение" которых ожидается перед показом
	presentInfo.swapchainCount = 1;                                        // Кол-во swap-chain'ов
	presentInfo.pSwapchains = &(this->swapchain_.vkSwapchain);             // Указание текущего swap-chain
	presentInfo.pImageIndices = &imageIndex;                               // Индекс текущего изображения, куда осуществляется показ
	presentInfo.pResults = nullptr;

	// Инициировать представление
	VkResult presentStatus = vkQueuePresentKHR(this->device_.queues.present, &presentInfo);

	// Представление могло не выполниться если поверхность изменилась или swap-chain более ей не соответствует
	if (presentStatus != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error. Failed to present!");
	}
}

/**
* В методе обновления происходит отправка новых данных в UBO буферы, то есть
* учитываются положения камеры, отдельных примитивов и сцены в целом
*/
void VkRenderer::Update()
{
	// Соотношение сторон (используем размеры поверхности определенные при создании swap-chain)
	this->camera_.aspectRatio = (float)(this->swapchain_.imageExtent.width) / (float)(this->swapchain_.imageExtent.height);

	// Настройка матрицы проекции
	// При помощи данной матрицы происходит проекция 3-мерных точек на плоскость
	// Считается что наблюдатель (камера) в центре системы координат
	this->uboWorld_.projectionMatrix = this->camera_.MakeProjectionMatrix();

	// Настройка матрицы вида
	// Отвечает за положение и поворот камеры (по сути приводит систему координат мира к системе координат наблюдателя)
	this->uboWorld_.viewMatrix = this->camera_.MakeViewMatrix();

	// Матрица модели мира
	// Позволяет осуществлять глобальные преобразования всей сцены (пока что не используется)
	this->uboWorld_.worldMatrix = glm::mat4();

	// Копировать данные в uniform-буфер
	memcpy(this->uniformBufferWorld_.pMapped, &(this->uboWorld_), (size_t)(this->uniformBufferWorld_.size));

	// Теперь необходимо обновить динамический буфер формы объектов (если они есть)
	if (!this->primitives_.empty()) {

		// Динамическое выравнивание для одного элемента массива
		VkDeviceSize dynamicAlignment = this->device_.GetDynamicAlignment<glm::mat4>();

		// Пройтись по всем объектам
		for (unsigned int i = 0; i < this->primitives_.size(); i++) {

			// Используя выравнивание получить указатель на нужный элемент массива
			glm::mat4* modelMat = (glm::mat4*)(((uint64_t)(this->uboModels_) + (i * dynamicAlignment)));

			// Вписать данные матрицы в элемент
			*modelMat = glm::translate(glm::mat4(), this->primitives_[i].position);
			*modelMat = glm::rotate(*modelMat, glm::radians(this->primitives_[i].rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			*modelMat = glm::rotate(*modelMat, glm::radians(this->primitives_[i].rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			*modelMat = glm::rotate(*modelMat, glm::radians(this->primitives_[i].rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		}

		// Копировать данные в uniform-буфер
		memcpy(this->uniformBufferModels_.pMapped, this->uboModels_, (size_t)(this->uniformBufferModels_.size));

		// Гарантировать видимость обновленной памяти устройством
		VkMappedMemoryRange memoryRange = {};
		memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		memoryRange.memory = this->uniformBufferModels_.vkDeviceMemory;
		memoryRange.size = this->uniformBufferModels_.size;
		vkFlushMappedMemoryRanges(this->device_.logicalDevice, 1, &memoryRange);
	}
}

/**
* Настройка параметров перспективы камеры (угол обзора, грани отсечения)
* @param float fFOV - угол обзора
* @param float fNear - ближняя грань отсечения
* @param float fFar - дальняя гран отсечения
*/
void VkRenderer::SetCameraPerspectiveSettings(float fFOV, float fNear, float fFar)
{
	this->camera_.fFOV = fFOV;
	this->camera_.fNear = fNear;
	this->camera_.fFar = fFar;
}

/**
* Настройка положения камеры
* @param float x - положение по оси X
* @param float y - положение по оси Y
* @param float z - положение по оси Z
*/
void VkRenderer::SetCameraPosition(float x, float y, float z)
{
	this->camera_.position = glm::vec3(x, y, z);
}

/**
* Настройка поворота камеры
* @param float x - поворот вокруг оси X
* @param float y - поворот вокруг оси Y
* @param float z - поворот вокруг оси Z
*/
void VkRenderer::SetCameraRotation(float x, float y, float z)
{
	this->camera_.roation = glm::vec3(x, y, z);
}

/**
* Добавление нового примитива
* @param const std::vector<vktoolkit::Vertex> &vertices - массив вершин
* @param const std::vector<unsigned int> &indices - массив индексов
* @param glm::vec3 position - положение относительно глобального центра
* @param glm::vec3 rotaton - вращение вокруг локального центра
* @param glm::vec3 scale - масштаб
* @return unsigned int - индекс примитива
*/
unsigned int VkRenderer::AddPrimitive(
	const std::vector<vktoolkit::Vertex> &vertices,
	const std::vector<unsigned int> &indices,
	const vktoolkit::Texture *texture,
	glm::vec3 position,
	glm::vec3 rotaton,
	glm::vec3 scale)
{
	// Новый примитив
	vktoolkit::Primitive primitive;
	primitive.position = position;
	primitive.rotation = rotaton;
	primitive.scale = scale;
	primitive.texture = texture;
	primitive.drawIndexed = !indices.empty();

	// Буфер вершин (временный)
	std::vector<vktoolkit::Vertex> vertexBuffer = vertices;
	VkDeviceSize vertexBufferSize = ((unsigned int)vertexBuffer.size()) * sizeof(vktoolkit::Vertex);
	unsigned int vertexCount = (unsigned int)vertexBuffer.size();

	// Создать буфер вершин в памяти хоста
	vktoolkit::Buffer tmp = vktoolkit::CreateBuffer(this->device_, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	primitive.vertexBuffer.vkBuffer = tmp.vkBuffer;
	primitive.vertexBuffer.vkDeviceMemory = tmp.vkDeviceMemory;
	primitive.vertexBuffer.size = tmp.size;
	primitive.vertexBuffer.count = vertexCount;

	// Разметить память буфера вершин и скопировать в него данные, после чего убрать разметку
	void * verticesMemPtr;
	vkMapMemory(this->device_.logicalDevice, primitive.vertexBuffer.vkDeviceMemory, 0, vertexBufferSize, 0, &verticesMemPtr);
	memcpy(verticesMemPtr, vertexBuffer.data(), (std::size_t)vertexBufferSize);
	vkUnmapMemory(this->device_.logicalDevice, primitive.vertexBuffer.vkDeviceMemory);

	// Если необходимо рисовать индексированную геометрию
	if (primitive.drawIndexed) {

		// Буфер индексов (временный)
		std::vector<unsigned int> indexBuffer = indices;
		VkDeviceSize indexBufferSize = ((unsigned int)indexBuffer.size()) * sizeof(unsigned int);
		unsigned int indexCount = (unsigned int)indexBuffer.size();

		// Cоздать буфер индексов в памяти хоста
		tmp = vktoolkit::CreateBuffer(this->device_, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		primitive.indexBuffer.vkBuffer = tmp.vkBuffer;
		primitive.indexBuffer.vkDeviceMemory = tmp.vkDeviceMemory;
		primitive.indexBuffer.size = tmp.size;
		primitive.indexBuffer.count = indexCount;

		// Разметить память буфера индексов и скопировать в него данные, после чего убрать разметку
		void * indicesMemPtr;
		vkMapMemory(this->device_.logicalDevice, primitive.indexBuffer.vkDeviceMemory, 0, indexBufferSize, 0, &indicesMemPtr);
		memcpy(indicesMemPtr, indexBuffer.data(), (std::size_t)indexBufferSize);
		vkUnmapMemory(this->device_.logicalDevice, primitive.indexBuffer.vkDeviceMemory);
	}


	// Впихнуть новый примитив в массив
	this->primitives_.push_back(primitive);

	// Обновить командный буфер
	this->ResetCommandBuffers(this->device_, this->commandBuffersDraw_);
	this->PrepareDrawCommands(this->commandBuffersDraw_, this->renderPass_, this->pipelineLayout_, this->descriptorSetMain_, this->pipeline_, this->swapchain_, this->primitives_);

	// Вернуть индекс
	return (unsigned int)(this->primitives_.size() - 1);
}

/**
* Создание текстуры по данным о пикселях
* @param const unsigned char* pixels - пиксели загруженные из файла
* @return vktoolkit::Texture - структура с набором хендлов изображения и дескриптора
*
* @note - при загрузке используется временный буфер (временное изображение) для перемещения
* в буфер распологающийся в памяти устройства. Нельзя сразу создать буфер в памяти устройства и переместить
* в него данные. Это можно сделать только пр помощи команды копирования (из памяти доступной хосту в память
* доступную только устройству)
*/
vktoolkit::Texture VkRenderer::CreateTexture(const unsigned char* pixels, uint32_t width, uint32_t height, uint32_t channels, uint32_t bpp)
{
	// Приостановить выполнение основных команд (если какие-либо в процессе)
	this->Pause();

	// Результат
	vktoolkit::Texture resultTexture = {};

	// Размер изображения (ожидаем по умолчанию 4 байта на пиксель, в режиме RGBA)
	VkDeviceSize size = (VkDeviceSize)(width * height * bpp);

	// Если данных не обнаружено
	if (!pixels) {
		throw std::runtime_error("Vulkan: Error while creating texture. Empty pixel buffer recieved");
	}

	// Создать промежуточное изображение
	vktoolkit::Image stagingImage = vktoolkit::CreateImageSingle(
		this->device_,
		VK_IMAGE_TYPE_2D,
		VK_FORMAT_R8G8B8A8_UNORM,
		{ width,height,1 },
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_IMAGE_TILING_LINEAR);

	// Выбрать подресурс изображения (мип-уровень 0, слой - 0)
	VkImageSubresource subresource = {};
	subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource.mipLevel = 0;
	subresource.arrayLayer = 0;

	// Размещение байт в подресурсе
	VkSubresourceLayout stagingImageSubresourceLayout = {};
	vkGetImageSubresourceLayout(this->device_.logicalDevice, stagingImage.vkImage, &subresource, &stagingImageSubresourceLayout);

	// Разметить память под изображение
	void* data;
	vkMapMemory(this->device_.logicalDevice, stagingImage.vkDeviceMemory, 0, size, 0, &data);

	// Если "ширина строки" равна кол-ву пиксилей по ширине помноженному на bpp - можно исользовать обычный memcpy
	if (stagingImageSubresourceLayout.rowPitch == width * bpp) {
		memcpy(data, pixels, (unsigned int)size);
	}
	// Если нет (например размер изображения не кратен степени двойки) - перебираем байты со смещением и копируем каждую стороку
	else {
		unsigned char* dataBytes = reinterpret_cast<unsigned char*>(data);
		for (unsigned int y = 0; y < height; y++) {
			memcpy(
				&dataBytes[y * (stagingImageSubresourceLayout.rowPitch)],
				&pixels[y * width * bpp],
				width * bpp
			);
		}
	}

	// Убрать разметку памяти
	vkUnmapMemory(this->device_.logicalDevice, stagingImage.vkDeviceMemory);

	// Создать финальное изображение (в памяти устройства)
	resultTexture.image = vktoolkit::CreateImageSingle(
		this->device_,
		VK_IMAGE_TYPE_2D,
		VK_FORMAT_R8G8B8A8_UNORM,
		{ width,height,1 },
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		VK_IMAGE_TILING_OPTIMAL);

	
	// Создать командный буфер для команд перевода размещения изображений
	VkCommandBuffer transitionCmdBuffer = vktoolkit::CreateSingleTimeCommandBuffer(this->device_, this->commandPoolDraw_);

	// Подресурс подвергающийся смере размещения в изображениях (описываем его)
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;

	// Сменить размещение памяти промежуточного изображения в VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	vktoolkit::CmdImageLayoutTransition(transitionCmdBuffer, stagingImage.vkImage, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresourceRange);

	// Сменить размещение памяти целевого изображения в VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	vktoolkit::CmdImageLayoutTransition(transitionCmdBuffer, resultTexture.image.vkImage, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

	// Выполнить команды перевода размещения
	vktoolkit::FlushSingleTimeCommandBuffer(this->device_, this->commandPoolDraw_, transitionCmdBuffer, this->device_.queues.graphics);

	// Создать командный буфер для копирования изображения
	VkCommandBuffer copyCmdBuffer = vktoolkit::CreateSingleTimeCommandBuffer(this->device_, this->commandPoolDraw_);
	
	// Копирование из промежуточной картинки в основную
	vktoolkit::CmdImageCopy(copyCmdBuffer, stagingImage.vkImage, resultTexture.image.vkImage, width, height);

	// Выполнить команды копирования
	vktoolkit::FlushSingleTimeCommandBuffer(this->device_, this->commandPoolDraw_, copyCmdBuffer, this->device_.queues.graphics);

	// Очистить промежуточное изображение
	stagingImage.Deinit(this->device_.logicalDevice);


	// Получить новый набор дескрипторов из дескриптороного пула
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = this->descriptorPoolTextures_;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &(this->descriptorSetLayoutTextures_);

	if (vkAllocateDescriptorSets(this->device_.logicalDevice, &descriptorSetAllocInfo, &(resultTexture.descriptorSet)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkAllocateDescriptorSets. Can't allocate descriptor set for texture");
	}

	// Информация о передаваемом изображении
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = resultTexture.image.vkImageView;
	imageInfo.sampler = this->textureSampler_;

	// Конфигурация добавляемых в набор дескрипторов
	std::vector<VkWriteDescriptorSet> writes =
	{
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // Тип структуры
			nullptr,                                     // pNext
			resultTexture.descriptorSet,                 // Целевой набор дескрипторов
			0,                                           // Точка привязки (у шейдера)
			0,                                           // Элемент массив (массив не используется)
			1,                                           // Кол-во дескрипторов
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,   // Тип дескриптора
			&imageInfo,                                  // Информация о параметрах изображения
			nullptr,
			nullptr
		}
	};

	// Обновить наборы дескрипторов
	vkUpdateDescriptorSets(this->device_.logicalDevice, (uint32_t)writes.size(), writes.data(), 0, nullptr);

	// Исполнение основых команд снова возможно
	this->Continue();

	// Вернуть результат
	return resultTexture;
}