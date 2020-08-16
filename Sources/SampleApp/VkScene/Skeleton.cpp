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
                bones_(1),
                updateCallback_(nullptr)
        {
            // Создать корневую кость
            rootBone_ = std::make_shared<SkeletonBone>(this,0,nullptr,glm::mat4(1),glm::mat4(1));
            // Добавить указатель на корневую кость в линейный массив
            bones_.push_back(rootBone_);
        }

        /**
         * Основной конструктор
         * @param boneTotalCount Общее количество костей
         * @param updateCallback Функция обратного вызова при пересчете матриц
         */
        Skeleton::Skeleton(size_t boneTotalCount, const std::function<void()> &updateCallback)
        {
            // Изначально у скелета есть как минимум 1 кость
            modelSpaceFinalTransforms_.resize(std::max<size_t>(1,boneTotalCount));
            boneSpaceFinalTransforms_.resize(std::max<size_t>(1,boneTotalCount));
            bones_.resize(std::max<size_t>(1,boneTotalCount));

            // Создать корневую кость
            rootBone_ = std::make_shared<SkeletonBone>(this,0,nullptr,glm::mat4(1),glm::mat4(1));
            // Добавить указатель на корневую кость в линейный массив
            bones_.push_back(rootBone_);
            // Установка callBack функции при обновлении матриц
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
        size_t Skeleton::getBonesCount() const {
            return modelSpaceFinalTransforms_.size();
        }

        /**
         * Получить размер массива трансформаций в байтах
         * @return Целое положительное число
         */
        size_t Skeleton::getTransformsDataSize() const {
            return sizeof(glm::mat4) * this->modelSpaceFinalTransforms_.size();
        }

        /**
         * Получить линейный массив костей
         * @return Массив указателей на кости
         */
        const std::vector<SkeletonBonePtr> &Skeleton::getBones() {
            return bones_;
        }

        /**
         * Получить указатель на кость по индексу
         * @param index Индекс
         * @return Smart-pointer кости
         */
        SkeletonBonePtr Skeleton::getBoneByIndex(size_t index) {
            return bones_[index];
        }
    }
}