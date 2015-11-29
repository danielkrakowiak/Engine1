#include "Scene.h"


Scene::Scene()
{}


Scene::~Scene()
{}

void Scene::addActor( std::shared_ptr<Actor> actor )
{
    if ( !actor )
        throw std::exception( "Scene::addActor - nullptr passed." );

    actors.insert( actor );
}

void Scene::removeActor( std::shared_ptr<Actor> actor )
{
    if ( !actor )
        throw std::exception( "Scene::removeActor - nullptr passed." );

    actors.erase( actor );
}

void Scene::removeAllActors()
{
    actors.clear();
}

const std::unordered_set< std::shared_ptr<Actor> >& Scene::getActors( )
{
    return actors;
}
