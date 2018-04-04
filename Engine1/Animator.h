#pragma once

#include <memory>
#include <map>
#include <vector>

#include "Animation.h"
#include "MathUtil.h"

namespace Engine1
{
    template <typename T>
    class Animator
    {
        public:

        Animator();
        ~Animator();

        void loadAnimationFromMemory( const std::shared_ptr< T >& obj, const std::vector< char >& data );
        void loadAnimationFromFile( const std::shared_ptr< T >& obj, const std::string& path );

        void saveAnimationToMemory( const std::shared_ptr< T >& obj, std::vector< char >& data );
        void saveAnimationToFile( const std::shared_ptr< T >& obj, const std::string& path );

        void update( float timeDelta );

        // Negative time is treated as last keyframe time + 1 second (or 0 if there are no keyframes yet).
        void addKeyframe( const std::shared_ptr< T >& obj, float time = -1.0f );

        void removeLastKeyframe( const std::shared_ptr< T >& obj );

        int getKeyframeCount( const std::shared_ptr< T >& obj );

        void setPlaying( const std::shared_ptr< T >& obj, const bool playing );
        void setPlaybackTime( const std::shared_ptr< T >& obj, const float time );
        void setSmoothstepInterpolation( const std::shared_ptr< T >& obj, bool smoothstep );

        bool isPlaying( const std::shared_ptr< T >& obj );

        float getSpeedMultiplier( const std::shared_ptr< T >& obj ) const;
        void  setSpeedMultiplier( const std::shared_ptr< T >& obj, const float speedMultiplier );

        // Deletes key frames for objects which got deleted (ref count = 0)
        void removeKeyframesForDeletedObjects();

        private:

        // #WARNING: Using weak_ptr in the map is potentially dangerous
        // as weak_ptr changes over time and that may influence the way it's sorted inside the map.
        // #TODO: Think how to overcome this problem. Use raw data ptr as key?
        std::map< 
            std::weak_ptr< T >, 
            Animation< T >,
            std::owner_less< std::weak_ptr< T > >
        > animations;
    };

    template< typename T >
    Animator< T >::Animator()
    {}

    template< typename T >
    Animator< T >::~Animator()
    {}

    template< typename T >
    void Animator< T >::loadAnimationFromMemory( const std::shared_ptr< T >& obj, const std::vector< char >& data )
    {
        auto animation = Animation< T >::createFromMemory( data.begin(), data.end() );

        std::weak_ptr< T > objWeakPtr = obj;

        // Insert or find the key.
        auto  result = animations.insert( std::make_pair( objWeakPtr, Animation< T >() ) );
        auto& anim   = result.first->second;

        anim.clearKeyframes();
        anim = std::move( *animation );
    }

    template< typename T >
    void Animator< T >::loadAnimationFromFile( const std::shared_ptr< T >& obj, const std::string& path )
    {
        auto animation = Animation< T >::createFromFile( path );

        std::weak_ptr< T > objWeakPtr = obj;

        // Insert or find the key.
        auto  result = animations.insert( std::make_pair( objWeakPtr, Animation< T >() ) );
        auto& anim   = result.first->second;

        anim.clearKeyframes();
        anim = std::move( *animation );
    }

    template< typename T >
    void Animator< T >::saveAnimationToMemory( const std::shared_ptr< T >& obj, std::vector< char >& data )
    {
        std::weak_ptr< T > objWeakPtr = obj;

        auto resultIt = animations.find( objWeakPtr );
        if ( resultIt == animations.end() )
            return;

        auto& anim = resultIt->second;

        anim.saveToMemory( data );
    }

    template< typename T >
    void Animator< T >::saveAnimationToFile( const std::shared_ptr< T >& obj, const std::string& path )
    {
        std::weak_ptr< T > objWeakPtr = obj;

        auto resultIt = animations.find( objWeakPtr );
        if ( resultIt == animations.end() )
            return;

        auto& anim = resultIt->second;

        anim.saveToFile( path );
    }

    template< typename T >
    void Animator< T >::update( float timeDelta )
    {
        // Limit time delta in case of pauses/debugging.
        timeDelta = std::min( timeDelta, 0.1f );

        // #TODO: Maybe remove only once a second - not at every update.
        removeKeyframesForDeletedObjects();

        for ( auto& animation : animations )
        {
            auto& objWeakPtr = animation.first;
            auto& anim       = animation.second;

            // Skip lights with 0 or 1 keyframes or the ones with animation disabled.
            if ( anim.getKeyframes().size() <= 1 || !anim.isEnabled())
                continue;

            const float animDuration = anim.getDuration();

            auto newPlaybackTime = anim.getCurrentPlaybackTime() + timeDelta * anim.getSpeedMultiplier() * anim.getCurrentPlaybackDirection();

            // Reverse the animation direction if finished playback.
            if (anim.getCurrentPlaybackDirection() >= 0.0f && newPlaybackTime > animDuration) 
            {
                newPlaybackTime = animDuration - (newPlaybackTime - animDuration);
                newPlaybackTime = std::max( 0.0f, newPlaybackTime );
                anim.setCurrentPlaybackDirection( anim.getCurrentPlaybackDirection() * -1.0f );
            } 
            else if (anim.getCurrentPlaybackDirection() < 0.0f && newPlaybackTime <= 0.0f) 
            {
                newPlaybackTime = -newPlaybackTime;
                newPlaybackTime = std::min( animDuration, newPlaybackTime );
                anim.setCurrentPlaybackDirection( anim.getCurrentPlaybackDirection() * -1.0f );
            }

            anim.setCurrentPlaybackTime( newPlaybackTime );

            // Find the closest keyframe before the desired playback time.
            int idx, prevIdx, nextIdx;
            if ( anim.getCurrentPlaybackDirection() > 0.0f ) 
            {
                for ( idx = 0; idx < (int)anim.getKeyframes().size(); ++idx ) 
                {
                    const float keyframeTime = std::get< 1 >( anim.getKeyframes()[ idx ] );

                    if ( keyframeTime > anim.getCurrentPlaybackTime() )
                        break;
                }

                prevIdx = idx - 1;
                nextIdx = idx;
            }
            else
            {
                for ( idx = (int)anim.getKeyframes().size() - 1; idx >= 0; --idx ) 
                {
                    const float keyframeTime = std::get< 1 >( anim.getKeyframes()[ idx ] );

                    if ( keyframeTime < anim.getCurrentPlaybackTime() )
                        break;
                }

                prevIdx = idx;
                nextIdx = idx + 1;
            }

            // Get keyframes before and after the desired playback time.
            const auto& obj1 = std::get< 0 >( anim.getKeyframes()[ prevIdx ] );
            const float time1  = std::get< 1 >( anim.getKeyframes()[ prevIdx ] );
            const auto& obj2 = std::get< 0 >( anim.getKeyframes()[ nextIdx ] );
            const float time2  = std::get< 1 >( anim.getKeyframes()[ nextIdx ] );

            auto obj = objWeakPtr.lock();

            // Check if light hasn't been deleted.
            if ( !obj )
                continue;

            // Interpolate the keyframes.
            float ratio = (anim.getCurrentPlaybackTime() - time1) / (time2 - time1);

            if ( anim.isSmoothstepInterpolated() )
                ratio = MathUtil::smoothstep(ratio);

            obj->setInterpolated( obj1, obj2, ratio );
        }
    }

