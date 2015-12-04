#include "MyOBJFileParser.h"

#include "BlockMesh.h"

#include <assert.h>
#include <sstream>

using namespace Engine1;

//invertZCoordinate is useful when loading a model created in right-hand space into left-hand space (or the other way)
//invertVertexWindingOrder is useful when vertex winding order in a model is different than used by the renderer (OBJ file should have anti-clockwise order)
std::shared_ptr<BlockMesh> MyOBJFileParser::parseBlockMeshFile( const std::vector<char>& file, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs )
{
	if ( file.empty() ) throw std::exception( "MyOBJFileParser::parseBlockMeshFile() - empty input." );

	std::shared_ptr<BlockMesh> mesh = std::make_shared<BlockMesh>( );

	int uniqueVertexCount = 0, uniqueNormalCount = 0, uniqueTexcoordCount = 0, triangleCount = 0, trianglesFromPolygonsCount = 0;

	{ //count vertices, normals, texcoords, triangles in the file
		char type[ 2 ];
		std::vector<char>::const_iterator it = file.begin();
		std::vector<char>::const_iterator fileEnd = file.end();

		while ( it != fileEnd && ( it + 1 ) != fileEnd ) {
			type[ 0 ] = *it;
			type[ 1 ] = *( it + 1 );


			if ( type[ 0 ] == 'v' && type[ 1 ] == ' ' ) {
				++uniqueVertexCount;
			} else if ( type[ 0 ] == 'v' && type[ 1 ] == 'n' ) {
				++uniqueNormalCount;
			} else if ( type[ 0 ] == 'v' && type[ 1 ] == 't' ) {
				++uniqueTexcoordCount;
			} else if ( type[ 0 ] == 'f' && type[ 1 ] == ' ' ) {
				++triangleCount; //250457

				//count slashes in the line
				int slashCount = 0;
				while ( *( it + 1 ) != '\n' && ( it + 1 ) != fileEnd ) {
					if ( *it == '/' ) ++slashCount;
					++it;
				}

				trianglesFromPolygonsCount += ( slashCount / 2 ) - 3; //every pair of slashes above 3 defines one additional face
			}

			while ( *it != '\n' && ( it + 1 ) != fileEnd ) ++it; //jump to end of line
			while ( it != fileEnd && *it == '\n' ) ++it; //jump to next line and skip empty lines
		}

		triangleCount += trianglesFromPolygonsCount;
	}

	std::vector<float3> uniqueVertices;
	std::vector<float3> uniqueNormals;
	std::vector<float2> uniqueTexcoords;

	{ //allocate memory for unique vertices, unique normals, unique texcoords
		uniqueVertices.reserve( uniqueVertexCount );
		uniqueNormals.reserve( uniqueNormalCount + 1 ); //+1 default normal
		uniqueTexcoords.reserve( uniqueTexcoordCount + 1 ); //+1 default texcoord
	}

	const int verticesPerTriangle = 3;

	{ //allocate memory for mesh vertices, normals, texcoords, triangles
		mesh->vertices.reserve( verticesPerTriangle * triangleCount );
		mesh->normals.reserve( verticesPerTriangle * triangleCount );

		if ( uniqueTexcoordCount > 0 ) {
			mesh->texcoords.push_back( std::vector<float2>() );
			mesh->texcoords.back().reserve( verticesPerTriangle * triangleCount );
		}
	}

	//shortcuts to mesh data
	std::vector<float3>* meshVertices = &mesh->vertices;
	std::vector<float3>* meshNormals = &mesh->normals;
	std::vector<float2>* meshTexcoords = nullptr;
	if ( uniqueTexcoordCount > 0 ) meshTexcoords = &mesh->texcoords.back();
	std::vector<uint3>* meshTriangles = &mesh->triangles;

	{ //load data from file
		const bool hasNormals = ( uniqueNormalCount > 0 );
		const bool hasTexcoords = ( uniqueTexcoordCount > 0 );
		const bool hasPolygons = ( trianglesFromPolygonsCount > 0 );

		std::vector<uint3> meshVertexTexcoordNormalIndices; //used to look for reused vertices - when texcoords and normals are available
		std::vector<uint2> meshVertexTexcoordIndices; //used to look for reused vertices - when texcoords are available
		std::vector<uint2> meshVertexNormalIndices; //used to look for reused vertices - when normals are available
		std::vector<unsigned int> meshVertexIndices; //used to look for reused vertices - when texcoords and normals are not available
		if ( hasNormals   && hasTexcoords )  meshVertexTexcoordNormalIndices.reserve( verticesPerTriangle * triangleCount ); //reserve as much as can be needed in worst case, where no vertices are reused
		else if ( !hasNormals  && hasTexcoords )  meshVertexTexcoordIndices.reserve( verticesPerTriangle * triangleCount );
		else if ( hasNormals   && !hasTexcoords ) meshVertexNormalIndices.reserve( verticesPerTriangle * triangleCount );
		else if ( !hasNormals  && !hasTexcoords ) meshVertexIndices.reserve( verticesPerTriangle * triangleCount );

		//create default texcoords and normals for faces missing that information
		uniqueNormals.push_back( float3( 0.0f, 1.0f, 0.0f ) );
		uniqueTexcoords.push_back( float2( 0.0f, 0.0f ) );


		char type[ 2 ];
		std::vector<char>::const_iterator tmpIt, it = file.cbegin();
		std::vector<char>::const_iterator fileEnd = file.cend();

		uint3 vertexTexcoordNormalIndices;
		uint2 vertexTexcoordIndices, vertexNormalIndices;
		int  vertexIndex;

		while ( it != fileEnd && ( it + 1 ) != fileEnd ) {
			type[ 0 ] = *it;
			type[ 1 ] = *( it + 1 );

			if ( type[ 0 ] == 'v' && type[ 1 ] == ' ' ) {

				it += 3; //jump to the first number in the line
				float3 vertex = parseFloat3( it );
				if ( invertZCoordinate ) vertex.z = -vertex.z;
				uniqueVertices.push_back( vertex ); //TODO: can be accelerated by using resize and then accessing elements through []

			} else if ( type[ 0 ] == 'v' && type[ 1 ] == 'n' ) {

				it += 3; //jump to the first number in the line
				float3 normal = parseFloat3( it );
				if ( invertZCoordinate ) normal.z = -normal.z;
				uniqueNormals.push_back( normal );

			} else if ( type[ 0 ] == 'v' && type[ 1 ] == 't' ) {

				it += 3; //jump to the first number in the line
				float2 texcoord = parseFloat2( it );
				if ( flipUVs ) texcoord.y = 1.0f - texcoord.y;
				uniqueTexcoords.push_back( texcoord );

			} else if ( type[ 0 ] == 'f' && type[ 1 ] == ' ' ) {

				it += 2; //jump to the first number in the line

				if ( hasNormals && hasTexcoords ) {
					{ //load 3 triangle vertices
						int triangle[ 3 ];

						for ( int i = 0; i < 3; ++i ) {
							vertexTexcoordNormalIndices = parseVertexTexcoordNormalIndices( it );
							++it;

							bool vertexFound = false;
							unsigned int reusedVertexIndex = 0;
							std::tie( vertexFound, reusedVertexIndex ) = lookForReusedVertex( vertexTexcoordNormalIndices, meshVertexTexcoordNormalIndices.cend( ), meshVertexTexcoordNormalIndices.cbegin( ) );

							if ( vertexFound ) {
								//found reused vertex
								triangle[ i ] = reusedVertexIndex;
							} else {
								//didn't find reused vertex
								meshVertices->push_back( uniqueVertices.at( vertexTexcoordNormalIndices.x - 1 ) ); //-1 because file indexing starts at 1, ours at 0
								meshTexcoords->push_back( uniqueTexcoords.at( vertexTexcoordNormalIndices.y ) ); //0-indexed texcoord is the default texcoord -> indexing from 1
								meshNormals->push_back( uniqueNormals.at( vertexTexcoordNormalIndices.z ) ); //0-indexed normal is the default normal -> indexing from 1
								triangle[ i ] = meshVertices->size() - 1;

								meshVertexTexcoordNormalIndices.push_back( vertexTexcoordNormalIndices );
							}
						}

						if ( !invertVertexWindingOrder )	meshTriangles->push_back( uint3( triangle[ 0 ], triangle[ 1 ], triangle[ 2 ] ) );
						else								meshTriangles->push_back( uint3( triangle[ 2 ], triangle[ 1 ], triangle[ 0 ] ) );
						
					}

					--it; //move back one character (to avoid end line)

					if ( hasPolygons ) {

						int trianglesFromPolygonCount = 0;

						if ( *it != '\n' ) { //count slashes in the remaining part of the line
							tmpIt = it; //save current cursor position in the file

							int slashCount = 0;
							while ( *( it + 1 ) != '\n' && ( it + 1 ) != fileEnd ) {
								if ( *it == '/' ) ++slashCount;
								++it;
							}

							trianglesFromPolygonCount = slashCount / 2; //every two slashes mean one triangle

							it = tmpIt; //restore cursor position
						}

						{ //load triangles made by polygon triangulation
							uint3 triangle;

							for ( int i = 0; i < trianglesFromPolygonCount; ++i ) {
								uint3& prevTriangle = meshTriangles->back();

								//first triangle vertex - same as first vertex in previous triangle
								triangle.x = prevTriangle.x;
								//second triangle vertex - same as third vertex in previous triangle
								triangle.y = prevTriangle.z;

								vertexTexcoordNormalIndices = parseVertexTexcoordNormalIndices( it );
								if ( *it != '\n' ) ++it;

								//third triangle vertex - from file
								bool vertexFound = false;
								unsigned int reusedVertexIndex = 0;
								std::tie( vertexFound, reusedVertexIndex ) = lookForReusedVertex( vertexTexcoordNormalIndices, meshVertexTexcoordNormalIndices.cend( ), meshVertexTexcoordNormalIndices.cbegin( ) );

								if ( vertexFound ) {
									//found reused vertex
									triangle.z = reusedVertexIndex;
								} else {
									//didn't find reused vertex
									meshVertices->push_back( uniqueVertices.at( vertexTexcoordNormalIndices.x - 1 ) ); //-1 because file indexing starts at 1, ours at 0
									meshTexcoords->push_back( uniqueTexcoords.at( vertexTexcoordNormalIndices.y ) ); //0-indexed texcoord is the default texcoord -> indexing from 1
									meshNormals->push_back( uniqueNormals.at( vertexTexcoordNormalIndices.z ) ); //0-indexed normal is the default normal -> indexing from 1
									triangle.z = meshVertices->size() - 1;

									meshVertexTexcoordNormalIndices.push_back( vertexTexcoordNormalIndices );
								}

								meshTriangles->push_back( triangle );
							}
						}
					}
				} else if ( !hasNormals && hasTexcoords ) {
					{ //load 3 triangle vertices
						unsigned int triangle[ 3 ];

						for ( int i = 0; i < 3; ++i ) {
							vertexTexcoordIndices = parseVertexTexcoordIndices( it );
							++it;

							bool vertexFound = false;
							unsigned int reusedVertexIndex = 0;
							std::tie( vertexFound, reusedVertexIndex ) = lookForReusedVertex( vertexTexcoordIndices, meshVertexTexcoordIndices.cend( ), meshVertexTexcoordIndices.cbegin( ) );

							if ( vertexFound ) {
								//found reused vertex
								triangle[ i ] = reusedVertexIndex;
							} else {
								//didn't find reused vertex
								meshVertices->push_back( uniqueVertices.at( vertexTexcoordIndices.x - 1 ) ); //-1 because file indexing starts at 1, ours at 0
								meshTexcoords->push_back( uniqueTexcoords.at( vertexTexcoordIndices.y ) ); //0-indexed texcoord is the default texcoord -> indexing from 1
								triangle[ i ] = meshVertices->size() - 1;

								meshVertexTexcoordIndices.push_back( vertexTexcoordIndices );
							}
						}

						if ( !invertVertexWindingOrder )	meshTriangles->push_back( uint3( triangle[ 0 ], triangle[ 1 ], triangle[ 2 ] ) );
						else								meshTriangles->push_back( uint3( triangle[ 2 ], triangle[ 1 ], triangle[ 0 ] ) );
					}

					--it; //move back one character (to avoid end line)

					if ( hasPolygons ) {

						int trianglesFromPolygonCount;

						if ( *it != '\n' ) { //count slashes in the remaining part of the line
							tmpIt = it; //save current cursor position in the file

							int slashCount = 0;
							while ( *( it + 1 ) != '\n' && ( it + 1 ) != fileEnd ) {
								if ( *it == '/' ) ++slashCount;
								++it;
							}

							trianglesFromPolygonCount = slashCount; //every slash means one triangle

							it = tmpIt; //restore cursor position
						}

						{ //load triangles made by polygon triangulation
							uint3 triangle;

							for ( int i = 0; i < trianglesFromPolygonCount; ++i ) {
								uint3& prevTriangle = meshTriangles->back();

								//first triangle vertex - same as first vertex in previous triangle
								triangle.x = prevTriangle.x;
								//second triangle vertex - same as third vertex in previous triangle
								triangle.y = prevTriangle.z;

								vertexTexcoordIndices = parseVertexTexcoordIndices( it );
								if ( *it != '\n' ) ++it;

								//third triangle vertex - from file
								bool vertexFound = false;
								unsigned int reusedVertexIndex = 0;
								std::tie( vertexFound, reusedVertexIndex ) = lookForReusedVertex( vertexTexcoordIndices, meshVertexTexcoordIndices.cend( ), meshVertexTexcoordIndices.cbegin( ) );

								if ( vertexFound ) {
									//found reused vertex
									triangle.z = reusedVertexIndex;
								} else {
									//didn't find reused vertex
									meshVertices->push_back( uniqueVertices.at( vertexTexcoordIndices.x - 1 ) ); //-1 because file indexing starts at 1, ours at 0
									meshTexcoords->push_back( uniqueTexcoords.at( vertexTexcoordIndices.y ) ); //0-indexed texcoord is the default texcoord -> indexing from 1
									triangle.z = meshVertices->size() - 1;

									meshVertexTexcoordIndices.push_back( vertexTexcoordIndices );
								}

								meshTriangles->push_back( triangle );
							}
						}
					}
				}if ( hasNormals && !hasTexcoords ) {
					{ //load 3 triangle vertices
						unsigned int triangle[ 3 ];

						for ( int i = 0; i < 3; ++i ) {
							vertexNormalIndices = parseVertexNormalIndices( it );
							++it;

							bool vertexFound = false;
							unsigned int reusedVertexIndex = 0;
							std::tie( vertexFound, reusedVertexIndex ) = lookForReusedVertex( vertexNormalIndices, meshVertexNormalIndices.cend( ), meshVertexNormalIndices.cbegin( ) );

							if ( vertexFound ) {
								//found reused vertex
								triangle[ i ] = reusedVertexIndex;
							} else {
								//didn't find reused vertex
								meshVertices->push_back( uniqueVertices.at( vertexNormalIndices.x - 1 ) ); //-1 because file indexing starts at 1, ours at 0
								meshNormals->push_back( uniqueNormals.at( vertexNormalIndices.y ) ); //0-indexed normal is the default normal -> indexing from 1
								triangle[ i ] = meshVertices->size() - 1;

								meshVertexNormalIndices.push_back( vertexNormalIndices );
							}
						}

						if ( !invertVertexWindingOrder )	meshTriangles->push_back( uint3( triangle[ 0 ], triangle[ 1 ], triangle[ 2 ] ) );
						else								meshTriangles->push_back( uint3( triangle[ 2 ], triangle[ 1 ], triangle[ 0 ] ) );
					}

					--it; //move back one character (to avoid end line)

					if ( hasPolygons ) {

						int trianglesFromPolygonCount;

						if ( *it != '\n' ) { //count slashes in the remaining part of the line
							tmpIt = it; //save current cursor position in the file

							int slashCount = 0;
							while ( *( it + 1 ) != '\n' && ( it + 1 ) != fileEnd ) {
								if ( *it == '/' ) ++slashCount;
								++it;
							}

							trianglesFromPolygonCount = slashCount / 2; //every two slashes mean one triangle

							it = tmpIt; //restore cursor position
						}

						{ //load triangles made by polygon triangulation
							uint3 triangle;

							for ( int i = 0; i < trianglesFromPolygonCount; ++i ) {
								uint3& prevTriangle = meshTriangles->back();

								//first triangle vertex - same as first vertex in previous triangle
								triangle.x = prevTriangle.x;
								//second triangle vertex - same as third vertex in previous triangle
								triangle.y = prevTriangle.z;

								vertexNormalIndices = parseVertexNormalIndices( it );
								if ( *it != '\n' ) ++it;

								//third triangle vertex - from file
								bool vertexFound = false;
								unsigned int reusedVertexIndex = 0;
								std::tie( vertexFound, reusedVertexIndex ) = lookForReusedVertex( vertexNormalIndices, meshVertexNormalIndices.cend( ), meshVertexNormalIndices.cbegin( ) );

								if ( vertexFound ) {
									//found reused vertex
									triangle.z = reusedVertexIndex;
								} else {
									//didn't find reused vertex
									meshVertices->push_back( uniqueVertices.at( vertexNormalIndices.x - 1 ) ); //-1 because file indexing starts at 1, ours at 0
									meshNormals->push_back( uniqueNormals.at( vertexNormalIndices.y ) ); //0-indexed normal is the default normal -> indexing from 1
									triangle.z = meshVertices->size() - 1;

									meshVertexNormalIndices.push_back( vertexNormalIndices );
								}

								meshTriangles->push_back( triangle );
							}
						}
					}
				} else if ( !hasNormals && !hasTexcoords ) {
					{ //load 3 triangle vertices
						unsigned int triangle[ 3 ];

						for ( int i = 0; i < 3; ++i ) {
							vertexIndex = parseVertexIndex( it );
							++it;

							bool vertexFound = false;
							unsigned int reusedVertexIndex = 0;
							std::tie( vertexFound, reusedVertexIndex ) = lookForReusedVertex( vertexIndex, meshVertexIndices.cend( ), meshVertexIndices.cbegin( ) );

							if ( vertexFound ) {
								//found reused vertex
								triangle[ i ] = reusedVertexIndex;
							} else {
								//didn't find reused vertex
								meshVertices->push_back( uniqueVertices.at( vertexIndex - 1 ) ); //-1 because file indexing starts at 1, ours at 0
								triangle[ i ] = meshVertices->size() - 1;

								meshVertexIndices.push_back( vertexIndex );
							}
						}

						if ( !invertVertexWindingOrder )	meshTriangles->push_back( uint3( triangle[ 0 ], triangle[ 1 ], triangle[ 2 ] ) );
						else								meshTriangles->push_back( uint3( triangle[ 2 ], triangle[ 1 ], triangle[ 0 ] ) );
					}

					--it; //move back one character (to avoid end line)

					if ( hasPolygons ) {

						int trianglesFromPolygonCount;

						if ( *it != '\n' ) { //count slashes in the remaining part of the line
							tmpIt = it; //save current cursor positon in the file

							int slashCount = 0;
							while ( *( it + 1 ) != '\n' && ( it + 1 ) != fileEnd ) {
								if ( *it == '/' ) ++slashCount;
								++it;
							}

							trianglesFromPolygonCount = slashCount / 2; //every two slashes mean one triangle

							it = tmpIt; //restore cursor position
						}

						{ //load triangles made by polygon triangulation
							uint3 triangle;

							for ( int i = 0; i < trianglesFromPolygonCount; ++i ) {
								uint3& prevTriangle = meshTriangles->back();

								//first triangle vertex - same as first vertex in previous triangle
								triangle.x = prevTriangle.x;
								//second triangle vertex - same as third vertex in previous triangle
								triangle.y = prevTriangle.z;

								vertexIndex = parseVertexIndex( it );
								if ( *it != '\n' ) ++it;

								//third triangle vertex - from file
								bool vertexFound = false;
								unsigned int reusedVertexIndex = 0;
								std::tie( vertexFound, reusedVertexIndex ) = lookForReusedVertex( vertexIndex, meshVertexIndices.cend( ), meshVertexIndices.cbegin( ) );

								if ( vertexFound ) {
									//found reused vertex
									triangle.z = reusedVertexIndex;
								} else {
									//didn't find reused vertex
									meshVertices->push_back( uniqueVertices.at( vertexIndex - 1 ) ); //-1 because file indexing starts at 1, ours at 0
									triangle.z = meshVertices->size() - 1;

									meshVertexIndices.push_back( vertexIndex );
								}

								meshTriangles->push_back( triangle );
							}
						}
					}
				}
			}

			std::vector<char>::const_iterator prevIt = it;

			while ( it != fileEnd && ( it + 1 ) != fileEnd && *it != '\n' ) ++it; //jump to end of line
			while ( it != fileEnd && *it == '\n' ) ++it; //jump to next line and skip empty lines
		}
	}

	return mesh;
}

