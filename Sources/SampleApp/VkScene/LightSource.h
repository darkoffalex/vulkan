#pragma once

#include "SceneElement.h"

namespace vk
{
    namespace scene
    {
        /**
         * Размер одного элемента в массиве источников (в буфере UBO)
         * С учетом выравнивания std140
         */
        const size_t LIGHT_ENTRY_SIZE = 64;

        enum LightSourceType
        {
            ePoint = 0,
            eSpot = 1,
            eDirectional = 2
        };

        class LightSource : public SceneElement
        {
        private:
            // Классу LightSourceSet доступны private поля
            friend class LightSourceSet;

            /// Смещение в UBO буфере источников света
            size_t uboOffset_;
            /// Указатель на область памяти UBO буфера источников света
            unsigned char* pUboData_;

            /// Тип источника света, от него зависит то как источник обрабатывается шейдером
            LightSourceType type_ = LightSourceType::ePoint;
            /// Радиус сферы источника света (может использоваться для мягких теней)
            glm::float32 radius_ = 0.0f;
            /// Цвет источника света
            glm::vec3 color_ = {1.0f,1.0f,1.0f};
            /// Линейный коэффициент затухания
            glm::float32 attenuationLinear_ = 0.20f;
            /// Квадратичный коэффициент затухания
            glm::float32 attenuationQuadratic_ = 0.22f;
            /// Внутренний угол отсечения света (для типа eSpot)
            glm::float32 cutOffAngle_ = 40.0f;
            /// Внешний угол отсечения света (для типа eSpot)
            glm::float32 cutOffOuterAngle_ = 45.0f;
            /// Вектор ориентации источника
            glm::vec3 orientationVector_ = {0.0f,0.0f,-1.0f};

            /**
             * Событие смены положения
             * @param updateMatrices Запрос обновить матрицы
             */
            void onPlacementUpdated(bool updateMatrices) override;

            /**
             * Обновить в UBO соответствующую источнику область
             */
            void updateUboRegion();

        public:

            /**
             * Конструктор по умолчанию
             */
            LightSource();

            /**
             * Основной конструктор
             * @param pUboData Указатель на область памяти в UBO буфере
             * @param uboOffset Сдвиг элемента в массиве UBO буфера
             * @param type Тип источника света
             * @param position Положение источника света
             * @param color Цвет источника света
             * @param attenuationLinear Линейный коэффициент затухания
             * @param attenuationQuadratic Квадратичный коэффициент затухания
             * @param cutOffAngle Внутренний угол отсечения света (для типа eSpot)
             * @param cutOffOuterAngle Внешний угол отсечения света (для типа eSpot)
             */
            LightSource(unsigned char* pUboData,
                    size_t uboOffset,
                    const LightSourceType& type,
                    const glm::vec3& position = {0.0f,0.0f,0.0f},
                    const glm::vec3& color = {1.0f,1.0f,1.0f},
                    glm::float32 attenuationLinear = 0.20f,
                    glm::float32 attenuationQuadratic = 0.22f,
                    glm::float32 cutOffAngle = 40.0f,
                    glm::float32 cutOffOuterAngle = 45.0f);

            /**
             * Запрет копирования через инициализацию
             * @param other Ссылка на копируемый объекта
             */
            LightSource(const LightSource& other) = delete;

            /**
             * Запрет копирования через присваивание
             * @param other Ссылка на копируемый объекта
             * @return Ссылка на текущий объект
             */
            LightSource& operator=(const LightSource& other) = delete;

            /**
             * Конструктор перемещения
             * @param other R-value ссылка на другой объект
             * @details Нельзя копировать объект, но можно обменяться с ним ресурсом
             */
            LightSource(LightSource&& other) noexcept;

            /**
             * Перемещение через присваивание
             * @param other R-value ссылка на другой объект
             * @return Ссылка на текущий объект
             */
            LightSource& operator=(LightSource&& other) noexcept;

            /**
             * Деструктор по умолчанию
             */
            ~LightSource() override = default;

            /**
             * Установить тип источника
             * @param type Тип источника
             * @param updateUbo Обновить UBO буфер источников
             */
            void setType(LightSourceType& type, bool updateUbo = true);

            /**
             * Получить тип источника
             * @return Тип источника
             */
            LightSourceType getType() const;

            /**
             * Установить радиус
             * @param radius Число с плавающей точкой
             * @param updateUbo Обновить UBO буфер источников
             */
            void setRadius(glm::float32 radius,  bool updateUbo = true);

            /**
             * Получить радиус
             * @return Число с плавающей точкой
             */
            glm::float32 getRadius() const;

            /**
             * Установить цвет
             * @param color Вектор цвета
             * @param updateUbo Обновить UBO буфер источников
             */
            void setColor(const glm::vec3& color, bool updateUbo = true);

            /**
             * Получить цвет
             * @return Вектор цвета
             */
            glm::vec3 getColor() const;

            /**
             * Установить коэффициент линейного затухания
             * @param attenuationLinear Число с плавающей точкой
             * @param updateUbo Обновить UBO буфер источников
             */
            void setAttenuationLinear(glm::float32 attenuationLinear, bool updateUbo = true);

            /**
             * Получить коэффициент линейного затухания
             * @return Число с плавающей точкой
             */
            glm::float32 getAttenuationLinear() const;

            /**
             * Установить коэффициент квадратичного затухания
             * @param attenuationQuadratic Число с плавающей точкой
             * @param updateUbo Обновить UBO буфер источников
             */
            void setAttenuationQuadratic(glm::float32 attenuationQuadratic, bool updateUbo = true);

            /**
             * Получить коэффициент квадратичного затухания
             * @return Число с плавающей точкой
             */
            glm::float32 getAttenuationQuadratic() const;

            /**
             * Установить угол (внутренний) отсечения для источника типа spot-light
             * @param cutOffAngle Число с плавающей точкой
             * @param updateUbo Обновить UBO буфер источников
             */
            void setCutOffAngle(glm::float32 cutOffAngle, bool updateUbo = true);

            /**
             * Получить угол (внутренний) отсечения для источника типа spot-light
             * @return Число с плавающей точкой
             */
            glm::float32 getCutOffAngle() const;

            /**
             * Установить угол (внешний) отсечения для источника типа spot-light
             * @param cutOffOuterAngle Число с плавающей точкой
             * @param updateUbo Обновить UBO буфер источников
             */
            void setCutOffOuterAngle(glm::float32 cutOffOuterAngle, bool updateUbo = true);

            /**
             * Получить угол (внешний) отсечения для источника типа spot-light
             * @return Число с плавающей точкой
             */
            glm::float32 getCutOffOuterAngle() const;
        };

        /**
         * Smart-pointer объекта источника света
         * @rdetails Данный указатель может быть возвращен пользователю объектом рендерера при добавлении меша на сцену
         */
        typedef std::shared_ptr<LightSource> LightSourcePtr;
    }
}