#pragma once

#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>

// Активация расширения интеграции с ОС (Window System Integration)
// Даем понять что работаем с Windows
#define VK_USE_PLATFORM_WIN32_KHR

// Подключаем vulkan
#include <vulkan\vulkan.h>

// Подключаем glm для работы с матрицами
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

// Вспомогательные инструменты
#include "Toolkit.h"
#include "VkToolkit.h"

// Переменная IS_VK_DEBUG будет true если используется debug конфиуграция
// В зависимости от данной переменной некоторое поведение может меняться
#ifdef NDEBUG
const bool IS_VK_DEBUG = false;
#else
const bool IS_VK_DEBUG = true;
#endif

class VkRenderer
{
private:
	bool isReady_;                                        // Состояние готовности к рендерингу
	bool isRendering_;                                    // В процессе ли рендеринг

	VkInstance instance_;                                 // Хендл instance'а vulkan'а
	VkDebugReportCallbackEXT validationReportCallback_;   // Хендл объекта callback'а
	VkSurfaceKHR surface_;                                // Хендл поверхности отображения
	vktoolkit::Device device_;                            // Устройство (структура с хендлами физ-го и лог-го ус-ва, очередей)
	VkRenderPass renderPass_;                             // Основной проход рендеринга
	vktoolkit::Swapchain swapchain_;                      // Своп-чейн (набор показа, набор сменяющихся буферов кадра)
	VkCommandPool commandPoolDraw_;                       // Командный пул (для выделения командных буферов)
	std::vector<VkCommandBuffer> commandBuffersDraw_;     // Командные буферы (свой на каждое изобр. swap-chain, с набором команд что и в других, в целях синхронизации)
	vktoolkit::UniformBuffer uniformBufferWorld_;         // Буфер формы сцены (содержит хендлы буфера, памяти, инфо для дескриптора)
	vktoolkit::UniformBuffer uniformBufferModels_;        // Буфер формы объектов (содержит хендлы буфера, памяти, инфо для дескриптора)
	vktoolkit::UboWorld uboWorld_;                        // Структура с матрицами для общих преобразований сцены (данный объект буедт передаваться в буфер формы сцены)
	vktoolkit::UboModelArray uboModels_;                  // Массив матриц (указатель на него) для отдельный объектов (матрицы модели, передаются в буфер формы объектов)
	VkDescriptorPool descriptorPool_;                     // Пул дескрипторов (для выделения наборов дескрипторов)
	VkDescriptorSetLayout descriptorSetLayout_;           // Размещение набора дескрипторов (дескрипторы каких типов к каким стадиям конвейера привязаываются)
	VkDescriptorSet descriptorSet_;                       // Набор дескрипторов
	VkPipelineLayout pipelineLayout_;                     // Размещение конвейера
	VkPipeline pipeline_;                                 // Основной графический конвейер
	vktoolkit::Synchronization sync_;                     // Примитивы синхронизации

	unsigned int primitivesMaxCount_;                     // Максимальное кол-во примитивов (необходимо для аллокации динамического UBO буфера)
	std::vector<vktoolkit::Primitive> primitives_;        // Набор геометр. примитивов для отображения


	/* I N S T A N C E */

	/**
	* Метод инициализации instance'а vulkan'а
	* @param std::string applicationName - наименование приложения (информация может быть полезна разработчикам драйверов)
	* @param std::string engineName - наименование движка (информация может быть полезна разработчикам драйверов)
	* @param std::vector<const char*> extensionsRequired - запрашиваемые расширения экземпляра
	* @param std::vector<const char*>validationLayersRequired - запрашиваемые слои валидации
	* @return VkInstance - хендл экземпляра Vulkan
	*/
	VkInstance InitInstance(
		std::string applicationName,
		std::string engineName,
		std::vector<const char*> extensionsRequired,
		std::vector<const char*> validationLayersRequired);

	/**
	* Метод деинициализации Instance'а
	* @param VkInstance * vkInstance - указатель на хендл Instance'а который нужно уничтожить
	*/
	void DeinitInstance(VkInstance * vkInstance);

	/* П О В Е Р Х Н О С Т Ь */

