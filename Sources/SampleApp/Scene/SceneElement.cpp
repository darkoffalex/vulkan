/**
 * Базовый класс пространственного (мерного) объекта - прародитель всех объектов сцены
 * Copyright (C) 2020 by Alex "DarkWolf" Nem - https://github.com/darkoffalex
 */

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS

#include "SceneElement.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace vk
{
    namespace scene
    {
        /**
         * Конструктор объекта
         * @param position Положение в пространстве
         * @param orientation Ориентация по осям
         * @param scale Масштабирование объекта
         * @param origin Локальный центр
         */
        SceneElement::SceneElement(const glm::vec3 &position,
                                   const glm::vec3 &orientation,
                                   const glm::vec3 &scale,
                                   const glm::vec3 &origin):
                position_(position),
                orientation_(orientation),
                scale_(scale),
                origin_(origin)
        {
            this->updateModelMatrix();
            this->updateViewMatrix();
        }

        /**
         * Конструктор по умолчанию
         */
        SceneElement::SceneElement():
                position_({}),
                orientation_({}),
                scale_({1.0f,1.0f,1.0f}),
                origin_({})
        {
            this->updateModelMatrix();
            this->updateViewMatrix();
        }

        /**
         * Построить матрицу смещения
         * @return Матрица 4*4
         */
        glm::mat4 SceneElement::makeTranslationMatrix() const
        {
            glm::mat4 result(1);
            result = glm::translate(result, this->position_);
            return result;
        }

        /**
         * Построить матрицу поворота (углы Эйлера)
         * @param r0 Порядок вращения - первая ось
         * @param r1 Порядок вращения - вторая ось
         * @param r2 Порядок вращения - третья ось
         * @return Матрица 4*4
         */
        glm::mat4 SceneElement::makeRotationMatrix(const tools::Axis& r0, const tools::Axis& r1, const tools::Axis& r2) const
        {
            glm::float32 angles[3] = { glm::radians(this->orientation_.x), glm::radians(this->orientation_.y), glm::radians(this->orientation_.z) };
            glm::vec3 vectors[3] = { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) };

            glm::mat4 rot(1);
            rot = glm::rotate(rot, angles[r0], vectors[r0]);
            rot = glm::rotate(rot, angles[r1], vectors[r1]);
            rot = glm::rotate(rot, angles[r2], vectors[r2]);

            return rot;
        }

        /**
         * Построить матрицу поворота (кватернионы)
         * @return Матрица 4*4
         */
        glm::mat4 SceneElement::makeRotationMatrixQuaternion() const
        {
            const glm::quat rot = glm::quat(glm::vec3(
                    glm::radians(this->orientation_.x),
                    glm::radians(this->orientation_.y),
                    glm::radians(this->orientation_.z)));

            return glm::toMat4(rot);
        }

        /**
         * Получить матрицу масштабирования
         * @return Матрица 4*4
         */
        glm::mat4 SceneElement::makeScaleMatrix() const
        {
            glm::mat4 result(1);
            result = glm::scale(result, this->scale_);
            return result;
        }

        /**
         * Обновление матрицы модели
         */
        void SceneElement::updateModelMatrix()
        {
            this->modelMatrix_ =
                    // 5 - возвращаем на место (получив в итоге новый локальный центр)
                    glm::translate(glm::mat4(1), this->origin_) *
                    // 4 - сдвигаем (обычный сдвиг)
                    this->makeTranslationMatrix() *
                    // 3 - вращаем (вокруг начала координат)
                    this->makeRotationMatrixQuaternion() *
                    // 2 - масштабируем (относительно начала координат)
                    this->makeScaleMatrix() *
                    // 1 - сдвигаем в противоположном направлении от начала координат
                    glm::translate(glm::mat4(1), -this->origin_);

        }

        /**
         * Обновление матрицы вида
         */
        void SceneElement::updateViewMatrix()
        {
            //TODO: необходима оптимизация, считать обратную матрицу 4*4 всякий раз - пиздец
            const auto model =
                    // 4 - возвращаем на место (получив в итоге новый локальный центр)
                    glm::translate(glm::mat4(1), this->origin_) *
                    // 3 - сдвигаем (обычный сдвиг)
                    this->makeTranslationMatrix() *
                    // 2 - вращаем (вокруг начала координат)
                    this->makeRotationMatrixQuaternion() *
                    // 1 - сдвигаем в противоположном направлении от начала координат
                    glm::translate(glm::mat4(1), -this->origin_);

            this->viewMatrix_ = glm::inverse(model);
        }

        /**
         * Получить ссылку на матрицу модели
         * @return Константная ссылка на матрицу 4*4
         */
        const glm::mat4 &SceneElement::getModelMatrix() const
        {
            return modelMatrix_;
        }

        /**
         * Получить ссылку на матрицу вида
         * @return Константная ссылка на матрицу 4*4
         */
        const glm::mat4 &SceneElement::getViewMatrix() const
        {
            return viewMatrix_;
        }

        /**
         * Установить положение
         * @param position Точка в пространстве
         * @param updateMatrices Обновление матриц
         */
        void SceneElement::setPosition(const glm::vec3 &position, bool updateMatrices)
        {
            this->position_ = position;

            if(updateMatrices) {
                this->updateModelMatrix();
                this->updateViewMatrix();
                this->onMatricesUpdated();
            }
        }

        /**
         * Получить положение
         * @return очка в пространстве
         */
        const glm::vec3 &SceneElement::getPosition() const
        {
            return position_;
        }

        /**
         * Установить ориентацию
         * @param orientation Углы поворота вокруг осей
         * @param updateMatrices Обновление матриц
         */
        void SceneElement::setOrientation(const glm::vec3 &orientation, bool updateMatrices)
        {
            this->orientation_ = orientation;

            if(updateMatrices) {
                this->updateModelMatrix();
                this->updateViewMatrix();
                this->onMatricesUpdated();
            }
        }

        /**
         * Получить ориентацию
         * @return Углы поворота вокруг осей
         */
        const glm::vec3 &SceneElement::getOrientation() const
        {
            return orientation_;
        }

        /**
         * Установить масштабирование
         * @param scale Вектор масштабирования
         * @param updateMatrices Обновление матриц
         */
        void SceneElement::setScale(const glm::vec3 &scale, bool updateMatrices)
        {
            this->scale_ = scale;

            if(updateMatrices) {
                this->updateModelMatrix();
                this->updateViewMatrix();
                this->onMatricesUpdated();
            }
        }

        /**
         * Получить масштаб
         * @return Вектор масштабирования
         */
        const glm::vec3 &SceneElement::getScale() const
        {
            return scale_;
        }

        /**
         * Установка локального цертра
         * @param origin Точка локального центра
         * @param updateMatrices Обновление матриц
         */
        void SceneElement::setOrigin(const glm::vec3 &origin, bool updateMatrices)
        {
            this->origin_ = origin;

            if(updateMatrices) {
                this->updateModelMatrix();
                this->updateViewMatrix();
                this->onMatricesUpdated();
            }
        }

        /**
         * Получить локальный центр
         * @return Точка локального центра
         */
        const glm::vec3 &SceneElement::getOrigin() const
        {
            return origin_;
        }
    }
}


