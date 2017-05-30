#include "Selection.h"

#include <algorithm>

#include "Actor.h"
#include "BlockActor.h"
#include "SkeletonActor.h"
#include "Light.h"
#include "PointLight.h"
#include "SpotLight.h"

using namespace Engine1;

bool Selection::add( const std::shared_ptr< Actor >& actor )
{
    if ( !actor || contains( actor ) )
        return false;

    m_actors.push_back( actor );

    if ( actor->getType() == Actor::Type::BlockActor )
        m_blockActors.push_back( std::static_pointer_cast< BlockActor >( actor ) );
    else if ( actor->getType() == Actor::Type::SkeletonActor )
        m_skeletonActors.push_back( std::static_pointer_cast< SkeletonActor >( actor ) );

    return true;
}

bool Selection::add( const std::vector< std::shared_ptr< Actor > >& actors )
{
    bool added = false;

    for ( auto& actor : actors )
        added |= add( actor );

    return added;
}

bool Selection::add( const std::vector< std::shared_ptr< BlockActor > >& actors )
{
    return add( reinterpret_cast< const std::vector< std::shared_ptr< Actor > >& >( actors ) );
}

bool Selection::add( const std::vector< std::shared_ptr< SkeletonActor > >& actors )
{
    return add( reinterpret_cast< const std::vector< std::shared_ptr< Actor > >& >( actors ) );
}

bool Selection::add( const std::shared_ptr< Light >& light )
{
    if ( !light || contains( light ))
        return false;

    m_lights.push_back( light );

    if ( light->getType() == Light::Type::PointLight )
        m_pointLights.push_back( std::static_pointer_cast< PointLight >( light ) );
    else if ( light->getType() == Light::Type::SpotLight )
        m_spotLights.push_back( std::static_pointer_cast< SpotLight >( light ) );

    return true;
}

bool Selection::add( const std::vector< std::shared_ptr< Light > >& lights )
{
    bool added = false;

    for ( auto& light : lights )
        added |= add( light );

    return added;
}

bool Selection::add( const std::vector< std::shared_ptr< PointLight > >& lights )
{
    return add( reinterpret_cast< const std::vector< std::shared_ptr< Light > >& >( lights ) );
}

bool Selection::add( const std::vector< std::shared_ptr< SpotLight > >& lights )
{
    return add( reinterpret_cast< const std::vector< std::shared_ptr< Light > >& >( lights ) );
}

void Selection::replace( const std::vector< std::shared_ptr< Actor > >& actors )
{
    clear();

    for ( auto& actor : actors ) 
    {
        m_actors.push_back( actor );

        if ( actor->getType() == Actor::Type::BlockActor )
            m_blockActors.push_back( std::static_pointer_cast< BlockActor >( actor ) );
        else if ( actor->getType() == Actor::Type::SkeletonActor )
            m_skeletonActors.push_back( std::static_pointer_cast< SkeletonActor >( actor ) );
    }
}

void Selection::replace( const std::vector< std::shared_ptr< Light > >& lights )
{
    clear();

    for ( auto& light : lights ) 
    {
        m_lights.push_back( light );

        if ( light->getType() == Light::Type::PointLight )
            m_pointLights.push_back( std::static_pointer_cast< PointLight >( light ) );
        else if ( light->getType() == Light::Type::SpotLight )
            m_spotLights.push_back( std::static_pointer_cast< SpotLight >( light ) );
    }
}

bool Selection::remove( const std::shared_ptr< Actor >& actor )
{
    if ( !actor )
        return false;

    auto it = std::find( m_actors.begin(), m_actors.end(), actor );
    if ( it != m_actors.end() )
        m_actors.erase( it );
    else
        return false;

    if ( actor->getType() == Actor::Type::BlockActor )
    {
        auto it = std::find( m_blockActors.begin(), m_blockActors.end(), actor );
        if ( it != m_blockActors.end() )
            m_blockActors.erase( it );
    }
    else if ( actor->getType() == Actor::Type::SkeletonActor )
    {
        auto it = std::find( m_skeletonActors.begin(), m_skeletonActors.end(), actor );
        if ( it != m_skeletonActors.end() )
            m_skeletonActors.erase( it );
    }

    return true;
}

