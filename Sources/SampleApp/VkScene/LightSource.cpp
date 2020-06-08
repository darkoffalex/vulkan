#include "LightSource.h"

namespace vk
{
    namespace scene
    {
        /**
         * Событие смены положения
         * @param updateMatrices Запрос обновить матрицы
         */
        void LightSource::onPlacementUpdated(bool updateMatrices)
        {
            // Используем флаг "обновлять матрицы" как флаг сигнала о том что нужно обновить вектор ориентации
            if(updateMatrices){
                orientationVector_ = glm::vec3(this->makeRotationMatrixQuaternion() * glm::vec4(0.0f,0.0f,-1.0f,0.0f));

                // Обновить регион в UBO буфере
                this->updateUboRegion();
            }
        }

        /**
         * Обновить в UBO соответствующую источнику область
         */
        void LightSource::updateUboRegion()
        {
            if(pUboData_ != nullptr)
            {
                // Косинусы углов отсечения (для источников типа spot)
                auto cutOffAngleCos = glm::cos(glm::radians(this->cutOffAngle_));
                auto cutOffOuterAngleCos = glm::cos(glm::radians(this->cutOffOuterAngle_));

                // Побайтовый сдвиг в массиве источников света
                size_t elementByteOffset = uboOffset_ * LIGHT_ENTRY_SIZE;

                // Копирование в буфер (с учетом выравнивания std140)
                memcpy(pUboData_ + elementByteOffset + 0, &position_, 12);
                memcpy(pUboData_ + elementByteOffset + 12 , &radius_, 4);
                memcpy(pUboData_ + elementByteOffset + 16 , &color_, 12);
                memcpy(pUboData_ + elementByteOffset + 32, &orientationVector_, 12);
                memcpy(pUboData_ + elementByteOffset + 44, &attenuationQuadratic_, 4);
                memcpy(pUboData_ + elementByteOffset + 48, &attenuationLinear_, 4);
                memcpy(pUboData_ + elementByteOffset + 52, &cutOffAngleCos, 4);
                memcpy(pUboData_ + elementByteOffset + 56, &cutOffOuterAngleCos, 4);
                memcpy(pUboData_ + elementByteOffset + 60, &type_, 4);
            }
        }

        /**
         * Конструктор по умолчанию
         */
        LightSource::LightSource():SceneElement({0.0f,0.0f,0.0f}),uboOffset_(0),pUboData_(nullptr){}

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
        LightSource::LightSource(unsigned char* pUboData, size_t uboOffset, const LightSourceType &type,
                                 const glm::vec3 &position, const glm::vec3 &color, glm::float32 attenuationLinear,
                                 glm::float32 attenuationQuadratic, glm::float32 cutOffAngle,
                                 glm::float32 cutOffOuterAngle):SceneElement(position),
                pUboData_(pUboData),
                uboOffset_(uboOffset),
                type_(type),
                color_(color),
                attenuationLinear_(attenuationLinear),
                attenuationQuadratic_(attenuationQuadratic),
                cutOffAngle_(cutOffAngle),
                cutOffOuterAngle_(cutOffOuterAngle)
        {
            // Обновляем соответствующий объекту регион в UBO буфере
            this->updateUboRegion();
        }

        /**
         * Перемещение через присваивание
         * @param other R-value ссылка на другой объект
         * @return Ссылка на текущий объект
         */
        LightSource::LightSource(LightSource &&other) noexcept :LightSource()
        {
            std::swap(uboOffset_,other.uboOffset_);
            std::swap(pUboData_,other.pUboData_);
            std::swap(type_ ,other.type_ );
            std::swap(radius_,other.radius_);
            std::swap(color_,other.color_);
            std::swap(attenuationLinear_,other.attenuationLinear_);
            std::swap(attenuationQuadratic_,other.attenuationQuadratic_);
            std::swap(cutOffAngle_,other.cutOffAngle_);
            std::swap(cutOffOuterAngle_,other.cutOffOuterAngle_);
        }

