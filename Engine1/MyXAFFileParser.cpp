#include "MyXAFFileParser.h"

#include <sstream>

using namespace Engine1;

std::vector<std::string> MyXAFFileParser::debug;

MyXAFFileParser::MyXAFFileParser() {}


MyXAFFileParser::~MyXAFFileParser() {}

void MyXAFFileParser::parseSkeletonAnimationFile( const std::vector<char>& file, const SkeletonMesh& skeletonMesh, SkeletonAnimation& skeletonAnimation, const bool invertZCoordinate ) {
	// XML parser modifies the input text so copy is needed.
	std::vector<char> xmlText( file );

	debug.clear();

	rapidxml::xml_document<> xmlDocument;
	xmlDocument.parse<0>( (char*)xmlText.data( ) );    // 0 means default parse flags.

	SceneInfo sceneInfo;

	const rapidxml::xml_node<char>* node = xmlDocument.first_node( );
	
	// Parse scene info.
	while ( node )  
	{
		if ( node && nameEquals( *node, "SceneInfo" ) ) {
			sceneInfo = parseSceneInfo( *node );
			break;
		}

		node = next( node );
	}

	// Throw if scene info wasn't found.
	if ( !node )
		throw std::exception( "MyXAFFileParser::parseSkeletonAnimationFile() - xml node \"SceneInfo\" wasn't found." ); 
	
	// Find first "Node" node.
	node = findNext( xmlDocument.first_node( ), "Node" );

	// Parse all "Node" nodes.
	while ( node ) {
		parseNode( *node, sceneInfo, skeletonMesh, skeletonAnimation, invertZCoordinate );
		node = findNextSibling( node, "Node" );
	}
}

bool MyXAFFileParser::nameEquals( const rapidxml::xml_node<char>& node, std::string name )
{
	return name.compare( 0, node.name_size(), node.name() ) == 0;
}

const rapidxml::xml_node<char>* MyXAFFileParser::next( const rapidxml::xml_node<char>* node )
{
	if ( !node )
		return nullptr;

	if ( node->first_node( ) ) 
		return node->first_node( );
	else if ( node->next_sibling( ) ) 
		return node->next_sibling( );
	else if ( node->parent( ) ) {
		// Find first parent node which has siblings.
		while ( node->parent( ) && !node->next_sibling( ) ) 
			node = node->parent( );

		if ( node->parent() && node->next_sibling() )
			return node->next_sibling();
		else
			return nullptr; // Next node wasn't found.
	} else {
		return nullptr;
	}
}

const rapidxml::xml_node<char>* MyXAFFileParser::findNext( const rapidxml::xml_node<char>* startNode, std::string name )
{
	const rapidxml::xml_node<char>* node = startNode;

	while ( node && !nameEquals( *node, name ) )
		node = next( node );

	return node;
}

const rapidxml::xml_node<char>* MyXAFFileParser::findNextSibling( const rapidxml::xml_node<char>* startNode, std::string name )
{
	const rapidxml::xml_node<char>* node = startNode->next_sibling();

	while ( node && !nameEquals( *node, name ) )
		node = node->next_sibling();

	return node;
}

MyXAFFileParser::SceneInfo MyXAFFileParser::parseSceneInfo( const rapidxml::xml_node<char>& node )
{
	rapidxml::xml_attribute<char>* startTickAttribute     = node.first_attribute( "startTick", 9, true );
	rapidxml::xml_attribute<char>* endTickAttribute       = node.first_attribute( "endTick", 7, true );
	rapidxml::xml_attribute<char>* frameRateAttribute     = node.first_attribute( "frameRate", 9, true );
	rapidxml::xml_attribute<char>* ticksPerFrameAttribute = node.first_attribute( "ticksPerFrame", 13, true );

	if ( !startTickAttribute || !endTickAttribute || !frameRateAttribute || !ticksPerFrameAttribute )
		throw std::exception( "MyXAFFileParser::parseSceneInfo() - xml node \"SceneInfo\" misses one or more attributes (\"startTick\", \"endTick\", \"frameRate\", \"ticksPerFrame\")." );

	SceneInfo sceneInfo;

	sceneInfo.startTick     = parseInt( std::string( startTickAttribute->value(), startTickAttribute->value_size() ) );
	sceneInfo.endTick       = parseInt( std::string( endTickAttribute->value( ), endTickAttribute->value_size( ) ) );
	sceneInfo.frameRate     = parseInt( std::string( frameRateAttribute->value( ), frameRateAttribute->value_size( ) ) );
	sceneInfo.ticksPerFrame = parseInt( std::string( ticksPerFrameAttribute->value( ), ticksPerFrameAttribute->value_size( ) ) );

	return sceneInfo;
}

