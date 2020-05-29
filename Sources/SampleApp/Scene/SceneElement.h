/**
 * Базовый класс пространственного (мерного) объекта - прародитель всех объектов сцены
 * Copyright (C) 2020 by Alex "DarkWolf" Nem - https://github.com/darkoffalex
 */

#pragma once

#include "../VkTools.h"
#include <glm/glm.hpp>

namespace vk
{
    namespace scene
    {
        class SceneElement
        {
        private:
            /// Матрица модели (координаты объекта с точки зрения мира)
            glm::mat4 modelMatrix_ = glm::mat4(1);
            /// Матрица вида (координаты мира с точки зрения объекта)
            glm::mat4 viewMatrix_ = glm::mat4(1);

            /// Абсолютное положение в пространстве
            glm::vec3 position_;
            /// Ориентация в пространстве
            glm::vec3 orientation_;
            /// Масштаб
            glm::vec3 scale_;
            /// Начальная точка (локальный центр)
            glm::vec3 origin_;

            /**
             * Построить матрицу смещения
             * @return Матрица 4*4
             */
            [[nodiscard]] glm::mat4 makeTranslationMatrix() const;

            /**
             * Построить матрицу поворота (углы Эйлера)
             * @param r0 Порядок вращения - первая ось
             * @param r1 Порядок вращения - вторая ось
             * @param r2 Порядок вращения - третья ось
             * @return Матрица 4*4
             */
            [[nodiscard]] glm::mat4 makeRotationMatrix(const tools::Axis& r0, const tools::Axis& r1, const tools::Axis& r2) const;

            /**
             * Построить матрицу поворота (кватернионы)
             * @return Матрица 4*4
             */
            [[nodiscard]] glm::mat4 makeRotationMatrixQuaternion() const;

            /**
             * Получить матрицу масштабирования
             * @return Матрица 4*4
             */
            [[nodiscard]] glm::mat4 makeScaleMatrix() const;

            /**
             * Обновление матрицы модели
             */
            void updateModelMatrix();

            /**
             * Обновление матрицы вида
             */
            void updateViewMatrix();

            /**
             * Вызывается после того капк матрицы были обновлены
             *
             * @details Полностью виртуальный метод, должен быть переопределен в классе-потомке. Можно использовать
             * для обновления буферов UBO и аналогичных целей
             */
            virtual void onMatricesUpdated() = 0;

        public:

            /**
             * Конструктор по умолчанию
             */
            SceneElement();

            /**
             * Явный конструктор объекта
             * @param position Положение в пространстве
             * @param orientation Ориентация по осям
             * @param scale Масштабирование объекта
             * @param origin Локальный центр
             */
            explicit SceneElement(const glm::vec3& position,
                                  const glm::vec3& orientation = { 0.0f, 0.0f, 0.0f },
                                  const glm::vec3& scale = {1.0f, 1.0f, 1.0f },
                                  const glm::vec3& origin = {0.0f, 0.0f, 0.0f});

            /**
             * Уничтожение объекта
             */
            virtual ~SceneElement() = default;

            /**
             * Получить ссылку на матрицу модели
             * @return Константная ссылка на матрицу 4*4
             */
            [[nodiscard]] const glm::mat4& getModelMatrix() const;

            /**
             * Получить ссылку на матрицу вида
             * @return Константная ссылка на матрицу 4*4
             */
            [[nodiscard]] const glm::mat4& getViewMatrix() const;

            /**
             * Установить положение
             * @param position Тчока в пространстве
             * @param updateMatrices Обновление матриц
             */
            void setPosition(const glm::vec3& position, bool updateMatrices = true);

            /**
             * Получить положение
             * @return Тчока в пространстве
             */
            [[nodiscard]] const glm::vec3& getPosition() const;

            /**
             * Установить ориентацию
             * @param orientation Углы поворота вокруг осей
             * @param updateMatrices Обновление матриц
             */
            void setOrientation(const glm::vec3& orientation, bool updateMatrices = true);

            /**
             * Получить ориентацию
             * @return Углы поворота вокруг осей
             */
            [[nodiscard]] const glm::vec3& getOrientation() const;

            /**
             * Установить масштабирование
             * @param scale Вектор масштабирования
             * @param updateMatrices Обновление матриц
             */
            void setScale(const glm::vec3& scale, bool updateMatrices = true);

            /**
             * Получить масштаб
             * @return Вектор масштабирования
             */
            [[nodiscard]] const glm::vec3& getScale() const;

            /**
             * Установка локального центра
             * @param origin Точка локального центра
             * @param updateMatrices Обновление матриц
             */
            void setOrigin(const glm::vec3& origin, bool updateMatrices = true);

            /**
             * Получить локальный центр
             * @return Точка локального центра
             */
            [[nodiscard]] const glm::vec3& getOrigin() const;
        };
    }
}



