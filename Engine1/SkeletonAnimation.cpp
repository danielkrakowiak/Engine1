#include "SkeletonAnimation.h"

#include <fstream>
#include <assert.h>
#include <algorithm>

#include "StringUtil.h"

#include "MyXAFFileParser.h"

#include "File.h"


std::shared_ptr<SkeletonAnimation> SkeletonAnimation::createFromFile( const std::string& path, const FileFormat format, const SkeletonMesh& mesh, const bool invertZCoordinate )
{
	std::shared_ptr< std::vector<char> > fileData = File::loadText( path );

	return createFromMemory( *fileData, format, mesh, invertZCoordinate );
}

std::shared_ptr<SkeletonAnimation> SkeletonAnimation::createFromMemory( std::vector<char>& fileData, const FileFormat format, const SkeletonMesh& mesh, const bool invertZCoordinate )
{
	std::shared_ptr<SkeletonAnimation> animation = std::make_shared<SkeletonAnimation>( );

	if ( FileFormat::XAF == format ) {
		MyXAFFileParser::parseSkeletonAnimationFile( fileData, mesh, *animation, invertZCoordinate );
	}

	return animation;
}

std::shared_ptr<SkeletonAnimation> SkeletonAnimation::calculateAnimationInSkeletonSpace( const SkeletonAnimation& animationInParentSpace, const SkeletonMesh& skeletonMesh )
{
	std::shared_ptr<SkeletonAnimation> animationInSkeletonSpace = std::make_shared<SkeletonAnimation>( );

	for ( const SkeletonPose& poseInParentSpace : animationInParentSpace.skeletonPoses )
		animationInSkeletonSpace->skeletonPoses.push_back( SkeletonPose::calculatePoseInSkeletonSpace( poseInParentSpace, skeletonMesh ) );

	return animationInSkeletonSpace;
}

std::shared_ptr<SkeletonAnimation> SkeletonAnimation::calculateAnimationInParentSpace( const SkeletonAnimation& animationInSkeletonSpace, const SkeletonMesh& skeletonMesh )
{
	std::shared_ptr<SkeletonAnimation> animationInParentSpace = std::make_shared<SkeletonAnimation>( );

	for ( const SkeletonPose& poseInSkeletonSpace : animationInSkeletonSpace.skeletonPoses )  
		animationInParentSpace->skeletonPoses.push_back( SkeletonPose::calculatePoseInParentSpace( poseInSkeletonSpace, skeletonMesh ) );

	return animationInParentSpace;
}

SkeletonAnimation::SkeletonAnimation()
{}

SkeletonAnimation::SkeletonAnimation( SkeletonAnimation&& other )
{
	// TODO: should be tested.
	skeletonPoses = std::move( other.skeletonPoses );
}

SkeletonAnimation::~SkeletonAnimation() 
{}

void SkeletonAnimation::addPose( SkeletonPose& pose, float time ) 
{
	skeletonPoses.push_back( pose );
}

SkeletonPose SkeletonAnimation::getInterpolatedPose( float progress )
{
	const float frame               = progress * (float)( skeletonPoses.size() - 1 );
	const unsigned int prevKeyframe = std::max( 0u, (unsigned int)frame );
	const unsigned int nextKeyframe = std::min( skeletonPoses.size( ) - 1, (unsigned int)frame + 1 );
	const float fraction            = frame - (float)prevKeyframe;

	if ( prevKeyframe == nextKeyframe )
		return skeletonPoses.at( prevKeyframe );
	else
		return SkeletonPose::blendPoses( skeletonPoses.at( prevKeyframe ), skeletonPoses.at( nextKeyframe ), fraction );
}

SkeletonPose& SkeletonAnimation::getPose( unsigned int keyframe )
{
	if ( keyframe >= skeletonPoses.size() ) throw std::exception( "SkeletonAnimation::getPose() - keyframe is out of range." );

	return skeletonPoses.at( keyframe );
}

SkeletonPose& SkeletonAnimation::getOrAddPose( unsigned int keyframe )
{
	while ( skeletonPoses.size( ) <= keyframe )
		skeletonPoses.push_back( SkeletonPose() );

	return skeletonPoses.at( keyframe );
}

unsigned int SkeletonAnimation::getKeyframeCount()
{
	return skeletonPoses.size();
}
