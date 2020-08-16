/**
 * Класс одиночной кости скелета. Используется для скелетной анимации одиночного меша
 * Copyright (C) 2020 by Alex "DarkWolf" Nem - https://github.com/darkoffalex
 */

#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <functional>
#include <memory>

namespace vk
{
    namespace scene
    {
        class Skeleton;

        class SkeletonBone
        {
        public:
            /// Флаги вычисления матриц
            enum CalcFlags
            {
                eNone = (0),
                eFullTransform = (1u << 0u),
                eBindTransform = (1u << 1u),
                eInverseBindTransform = (1u << 2u),
            };

        private:
            friend class Mesh;
            friend class Skeleton;

            /// Указатель на скелет
            Skeleton* pSkeleton_;
            /// Индекс кости в линейном массиве
            size_t index_;

            /// Указатель на родительскую кость
            SkeletonBone* pParentBone_;
            /// Массив указателей на дочерние кости
            std::vector<std::shared_ptr<SkeletonBone>> childrenBones_;

            /// Смещение (расположение) относительно родительской кости (можно считать это initial-положением)
            glm::mat4 localBindTransform_;
            /// Локальная трансформация относительно bind (та трансформация, которая может назначаться во время анимации)
            glm::mat4 localTransform_;

            /// Результирующая трансформация кости с учетом локальной трансформации и результирующий трансформаций родительских костей
            /// Данная трансформация может быть применена к точкам находящимся В ПРОСТРАНСТВЕ КОСТИ
            glm::mat4 totalTransform_;
            /// Результирующая трансформация кости БЕЗ учета задаваемой, но с учетом bind-трансформаций родительских костей
            glm::mat4 totalBindTransform_;
            /// Инвертированная bind матрица может быть использована для перехода в пространство кости ИЗ ПРОСТРАНСТВА МОДЕЛИ
            glm::mat4 totalBindTransformInverse_;

            /**
             * Рекурсивное вычисление матриц для текущей кости и всех дочерних её костей
             * @param callUpdateCallbackFunction Вызывать функцию обратного вызова установленную к скелета
             * @param calcFlags Опции вычисления матриц (какие матрицы считать)
             */
            void calculateBranch(bool callUpdateCallbackFunction = true, unsigned calcFlags = CalcFlags::eFullTransform | CalcFlags::eBindTransform | CalcFlags::eInverseBindTransform);

        public:
            /**
             * Конструктор по умолчанию
             */
            SkeletonBone();

            /**
             * Основной конструктор кости
             * @param pSkeleton Указатель на объект скелета
             * @param index Индекс кости в линейном массиве трансформаций
             * @param parentBone Указатель на родительскую кость
             * @param localBindTransform Смещение (расположение) относительно родительской кости
             * @param localTransform  Локальная трансформация (та трансформация, которая может назначаться, например "поворот руки на n градусов")
             */
            explicit SkeletonBone(Skeleton* pSkeleton,
                                  size_t index, SkeletonBone* parentBone = nullptr,
                                  const glm::mat4& localBindTransform = glm::mat4(1.0f),
                                  const glm::mat4& localTransform = glm::mat4(1.0f));

            /**
             * Деструктор по умолчанию
             */
            ~SkeletonBone() = default;

            /**
             * Добавление дочерней кости
             * @param index Индекс
             * @param localBindTransform Изначальная трансформация
             * @param localTransform Задаваемая трансформация
             * @return Указатель на добавленную кость
             */
            std::shared_ptr<SkeletonBone> addChildBone(size_t index,
                               const glm::mat4& localBindTransform,
                               const glm::mat4& localTransform = glm::mat4(1.0f));

            /**
             * Установить локальную (анимируемую) трансформацию
             * @param transform Матрица 4*4
             * @param recalculateBranch Пересчитать ветвь
             */
            void setLocalTransform(const glm::mat4& transform, bool recalculateBranch = true);

            /**
             * Установить изначальную (initial) трансформацию кости относительно родителя
             * @param transform Матрица 4*4
             * @param recalculateBranch Пересчитать ветвь
             */
            void setLocalBindTransform(const glm::mat4& transform, bool recalculateBranch = true);

            /**
             * Установить изначальную (initial, bind) и добавочную (animated) трансформацию
             * @param localBind Матрица 4*4
             * @param local Матрица 4*4
             * @param recalculateBranch Пересчитать ветвь
             */
            void setTransformations(const glm::mat4& localBind, const glm::mat4& local, bool recalculateBranch = true);

            /**
             * Получить массив дочерних костей
             * @return Ссылка на массив указателей
             */
            std::vector<std::shared_ptr<SkeletonBone>>& getChildrenBones();

            /**
             * Получить указатель на родительскую кость
             * @return Указатель
             */
            SkeletonBone* getParentBone();

            /**
             * Получить индекс кости
             * @return Целое положительное число
             */
            size_t getIndex();
        };

        /**
         * Smart-pointer объекта скелетной кости
         */
        typedef std::shared_ptr<SkeletonBone> SkeletonBonePtr;
    }
}