float2 MyOBJFileParser::parseFloat2( std::vector<char>::const_iterator& it ) {
	int valueLength;
	float2 tempFloat2;
	std::stringstream ss; //for type conversion

	//load first component
	valueLength = 0;
	while ( *( it + valueLength ) != ' ' && *( it + valueLength ) != '\n' ) ++valueLength;
	if ( valueLength == 0 ) throw std::exception( "OBJMeshFileParser::parseFloat2 - input is missing first float" );
	ss.str( "" );
	ss << std::string( &( *it ), valueLength ) << '\0';
	ss >> tempFloat2.x;
	it += valueLength + 1;

	//load second component
	valueLength = 0;
	while ( *( it + valueLength ) != ' ' && *( it + valueLength ) != '\n' ) ++valueLength;
	if ( valueLength == 0 ) throw std::exception( "OBJMeshFileParser::parseFloat2 - input is missing second float" );
	ss.str( "" );
	ss << std::string( &( *it ), valueLength ) << '\0';
	ss >> tempFloat2.y;

	return tempFloat2;
}

float3 MyOBJFileParser::parseFloat3( std::vector<char>::const_iterator& it ) {
	int valueLength;
	float3 tempFloat3;
	std::stringstream ss; //for type conversion


	//load first component
	valueLength = 0;
	while ( *( it + valueLength ) != ' ' && *( it + valueLength ) != '\n' ) ++valueLength;
	if ( valueLength == 0 ) throw std::exception( "OBJMeshFileParser::parseFloat3 - input is missing first float" );
	ss.str( "" );
	ss << std::string( &( *it ), valueLength ) << '\0';
	ss >> tempFloat3.x;
	it += valueLength + 1;

	//load second component
	valueLength = 0;
	while ( *( it + valueLength ) != ' ' && *( it + valueLength ) != '\n' ) ++valueLength;
	if ( valueLength == 0 ) throw std::exception( "OBJMeshFileParser::parseFloat3 - input is missing second float" );
	ss.str( "" );
	ss << std::string( &( *it ), valueLength ) << '\0';
	ss >> tempFloat3.y;
	it += valueLength + 1;

	//load third component
	valueLength = 0;
	while ( *( it + valueLength ) != ' ' && *( it + valueLength ) != '\n' ) ++valueLength;
	if ( valueLength == 0 ) throw std::exception( "OBJMeshFileParser::parseFloat3 - input is missing third float" );
	ss.str( "" );
	ss << std::string( &( *it ), valueLength ) << '\0';
	ss >> tempFloat3.z;

	return tempFloat3;
}

