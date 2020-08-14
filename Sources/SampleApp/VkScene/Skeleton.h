/**
 * Класс скелета. Используется для скелетной анимации одиночного меша
 * Copyright (C) 2020 by Alex "DarkWolf" Nem - https://github.com/darkoffalex
 */

#pragma once

#include "SkeletonBone.h"

namespace vk
{
    namespace scene
    {
        class Skeleton
        {
        private:
            /// Открыть доступ для классов кости и меша
            friend class SkeletonBone;
            friend class Mesh;

            /// Массив итоговых трансформаций для вершин в пространстве модели
            std::vector<glm::mat4> modelSpaceFinalTransforms_;

            /// Массив итоговых трансформаций для вершин в пространстве костей
            std::vector<glm::mat4> boneSpaceFinalTransforms_;

            /// Корневая кость
            SkeletonBonePtr rootBone_;

            /// Функция обратного вызова при пересчете матриц
            std::function<void()> updateCallback_;

            /**
             * Установить функцию обратного вызова при пересчете матриц скелета
             * @param updateCallback Функция обратного вызова или лямбда-выражение
             */
            void setUpdateCallback(const std::function<void()>& updateCallback);

        public:
            /**
             * Конструктор по умолчанию
             * Изначально у скелета всегда есть одна кость
             */
            Skeleton();

            /**
             * Основной конструктор
             * @param boneTotalCount Общее количество костей
             * @param updateCallback Функция обратного вызова при пересчете матриц
             */
            explicit Skeleton(size_t boneTotalCount, const std::function<void()>& updateCallback = nullptr);

            /**
             * Деструктор по умолчанию
             */
            ~Skeleton() = default;

            /**
             * Получить корневую кость
             * @return Указатель на объект кости
             */
            SkeletonBonePtr getRootBone();

            /**
             * Получить массив итоговых трансформаций костей
             * @param fromBoneSpace Если точки заданы в пространстве кости
             * @return Ссылка на массив матриц
             */
            const std::vector<glm::mat4>& getFinalBoneTransforms(bool fromBoneSpace = false) const;

            /**
             * Получить общее кол-во матриц
             * @return Целое положительное число
             */
            size_t getTotalBones() const;

            /**
             * Получить размер массива трансформаций в байтах
             * @return Целое положительное число
             */
            size_t getTransformsDataSize() const;
        };

        /**
         * Smart-pointer объекта скелета
         */
        typedef std::shared_ptr<Skeleton> SkeletonPtr;

        /**
         * Smart-unique-pointer объекта скелета
         */
        typedef std::unique_ptr<Skeleton> UniqueSkeleton;
    }
}