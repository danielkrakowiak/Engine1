#pragma once

#include <memory>

#include "Actor.h"

#include "float43.h"
#include "SkeletonPose.h"

namespace Engine1
{
    class SkeletonModel;
    class SkeletonAnimation;

    class SkeletonActor : public Actor
    {
        public:

        SkeletonActor( std::shared_ptr<SkeletonModel> model, const float43& pose = float43::IDENTITY );
        SkeletonActor( std::shared_ptr<SkeletonModel> model, const float43& pose, const SkeletonPose& skeletonPose );
        ~SkeletonActor();

        Type getType() const;

        const float43&                       getPose() const;
        float43&                             getPose();
        const SkeletonPose&                  getSkeletonPose() const;
        SkeletonPose&                        getSkeletonPose();
        std::shared_ptr<const SkeletonModel> getModel() const;
        std::shared_ptr<SkeletonModel>       getModel();

        void setPose( const float43& pose );
        void setSkeletonPose( const SkeletonPose& poseInSkeletonSpace );
        void setModel( std::shared_ptr<SkeletonModel> model );

        void resetSkeletonPose();

        // Temporary.
        void startAnimation( const std::shared_ptr< SkeletonAnimation > animationInSkeletonSpace );
        void updateAnimation( const float deltaTime );

        private:

        float43 pose;
        SkeletonPose skeletonPose;

        std::shared_ptr< SkeletonModel > model;

        // Temporary.
        std::shared_ptr< SkeletonAnimation > animation;
        float animationProgress;
        float animationSpeed;
    };
}