	/**
	* Инициализация поверхности отображения (поверхность окна)
	* @param VkInstance vkInstance - хендл экземпляра Vulkan
	* @param HINSTANCE hInstance - хендл Windows приложения
	* @param HWND hWnd - хендл Windows окна
	* @return VkSurfaceKHR - хендл созданной поверхности
	*/
	VkSurfaceKHR InitWindowSurface(VkInstance vkInstance, HINSTANCE hInstance, HWND hWnd);

	/**
	* Деинициализация поверхности отображения (поверхность окна)
	* @param VkInstance vkInstance - хендл экземпляра Vulkan
	* @param VkSurfaceKHR * surface - указатель на хендл поверхности которую нужно уничтожить
	*/
	void DeinitWindowSurface(VkInstance vkInstance, VkSurfaceKHR * surface);

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
	vktoolkit::Device InitDevice(
		VkInstance vkInstance,
		VkSurfaceKHR surface,
		std::vector<const char*> extensionsRequired,
		std::vector<const char*> validationLayersRequired,
		bool uniqueQueueFamilies);

	/**
	* Деинициализация поверхности отображения (поверхность окна)
	* @param vktoolkit::Device * device - указатель на структуру с хендлами устройства
	*/
	void DeinitDevice(vktoolkit::Device * device);

	/* П Р О Х О Д  Р Е Н Д Е Р И Н Г А */

	/**
	* Инициализация прохода рендеринга.
	* @param const vktoolkit::Device &device - устройство
	* @param VkSurfaceKHR surface - хендо поверхности, передается лишь для проверки поддержки запрашиваемого формата
	* @param VkFormat colorAttachmentFormat - формат цветовых вложений/изображений, должен поддерживаться поверхностью
	* @return VkRenderPass - хендл прохода рендеринга
	*
	* @note - Проход рендеринга можно понимать как некую стадию на которой выполняются все команды рендерига и происходит цикл конвейера
	* Проход состоит из под-проходов, и у каждого под-прохода может быть своя конфигурация конвейера. Конфигурация же прохода
	* определяет в каком состоянии (размещении памяти) будет вложение (цветовое, глубины и тд)
	*/
	VkRenderPass InitRenderPass(const vktoolkit::Device &device, VkSurfaceKHR surface, VkFormat colorAttachmentFormat);

	/**
	* Деинициализация прохода рендеринга
	* @param const vktoolkit::Device &device - устройство которой принимало участие в создании прохода
	* @param VkRenderPass * renderPass - указатель на хендл прохода
	*/
	void DeinitRenderPass(const vktoolkit::Device &device, VkRenderPass * renderPass);

	/* S W A P C H A I N */

	/**
	* Swap-chain (список показа, цепочка свопинга) - представляет из себя набор сменяемых изображений
	* @param const vktoolkit::Device &device - устройство, необходимо для создания
	* @param VkSurfaceKHR surface - хкндл поверхоности, необходимо для создания объекта swap-chain и для проверки поддержки формата
	* @param VkSurfaceFormatKHR surfaceFormat - формат изображений и цветовое пространство (должен поддерживаться поверхностью)
	* @param VkRenderPass renderPass - хендл прохода рендеринга, нужен для создания фрейм-буферов swap-chain
	* @param unsigned int bufferCount - кол-во буферов кадра (напр. для тройной буферизации - 3)
	* @param vktoolkit::Swapchain * oldSwapchain - передыдуший swap-chain (полезно в случае пересоздания свап-чейна, например, сменив размеро поверхности)
	* @return vktoolkit::Swapchain структура описывающая swap-chain cодержащая необходимые хендлы
	* @note - в одно изображение может происходить запись (рендеринг) в то время как другое будет показываться (презентация)
	*/
	vktoolkit::Swapchain InitSwapChain(
		const vktoolkit::Device &device,
		VkSurfaceKHR surface,
		VkSurfaceFormatKHR surfaceFormat,
		VkRenderPass renderPass,
		unsigned int bufferCount,
		vktoolkit::Swapchain * oldSwapchain = nullptr);

	/**
	* Деинициализация swap-chain
	* @param const vktoolkit::Device &device - устройство которое принимало участие в создании swap-chain
	* @param vktoolkit::Swapchain * swapchain - указатель на объект своп-чейна
	*/
	void DeinitSwapchain(const vktoolkit::Device &device, vktoolkit::Swapchain * swapchain);

