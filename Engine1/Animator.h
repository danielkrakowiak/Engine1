#pragma once

#include <memory>
#include <map>
#include <vector>

namespace Engine1
{
    class SpotLight;

    class Animator
    {
        public:

        Animator();
        ~Animator();

        void update( const float timeDelta );

        // Negative time is treated as last keyframe time + 1 second (or 0 if there are no keyframes yet).
        void addKeyframe( std::shared_ptr< SpotLight >& light, float time = -1.0f );

        // Deletes key frames for objects which got deleted (ref count = 0)
        void removeKeyframesForDeletedObjects();

        private:

        float m_time;

        // #WARNING: Using weak_ptr in the map is potentially dangerous
        // as weak_ptr changes over time and that may influence the way it's sorted inside the map.
        // #TODO: Think how to overcome this problem. Use raw data ptr as key?
        std::map< 
            std::weak_ptr< SpotLight >, 
            std::vector< std::tuple< SpotLight, float > >,
            std::owner_less< std::weak_ptr< SpotLight > >
        > spotlightsKeyframes;
    };
}

