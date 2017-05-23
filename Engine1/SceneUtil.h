#pragma once

#include <vector>
#include <memory>

#include "Light.h"

namespace Engine1
{
    class Actor;

    class SceneUtil
    {
        public:

        static std::vector< std::shared_ptr< Light > > filterLightsByShadowCasting( const std::vector< std::shared_ptr< Light > >& lights, const bool castShadows );
        static std::vector< std::shared_ptr< Light > > filterLightsByType( const std::vector< std::shared_ptr< Light > >& lights, const Light::Type type );

        template< typename ActorType >
        static std::vector< std::shared_ptr< ActorType > > filterActorsByType( const std::vector< std::shared_ptr< Actor > >& actors ); 
    };
}