uint3 MyOBJFileParser::parseVertexTexcoordNormalIndices( std::vector<char>::const_iterator& it ) {
	int valueLength;
	uint3 tempInt3;
	std::stringstream ss; //for type conversion

	//load first component
	valueLength = 0;
	while ( *( it + valueLength ) != '/' && *( it + valueLength ) != '\n' ) ++valueLength;
	if ( valueLength == 0 ) throw std::exception( "OBJMeshFileParser::parseVertexTexcoordNormalIndices - input is missing vertex index" );
	ss.str( "" );
	ss << std::string( &( *it ), valueLength ) << '\0';
	ss >> tempInt3.x;
	it += valueLength + 1;

	//load second component
	valueLength = 0;
	while ( *( it + valueLength ) != '/' && *( it + valueLength ) != '\n' ) ++valueLength;
	if ( valueLength > 0 ) {
		ss.str( "" );
		ss << std::string( &( *it ), valueLength ) << '\0';
		ss >> tempInt3.y;
	} else {
		tempInt3.y = 0; //if texcoord index is missing
	}
	it += valueLength + 1;


	//load third component
	valueLength = 0;
	while ( *( it + valueLength ) != ' ' && *( it + valueLength ) != '\n' ) ++valueLength;
	if ( valueLength > 0 ) {
		ss.str( "" );
		ss << std::string( &( *it ), valueLength ) << '\0';
		ss >> tempInt3.z;
	} else {
		tempInt3.z = 0; //if normal index is missing
	}
	it += valueLength;

	return tempInt3;
}

