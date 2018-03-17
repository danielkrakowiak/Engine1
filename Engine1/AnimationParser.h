#pragma once

#include <memory>

namespace Engine1
{
    template< typename T >
    class AnimationParser
    {
        public:

        template< typename T > friend class Animation;

        private:

        static std::shared_ptr< Animation< T > > parseBinary( 
            std::vector< char >::const_iterator& dataIt, 
            std::vector< char >::const_iterator& dataEndIt 
        );

        static void writeBinary( std::vector< char >& data, const Animation< T >& animation );

    };

    template< typename T >
    std::shared_ptr< Animation< T > > AnimationParser< T >::parseBinary( 
        std::vector< char >::const_iterator& dataIt,
        std::vector< char >::const_iterator& dataEndIt )
    {
        const auto enabled                  = BinaryFile::readBool( dataIt );
        const auto lastUsedKeyframe         = BinaryFile::readInt( dataIt );
        const auto speedMultiplier          = BinaryFile::readFloat( dataIt );
        const auto currentPlaybackTime      = BinaryFile::readFloat( dataIt );
        const auto currentPlaybackDirection = BinaryFile::readFloat( dataIt );
        const auto smoothstepInterpolation  = BinaryFile::readBool( dataIt );

        auto animation = std::make_shared< Animation< T > >();

        animation->m_enabled                  = enabled;         
        animation->m_lastUsedKeyframe         = lastUsedKeyframe;    
        animation->m_speedMultiplier          = speedMultiplier;      
        animation->m_currentPlaybackTime      = currentPlaybackTime;    
        animation->m_currentPlaybackDirection = currentPlaybackDirection;
        animation->m_smoothstepInterpolation  = smoothstepInterpolation;

        const auto keyframeCount = BinaryFile::readInt( dataIt );

        for ( int keyframeIdx = 0; keyframeIdx < keyframeCount; ++keyframeIdx )
        {
            const auto keyframeTime = BinaryFile::readFloat( dataIt );
            std::shared_ptr< T > objPtr = T::createFromMemory( dataIt, dataEndIt );

            animation->addKeyframe( std::move( *objPtr ), keyframeTime );
        }

        return animation;
    }

    template< typename T >
    void AnimationParser< T >::writeBinary( std::vector<char>& data, const Animation< T >& animation )
    {
        BinaryFile::writeBool( data, animation.m_enabled );
        BinaryFile::writeInt( data, animation.m_lastUsedKeyframe );
        BinaryFile::writeFloat( data, animation.m_speedMultiplier );
        BinaryFile::writeFloat( data, animation.m_currentPlaybackTime );
        BinaryFile::writeFloat( data, animation.m_currentPlaybackDirection );
        BinaryFile::writeBool( data, animation.m_smoothstepInterpolation );

        BinaryFile::writeInt( data, static_cast< int >( animation.m_keyframes.size() ) );

        for ( const auto& keyframe : animation.m_keyframes )
        {
            const auto keyframeObj  = std::get< 0 >( keyframe );
            const auto keyframeTime = std::get< 1 >( keyframe );

            BinaryFile::writeFloat( data, keyframeTime );
            keyframeObj.saveToMemory( data );
        }
    }
}

