/**
 * Класс скелета. Используется для скелетной анимации одиночного меша
 * Copyright (C) 2020 by Alex "DarkWolf" Nem - https://github.com/darkoffalex
 */

#pragma once

#include "MeshSkeletonAnimation.hpp"

#include <vector>
#include <functional>
#include <memory>

#include <glm/glm.hpp>

namespace vk
{
    namespace scene
    {
        /**
         * Класс скелета
         */
        class MeshSkeleton
        {
        public:

            /**
             * Класс кости скелета (внутренний класс скелета)
             */
            class Bone
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
                /// Открыть доступ для дружественных классов
                friend class Mesh;
                friend class MeshSkeleton;

                /// Указатель на скелет
                MeshSkeleton* pSkeleton_;
                /// Индекс кости в линейном массиве
                size_t index_;

                /// Указатель на родительскую кость
                MeshSkeleton::Bone* pParentBone_;
                /// Массив указателей на дочерние кости
                std::vector<std::shared_ptr<MeshSkeleton::Bone>> childrenBones_;

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
                void calculateBranch(bool callUpdateCallbackFunction = true, unsigned calcFlags = CalcFlags::eFullTransform | CalcFlags::eBindTransform | CalcFlags::eInverseBindTransform)
                {
                    // Если у кости есть родительская кость
                    if(pParentBone_ != nullptr)
                    {
                        // Общая initial (bind) трансформация для кости учитывает текущую и родительскую (что в свою очередь справедливо и для родительской)
                        if(calcFlags & CalcFlags::eBindTransform)
                            totalBindTransform_ = pParentBone_->totalBindTransform_ * this->localBindTransform_;

                        // Общая полная (с учетом задаваемой) трансформация кости (смещаем на localTransform_, затем на initial, затем на общую родительскую трансформацию)
                        if(calcFlags & CalcFlags::eFullTransform)
                            totalTransform_ = pParentBone_->totalTransform_ * this->localBindTransform_ * this->localTransform_;
                    }
                    // Если нет родительской кости - считать кость корневой
                    else
                    {
                        if(calcFlags & CalcFlags::eBindTransform)
                            totalBindTransform_ = this->localBindTransform_;

                        if(calcFlags & CalcFlags::eFullTransform)
                            totalTransform_ = this->localBindTransform_ * this->localTransform_;
                    }

                    // Инвертированная матрица bind трансформации
                    if(calcFlags & CalcFlags::eInverseBindTransform)
                        totalBindTransformInverse_ = glm::inverse(totalBindTransform_);

                    // Если есть указатель на объект скелета и индекс корректный
                    if(pSkeleton_ != nullptr && index_ < pSkeleton_->modelSpaceFinalTransforms_.size())
                    {
                        // Итоговая матрица трансформации для точек находящихся в пространстве модели
                        // Поскольку общая трансформация кости работает с вершинами находящимися в пространстве модели,
                        // они в начале должны быть переведены в пространство кости.
                        pSkeleton_->modelSpaceFinalTransforms_[index_] = pSkeleton_->globalInverseTransform_ * totalTransform_ * totalBindTransformInverse_;

                        // Для ситуаций, если вершины задаются сразу в пространстве кости
                        pSkeleton_->boneSpaceFinalTransforms_[index_] = pSkeleton_->globalInverseTransform_ * totalTransform_;
                    }

                    // Рекурсивно выполнить для дочерних элементов (если они есть)
                    if(!this->childrenBones_.empty()){
                        for(auto& childBone : this->childrenBones_){
                            childBone->calculateBranch(false, calcFlags);
                        }
                    }

                    // Если нужно вызвать функцию обновления UBO
                    if(callUpdateCallbackFunction && this->pSkeleton_ != nullptr && this->pSkeleton_->updateCallback_ != nullptr){
                        this->pSkeleton_->updateCallback_();
                    }
                }

            public:
                /**
                 * Конструктор по умолчанию
                 */
                Bone():
                pSkeleton_(nullptr),
                index_(0),
                pParentBone_(nullptr),
                localBindTransform_(glm::mat4(1.0f)),
                localTransform_(glm::mat4(1.0f)),
                totalTransform_(glm::mat4(1.0f)),
                totalBindTransform_(glm::mat4(1.0f)),
                totalBindTransformInverse_(glm::mat4(1.0f)){}