bool Selection::remove( const std::shared_ptr< Light >& light )
{
    if ( !light )
        return false;

    auto it = std::find( m_lights.begin(), m_lights.end(), light );
    if ( it != m_lights.end() )
        m_lights.erase( it );
    else
        return false;

    if ( light->getType() == Light::Type::PointLight ) 
    {
        auto it = std::find( m_pointLights.begin(), m_pointLights.end(), light );
        if ( it != m_pointLights.end() )
            m_pointLights.erase( it );
    } 
    else if ( light->getType() == Light::Type::SpotLight ) 
    {
        auto it = std::find( m_spotLights.begin(), m_spotLights.end(), light );
        if ( it != m_spotLights.end() )
            m_spotLights.erase( it );
    }

    return true;
}

bool Selection::contains( const std::shared_ptr< Actor >& actor ) const
{
    if ( actor->getType() == Actor::Type::BlockActor )
        return std::find( m_blockActors.begin(), m_blockActors.end(), actor ) != m_blockActors.end();
    if ( actor->getType() == Actor::Type::SkeletonActor )
        return std::find( m_skeletonActors.begin(), m_skeletonActors.end(), actor ) != m_skeletonActors.end();

    return false;
}

bool Selection::contains( const std::shared_ptr< Light >& light ) const
{
    if ( light->getType() == Light::Type::PointLight )
        return std::find( m_pointLights.begin(), m_pointLights.end(), light ) != m_pointLights.end();
    if ( light->getType() == Light::Type::SpotLight )
        return std::find( m_spotLights.begin(), m_spotLights.end(), light ) != m_spotLights.end();

    return false;
}

bool Selection::isEmpty() const
{
    return m_actors.empty() && m_lights.empty();
}

void Selection::clear()
{
    clearActors();
    clearLights();
}

void Selection::clearActors()
{
    m_actors.clear();
    m_blockActors.clear();
    m_skeletonActors.clear();
}

void Selection::clearLights()
{
    m_lights.clear();
    m_pointLights.clear();
    m_spotLights.clear();
}

bool Selection::containsOnlyActors() const
{
    return !m_actors.empty() && m_lights.empty();
}

bool Selection::containsOnlyOneActor() const
{
    return m_actors.size() == 1 && m_lights.empty();
}

bool Selection::containsOnlyOneBlockActor() const
{
    return m_blockActors.size() == 1 && m_skeletonActors.empty() && m_lights.empty();
}

bool Selection::containsOnlyOneSkeletonActor() const
{
    return m_skeletonActors.size() == 1 && m_blockActors.empty() && m_lights.empty();
}

bool Selection::containsOnlyLights() const
{
    return !m_lights.empty() && m_actors.empty();
}

bool Selection::containsOnlyOneLight() const
{
    return m_lights.size() == 1 && m_actors.empty();
}

bool Selection::containsOnlyOnePointLight() const
{
    return m_pointLights.size() == 1 && m_spotLights.empty() && m_actors.empty();
}

bool Selection::containsOnlyOneSpotLight() const
{
    return m_spotLights.size() == 1 && m_pointLights.empty() && m_actors.empty();
}

const std::vector< std::shared_ptr< Actor > >& Selection::getActors() const
{
    return m_actors;
}

const std::vector< std::shared_ptr< BlockActor > >& Selection::getBlockActors() const
{
    return m_blockActors;
}

const std::vector< std::shared_ptr< SkeletonActor > >& Selection::getSkeletonActors() const
{
    return m_skeletonActors;
}

const std::vector< std::shared_ptr< Light > >& Selection::getLights() const
{
    return m_lights;
}

const std::vector< std::shared_ptr< PointLight > >& Selection::getPointLights() const
{
    return m_pointLights;
}

const std::vector< std::shared_ptr< SpotLight > >& Selection::getSpotLights() const
{
    return m_spotLights;
}