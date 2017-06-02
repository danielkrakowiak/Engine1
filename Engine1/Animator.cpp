#include "Animator.h"

#include "MathUtil.h"

using namespace Engine1;

Animator::Animator()
{}

Animator::~Animator()
{}

void Animator::update( float timeDelta )
{
    // Limit time delta in case of pauses/debugging.
    timeDelta = std::min( timeDelta, 0.1f );

    // #TODO: Maybe remove only once a second - not at every update.
    removeKeyframesForDeletedObjects();

    for ( auto& lightKeyframes : spotlightsKeyframes )
    {
        auto& lightWeakPtr = lightKeyframes.first;
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
        const auto& light1 = std::get< 0 >( anim.keyframes[ prevIdx ] );
        const float time1  = std::get< 1 >( anim.keyframes[ prevIdx ] );
        const auto& light2 = std::get< 0 >( anim.keyframes[ nextIdx ] );
        const float time2  = std::get< 1 >( anim.keyframes[ nextIdx ] );

        auto light = lightWeakPtr.lock();

        // Check if light hasn't been deleted.
        if ( !light )
            continue;

        // Interpolate the keyframes.
        float ratio = (anim.currentPlaybackTime - time1) / (time2 - time1);

        if ( anim.smoothstep )
            ratio = MathUtil::smoothstep(ratio);

        light->setInterpolated( light1, light2, ratio );
    }
}

void Animator::addKeyframe( std::shared_ptr< SpotLight >& light, float time )
{
    std::weak_ptr< SpotLight > lightWeakPtr = light;

    // Insert or find the key.
    auto  result = spotlightsKeyframes.insert( std::make_pair( lightWeakPtr, Animation() ) );
    auto& anim   = result.first->second;

    if ( time < 0.0f && !anim.keyframes.empty() )
        time = std::get< 1 >( anim.keyframes.back() ) + 1.0f;
    else
        time = 0.0f;

    anim.keyframes.push_back( std::make_tuple( *light, time ) );
}

void Animator::playPause( std::shared_ptr< SpotLight >& light )
{
    std::weak_ptr< SpotLight > lightWeakPtr = light;

    auto resultIt = spotlightsKeyframes.find( lightWeakPtr );
    if ( resultIt == spotlightsKeyframes.end() )
        return;

    auto& anim = resultIt->second;

    anim.enabled = !anim.enabled;
}

float Animator::getSpeedMultiplier( std::shared_ptr< SpotLight >& light ) const
{
    std::weak_ptr< SpotLight > lightWeakPtr = light;

    auto resultIt = spotlightsKeyframes.find( lightWeakPtr );
    if ( resultIt == spotlightsKeyframes.end() )
        return 0.0f;

    auto& anim = resultIt->second;

    return anim.speedMultiplier;
}

void  Animator::setSpeedMultiplier( std::shared_ptr< SpotLight >& light, const float speedMultiplier )
{
    std::weak_ptr< SpotLight > lightWeakPtr = light;

    auto resultIt = spotlightsKeyframes.find( lightWeakPtr );
    if ( resultIt == spotlightsKeyframes.end() )
        return;

    auto& anim = resultIt->second;

    anim.speedMultiplier = speedMultiplier;
}

void Animator::removeKeyframesForDeletedObjects()
{
    for ( auto it = spotlightsKeyframes.cbegin(); it != spotlightsKeyframes.cend(); ) 
    {
        if ( it->first.expired() )
            it = spotlightsKeyframes.erase( it );
        else
            ++it;
    }
}
