/**
 * Класс скелета. Используется для скелетной анимации одиночного меша
 * Copyright (C) 2020 by Alex "DarkWolf" Nem - https://github.com/darkoffalex
 */

#include "Skeleton.h"
#include <algorithm>

namespace vk
{
    namespace scene
    {
        /**
         * Установить функцию обратного вызова при пересчете матриц скелета
         * @param updateCallback Функция обратного вызова или лямбда-выражение
         */
        void scene::Skeleton::setUpdateCallback(const std::function<void()> &updateCallback){
            this->updateCallback_ = updateCallback;
        }

        /**
         * Конструктор по умолчанию
         * Изначально у скелета всегда есть одна кость
         */
        Skeleton::Skeleton():
                modelSpaceFinalTransforms_(1),
                boneSpaceFinalTransforms_(1),
                updateCallback_(nullptr)
        {
            rootBone_ = std::make_shared<SkeletonBone>(this,0,nullptr,glm::mat4(1),glm::mat4(1));
        }

        /**
         * Основной конструктор
         * @param boneTotalCount Общее количество костей
         * @param updateCallback Функция обратного вызова при пересчете матриц
         */
        Skeleton::Skeleton(size_t boneTotalCount, const std::function<void()> &updateCallback) {
            modelSpaceFinalTransforms_.resize(std::max<size_t>(1,boneTotalCount));
            boneSpaceFinalTransforms_.resize(std::max<size_t>(1,boneTotalCount));
            rootBone_ = std::make_shared<SkeletonBone>(this,0,nullptr,glm::mat4(1),glm::mat4(1));
            updateCallback_ = updateCallback;
        }

        /**
         * Получить корневую кость
         * @return Указатель на объект кости
         */
        SkeletonBonePtr Skeleton::getRootBone()
        {
            return rootBone_;
        }


        /**
         * Получить массив итоговых трансформаций костей
         * @param fromBoneSpace Если точки заданы в пространстве кости
         * @return Ссылка на массив матриц
         */
        const std::vector<glm::mat4> &Skeleton::getFinalBoneTransforms(bool fromBoneSpace) const {
            return fromBoneSpace ? boneSpaceFinalTransforms_ : modelSpaceFinalTransforms_;
        }

        /**
         * Получить общее кол-во матриц
         * @return Целое положительное число
         */
        size_t Skeleton::getTotalBones() const {
            return modelSpaceFinalTransforms_.size();
        }

        /**
         * Получить размер массива трансформаций в байтах
         * @return Целое положительное число
         */
        size_t Skeleton::getTransformsDataSize() const {
            return sizeof(glm::mat4) * this->modelSpaceFinalTransforms_.size();
        }
    }
}