	/* К О М А Н Д Н Ы Е  П У Л Ы  И  Б У Ф Е Р Ы */

	/**
	* Инциализация командного пула
	* @param const vktoolkit::Device &device - устройство
	* @param unsigned int queueFamilyIndex - индекс семейства очередей команды которого будут передаваться в аллоцированных их пуда буферах
	* @return VkCommandPool - хендл командного пула
	* @note - из командных пулов осуществляется аллокация буферов команд, следует учитывать что для отедльных очередей нужны отдельные пулы
	*/
	VkCommandPool InitCommandPool(const vktoolkit::Device &device, unsigned int queueFamilyIndex);

	/**
	* Деинциализация командного пула
	* @param const vktoolkit::Device &device - устройство
	* @param VkCommandPool * commandPool - указатель на хендл командного пула
	*/
	void DeinitCommandPool(const vktoolkit::Device &device, VkCommandPool * commandPool);

	/**
	* Аллокация командных буферов
	* @param const vktoolkit::Device &device - устройство
	* @param VkCommandPool commandPool - хендл командного пула из которого будет осуществляться аллокация
	* @param unsigned int count - кол-во аллоцируемых буферов
	* @return std::vector<VkCommandBuffer> массив хендлов командных буферов
	*/
	std::vector<VkCommandBuffer> InitCommandBuffers(const vktoolkit::Device &device, VkCommandPool commandPool, unsigned int count);

	/**
	* Деинициализация (очистка) командных буферов
	* @param const vktoolkit::Device &device - устройство
	* @param VkCommandPool commandPool - хендл командного пула из которого были аллоцированы буферы
	* @param std::vector<VkCommandBuffer> * buffers - указатель на массив с хендлами буферов (он будет обнулен после очистки)
	*/
	void DeinitCommandBuffers(const vktoolkit::Device &device, VkCommandPool commandPool, std::vector<VkCommandBuffer> * buffers);

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
	vktoolkit::UniformBuffer InitUnformBufferWorld(const vktoolkit::Device &device);

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
	vktoolkit::UniformBuffer InitUniformBufferModels(const vktoolkit::Device &device, unsigned int maxObjects);

	/**
	* Деинициализация (очистка) командного буфера
	* @param const vktoolkit::Device &device - устройство
	* @param vktoolkit::UniformBuffer * uniformBuffer - указатель на структуру буфера
	*/
	void DeinitUniformBuffer(const vktoolkit::Device &device, vktoolkit::UniformBuffer * uniformBuffer);

	/* Д Е С К Р И П Т О Р Н Ы Й  П У Л */

	/**
	* Инициализация декскрипторного пула
	* @param const vktoolkit::Device &device - устройство
	* @return VkDescriptorPool - хендл дескрипторного пула
	* @note - дескрипторный пул позволяет выделять специальные наборы дескрипторов, обеспечивающие доступ к определенным буферам из шейдера
	*/
	VkDescriptorPool InitDescriptorPool(const vktoolkit::Device &device);

	/**
	* Деинициализация дескрипторного пула
	* @param const vktoolkit::Device &device - устройство
	* @VkDescriptorPool * descriptorPool - указатель на хендл дескрипторного пула
	*/
	void DeinitDescriptorPool(const vktoolkit::Device &device, VkDescriptorPool * descriptorPool);

	/* Р А З М Е Щ Е Н И Е  Д Е С К Р И П Т О Р Н О Г О  Н А Б О Р А */

	/**
	* Инициализация описания размещения дескрипторного пула
	* @param const vktoolkit::Device &device - устройство
	* @return VkDescriptorSetLayout - хендл размещения дескрипторного пула
	* @note - Размещение - информация о том сколько и каких именно (какого типа) дескрипторов следует ожидать на определенных этапах конвейера
	*/
	VkDescriptorSetLayout InitDescriptorSetLayout(const vktoolkit::Device &device);

	/**
	* Деинициализация размещения
	* @param const vktoolkit::Device &device - устройство
	* @VkDescriptorSetLayout * descriptorSetLayout - указатель на хендл размещения
	*/
	void DeinitDescriporSetLayout(const vktoolkit::Device &device, VkDescriptorSetLayout * descriptorSetLayout);

