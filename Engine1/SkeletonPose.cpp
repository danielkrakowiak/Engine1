#include "SkeletonPose.h"

#include <string>

#include "SkeletonMesh.h"

#include "MathUtil.h"

using namespace Engine1;

SkeletonPose SkeletonPose::createIdentityPoseInSkeletonSpace( const SkeletonMesh& skeletonMesh )
{
    return calculatePoseInSkeletonSpace( createIdentityPoseInParentSpace( skeletonMesh ), skeletonMesh );
}

SkeletonPose SkeletonPose::createIdentityPoseInParentSpace( const SkeletonMesh& skeletonMesh )
{
    SkeletonPose poseInParentSpace;
    const unsigned char boneCount = skeletonMesh.getBoneCount();
    for ( unsigned char boneIndex = 1; boneIndex <= boneCount; ++boneIndex )
        poseInParentSpace.setBonePose( boneIndex, float43::IDENTITY );

    return poseInParentSpace;
}

SkeletonPose SkeletonPose::blendPoses( const SkeletonPose& pose1, const SkeletonPose& pose2, float factor )
{
	SkeletonPose combinedPose;

	// Iterate over bones from pose 1 and combine them with bones from pose 2.
	for ( const std::pair<unsigned char, float43>& boneFromPose1 : pose1.bonesPoses ) {
		const unsigned char boneIndex = boneFromPose1.first;

		if ( pose2.hasBone( boneIndex ) ) { // Bone is present in both poses.

			const float43& bonePose1 = pose1.getBonePose( boneIndex );
			const float43& bonePose2 = pose2.getBonePose( boneIndex );

			// Blend poses.
			const float43 combinedBonePose = float43::slerp( bonePose1, bonePose2, factor );

			// Save combined pose.
			combinedPose.setBonePose( boneIndex, combinedBonePose );
		}
	}

	// Iterate over bones from pose 1 and add those which are not present in the combined pose.
	for ( const std::pair<unsigned char, float43>& boneFromPose1 : pose1.bonesPoses ) {
		const unsigned char boneIndex = boneFromPose1.first;

		if ( !combinedPose.hasBone( boneIndex ) ) { // Bone is not present in the combined pose.
			// Save bone from pose 1.
			combinedPose.setBonePose( boneIndex, boneFromPose1.second );
		}
	}

	// Iterate over bones from pose 2 and add those which are not present in the combined pose.
	for ( const std::pair<unsigned char, float43>& boneFromPose2 : pose2.bonesPoses ) {
		const unsigned char boneIndex = boneFromPose2.first;

		if ( !combinedPose.hasBone( boneIndex ) ) { // Bone is not present in the combined pose.
			// Save bone from pose 2.
			combinedPose.setBonePose( boneIndex, boneFromPose2.second );
		}
	}

	return combinedPose;
}

SkeletonPose SkeletonPose::calculatePoseInSkeletonSpace( const SkeletonPose& poseInParentSpace, const SkeletonMesh& skeletonMesh ) {
	SkeletonPose poseInSkeletonSpace;

	// Check if pose in parent space contains also parent bones for all the bones. Otherwise, calculating pose in skeleton space is impossible.
	for ( std::pair<unsigned char, float43> bone : poseInParentSpace.bonesPoses ) {
		unsigned char parentBoneIndex = skeletonMesh.getBone( bone.first ).getParentBoneIndex( );
		if ( parentBoneIndex != 0 && !poseInParentSpace.hasBone( parentBoneIndex ) ) throw std::exception( ( std::string( "SkeletonPose::calculatePoseInSkeletonSpace - cannot calculate pose in skeleton space, because parent pose is not known for some bones (for bone" ) + std::to_string( parentBoneIndex ) + std::string( ")" ) ).c_str( ) );
	}

	unsigned char boneIndex = 1;
	while ( poseInSkeletonSpace.getBonesCount( ) < poseInParentSpace.getBonesCount() ) { // Iterate the bones until they are all calculated.
	
		if ( !poseInSkeletonSpace.hasBone( boneIndex ) ) {							 // If the bone hasn't yet been calculated.
			unsigned char parentBoneIndex = skeletonMesh.getBone( boneIndex ).getParentBoneIndex();
			if ( parentBoneIndex == 0 ) {											 // And it has no parent (is a root bone)
	
				float43 bonePoseInParentSpace = poseInParentSpace.getBonePose( boneIndex );
	
				// Root bone's pose in skeleton space is the same as it's pose in parent space
				float43 bonePoseInSkeletonSpace = bonePoseInParentSpace;

				// Save bone's pose in skeleton space.
				poseInSkeletonSpace.setBonePose( boneIndex, bonePoseInSkeletonSpace );

			} else if ( poseInSkeletonSpace.hasBone( parentBoneIndex ) ) {		     // But it's parent has already been calculated (otherwise, we cannot calculate that bone yet).
				float43 parentBonePoseInSkeletonSpace = poseInSkeletonSpace.getBonePose( parentBoneIndex );
				float43 bonePoseInParentSpace = poseInParentSpace.getBonePose( boneIndex );
	
				// Calculate bone's pose in skeleton space
				float43 bonePoseInSkeletonSpace = bonePoseInParentSpace * parentBonePoseInSkeletonSpace;

				// Save bone's pose in skeleton space.
				poseInSkeletonSpace.setBonePose( boneIndex, bonePoseInSkeletonSpace );
			}
		}
	
											
		if ( !poseInParentSpace.hasBone( boneIndex ) ) boneIndex = 0;		// If reached the last bone, itarate the bones again.
		else ++boneIndex;													// Otherwise, go to the next bone.
	}

	return poseInSkeletonSpace;
}

