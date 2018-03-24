#pragma once

#include <string>

namespace Engine1
{
    enum class RenderingStageType : int
    {
        Main         = 0,
        Reflection   = 1,
        Transmission = 2
    };

    // Note: Main - main image, R - reflection, T - transmission (refraction).
    enum class RenderingStage : int
    {
        None = 0,
        Main = 1,
        R = 0b10,
        T = 0b11,
        // Level 2
        RR = 0b100,
        RT = 0b101,
        TR = 0b110,
        TT = 0b111,
        // Level 3
        RRR = 0b1000,
        RRT = 0b1001,
        RTR = 0b1010,
        RTT = 0b1011,
        TRR = 0b1100,
        TRT = 0b1101,
        TTR = 0b1110,
        TTT = 0b1111,
        // Level 4
        RRRR = 0b10000,
        RRRT = 0b10001,
        RRTR = 0b10010,
        RRTT = 0b10011,
        RTRR = 0b10100,
        RTRT = 0b10101,
        RTTR = 0b10110,
        RTTT = 0b10111,
        TRRR = 0b11000,
        TRRT = 0b11001,
        TRTR = 0b11010,
        TRTT = 0b11011,
        TTRR = 0b11100,
        TTRT = 0b11101,
        TTTR = 0b11110,
        TTTT = 0b11111,
        MAX_VALUE = 32 // There can be more stages than this, but only that many have label (and are important in most cases).
    };

    RenderingStageType getLastRenderingStageType( const RenderingStage stage );
    RenderingStage     getNextRenderingStage( const RenderingStage stage, const RenderingStageType nextStageType );
    RenderingStage     getPrevRenderingStage( const RenderingStage stage );
    int                getRenderingStageLevel( const RenderingStage stage );
    int                getRenderingStageRefractionLevelCount( RenderingStage stage );

    std::string renderingStageToString( const RenderingStage stageType );
}