uint2 MyOBJFileParser::parseVertexTexcoordIndices( std::vector<char>::const_iterator& it ) {
	int valueLength;
	uint2 tempInt2;
	std::stringstream ss; //for type conversion

	//load first component
	valueLength = 0;
	while ( *( it + valueLength ) != '/' && *( it + valueLength ) != '\n' ) ++valueLength;
	if ( valueLength == 0 ) throw std::exception( "OBJMeshFileParser::parseVertexTexcoordIndices - input is missing vertex index" );
	ss.str( "" );
	ss << std::string( &( *it ), valueLength ) << '\0';
	ss >> tempInt2.x;
	it += valueLength + 1;

	//load second component
	valueLength = 0;
	while ( *( it + valueLength ) != ' ' && *( it + valueLength ) != '\n' ) ++valueLength;
	if ( valueLength > 0 ) {
		ss.str( "" );
		ss << std::string( &( *it ), valueLength ) << '\0';
		ss >> tempInt2.y;
	} else {
		tempInt2.y = 0; //if texcoord index is missing
	}
	it += valueLength;

	return tempInt2;
}

uint2 MyOBJFileParser::parseVertexNormalIndices( std::vector<char>::const_iterator& it ) {
	int valueLength;
	uint2 tempInt2;
	std::stringstream ss; //for type conversion

	//load first component
	valueLength = 0;
	while ( *( it + valueLength ) != '\n' && !( *( it + valueLength ) == '/' && *( it + valueLength + 1 ) == '/' ) ) ++valueLength;
	if ( valueLength == 0 ) throw std::exception( "OBJMeshFileParser::parseVertexNormalIndices - input is missing vertex index" );
	ss.str( "" );
	ss << std::string( &( *it ), valueLength ) << '\0';
	ss >> tempInt2.x;
	it += valueLength + 2;

	//load second component
	valueLength = 0;
	while ( *( it + valueLength ) != '\n' && *( it + valueLength ) != ' ' ) ++valueLength;
	if ( valueLength > 0 ) {
		ss.str( "" );
		ss << std::string( &( *it ), valueLength ) << '\0';
		ss >> tempInt2.y;
		it += valueLength;
	} else {
		tempInt2.y = 0;
	}

	return tempInt2;
}