                /**
                 * Основной конструктор кости
                 * @param pSkeleton Указатель на объект скелета
                 * @param index Индекс кости в линейном массиве трансформаций
                 * @param parentBone Указатель на родительскую кость
                 * @param localBindTransform Смещение (расположение) относительно родительской кости
                 * @param localTransform  Локальная трансформация (та трансформация, которая может назначаться, например "поворот руки на n градусов")
                 */
                explicit Bone(MeshSkeleton* pSkeleton,
                              size_t index, Bone* parentBone = nullptr,
                              const glm::mat4& localBindTransform = glm::mat4(1.0f),
                              const glm::mat4& localTransform = glm::mat4(1.0f)):
                        index_(index),
                        pSkeleton_(pSkeleton),
                        pParentBone_(parentBone),
                        localBindTransform_(localBindTransform),
                        localTransform_(localTransform),
                        totalTransform_(glm::mat4(1.0f)),
                        totalBindTransform_(glm::mat4(1.0f)),
                        totalBindTransformInverse_(glm::mat4(1.0f))
                {
                    // Вычисление матриц кости
                    calculateBranch(CalcFlags::eFullTransform|CalcFlags::eBindTransform|CalcFlags::eInverseBindTransform);
                }

                /**
                 * Деструктор по умолчанию
                 */
                ~Bone() = default;

                /**
                 * Добавление дочерней кости
                 * @param index Индекс
                 * @param localBindTransform Изначальная трансформация
                 * @param localTransform Задаваемая трансформация
                 * @return Указатель на добавленную кость
                 */
                std::shared_ptr<Bone> addChildBone(size_t index,
                        const glm::mat4& localBindTransform,
                        const glm::mat4& localTransform = glm::mat4(1.0f))
                {
                    // Создать дочернюю кость
                    std::shared_ptr<Bone> child(new Bone(this->pSkeleton_,index,this,localBindTransform,localTransform));
                    // Добавить в массив дочерних костей
                    this->childrenBones_.push_back(child);
                    // Добавить в общий линейный массив по указанному индексу
                    if(this->pSkeleton_ != nullptr) this->pSkeleton_->bones_[index] = child;
                    // Вернуть указатель
                    return child;
                }

                /**
                 * Установить локальную (анимируемую) трансформацию
                 * @param transform Матрица 4*4
                 * @param recalculateBranch Пересчитать ветвь
                 */
                void setLocalTransform(const glm::mat4& transform, bool recalculateBranch = true)
                {
                    this->localTransform_ = transform;
                    if(recalculateBranch) this->calculateBranch(true, CalcFlags::eFullTransform);
                }

                /**
                 * Установить изначальную (initial) трансформацию кости относительно родителя
                 * @param transform Матрица 4*4
                 * @param recalculateBranch Пересчитать ветвь
                 */
                void setLocalBindTransform(const glm::mat4& transform, bool recalculateBranch = true)
                {
                    this->localBindTransform_ = transform;
                    if(recalculateBranch) this->calculateBranch(true, CalcFlags::eBindTransform|CalcFlags::eInverseBindTransform);
                }

                /**
                 * Установить изначальную (initial, bind) и добавочную (animated) трансформацию
                 * @param localBind Матрица 4*4
                 * @param local Матрица 4*4
                 * @param recalculateBranch Пересчитать ветвь
                 */
                void setTransformations(const glm::mat4& localBind, const glm::mat4& local, bool recalculateBranch = true)
                {
                    this->localBindTransform_ = localBind;
                    this->localTransform_ = local;
                    if(recalculateBranch) this->calculateBranch(true, CalcFlags::eFullTransform|CalcFlags::eBindTransform|CalcFlags::eInverseBindTransform);
                }

                /**
                 * Получить массив дочерних костей
                 * @return Ссылка на массив указателей
                 */
                std::vector<std::shared_ptr<Bone>>& getChildrenBones()
                {
                    return this->childrenBones_;
                }

                /**
                 * Получить указатель на родительскую кость
                 * @return Указатель
                 */
                Bone* getParentBone()
                {
                    return this->pParentBone_;
                }

                /**
                 * Получить индекс кости
                 * @return Целое положительное число
                 */
                size_t getIndex()
                {
                    return index_;
                }
            };

            /**
             * Smart-pointer объекта скелетной кости
             */
            typedef std::shared_ptr<Bone> BonePtr;

            /**
             * Состояние анимации
             */
            enum struct AnimationState
            {
                eStopped,
                ePlaying
            };

        private:
            /// Открыть доступ для класса Mesh
            friend class Mesh;

            /// Массив итоговых трансформаций для вершин в пространстве модели
            std::vector<glm::mat4> modelSpaceFinalTransforms_;
            /// Массив итоговых трансформаций для вершин в пространстве костей
            std::vector<glm::mat4> boneSpaceFinalTransforms_;
            /// Матрица глобальной инверсии (на случай если в программе для моделирования объекту задавалась глобальная трансформация)
            glm::mat4 globalInverseTransform_;

            /// Массив указателей на кости для доступа по индексам
            std::vector<BonePtr> bones_;
            /// Корневая кость
            BonePtr rootBone_;

            /// Функция обратного вызова при пересчете матриц
            std::function<void()> updateCallback_;