        /**
         * Перемещение через присваивание
         * @param other R-value ссылка на другой объект
         * @return Ссылка на текущий объект
         */
        LightSource &LightSource::operator=(LightSource &&other) noexcept
        {
            if (this == &other) return *this;

            pUboData_ = nullptr;
            uboOffset_ = 0;

            std::swap(uboOffset_,other.uboOffset_);
            std::swap(pUboData_,other.pUboData_);
            std::swap(type_ ,other.type_ );
            std::swap(radius_,other.radius_);
            std::swap(color_,other.color_);
            std::swap(attenuationLinear_,other.attenuationLinear_);
            std::swap(attenuationQuadratic_,other.attenuationQuadratic_);
            std::swap(cutOffAngle_,other.cutOffAngle_);
            std::swap(cutOffOuterAngle_,other.cutOffOuterAngle_);

            return *this;
        }

        /**
         * Установить тип источника
         * @param type Тип источника
         * @param updateUbo Обновить UBO буфер источников
         */
        void LightSource::setType(LightSourceType &type, bool updateUbo)
        {
            type_ = type;
            if(updateUbo) this->updateUboRegion();
        }

        /**
         * Получить тип источника
         * @return Тип источника
         */
        LightSourceType LightSource::getType() const
        {
            return type_;
        }

        /**
         * Установить радиус
         * @param radius Число с плавающей точкой
         * @param updateUbo Обновить UBO буфер источников
         */
        void LightSource::setRadius(glm::float32 radius, bool updateUbo)
        {
            radius_ = radius;
            if(updateUbo) this->updateUboRegion();
        }

        /**
         * Получить радиус
         * @return Число с плавающей точкой
         */
        glm::float32 LightSource::getRadius() const
        {
            return radius_;
        }

        /**
         * Установить цвет
         * @param color Вектор цвета
         * @param updateUbo Обновить UBO буфер источников
         */
        void LightSource::setColor(const glm::vec3 &color, bool updateUbo)
        {
            color_ = color;
            if(updateUbo) this->updateUboRegion();
        }

        /**
         * Получить цвет
         * @return Вектор цвета
         */
        glm::vec3 LightSource::getColor() const
        {
            return color_;
        }

        /**
         * Установить коэффициент линейного затухания
         * @param attenuationLinear Число с плавающей точкой
         * @param updateUbo Обновить UBO буфер источников
         */
        void LightSource::setAttenuationLinear(glm::float32 attenuationLinear, bool updateUbo)
        {
            attenuationLinear_ = attenuationLinear;
            if(updateUbo) this->updateUboRegion();
        }

        /**
         * Получить коэффициент линейного затухания
         * @return Число с плавающей точкой
         */
        glm::float32 LightSource::getAttenuationLinear() const
        {
            return attenuationLinear_;
        }

        /**
         * Установить коэффициент квадратичного затухания
         * @param attenuationQuadratic Число с плавающей точкой
         * @param updateUbo Обновить UBO буфер источников
         */
        void LightSource::setAttenuationQuadratic(glm::float32 attenuationQuadratic, bool updateUbo)
        {
            attenuationQuadratic_ = attenuationQuadratic;
            if(updateUbo) this->updateUboRegion();
        }

        /**
         * Получить коэффициент квадратичного затухания
         * @return Число с плавающей точкой
         */
        glm::float32 LightSource::getAttenuationQuadratic() const
        {
            return attenuationQuadratic_;
        }

        /**
         * Установить угол (внутренний) отсечения для источника типа spot-light
         * @param cutOffAngle Число с плавающей точкой
         * @param updateUbo Обновить UBO буфер источников
         */
        void LightSource::setCutOffAngle(glm::float32 cutOffAngle, bool updateUbo)
        {
            cutOffAngle_ = cutOffAngle;
            if(updateUbo) this->updateUboRegion();
        }

        /**
         * Получить угол (внутренний) отсечения для источника типа spot-light
         * @return Число с плавающей точкой
         */
        glm::float32 LightSource::getCutOffAngle() const
        {
            return cutOffAngle_;
        }

        /**
         * Установить угол (внешний) отсечения для источника типа spot-light
         * @param cutOffOuterAngle Число с плавающей точкой
         * @param updateUbo Обновить UBO буфер источников
         */
        void LightSource::setCutOffOuterAngle(glm::float32 cutOffOuterAngle, bool updateUbo)
        {
            cutOffOuterAngle_ = cutOffOuterAngle;
            if(updateUbo) this->updateUboRegion();
        }

        /**
         * Получить угол (внешний) отсечения для источника типа spot-light
         * @return Число с плавающей точкой
         */
        glm::float32 LightSource::getCutOffOuterAngle() const
        {
            return cutOffOuterAngle_;
        }
    }
}