SkeletonPose SkeletonPose::calculatePoseInParentSpace( const SkeletonPose& poseInSkeletonSpace, const SkeletonMesh& skeletonMesh ) {
	SkeletonPose poseInParentSpace;

	
	for ( std::pair<unsigned char, float43> bone : poseInSkeletonSpace.bonesPoses ) {
		unsigned char parentBoneIndex = skeletonMesh.getBone( bone.first ).getParentBoneIndex( );
		
		if ( parentBoneIndex != 0 ) { // If bone is not the root.
			
			float44 bonePoseInSkeletonSpace( bone.second );
			float43 bonePoseInParentSpace;
			
			// Calculate pose in parent space only for bones for which parent pose is also available.
			if ( poseInSkeletonSpace.hasBone( parentBoneIndex ) ) {
				float44 parentBonePoseInSkeletonSpace = poseInSkeletonSpace.getBonePose( parentBoneIndex );

				// Calculate bone's pose in parent space
				bonePoseInParentSpace = ( bonePoseInSkeletonSpace * parentBonePoseInSkeletonSpace.getScaleOrientationTranslationInverse( ) ).getOrientationTranslation( );

				// Safe bone's pose in parent space.
				poseInParentSpace.setBonePose( bone.first, bonePoseInParentSpace );
			}
		} else { // If bone is the root.
			// Safe bone's pose in parent space - same as in skeleton space.
			poseInParentSpace.setBonePose( bone.first, bone.second );
		}
	}

	return poseInParentSpace;
}

SkeletonPose::SkeletonPose( ) {}

SkeletonPose::SkeletonPose( const SkeletonPose& obj ) 
{
    bonesPoses.clear();

	// Copy the vector.
	bonesPoses.insert( bonesPoses.begin(), obj.bonesPoses.begin( ), obj.bonesPoses.end( ) );
}

SkeletonPose::~SkeletonPose( ) {}

SkeletonPose& SkeletonPose::operator = ( const SkeletonPose& obj )
{
    bonesPoses.clear();

    // Copy the vector.
    bonesPoses.insert( bonesPoses.begin( ), obj.bonesPoses.begin( ), obj.bonesPoses.end( ) );

    return *this;
}

void SkeletonPose::setBonePose( const unsigned char boneIndex, const float43& bonePose ) {
	// Note: boneIndex is in range 1 - 255.
	if ( boneIndex == 0 ) throw std::exception( "SkeletonPose::setBone - boneIndex cannot be 0." );

	// Find the bone or insert it in the vector.
	for ( unsigned int vectorIndex = 0; vectorIndex <= bonesPoses.size( ); ++vectorIndex ) {
		if ( ( bonesPoses.size( ) == vectorIndex ) || ( bonesPoses.size( ) > vectorIndex + 1 && bonesPoses.at( vectorIndex + 1 ).first < boneIndex ) ) { // There is no bone with such index.
			bonesPoses.insert( bonesPoses.begin( ) + vectorIndex, std::make_pair( boneIndex, bonePose ) ); // Insert the bone.
			break;
		} else if ( bonesPoses.at( vectorIndex ).first == boneIndex ) { // Bone with such index was found.
			bonesPoses.at( vectorIndex ).second = bonePose; // Edit the bone.
			break; 
		}
	}
}

float43 SkeletonPose::getBonePose( const unsigned char boneIndex ) const {
	// Find the bone.
	for ( unsigned int vectorIndex = 0; vectorIndex <= bonesPoses.size(); ++vectorIndex ) {
		if ( bonesPoses.at( vectorIndex ).first == boneIndex ) return bonesPoses.at( vectorIndex ).second;
	}

	throw std::exception( ( std::string( "SkeletonPose::getBonePose - there is no bone with such index (boneIndex = " ) + std::to_string( boneIndex ) + std::string( " )." ) ).c_str( ) );
}

unsigned char SkeletonPose::getBonesCount( ) const {
	return (unsigned char)bonesPoses.size( );
}

void SkeletonPose::clear()
{
    bonesPoses.clear();
}

bool SkeletonPose::hasBone( const unsigned char boneIndex ) const {
	// Find the bone.
	for ( unsigned int vectorIndex = 0; vectorIndex < bonesPoses.size( ); ++vectorIndex ) {
		if ( bonesPoses.at( vectorIndex ).first == boneIndex ) return true;
	}

	return false;
}