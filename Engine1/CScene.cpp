#include "CScene.h"

#include "SceneParser.h"
#include "BinaryFile.h"

#include <tuple>

using namespace Engine1;

std::tuple< std::shared_ptr<CScene>, std::shared_ptr<std::vector< std::shared_ptr<FileInfo> > > > CScene::createFromFile( std::string path )
{
    std::shared_ptr<std::vector<char>> data = BinaryFile::load( path );

    return SceneParser::parseBinary( *data );
}

CScene::CScene()
{}


CScene::~CScene()
{}

void CScene::addActor( std::shared_ptr<Actor> actor )
{
    if ( !actor )
        throw std::exception( "Scene::addActor - nullptr passed." );

    m_actors.insert( actor );
}

void CScene::removeActor( std::shared_ptr<Actor> actor )
{
    if ( !actor )
        throw std::exception( "Scene::removeActor - nullptr passed." );

    m_actors.erase( actor );
}

void CScene::removeAllActors()
{
    m_actors.clear();
}

void CScene::addLight( std::shared_ptr<Light> light )
{
    if ( !light )
        throw std::exception( "Scene::addLight - nullptr passed." );

    m_lights.insert( light );
}

void CScene::removeLight( std::shared_ptr<Light> light )
{
    if ( !light )
        throw std::exception( "Scene::removeLight - nullptr passed." );

    m_lights.erase( light );
}

void CScene::removeAllLights()
{
    m_lights.clear();
}

void CScene::saveToFile( const std::string& path ) const
{
    std::vector<char> data;

    SceneParser::writeBinary( data, *this );

    BinaryFile::save( path, data );
}

const std::unordered_set< std::shared_ptr<Actor> >& CScene::getActors( ) const
{
    return m_actors;
}

const std::unordered_set< std::shared_ptr<Light> >& CScene::getLights() const
{
    return m_lights;
}