            /// Текущая анимация скелета
            MeshSkeletonAnimationPtr currentAnimationPtr_;
            /// Скорость текущей анимации (сколько проходит миллисекунд анимации за миллисекунду времени рендерера)
            double currentAnimationSpeed_;
            /// Текущий момент анимации в миллисекундах
            double currentAnimationTime_;
            /// Состояние текущей анимации
            AnimationState currentAnimationState_;

            /**
             * Установить функцию обратного вызова при пересчете матриц скелета
             * @param updateCallback Функция обратного вызова или лямбда-выражение
             */
            void setUpdateCallback(const std::function<void()>& updateCallback)
            {
                this->updateCallback_ = updateCallback;
            }

        public:
            /**
             * Конструктор по умолчанию
             * Изначально у скелета всегда есть одна кость
             */
            MeshSkeleton():
                    modelSpaceFinalTransforms_(1),
                    boneSpaceFinalTransforms_(1),
                    globalInverseTransform_(glm::mat4(1.0f)),
                    bones_(1),
                    updateCallback_(nullptr),
                    currentAnimationPtr_(nullptr),
                    currentAnimationSpeed_(1.0f),
                    currentAnimationTime_(0.0),
                    currentAnimationState_(AnimationState::eStopped)
            {
                // Создать корневую кость
                rootBone_ = std::make_shared<Bone>(this,0,nullptr,glm::mat4(1),glm::mat4(1));
                // Нулевая кость в линейном массиве указателей
                bones_[0] = rootBone_;
            }

            /**
             * Основной конструктор
             * @param boneTotalCount Общее количество костей
             * @param updateCallback Функция обратного вызова при пересчете матриц
             */
            explicit MeshSkeleton(size_t boneTotalCount, const std::function<void()>& updateCallback = nullptr):
                    globalInverseTransform_(glm::mat4(1.0f)),
                    currentAnimationPtr_(nullptr),
                    currentAnimationSpeed_(1.0f),
                    currentAnimationTime_(0.0),
                    currentAnimationState_(AnimationState::eStopped)
            {
                // Изначально у скелета есть как минимум 1 кость
                modelSpaceFinalTransforms_.resize(std::max<size_t>(1,boneTotalCount));
                boneSpaceFinalTransforms_.resize(std::max<size_t>(1,boneTotalCount));
                bones_.resize(std::max<size_t>(1,boneTotalCount));

                // Создать корневую кость
                rootBone_ = std::make_shared<Bone>(this,0,nullptr,glm::mat4(1),glm::mat4(1));
                // Нулевая кость в линейном массиве указателей
                bones_[0] = rootBone_;
                // Установка callBack функции при обновлении матриц
                updateCallback_ = updateCallback;
            }

            /**
             * Деструктор по умолчанию
             */
            ~MeshSkeleton() = default;

            /**
             * Получить корневую кость
             * @return Указатель на объект кости
             */
            BonePtr getRootBone()
            {
                return rootBone_;
            }

            void setGlobalInverseTransform(const glm::mat4& m)
            {
                this->globalInverseTransform_ = m;
                this->getRootBone()->calculateBranch(true,Bone::CalcFlags::eNone);
            }

            /**
             * Получить массив итоговых трансформаций костей
             * @param fromBoneSpace Если точки заданы в пространстве кости
             * @return Ссылка на массив матриц
             */
            const std::vector<glm::mat4>& getFinalBoneTransforms(bool fromBoneSpace = false) const
            {
                return fromBoneSpace ? boneSpaceFinalTransforms_ : modelSpaceFinalTransforms_;
            }

            /**
             * Получить общее кол-во костей
             * @return Целое положительное число
             */
            size_t getBonesCount() const
            {
                return bones_.size();
            }

            /**
             * Получить размер массива трансформаций в байтах
             * @return Целое положительное число
             */
            size_t getTransformsDataSize() const
            {
                return sizeof(glm::mat4) * this->modelSpaceFinalTransforms_.size();
            }

            /**
             * Получить линейный массив костей
             * @return Массив указателей на кости
             */
            const std::vector<BonePtr>& getBones()
            {
                return bones_;
            }

            /**
             * Получить указатель на кость по индексу
             * @param index Индекс
             * @return Smart-pointer кости
             */
            BonePtr getBoneByIndex(size_t index)
            {
                return bones_[index];
            }

            /**
             * Установить текущую анимацию
             */
            void setCurrentAnimation(const vk::scene::MeshSkeletonAnimationPtr& animation)
            {
                // Установка указателя на анимацию
                this->currentAnimationPtr_ = animation;

                // Обнулить состояние анимации
                this->currentAnimationState_ = AnimationState::eStopped;
                this->currentAnimationTime_ = 0.0f;
            }

            /**
             * Установить состояние анимации
             * @param animationState
             */
            void setAnimationState(const vk::scene::MeshSkeleton::AnimationState& animationState)
            {
                this->currentAnimationState_ = animationState;
            }