    template< typename T >
    void Animator< T >::addKeyframe( const std::shared_ptr< T >& obj, float time )
    {
        std::weak_ptr< T > objWeakPtr = obj;

        // Insert or find the key.
        auto  result = animations.insert( std::make_pair( objWeakPtr, Animation< T >() ) );
        auto& anim   = result.first->second;

        if ( time < 0.0f && !anim.getKeyframes().empty() )
            time = std::get< 1 >( anim.getKeyframes().back() ) + 1.0f;
        else
            time = 0.0f;

        anim.addKeyframe( *obj, time );
    }

    template< typename T >
    void Animator< T >::removeLastKeyframe( const std::shared_ptr< T >& obj )
    {
        std::weak_ptr< T > objWeakPtr = obj;

        auto result = animations.find( objWeakPtr );
        if ( result != animations.end() )
        {
            auto& anim = result->second;

            if ( !anim.getKeyframes().empty() )
                anim.getKeyframes().pop_back();
        }
    }

    template< typename T >
    int Animator< T >::getKeyframeCount( const std::shared_ptr< T >& obj )
    {
        std::weak_ptr< T > objWeakPtr = obj;

        auto result = animations.find( objWeakPtr );
        if ( result != animations.end() )
        {
            auto& anim = result->second;

            return static_cast< int > ( anim.getKeyframes().size() );
        }

        return 0;
    }

    template< typename T >
    void Animator< T >::setPlaying( const std::shared_ptr< T >& obj, const bool playing )
    {
        std::weak_ptr< T > objWeakPtr = obj;

        auto resultIt = animations.find( objWeakPtr );
        if ( resultIt == animations.end() )
            return;

        auto& anim = resultIt->second;

        anim.setEnabled( playing );
    }

    template< typename T >
    void Animator< T >::setPlaybackTime( const std::shared_ptr< T >& obj, const float time )
    {
        std::weak_ptr< T > objWeakPtr = obj;

        auto resultIt = animations.find( objWeakPtr );
        if ( resultIt == animations.end() )
            return;

        auto& anim = resultIt->second;

        anim.setCurrentPlaybackTime( time );
    }

    template< typename T >
    void Animator< T >::setSmoothstepInterpolation( const std::shared_ptr< T >& obj, bool smoothstep )
    {
        std::weak_ptr< T > objWeakPtr = obj;

        auto resultIt = animations.find( objWeakPtr );
        if ( resultIt == animations.end() )
            return;

        auto& anim = resultIt->second;

        anim.setSmoothstepInterpolation( smoothstep );
    }

    template< typename T >
    bool Animator< T >::isPlaying( const std::shared_ptr< T >& obj )
    {
        std::weak_ptr< T > objWeakPtr = obj;

        auto resultIt = animations.find( objWeakPtr );
        if ( resultIt == animations.end() )
            return false;

        auto& anim = resultIt->second;

        return anim.isEnabled();
    }

    template< typename T >
    float Animator< T >::getSpeedMultiplier( const std::shared_ptr< T >& obj ) const
    {
        std::weak_ptr< T > objWeakPtr = obj;

        auto resultIt = animations.find( objWeakPtr );
        if ( resultIt == animations.end() )
            return 0.0f;

        auto& anim = resultIt->second;

        return anim.speedMultiplier;
    }

    template< typename T >
    void  Animator< T >::setSpeedMultiplier( const std::shared_ptr< T >& obj, const float speedMultiplier )
    {
        std::weak_ptr< T > objWeakPtr = obj;

        auto resultIt = animations.find( objWeakPtr );
        if ( resultIt == animations.end() )
            return;

        auto& anim = resultIt->second;

        anim.speedMultiplier = speedMultiplier;
    }

    template< typename T >
    void Animator< T >::removeKeyframesForDeletedObjects()
    {
        for ( auto it = animations.cbegin(); it != animations.cend(); ) 
        {
            if ( it->first.expired() )
                it = animations.erase( it );
            else
                ++it;
        }
    }

}