unsigned int MyOBJFileParser::parseVertexIndex( std::vector<char>::const_iterator& it ) {
	int valueLength;
	unsigned int tempInt;
	std::stringstream ss; //for type conversion

	valueLength = 0;
	while ( *( it + valueLength ) != ' ' && *( it + valueLength ) != '\n' ) ++valueLength;
	if ( valueLength == 0 ) throw std::exception( "OBJMeshFileParser::parseVertexIndex - input is missing vertex index" );
	ss.str( "" );
	ss << std::string( &( *it ), valueLength ) << '\0';
	ss >> tempInt;
	it += valueLength;

	return tempInt;
}

std::tuple<bool, unsigned int> MyOBJFileParser::lookForReusedVertex( uint3 vertexTexcoordNormalIndices, std::vector<uint3>::const_iterator vertexTexcoordNormalIndicesArrayEnd, std::vector<uint3>::const_iterator vertexTexcoordNormalIndicesArrayBegin ) {

	std::vector<uint3>::const_iterator& it = vertexTexcoordNormalIndicesArrayEnd;

	int verticesCheckedCount = 0;

	while ( it != vertexTexcoordNormalIndicesArrayBegin && verticesCheckedCount < reusedVerticesMaxChecksCount ) {
		if ( *( it - 1 ) == vertexTexcoordNormalIndices ) return std::make_tuple( true, it - 1 - vertexTexcoordNormalIndicesArrayBegin ); //return index of the reused vertex
		++verticesCheckedCount;
		--it;
	}

	return std::make_tuple( false, 0 );
}

