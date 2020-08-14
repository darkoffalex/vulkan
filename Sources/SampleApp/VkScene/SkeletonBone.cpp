/**
 * Класс одиночной кости скелета. Используется для скелетной анимации одиночного меша
 * Copyright (C) 2020 by Alex "DarkWolf" Nem - https://github.com/darkoffalex
 */

#include "SkeletonBone.h"
#include "Skeleton.h"

namespace vk
{
    namespace scene
    {
        /**
         * Рекурсивное вычисление матриц для текущей кости и всех дочерних её костей
         * @param callUpdateCallbackFunction Вызывать функцию обновления UBO буфера у скелета
         */
        void SkeletonBone::calculateBranch(bool callUpdateCallbackFunction)
        {
            // Если у кости есть родительская кость
            if(pParentBone_ != nullptr)
            {
                // Общая initial (bind) трансформация для кости учитывает текущую и родительскую (что в свою очередь справедливо и для родительской)
                totalBindTransform_ = pParentBone_->totalBindTransform_ * this->localBindTransform_;
                // Общая полная (с учетом задаваемой) трансформация кости (смещаем на localTransform_, затем на initial, затем на общую родительскую трансформацию)
                totalTransform_ = pParentBone_->totalTransform_ * this->localBindTransform_ * this->localTransform_;
            }
            // Если нет родительской кости - считать кость корневой
            else
            {
                totalBindTransform_ = this->localBindTransform_;
                totalTransform_ = this->localBindTransform_ * this->localTransform_;
            }

            // Если есть указатель на объект скелета и индекс валиден
            if(pSkeleton_ != nullptr && index_ < pSkeleton_->modelSpaceFinalTransforms_.size())
            {
                // Итоговая матрица трансформации для точек находящихся в пространстве модели
                // Поскольку общая трансформация кости работает с вершинами находящимися в пространстве модели,
                // они в начале должны быть переведены в пространство кости.
                pSkeleton_->modelSpaceFinalTransforms_[index_] = totalTransform_ * glm::inverse(totalBindTransform_);

                // Для ситуаций, если вершины задаются сразу в пространстве кости
                pSkeleton_->boneSpaceFinalTransforms_[index_] = totalTransform_;
            }

            // Рекурсивно выполнить для дочерних элементов (если они есть)
            if(!this->childrenBones_.empty()){
                for(auto& childBone : this->childrenBones_){
                    childBone->calculateBranch(false);
                }
            }

            // Если нужно вызвать функцию обновления UBO
            if(callUpdateCallbackFunction && this->pSkeleton_ != nullptr && this->pSkeleton_->updateCallback_ != nullptr){
                this->pSkeleton_->updateCallback_();
            }
        }

        /**
         * Конструктор по умолчанию
         */
        SkeletonBone::SkeletonBone():
                pSkeleton_(nullptr),
                index_(0),
                pParentBone_(nullptr),
                localBindTransform_(glm::mat4(1.0f)),
                localTransform_(glm::mat4(1.0f)),
                totalTransform_(glm::mat4(1.0f)),
                totalBindTransform_(glm::mat4(1.0f)){}

        /**
         * Основной конструктор кости
         * @param pSkeleton Указатель на объект скелета
         * @param index Индекс кости в линейном массиве трансформаций
         * @param parentBone Указатель на родительскую кость
         * @param localBindTransform Смещение (расположение) относительно родительской кости
         * @param localTransform  Локальная трансформация (та трансформация, которая может назначаться, например "поворот руки на n градусов")
         */
        SkeletonBone::SkeletonBone(Skeleton *pSkeleton, size_t index, SkeletonBone *parentBone,
                                   const glm::mat4 &localBindTransform, const glm::mat4 &localTransform):
                index_(index),
                pSkeleton_(pSkeleton),
                pParentBone_(parentBone),
                localBindTransform_(localBindTransform),
                localTransform_(localTransform),
                totalTransform_(glm::mat4(1.0f)),
                totalBindTransform_(glm::mat4(1.0f))
        {
            // Вычисление матриц кости
            calculateBranch();
        }

        /**
         * Добавление дочерней кости
         * @param index Индекс
         * @param localBindTransform Изначальная трансформация
         * @param localTransform Задаваемая трансформация
         * @return Указатель на добавленную кость
         */
        std::shared_ptr<SkeletonBone> SkeletonBone::addChildBone(size_t index,
                const glm::mat4 &localBindTransform,
                const glm::mat4 &localTransform)
        {
            std::shared_ptr<SkeletonBone> child(new SkeletonBone(this->pSkeleton_,index,this,localBindTransform,localTransform));
            this->childrenBones_.push_back(child);
            return child;
        }

        /**
         * Установить локальную (анимируемую) трансформацию
         * @param transform Матрица 4*4
         * @param recalculateBranch Пересчитать ветвь
         */
        void SkeletonBone::setLocalTransform(const glm::mat4 &transform, bool recalculateBranch)
        {
            this->localTransform_ = transform;
            if(recalculateBranch) this->calculateBranch(true);
        }

        /**
         * Установить изначальную (initial) трансформацию кости относительно родителя
         * @param transform Матрица 4*4
         * @param recalculateBranch Пересчитать ветвь
         */
        void SkeletonBone::setLocalBindTransform(const glm::mat4 &transform, bool recalculateBranch)
        {
            this->localBindTransform_ = transform;
            if(recalculateBranch) this->calculateBranch(true);
        }

        /**
         * Установить изначальную (initial, bind) и добавочную (animated) трансформацию
         * @param localBind Матрица 4*4
         * @param local Матрица 4*4
         * @param recalculateBranch Пересчитать ветвь
         */
        void SkeletonBone::setTransformations(const glm::mat4 &localBind, const glm::mat4 &local, bool recalculateBranch)
        {
            this->localBindTransform_ = localBind;
            this->localTransform_ = local;
            if(recalculateBranch) this->calculateBranch(true);
        }

        /**
         * Получить массив дочерних костей
         * @return Ссылка на массив указателей
         */
        std::vector<std::shared_ptr<SkeletonBone>> &SkeletonBone::getChildrenBones() {
            return this->childrenBones_;
        }

        /**
         * Получить указатель на родительскую кость
         * @return Указатель
         */
        SkeletonBone* SkeletonBone::getParentBone() {
            return this->pParentBone_;
        }

        /**
         * Получить индекс кости
         * @return Целое положительное число
         */
        size_t SkeletonBone::getIndex() {
            return index_;
        }
    }
}