void MyXAFFileParser::parseNode( const rapidxml::xml_node<char>& node, const SceneInfo& sceneInfo, const SkeletonMesh& skeletonMesh, SkeletonAnimation& skeletonAnimation, const bool invertZCoordinate )
{
	const rapidxml::xml_attribute<char>* nameAttribute = node.first_attribute( "name", 4, true );
	if ( !nameAttribute ) throw std::exception( "MyXAFFileParser::parseNode() - xml node \"Node\" has no attribute \"name\" which identifies a bone." );

	const std::string   boneName( nameAttribute->value( ), nameAttribute->value_size( ) );
	const unsigned char boneIndex = skeletonMesh.getBoneIndex( boneName );

	debug.push_back( boneName );

	const rapidxml::xml_node<char>* samplesNode = node.first_node();
	if ( samplesNode && !nameEquals( *samplesNode, "Samples" ) != 0 )
		findNextSibling( samplesNode, "Samples" );

	if ( !samplesNode ) throw std::exception( "MyXAFFileParser::parseNode() - xml node \"Node\" has no child \"Samples\" which contains bones' poses." );
	
	parseSamples( *samplesNode, sceneInfo, boneIndex, skeletonAnimation, invertZCoordinate );
}

void MyXAFFileParser::parseSamples( const rapidxml::xml_node<char>& node, const SceneInfo& sceneInfo, const unsigned char boneIndex, SkeletonAnimation& skeletonAnimation, const bool invertZCoordinate )
{
	const rapidxml::xml_node<char>* sNode = node.first_node();

	while ( sNode )
	{
		parseS( *sNode, sceneInfo, boneIndex, skeletonAnimation, invertZCoordinate );
		sNode = findNextSibling( sNode, "S" );
	}
}

void MyXAFFileParser::parseS( const rapidxml::xml_node<char>& node, const SceneInfo& sceneInfo, const unsigned char boneIndex, SkeletonAnimation& skeletonAnimation, const bool invertZCoordinate )
{
	rapidxml::xml_attribute<char>* tAttribute = node.first_attribute( "t", 1, true );
	rapidxml::xml_attribute<char>* vAttribute = node.first_attribute( "v", 1, true );
	if ( !tAttribute || !vAttribute ) throw std::exception( "MyXAFFileParser::parseS() - xml node \"S\" has no attribute \"t\" (time) or \"v\" (bone pose matrix)." );

	int     timeTick = parseInt( std::string( tAttribute->value( ), tAttribute->value_size( ) ) );
	float43 bonePose = parseMatrix( std::string( vAttribute->value(), vAttribute->value_size() ), invertZCoordinate );

	int poseIndexInAnimation = ( timeTick - sceneInfo.startTick ) / sceneInfo.ticksPerFrame;
	
	SkeletonPose& pose = skeletonAnimation.getOrAddPose( poseIndexInAnimation );

	pose.setBonePose( boneIndex, bonePose );
}

float43 MyXAFFileParser::parseMatrix( std::string text, const bool invertZCoordinate ) {
	float m11 = parseFloat( text ),
		m12 = parseFloat( text ),
		m13 = parseFloat( text ),
		m21 = parseFloat( text ),
		m22 = parseFloat( text ),
		m23 = parseFloat( text ),
		m31 = parseFloat( text ),
		m32 = parseFloat( text ),
		m33 = parseFloat( text ),
		m41 = parseFloat( text ),
		m42 = parseFloat( text ),
		m43 = parseFloat( text );

	if ( invertZCoordinate ) {
		m13 = -m13;
		m23 = -m23;
		m33 = -m33;
		m43 = -m43;
	}

	return float43( 
		m11, m12, m13,
		m21, m22, m23,
		m31, m32, m33,
		m41, m42, m43
	);
}

float MyXAFFileParser::parseFloat( std::string& text ) {
	std::stringstream ss;

	size_t startPos = text.find_first_not_of( " " );
	size_t endPos = text.find_first_of( " \n", startPos );
	
	size_t valueLength = 0;
	if ( endPos != std::string::npos )  
        valueLength = endPos - startPos;
	else			   
        valueLength = text.length() - startPos;

	if ( valueLength == 0 ) throw std::exception( "MyXAFFileParser::parseFloat - parsing failure." );
	ss.str( "" );
	ss << text.substr( startPos, valueLength ) << '\0';
	float value;
	ss >> value;

	text = text.substr( startPos + valueLength );

	return value;
}

int MyXAFFileParser::parseInt( std::string text )
{
	std::stringstream ss;

	size_t startPos = text.find_first_not_of( " " );
	size_t endPos = text.find_first_of( " \n", startPos );

	size_t valueLength = 0;
	if ( endPos != std::string::npos )  
        valueLength = endPos - startPos;
	else			   
        valueLength = text.length() - startPos;

	if ( valueLength == 0 ) throw std::exception( "MyXAFFileParser::parseInt - parsing failure." );
	ss.str( "" );
	ss << text.substr( startPos, valueLength ) << '\0';
	int value;
	ss >> value;

	return value;
}