std::tuple<bool, unsigned int> MyOBJFileParser::lookForReusedVertex( uint2 vertexTexcoordOrNormalIndices, std::vector<uint2>::const_iterator vertexTexcoordOrNormalIndicesArrayEnd, std::vector<uint2>::const_iterator vertexTexcoordOrNormalIndicesArrayBegin ) {

	std::vector<uint2>::const_iterator& it = vertexTexcoordOrNormalIndicesArrayEnd;

	int verticesCheckedCount = 0;

	while ( it != vertexTexcoordOrNormalIndicesArrayBegin && verticesCheckedCount < reusedVerticesMaxChecksCount ) {
		if ( *( it - 1 ) == vertexTexcoordOrNormalIndices ) return std::make_tuple( true, it - 1 - vertexTexcoordOrNormalIndicesArrayBegin ); //return index of the reused vertex
		++verticesCheckedCount;
		--it;
	}

	return std::make_tuple( false, 0 );
}

std::tuple<bool, unsigned int> MyOBJFileParser::lookForReusedVertex( unsigned int vertexIndices, std::vector<unsigned int>::const_iterator vertexIndicesArrayEnd, std::vector<unsigned int>::const_iterator vertexIndicesArrayBegin ) {

	std::vector<unsigned int>::const_iterator& it = vertexIndicesArrayEnd;

	int verticesCheckedCount = 0;

	while ( it != vertexIndicesArrayBegin && verticesCheckedCount < reusedVerticesMaxChecksCount ) {
		if ( *( it - 1 ) == vertexIndices ) return std::make_tuple( true, it - 1 - vertexIndicesArrayBegin ); //return index of the reused vertex
		++verticesCheckedCount;
		--it;
	}

	return std::make_tuple( false, 0 );
}