	/* Н А Б О Р  Д Е С К Р И П Т О Р О В */

	/**
	* Инициализация набор дескрипторов
	* @param const vktoolkit::Device &device - устройство
	* @param VkDescriptorPool descriptorPool - хендл дескрипторного пула из которого будет выделен набор
	* @param VkDescriptorSetLayout descriptorSetLayout - хендл размещения дескрипторно набора
	* @param const vktoolkit::UniformBuffer &uniformBufferWorld - буфер содержит необходимую для создания дескриптора информацию
	* @param const vktoolkit::UniformBuffer &uniformBufferModels - буфер содержит необходимую для создания дескриптора информацию
	*/
	VkDescriptorSet InitDescriptorSet(
		const vktoolkit::Device &device,
		VkDescriptorPool descriptorPool,
		VkDescriptorSetLayout descriptorSetLayout,
		const vktoolkit::UniformBuffer &uniformBufferWorld,
		const vktoolkit::UniformBuffer &uniformBufferModels);

	/**
	* Деинициализация набор дескрипторов
	* @param const vktoolkit::Device &device - устройство
	* @param VkDescriptorPool descriptorPool - хендл дескрипторного пула из которого будет выделен набор
	* @param VkDescriptorSet * descriptorSet - указатель на хендл набора дескрипторов
	*/
	void DeinitDescriptorSet(const vktoolkit::Device &device, VkDescriptorPool descriptorPool, VkDescriptorSet * descriptorSet);

	/* О Б Ъ Е К Т Ы  Д И Н А М И Ч. Д Е С К Р И П Т О Р Н Ы Х  Б У Ф Е Р О В */

	/**
	* Аллокация памяти под объект динамического UBO буфера
	* @param const vktoolkit::Device &device - устройство (необходимо для получения выравнивания)
	* @param unsigned int maxObjects - максимальное кол-вл объектов (для выяснения размера аллоцируемой памяти)
	* @return vktoolkit::UboModelArray - указатель на аллоцированный массив матриц
	*/
	vktoolkit::UboModelArray AllocateUboModels(const vktoolkit::Device &device, unsigned int maxObjects);

	/**
	* Освобождение памяти объекта динамического UBO буфера
	* @param vktoolkit::UboModelArray * uboModels - указатель на массив матриц, память которого будет очищена
	*/
	void FreeUboModels(vktoolkit::UboModelArray * uboModels);

	/* Г Р А Ф И Ч Е С К И Й  К О Н В Е Й Е Р */

	/**
	* Инициализация размещения графического конвейера
	* @param const vktoolkit::Device &device - устройство
	* @param VkDescriptorSetLayout descriptorSetLayout - хендл размещения дискрипторного набора (дает конвейеру инфу о дескрипторах)
	* @return VkPipelineLayout - хендл размещения конвейера
	*/
	VkPipelineLayout InitPipelineLayout(const vktoolkit::Device &device, VkDescriptorSetLayout descriptorSetLayout);

	/**
	* Деинициализация размещения графического конвейера
	* @param const vktoolkit::Device &device - устройство
	* @param VkPipelineLayout * pipelineLayout - указатель на хендл размещения
	*/
	void DeinitPipelineLayout(const vktoolkit::Device &device, VkPipelineLayout * pipelineLayout);

	/**
	* Инициализация графического конвейера
	* @param const vktoolkit::Device &device - устройство
	* @param VkPipelineLayout pipelineLayout - хендл размещения конвейера
	* @param vktoolkit::Swapchain &swapchain - swap-chain, для получения информации о разрешении
	* @param VkRenderPass renderPass - хендл прохода рендеринга (на него ссылается конвейер)
	* @return VkPipeline - хендл конвейера
	*
	* @note - графический конвейер производит рендериннг принимая вершинные данные на вход и выводя пиксели в 
	* буферы кадров. Конвейер состоит из множества стадий, некоторые из них программируемые (шейдерные). Конвейер 
	* относится к конкретному проходу рендеринга (и подпроходу). В методе происходит конфигурация всех стадий, 
	* загрузка шейдеров, настройка конвейера.
	*/
	VkPipeline InitGraphicsPipeline(
		const vktoolkit::Device &device,
		VkPipelineLayout pipelineLayout,
		const vktoolkit::Swapchain &swapchain,
		VkRenderPass renderPass);

