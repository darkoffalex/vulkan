#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace tools
{
    class Camera
    {
    private:
        /// Скорость движения (локальная)
        glm::vec3 translationVectorRelative_ = { 0.0f,0.0f,0.0f };
        /// Скорость движения (абсолютная)
        glm::vec3 translationVectorAbsolute_ = { 0.0f,0.0f,0.0f };
        /// Скорость вращения
        glm::vec3 rotation_ = { 0.0f,0.0f,0.0f };

    public:
        /// Абсолютное положение в пространстве
        glm::vec3 position = { 0.0f,0.0f,0.0f };
        /// Ориентация
        glm::vec3 orientation = { 0.0f,0.0f,0.0f };

        /**
         * Задать вектор скорости движения
         * @param localTranslationVector Локальный вектор скорости
         */
        void setTranslation(const glm::vec3& localTranslationVector)
        {
            translationVectorRelative_ = localTranslationVector;
        }

        /**
         * Задать абсолютный вектор скорости движения
         * @param absoluteTranslationVector Абсолютный вектор скорости
         */
        void setTranslationAbsolute(const glm::vec3& absoluteTranslationVector)
        {
            translationVectorAbsolute_ = absoluteTranslationVector;
        }

        /**
         * Задать вращение
         * @param rotationSpeed Вектор угловой скорости
         */
        void setRotation(const glm::vec3& rotationSpeed)
        {
            rotation_ = rotationSpeed;
        }

        /**
         * Движение - прирастить положение и ориентацию с учетом пройденного времени
         * @param deltaTime Время кадра
         */
        void translate(const float& deltaTime)
        {
            // Прирастить абсолютную скорость
            this->position += this->translationVectorAbsolute_ * deltaTime;
            // Прирастить вращение
            this->orientation += this->rotation_ * deltaTime;

            // Вычислить абсолютный вектор скорости учитывая поворот объекта
            const glm::quat rot = glm::quat(glm::vec3(glm::radians(this->orientation.x), glm::radians(this->orientation.y), glm::radians(this->orientation.z)));
            const glm::vec4 absVector = rot * glm::vec4(this->translationVectorRelative_.x, this->translationVectorRelative_.y, this->translationVectorRelative_.z, 0.0f);

            // Прирастить к позиции
            this->position += glm::vec3(absVector.x, absVector.y, absVector.z) * deltaTime;
        }
    };
}

