#include "Animator.h"

#include "MathUtil.h"

using namespace Engine1;

Animator::Animator() :
    m_time(0.0f)
{}

Animator::~Animator()
{}

void Animator::update( const float timeDelta )
{
    m_time += timeDelta;

    // #TODO: Maybe remove only once a second - not at every update.
    removeKeyframesForDeletedObjects();

    for ( auto& lightKeyframes : spotlightsKeyframes )
    {
        auto& lightWeakPtr = lightKeyframes.first;
        auto& animation    = lightKeyframes.second;
        auto& keyframes    = animation.keyframes;

        // Skip lights with 0 or 1 keyframes or the ones with animation disabled.
        if ( keyframes.size() <= 1 || !animation.enabled)
            continue;

        const float animDuration = std::get< 1 >( keyframes.back() );
        const float wrappedTime  = fmod( m_time * animation.speedMultiplier, animDuration );

        int playKeyframeIndex = 0;

        // Find the closest keyframe before the desired playback time.
        for (unsigned int i = 0; i < keyframes.size(); ++i )
        {
            const float keyframeTime = std::get< 1 >( keyframes[ i ] );

            if ( wrappedTime >= keyframeTime )
            {
                playKeyframeIndex = i;
                break;
            }
        }

        // Get keyframes before and after the desired playback time.
        // Note: We assume that there is always a keyframe after the found one, 
        // as we wrapped the time.
        const auto& light1 = std::get< 0 >( keyframes[ playKeyframeIndex ] );
        const float time1  = std::get< 1 >( keyframes[ playKeyframeIndex ] );
        const auto& light2 = std::get< 0 >( keyframes[ playKeyframeIndex + 1 ] );
        const float time2  = std::get< 1 >( keyframes[ playKeyframeIndex + 1 ] );

        auto light = lightWeakPtr.lock();

        // Check if light hasn't been deleted.
        if ( !light )
            continue;

        // Interpolate the keyframes.
        const float ratio = (wrappedTime - time1) / (time2 - time1);
        const float smoothRatio = MathUtil::smoothstep(ratio);
        light->setInterpolated( light1, light2, smoothRatio );
    }
}

void Animator::addKeyframe( std::shared_ptr< SpotLight >& light, float time )
{
    std::weak_ptr< SpotLight > lightWeakPtr = light; // #TODO: Created weak_ptr may be too different to match the one in the map... Check...

    // Insert or find the key.
    auto  result    = spotlightsKeyframes.insert( std::make_pair( lightWeakPtr, Animation() ) );
    auto& animation = result.first->second;
    auto& keyframes = animation.keyframes;

    if ( time < 0.0f && !keyframes.empty() )
        time = std::get< 1 >( keyframes.back() ) + 1.0f;
    else
        time = 0.0f;

    keyframes.push_back( std::make_tuple( *light, time ) );
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
