#include "Scene.h"

#include "SceneParser.h"
#include "BinaryFile.h"

#include <tuple>

using namespace Engine1;

std::tuple< std::shared_ptr<Scene>, std::shared_ptr<std::vector< std::shared_ptr<FileInfo> > > > Scene::createFromFile( std::string path )
{
    std::shared_ptr<std::vector<char>> data = BinaryFile::load( path );

    return SceneParser::parseBinary( *data );
}

Scene::Scene()
{}


Scene::~Scene()
{}

void Scene::addActor( std::shared_ptr<Actor> actor )
{
    if ( !actor )
        throw std::exception( "Scene::addActor - nullptr passed." );

    m_actors.insert( actor );
}

void Scene::removeActor( std::shared_ptr<Actor> actor )
{
    if ( !actor )
        throw std::exception( "Scene::removeActor - nullptr passed." );

    m_actors.erase( actor );
}

void Scene::removeAllActors()
{
    m_actors.clear();
}

void Scene::addLight( std::shared_ptr<Light> light )
{
    if ( !light )
        throw std::exception( "Scene::addLight - nullptr passed." );

    m_lights.insert( light );
}

void Scene::removeLight( std::shared_ptr<Light> light )
{
    if ( !light )
        throw std::exception( "Scene::removeLight - nullptr passed." );

    m_lights.erase( light );
}

void Scene::removeAllLights()
{
    m_lights.clear();
}

void Scene::saveToFile( const std::string& path ) const
{
    std::vector<char> data;

    SceneParser::writeBinary( data, *this );

    BinaryFile::save( path, data );
}

const std::unordered_set< std::shared_ptr<Actor> >& Scene::getActors( ) const
{
    return m_actors;
}

const std::unordered_set< std::shared_ptr<Light> >& Scene::getLights() const
{
    return m_lights;
}


