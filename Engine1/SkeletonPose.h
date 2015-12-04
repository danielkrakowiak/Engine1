#pragma once

#include <vector>
#include <tuple>
#include "float44.h"



namespace Engine1
{
    class SkeletonMesh;

    // Object of this class stores poses of skeleton bones. All bones are either in "skeleton space" or "parent space".
    // "Skeleton space" mean that bone pose is in local space of the mesh. This pose is used mainly for rendering.
    // "Parent space" means that bone pose in the space of its parent bone. This pose is used mainly to combine poses together.
    class SkeletonPose
    {

        public:

        static SkeletonPose createIdentityPoseInSkeletonSpace( const SkeletonMesh& skeletonMesh );
        static SkeletonPose createIdentityPoseInParentSpace( const SkeletonMesh& skeletonMesh );

        // Blends poses of the corresponding bones in both poses. If a bone is present only in one pose, it is used without blending.
        // Both blended poses should be in skeleton space or in parent space.
        static SkeletonPose blendPoses( const SkeletonPose& pose1, const SkeletonPose& pose2, float factor );

        // Blends poses of the corresponding bones in both poses. If a bone is present only in one pose, it is used without blending. 
        // If a pose contains only a subset of bones, the last bone in the chain is ignored, as it has no information about it's pose relative to it's parent (such bone has an identity pose).
        // Can be used to blend any poses in parent space (containing the same or different subset of bones).
        //static SkeletonPose blendPosesInParentSpace( const SkeletonPose& poseInParentSpace1, const SkeletonPose& poseInParentSpace2, const SkeletonMesh& mesh, float factor );

        static SkeletonPose calculatePoseInSkeletonSpace( const SkeletonPose& poseInParentSpace, const SkeletonMesh& skeletonMesh );
        static SkeletonPose calculatePoseInParentSpace( const SkeletonPose& poseInSkeletonSpace, const SkeletonMesh& skeletonMesh );

        SkeletonPose();
        SkeletonPose( const SkeletonPose& );
        ~SkeletonPose();

        SkeletonPose& operator=(const SkeletonPose& pose);

        // boneIndex is in range 1 - 255.
        void setBonePose( const unsigned char boneIndex, const float43& bonePose );

        // boneIndex is in range 1 - 255.
        float43 getBonePose( const unsigned char boneIndex ) const;

        unsigned char getBonesCount() const;

        void clear();

        private:

        // boneIndex is in range 1 - 255.
        bool hasBone( const unsigned char boneIndex ) const;

        // Pairs of <bone index, bone pose>.
        std::vector< std::pair<unsigned char, float43 > > bonesPoses;

    };
}