	/**
	* Деинициализация графического конвейера
	* @param const vktoolkit::Device &device - устройство
	* @param VkPipeline * pipeline - указатель на хендл конвейера
	*/
	void DeinitGraphicsPipeline(const vktoolkit::Device &device, VkPipeline * pipeline);

	/* С И Н Х Р О Н И З А Ц И Я */

	/**
	* Инициализация примитивов синхронизации
	* @param const vktoolkit::Device &device - устройство
	* @return vktoolkit::Synchronization - структура с набором хендлов семафоров
	* @note - семафоры синхронизации позволяют отслеживать состояние рендеринга и в нужный момент показывать изображение
	*/
	vktoolkit::Synchronization InitSynchronization(const vktoolkit::Device &device);

	/**
	* Деинициализация примитивов синхронизации
	* @param const vktoolkit::Device &device - устройство
	* @param vktoolkit::Synchronization * sync - указатель на структуру с хендлами семафоров
	*/
	void DeinitSynchronization(const vktoolkit::Device &device, vktoolkit::Synchronization * sync);

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
	void PrepareDrawCommands(
		std::vector<VkCommandBuffer> commandBuffers,
		VkRenderPass renderPass,
		VkPipelineLayout pipelineLayout,
		VkDescriptorSet descriptorSet,
		VkPipeline pipeline,
		const vktoolkit::Swapchain &swapchain,
		const std::vector<vktoolkit::Primitive> &primitives);

	/**
	* Сброс командных буферов (для перезаписи)
	* @param const vktoolkit::Device &device - устройство, для получения хендлов очередей
	* @param std::vector<VkCommandBuffer> commandBuffers - массив хендлов командных буферов, которые необходимо сбросить
	*/
	void ResetCommandBuffers(const vktoolkit::Device &device, std::vector<VkCommandBuffer> commandBuffers);

public:

	/**
	* Приостановка рендеринга с ожиданием завершения выполнения всех команд
	*/
	void Pause();

	/**
	* Возврат к состоянию "рендерится"
	*/
	void Continue();

	/**
	* Данный метод вызывается при смене разрешения поверхности отображения
	* либо (в дальнейшем) при смене каких-либо ниных настроек графики. В нем происходит
	* пересоздание swap-chain'а и всего того, что зависит от измененных настроек
	*/
	void VideoSettingsChanged();

	/**
	* В методе отрисовки происходит отправка подготовленных команд а так-же показ
	* готовых изображение на поверхности показа
	*/
	void Draw();

	/**
	* В методе обновления происходит отправка новых данных в UBO буферы, то есть
	* учитываются положения камеры, отдельных примитивов и сцены в целом
	*/
	void Update();

	/**
	* Добавление нового примитива
	* @param const std::vector<vktoolkit::Vertex> &vertices - массив вершин
	* @param const std::vector<unsigned int> &indices - массив индексов
	* @param glm::vec3 position - положение относительно глобального центра
	* @param glm::vec3 rotaton - вращение вокруг локального центра
	* @param glm::vec3 scale - масштаб
	* @return unsigned int - индекс примитива
	*/
	unsigned int AddPrimitive(
		const std::vector<vktoolkit::Vertex> &vertices,
		const std::vector<unsigned int> &indices,
		glm::vec3 position,
		glm::vec3 rotaton,
		glm::vec3 scale = { 1.0f,1.0f,1.0f });

	/**
	* Конструктор рендерера
	* @param HINSTANCE hInstance - хендл Windows приложения
	* @param HWND hWnd хендл - Windows окна
	* @param int primitivesMaxCount - максимальное кол-во отдельных объектов для отрисовки
	* @note - конструктор запистит инициализацию всех необходимых компоненстов Vulkan
	*/
	VkRenderer(HINSTANCE hInstance, HWND hWnd, unsigned int primitivesMaxCount = 100);

	/**
	* Деструктор рендерера
	* @note Деструктор уничтожит все используемые компоненты и освободит память
	*/
	~VkRenderer();
};

