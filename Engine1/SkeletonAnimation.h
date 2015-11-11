#pragma once

#include <vector>
#include <memory>

#include "Asset.h"

#include "SkeletonMesh.h"
#include "SkeletonPose.h"

class SkeletonAnimation {
public:

	enum class FileFormat : char
	{
		XAF
	};

	// Returns skeleton animation in skeleton space.
	static std::shared_ptr<SkeletonAnimation> createFromFile( const std::string& path, const FileFormat format, const SkeletonMesh& mesh, const bool invertZCoordinate = false );
	static std::shared_ptr<SkeletonAnimation> createFromMemory( std::vector<char>& fileData, const FileFormat format, const SkeletonMesh& mesh, const bool invertZCoordinate = false );

	static std::shared_ptr<SkeletonAnimation> calculateAnimationInSkeletonSpace( const SkeletonAnimation& animationInParentSpace, const SkeletonMesh& skeletonMesh );
	static std::shared_ptr<SkeletonAnimation> calculateAnimationInParentSpace( const SkeletonAnimation& animationInSkeletonSpace, const SkeletonMesh& skeletonMesh );

	SkeletonAnimation();
	SkeletonAnimation( SkeletonAnimation&& other );
	~SkeletonAnimation();

	void addPose( SkeletonPose& pose, float time );
	SkeletonPose getInterpolatedPose( float progress );

	SkeletonPose& getPose( unsigned int keyframe );

	// Returns pose at given keyframe. If there is no pose at such keyframe - new poses are added until (keyframe + 1) exist. 
	SkeletonPose& getOrAddPose( unsigned int keyframe );

	unsigned int getKeyframeCount( );

	private:

	std::vector< SkeletonPose > skeletonPoses;

	// Copying animation in not allowed.
	SkeletonAnimation( const SkeletonAnimation& ) = delete;
	SkeletonAnimation& operator=( const SkeletonAnimation& ) = delete;
	
};

