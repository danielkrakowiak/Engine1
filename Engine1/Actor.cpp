#include "Actor.h"

using namespace Engine1;

Actor::Actor() :
    m_castsShadow( true )
{
}

void Actor::setCastingShadows( bool castShadows )
{
    m_castsShadow = castShadows;
}

bool Actor::isCastingShadows() const
{
    return m_castsShadow;
}