#pragma once

#include <memory>
#include <map>
#include <vector>

#include "PointLight.h"
#include "SpotLight.h"

namespace Engine1
{
    class SpotLight;

    class Animator
    {
        public:

        Animator();
        ~Animator();

        void update( float timeDelta );

        // Negative time is treated as last keyframe time + 1 second (or 0 if there are no keyframes yet).
        void addKeyframe( std::shared_ptr< SpotLight >& light, float time = -1.0f );

        void playPause( std::shared_ptr< SpotLight >& light );

        float getSpeedMultiplier( std::shared_ptr< SpotLight >& light ) const;
        void  setSpeedMultiplier( std::shared_ptr< SpotLight >& light, const float speedMultiplier );

        // Deletes key frames for objects which got deleted (ref count = 0)
        void removeKeyframesForDeletedObjects();

        private:

        struct Animation
        {
            std::vector< std::tuple< SpotLight, float > > keyframes;

            bool  enabled;
            int   lastUsedKeyframe;
            float speedMultiplier;
            float currentPlaybackTime;
            float currentPlaybackDirection; // Equals 1 or -1.
            bool  smoothstep;

            Animation() : 
                enabled(false),
                lastUsedKeyframe(0),
                speedMultiplier(1.0f),
                currentPlaybackTime(0.0f),
                currentPlaybackDirection(1.0f),
                smoothstep(false)
            {}
        };

        // #WARNING: Using weak_ptr in the map is potentially dangerous
        // as weak_ptr changes over time and that may influence the way it's sorted inside the map.
        // #TODO: Think how to overcome this problem. Use raw data ptr as key?
        std::map< 
            std::weak_ptr< SpotLight >, 
            Animation,
            std::owner_less< std::weak_ptr< SpotLight > >
        > spotlightsKeyframes;
    };
}

