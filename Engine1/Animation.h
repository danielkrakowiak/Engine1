#pragma once

#include <algorithm>

#include "AnimationParser.h"

namespace Engine1
{
    template <typename T>
    class Animation
    {
        template <typename T>
        friend class AnimationParser;

        public:

        static std::shared_ptr< Animation< T > > createFromFile( const std::string& path );
        static std::shared_ptr< Animation< T > > createFromMemory( std::vector< char >::const_iterator dataIt, std::vector< char >::const_iterator dataEndIt );

        Animation();

        void saveToMemory( std::vector< char >& data );
        void saveToFile( const std::string& path );

        bool isEnabled() const;

        void setEnabled( bool enabled );
        void setCurrentPlaybackTime( float playbackTime );
        void setCurrentPlaybackDirection( float playbackDirection );
        void setSmoothstepInterpolation( bool useSmoothstep );

        float getCurrentPlaybackTime() const;
        float getCurrentPlaybackDirection() const;

        float getDuration() const;
        float getSpeedMultiplier() const;

        bool  isSmoothstepInterpolated() const;

        void addKeyframe( const T& obj, float time );
        void clearKeyframes();

        std::vector< std::tuple< T, float > >& getKeyframes();

        private:

        std::vector< std::tuple< T, float > > m_keyframes;

        bool  m_enabled;
        int   m_lastUsedKeyframe;
        float m_speedMultiplier;
        float m_currentPlaybackTime;
        float m_currentPlaybackDirection; // Equals 1 or -1.
        bool  m_smoothstepInterpolation;
    };

    template <typename T>
    std::shared_ptr< Animation< T > > Animation< T >::createFromFile( const std::string& path )
    {
        std::shared_ptr< std::vector< char > > fileData = BinaryFile::load( path );

        std::shared_ptr< Animation< T > > animation = createFromMemory( fileData->cbegin(), fileData->cend() );

        return animation;
    }

    template <typename T>
    std::shared_ptr< Animation< T > > Animation< T >::createFromMemory( 
        std::vector< char >::const_iterator dataIt, 
        std::vector< char >::const_iterator dataEndIt )
    {
        return AnimationParser< T >::parseBinary( dataIt, dataEndIt );
    }

    template <typename T>
    Animation< T >::Animation() : 
        m_enabled(false),
        m_lastUsedKeyframe(0),
        m_speedMultiplier(1.0f),
        m_currentPlaybackTime(0.0f),
        m_currentPlaybackDirection(1.0f),
        m_smoothstepInterpolation(false)
    {}

    template <typename T>
    void Animation< T >::saveToMemory( std::vector< char >& data )
    {
        AnimationParser< T >::writeBinary( data, *this );
    }
         
    template <typename T>
    void Animation< T >::saveToFile( const std::string& path )
    {
        std::vector< char > data;

        AnimationParser< T >::writeBinary( data, *this );

        BinaryFile::save( path, data );
    }

    template <typename T>
    bool Animation< T >::isEnabled() const
    {
        return m_enabled;
    }

    template <typename T>
    void Animation< T >::setEnabled( bool enabled )
    {
        m_enabled = enabled;
    }

    template <typename T>
    void Animation< T >::setCurrentPlaybackTime( float playbackTime )
    {
        playbackTime = std::max( playbackTime, 0.0f );
        playbackTime = std::min( playbackTime, getDuration() );

        m_currentPlaybackTime = playbackTime;
    }

    template <typename T>
    void Animation< T >::setCurrentPlaybackDirection( float playbackDirection )
    {
        playbackDirection = std::max( playbackDirection, -1.0f );
        playbackDirection = std::min( playbackDirection, 1.0f );

        m_currentPlaybackDirection = (float)((int)playbackDirection);
    }

    template <typename T>
    void Animation< T >::setSmoothstepInterpolation( bool useSmoothstep )
    {
        m_smoothstepInterpolation = useSmoothstep;
    }

    template <typename T>
    float Animation< T >::getCurrentPlaybackTime() const
    {
        return m_currentPlaybackTime;
    }

    template <typename T>
    float Animation< T >::getCurrentPlaybackDirection() const
    {
        return m_currentPlaybackDirection;
    }

    template <typename T>
    float Animation< T >::getDuration() const
    {
        if ( m_keyframes.empty() )
            return 0.0f;
        else
            return std::get< 1 >( m_keyframes.back() );
    }

    template <typename T>
    float Animation< T >::getSpeedMultiplier() const
    {
        return m_speedMultiplier;
    }

    template <typename T>
    bool Animation< T >::isSmoothstepInterpolated() const
    {
        return m_smoothstepInterpolation;
    }

    template <typename T>
    void Animation< T >::addKeyframe( const T& obj, float time )
    {
        if (time > getDuration() || m_keyframes.empty())
        {
            m_keyframes.push_back( std::make_tuple( obj, time ) );
        }
        else
        {
            //#TODO: Insert the key frame somewhere in the middle.
        }
    }

    template <typename T>
    void Animation< T >::clearKeyframes()
    {
        m_keyframes.clear();
    }

    template <typename T>
    std::vector< std::tuple< T, float > >& Animation< T >::getKeyframes()
    {
        return m_keyframes;
    }
}