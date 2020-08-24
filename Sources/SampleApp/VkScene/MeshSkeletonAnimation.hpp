/**
 * Класс объекта скелетной анимации
 * Copyright (C) 2020 by Alex "DarkWolf" Nem - https://github.com/darkoffalex
 */

#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace vk
{
    namespace scene
    {
        /**
         * Класс скелетной анимации
         */
        class MeshSkeletonAnimation
        {
        public:

            /**
             * Класс ключевого кадра анимации
             */
            class Keyframe
            {
            public:
                /**
                 * Трансформация кости
                 */
                struct BoneTransform
                {
                    glm::vec3 location = glm::vec3(0.0f,0.0f,0.0f);
                    glm::quat orientation = glm::quat(glm::vec3(glm::radians(0.0f),glm::radians(0.0f),glm::radians(0.0f)));
                    glm::vec3 scaling = glm::vec3(1.0f,1.0f,1.0f);
                    glm::mat4 composed = glm::mat4(1.0f);
                };

            private:
                /// Время ключевого кадра
                double timeMs_;
                /// Положение костей (индекс элементов массива совпадает с индексом костей скелета)
                std::vector<BoneTransform> boneTransformations_;

            public:
                /**
                 * Конструктор по умолчанию
                 */
                Keyframe(): timeMs_(0.0){};

                /**
                 * Основной конструктор
                 * @param timeMs Время кадра
                 * @param boneTransformations Положения костей
                 */
                explicit Keyframe(double timeMs, std::vector<BoneTransform> boneTransformations): timeMs_(timeMs),boneTransformations_(std::move(boneTransformations)){};

                /**
                 * Дополнительный конструктор
                 * @param timeMs Время кадра
                 * @param totalBones Количество костей
                 */
                explicit Keyframe(double timeMs, size_t totalBones): timeMs_(timeMs),boneTransformations_(totalBones){};

                /**
                 * Получить положения костей
                 * @return Константная ссылка на массив
                 */
                const std::vector<BoneTransform>& getBoneTransformations() const
                {
                    return boneTransformations_;
                }

                /**
                 * Установить положение кости
                 * @param index Индекс кости
                 * @param transform Положение
                 */
                void setBoneTransformation(size_t index, const BoneTransform& transform)
                {
                    this->boneTransformations_[index] = transform;
                }

                /**
                 * Получить время кадра
                 * @return время кадра
                 */
                double getFrameTime() const
                {
                    return timeMs_;
                }
            };

        private:
            /// Продолжительность анимации в миллисекундах
            double durationMs_;
            /// Ключевые кадры анимации
            std::vector<Keyframe> keyframes_;

        public:
            /**
             * Конструктор по умолчанию
             */
            MeshSkeletonAnimation(): durationMs_(0.0){};

            /**
             * Основной конструктор
             * @param durationMs Продолжительность в миллисекундах
             * @param keyframes Массив ключевых кадров
             */
            explicit MeshSkeletonAnimation(float durationMs, std::vector<Keyframe> keyframes = {}): durationMs_(durationMs),keyframes_(std::move(keyframes)){};

            /**
             * Получить массив ключевых кадров
             * @return Константная ссылка на массив
             */
            const std::vector<Keyframe>& getKeyFrames() const
            {
                return keyframes_;
            }

            /**
             * Добавить ключевой кадр
             * @param keyframe Ключевой кадр
             */
            void addKeyFrame(const Keyframe& keyframe)
            {
                this->keyframes_.push_back(keyframe);
            }

            /**
             * Получить продолжительность в миллисекундах
             * @return Дробное число
             */
            double getDurationMs() const
            {
                return durationMs_;
            }
        };

        /**
         * Smart-pointer объекта скелетной кости
         */
        typedef std::shared_ptr<MeshSkeletonAnimation> MeshSkeletonAnimationPtr;
    }
}