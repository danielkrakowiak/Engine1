#include "SceneUtil.h"

#include "Light.h"
#include "Actor.h"
#include "BlockActor.h"
#include "SkeletonActor.h"

using namespace Engine1;

std::vector< std::shared_ptr< Light > > SceneUtil::filterLightsByShadowCasting( const std::vector< std::shared_ptr< Light > >& lights, const bool castShadows )
{
    std::vector< std::shared_ptr< Light > > filteredLights;
    filteredLights.reserve( lights.size() );

    for ( auto& light : lights )
    {
        if ( light->isCastingShadows() == castShadows )
            filteredLights.push_back( light );
    }

    filteredLights.shrink_to_fit();

    return filteredLights;
}

std::vector< std::shared_ptr< Light > > SceneUtil::filterLightsByType( const std::vector< std::shared_ptr< Light > >& lights, const Light::Type type )
{
    std::vector< std::shared_ptr< Light > > filteredLights;
    filteredLights.reserve( lights.size() );

    for ( auto& light : lights ) {
        if ( light->getType() == type )
            filteredLights.push_back( light );
    }

    filteredLights.shrink_to_fit();

    return filteredLights;
}

template<>
std::vector< std::shared_ptr< BlockActor > > SceneUtil::filterActorsByType< BlockActor >( const std::vector< std::shared_ptr< Actor > >& actors )
{
    std::vector< std::shared_ptr< BlockActor > > filteredActors;
    filteredActors.reserve( actors.size() );

    for ( auto& actor : actors ) {
        if ( actor->getType() == Actor::Type::BlockActor )
            filteredActors.push_back( std::static_pointer_cast< BlockActor >( actor ) );
    }

    filteredActors.shrink_to_fit();

    return filteredActors;
}

template<>
std::vector< std::shared_ptr< SkeletonActor > > SceneUtil::filterActorsByType< SkeletonActor >( const std::vector< std::shared_ptr< Actor > >& actors )
{
    std::vector< std::shared_ptr< SkeletonActor > > filteredActors;
    filteredActors.reserve( actors.size() );

    for ( auto& actor : actors ) {
        if ( actor->getType() == Actor::Type::SkeletonActor )
            filteredActors.push_back( std::static_pointer_cast< SkeletonActor >( actor ) );
    }

    filteredActors.shrink_to_fit();

    return filteredActors;
}