#include "CScene.h"

using namespace Engine1;

CScene::CScene()
{}


CScene::~CScene()
{}

void CScene::addActor( std::shared_ptr<Actor> actor )
{
    if ( !actor )
        throw std::exception( "Scene::addActor - nullptr passed." );

    actors.insert( actor );
}

void CScene::removeActor( std::shared_ptr<Actor> actor )
{
    if ( !actor )
        throw std::exception( "Scene::removeActor - nullptr passed." );

    actors.erase( actor );
}

void CScene::removeAllActors()
{
    actors.clear();
}

const std::unordered_set< std::shared_ptr<Actor> >& CScene::getActors( )
{
    return actors;
}
