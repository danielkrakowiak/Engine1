#pragma once

#include <vector>
#include <memory>

namespace Engine1
{
    class Actor;
    class BlockActor;
    class SkeletonActor;
    class Light;
    class PointLight;
    class SpotLight;

    class Selection
    {
        public:

        bool add( const std::shared_ptr< Actor >& actor );
        bool add( const std::vector< std::shared_ptr< Actor > >& actors );
        bool add( const std::vector< std::shared_ptr< BlockActor > >& actors );
        bool add( const std::vector< std::shared_ptr< SkeletonActor > >& actors );
        bool add( const std::shared_ptr< Light >& light );
        bool add( const std::vector< std::shared_ptr< Light > >& lights );
        bool add( const std::vector< std::shared_ptr< PointLight > >& lights );
        bool add( const std::vector< std::shared_ptr< SpotLight > >& lights );

        void replace( const std::vector< std::shared_ptr< Actor > >& actors );
        void replace( const std::vector< std::shared_ptr< Light > >& lights );

        bool remove( const std::shared_ptr< Actor >& actor );
        bool remove( const std::shared_ptr< Light >& light );

        bool contains( const std::shared_ptr< Actor >& actor ) const;
        bool contains( const std::shared_ptr< Light >& light ) const;

        bool isEmpty() const;

        void clear();
        void clearActors();
        void clearLights();

        bool containsOnlyActors() const;
        bool containsOnlyOneActor() const;
        bool containsOnlyOneBlockActor() const;
        bool containsOnlyOneSkeletonActor() const;
        bool containsOnlyLights() const;
        bool containsOnlyOneLight() const;
        bool containsOnlyOnePointLight() const;
        bool containsOnlyOneSpotLight() const;

        const std::vector< std::shared_ptr< Actor > >&         getActors() const;
        const std::vector< std::shared_ptr< BlockActor > >&    getBlockActors() const;
        const std::vector< std::shared_ptr< SkeletonActor > >& getSkeletonActors() const;
        const std::vector< std::shared_ptr< Light > >&         getLights() const;
        const std::vector< std::shared_ptr< PointLight > >&    getPointLights() const;
        const std::vector< std::shared_ptr< SpotLight > >&     getSpotLights() const;

        private:

        std::vector< std::shared_ptr< Actor > >         m_actors;
        std::vector< std::shared_ptr< BlockActor > >    m_blockActors;
        std::vector< std::shared_ptr< SkeletonActor > > m_skeletonActors;
        std::vector< std::shared_ptr< Light > >         m_lights;
        std::vector< std::shared_ptr< PointLight > >    m_pointLights;
        std::vector< std::shared_ptr< SpotLight > >     m_spotLights;
    };
}

