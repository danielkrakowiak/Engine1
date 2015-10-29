#pragma once

#include <vector>
#include "SkeletonMesh.h"
#include "SkeletonAnimation.h"
#include "rapidxml.hpp"

class MyXAFFileParser {
	public:
	MyXAFFileParser();
	~MyXAFFileParser();

	static std::vector<std::string> debug;

	public:
	// Returns skeleton animation in skeleton space.
	static void parseSkeletonAnimationFile( const std::vector<char>& file, const SkeletonMesh& skeletonMesh, SkeletonAnimation& skeletonAnimation, const bool invertZCoordinate );

	private:

	struct SceneInfo
	{
		int startTick;
		int endTick;
		int frameRate;
		int ticksPerFrame;
	};

	static bool nameEquals( const rapidxml::xml_node<char>& node, std::string name );

	static const rapidxml::xml_node<char>* next( const rapidxml::xml_node<char>* node );

	static const rapidxml::xml_node<char>* findNext       ( const rapidxml::xml_node<char>* startNode, std::string name );
	static const rapidxml::xml_node<char>* findNextSibling( const rapidxml::xml_node<char>* startNode, std::string name );

	static SceneInfo parseSceneInfo( const rapidxml::xml_node<char>& node );

	static void      parseNode   ( const rapidxml::xml_node<char>& node, const SceneInfo& sceneInfo, const SkeletonMesh& skeletonMesh, SkeletonAnimation& skeletonAnimation, const bool invertZCoordinate );
	static void      parseSamples( const rapidxml::xml_node<char>& node, const SceneInfo& sceneInfo, const unsigned char boneIndex, SkeletonAnimation& skeletonAnimation, const bool invertZCoordinate );
	static void      parseS      ( const rapidxml::xml_node<char>& node, const SceneInfo& sceneInfo, const unsigned char boneIndex, SkeletonAnimation& skeletonAnimation, const bool invertZCoordinate );

	static float43   parseMatrix( std::string text, const bool invertZCoordinate = false );
	static float     parseFloat( std::string& text );
	static int       parseInt( std::string text );
};