            /**
             * Обновление анимации
             * @param deltaMs Время кадра в главном цикле
             */
            void updateAnimation(float deltaMs)
            {
                // Если анимация установлена
                if(currentAnimationPtr_ != nullptr)
                {
                    // Если анимация проигрывается - меняем текущий счетчик времени
                    if(this->currentAnimationState_ == AnimationState::ePlaying)
                    {
                        // Если время больше чем длительность анимации - начинаем снова (по кругу)
                        auto newTime = this->currentAnimationTime_ + (static_cast<double>(deltaMs) * this->currentAnimationSpeed_);
                        this->currentAnimationTime_ = std::fmod(newTime,currentAnimationPtr_->getDurationMs());
                    }

                    // Кадры
                    const auto& frames = currentAnimationPtr_->getKeyFrames();

                    // Найти 2 кадра между которыми время анимации
                    for(size_t i = 0; i < frames.size(); i++)
                    {
                        // Если время кадра меньше чем текущее время анимации - значит добрались до нужного кадра
                        if(frames[i].getFrameTime() <= this->currentAnimationTime_)
                        {
                            // Дельта между текущим вторым и первым кадром
                            auto frameTimeDelta = frames[i+1].getFrameTime() - frames[i].getFrameTime();
                            // Дельта между текущим временем анимации и первым кадром
                            auto animTimeDelta = currentAnimationTime_ - frames[i].getFrameTime();

                            // Получить коэффициент интерполяции
                            auto mixCoff = static_cast<float>(animTimeDelta/frameTimeDelta);

                            // Применить трансформацию костей
                            this->applyAnimationFrameBoneTransforms(static_cast<float>(i) + mixCoff);
                        }
                    }
                }
            }

            /**
             * Применить трансформацию костей из кадра анимации
             * @param frame Кадр анимации
             * @details Кадр может быть не целым числом, в таком случае значения будут интерполироваться
             */
            void applyAnimationFrameBoneTransforms(float frame = 0.0f)
            {
                // Если анимация установлена
                if(currentAnimationPtr_ != nullptr)
                {
                    // Кол-во кадров
                    auto totalFramesCount = currentAnimationPtr_->getKeyFrames().size();

                    // Если требуемый кадр больше чем кадров есть, начинаем с начала (по кругу)
                    auto frameSafe = std::fmod(frame, static_cast<float>(totalFramesCount));

                    // Индекс кадра по float значению
                    auto frameIndex = static_cast<size_t>(std::floor(frameSafe));

                    // Коэффициент интерполяции
                    float mixCoff = frameSafe - static_cast<float>(frameIndex);

                    // Считается что кол-во костей одинаково у всех кадров и оно равно кол-ву костей скелета
                    size_t boneCount = this->getBonesCount();

                    // Пройтись по костям и интерполировать значение между кадрами
                    for(size_t i = 0; i < boneCount; i++)
                    {
                        // Интерполированное значение
                        MeshSkeletonAnimation::Keyframe::BoneTransform interpolated{};

                        // Интерполяция между кадрами
                        interpolated.location = glm::mix(currentAnimationPtr_->getKeyFrames()[frameIndex].getBoneTransformations()[i].location,
                                currentAnimationPtr_->getKeyFrames()[frameIndex+1].getBoneTransformations()[i].location,
                                mixCoff);

                        interpolated.orientation = glm::slerp(currentAnimationPtr_->getKeyFrames()[frameIndex].getBoneTransformations()[i].orientation,
                                currentAnimationPtr_->getKeyFrames()[frameIndex+1].getBoneTransformations()[i].orientation,
                                mixCoff);

                        interpolated.scaling = glm::mix(currentAnimationPtr_->getKeyFrames()[frameIndex].getBoneTransformations()[i].scaling,
                                currentAnimationPtr_->getKeyFrames()[frameIndex+1].getBoneTransformations()[i].scaling,
                                mixCoff);


                        // Получить матрицы трансформации
                        glm::mat4 translation = glm::translate(glm::mat4(1.0f),interpolated.location);
                        glm::mat4 rotation = glm::toMat4(interpolated.orientation);
                        glm::mat4 scaling = glm::scale(glm::mat4(1.0f),interpolated.scaling);

                        // Установить локальную трансформацию (не вычисляя матрицы)
                        this->bones_[i]->setLocalTransform(scaling * translation * rotation, false);
                    }

                    // Вычислить матрицы начиная с корня
                    this->getRootBone()->calculateBranch(true,MeshSkeleton::Bone::CalcFlags::eFullTransform);
                }
            }
        };

        /**
         * Smart-unique-pointer объекта скелета
         */
        typedef std::unique_ptr<MeshSkeleton> UniqueMeshSkeleton;
    }
}
