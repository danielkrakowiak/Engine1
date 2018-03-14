#pragma once

#include <memory>
#include <map>
#include <vector>

#include "MathUtil.h"

namespace Engine1
{
    template <typename T>
    class Animator
    {
        public:

        Animator();
        ~Animator();

        void update( float timeDelta );

        // Negative time is treated as last keyframe time + 1 second (or 0 if there are no keyframes yet).
        void addKeyframe( std::shared_ptr< T >& obj, float time = -1.0f );

        void playPause( std::shared_ptr< T >& obj );

        float getSpeedMultiplier( std::shared_ptr< T >& obj ) const;
        void  setSpeedMultiplier( std::shared_ptr< T >& obj, const float speedMultiplier );

        // Deletes key frames for objects which got deleted (ref count = 0)
        void removeKeyframesForDeletedObjects();

        private:

        struct Animation
        {
            std::vector< std::tuple< T, float > > keyframes;

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
            std::weak_ptr< T >, 
            Animation,
            std::owner_less< std::weak_ptr< T > >
        > keyframes;
    };

    template< typename T >
    Animator< T >::Animator()
    {}

    template< typename T >
    Animator< T >::~Animator()
    {}

    template< typename T >
    void Animator< T >::update( float timeDelta )
    {
        // Limit time delta in case of pauses/debugging.
        timeDelta = std::min( timeDelta, 0.1f );

        // #TODO: Maybe remove only once a second - not at every update.
        removeKeyframesForDeletedObjects();

        for ( auto& lightKeyframes : keyframes )
        {
            auto& objWeakPtr = lightKeyframes.first;
            auto& anim         = lightKeyframes.second;

            // Skip lights with 0 or 1 keyframes or the ones with animation disabled.
            if ( anim.keyframes.size() <= 1 || !anim.enabled)
                continue;

            const float animDuration = std::get< 1 >( anim.keyframes.back() );
            anim.currentPlaybackTime += timeDelta * anim.speedMultiplier * anim.currentPlaybackDirection;

            // Reverse the animation direction if finished playback.
            if (anim.currentPlaybackDirection >= 0.0f && anim.currentPlaybackTime > animDuration) 
            {
                anim.currentPlaybackTime = animDuration - (anim.currentPlaybackTime - animDuration);
                anim.currentPlaybackTime = std::max( 0.0f, anim.currentPlaybackTime );
                anim.currentPlaybackDirection *= -1.0f;
            } 
            else if (anim.currentPlaybackDirection < 0.0f && anim.currentPlaybackTime <= 0.0f) 
            {
                anim.currentPlaybackTime = -anim.currentPlaybackTime;
                anim.currentPlaybackTime = std::min( animDuration, anim.currentPlaybackTime );
                anim.currentPlaybackDirection *= -1.0f;
            }

            // Find the closest keyframe before the desired playback time.
            int idx, prevIdx, nextIdx;
            if ( anim.currentPlaybackDirection > 0.0f ) 
            {
                for ( idx = 0; idx < (int)anim.keyframes.size(); ++idx ) 
                {
                    const float keyframeTime = std::get< 1 >( anim.keyframes[ idx ] );

                    if ( keyframeTime > anim.currentPlaybackTime )
                        break;
                }

                prevIdx = idx - 1;
                nextIdx = idx;
            }
            else
            {
                for ( idx = (int)anim.keyframes.size() - 1; idx >= 0; --idx ) 
                {
                    const float keyframeTime = std::get< 1 >( anim.keyframes[ idx ] );

                    if ( keyframeTime < anim.currentPlaybackTime )
                        break;
                }

                prevIdx = idx;
                nextIdx = idx + 1;
            }

            // Get keyframes before and after the desired playback time.
            const auto& obj1 = std::get< 0 >( anim.keyframes[ prevIdx ] );
            const float time1  = std::get< 1 >( anim.keyframes[ prevIdx ] );
            const auto& obj2 = std::get< 0 >( anim.keyframes[ nextIdx ] );
            const float time2  = std::get< 1 >( anim.keyframes[ nextIdx ] );

            auto obj = objWeakPtr.lock();

            // Check if light hasn't been deleted.
            if ( !obj )
                continue;

            // Interpolate the keyframes.
            float ratio = (anim.currentPlaybackTime - time1) / (time2 - time1);

            if ( anim.smoothstep )
                ratio = MathUtil::smoothstep(ratio);

            obj->setInterpolated( obj1, obj2, ratio );
        }
    }

    template< typename T >
    void Animator< T >::addKeyframe( std::shared_ptr< T >& obj, float time )
    {
        std::weak_ptr< SpotLight > objWeakPtr = obj;

        // Insert or find the key.
        auto  result = keyframes.insert( std::make_pair( objWeakPtr, Animation() ) );
        auto& anim   = result.first->second;

        if ( time < 0.0f && !anim.keyframes.empty() )
            time = std::get< 1 >( anim.keyframes.back() ) + 1.0f;
        else
            time = 0.0f;

        anim.keyframes.push_back( std::make_tuple( *obj, time ) );
    }

    template< typename T >
    void Animator< T >::playPause( std::shared_ptr< T >& obj )
    {
        std::weak_ptr< SpotLight > objWeakPtr = obj;

        auto resultIt = keyframes.find( objWeakPtr );
        if ( resultIt == keyframes.end() )
            return;

        auto& anim = resultIt->second;

        anim.enabled = !anim.enabled;
    }

    template< typename T >
    float Animator< T >::getSpeedMultiplier( std::shared_ptr< T >& obj ) const
    {
        std::weak_ptr< SpotLight > objWeakPtr = obj;

        auto resultIt = keyframes.find( objWeakPtr );
        if ( resultIt == keyframes.end() )
            return 0.0f;

        auto& anim = resultIt->second;

        return anim.speedMultiplier;
    }

    template< typename T >
    void  Animator< T >::setSpeedMultiplier( std::shared_ptr< T >& obj, const float speedMultiplier )
    {
        std::weak_ptr< SpotLight > objWeakPtr = obj;

        auto resultIt = keyframes.find( objWeakPtr );
        if ( resultIt == keyframes.end() )
            return;

        auto& anim = resultIt->second;

        anim.speedMultiplier = speedMultiplier;
    }

    template< typename T >
    void Animator< T >::removeKeyframesForDeletedObjects()
    {
        for ( auto it = keyframes.cbegin(); it != keyframes.cend(); ) 
        {
            if ( it->first.expired() )
                it = keyframes.erase( it );
            else
                ++it;
        }
    }

}

