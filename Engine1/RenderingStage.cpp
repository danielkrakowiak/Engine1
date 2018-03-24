#include "RenderingStage.h"

#include <assert.h>

using namespace Engine1;

RenderingStageType Engine1::getLastRenderingStageType( const RenderingStage stage )
{
    if (stage == RenderingStage::Main)
        return RenderingStageType::Main;
    else if (((int)stage & 0b1) == 1) // Check last bit.
        return RenderingStageType::Transmission;
    else
        return RenderingStageType::Reflection;
}

RenderingStage Engine1::getNextRenderingStage( const RenderingStage stage, const RenderingStageType nextStageType )
{
    // There is no rendering stage that can progress to Main stage.
    assert( nextStageType != RenderingStageType::Main );
    if (nextStageType == RenderingStageType::Main)
        return stage;

    const auto stageInt = (int)stage << 1;
    const auto nextBit = (int)(nextStageType == RenderingStageType::Transmission);

    const auto nextStageInt = stageInt | nextBit;

    return (RenderingStage)nextStageInt;
}

RenderingStage Engine1::getPrevRenderingStage( const RenderingStage stage )
{
    // There is no rendering stage previous to Main stage.
    assert( stage != RenderingStage::Main );
    if (stage == RenderingStage::Main)
        return RenderingStage::Main;

    const auto prevStageInt = (int)stage >> 1;

    return (RenderingStage)prevStageInt;
}

int Engine1::getRenderingStageLevel( const RenderingStage stage )
{
    return (int)log2((float)stage);
}

int Engine1::getRenderingStageRefractionLevelCount( RenderingStage stage )
{
    int refractionLevel = -1; // -1 as we don't count Main as Refraction (despite having bit 1 at the end).

    for (int i = 0; i < sizeof(RenderingStage) * 8; ++i)
    {
        const auto transmission = ((int)stage & 0b1) == 1;

        if ( transmission )
            ++refractionLevel;

        stage = (RenderingStage)((int)stage >> 1);
    }

    return refractionLevel;
}

std::string Engine1::renderingStageToString( const RenderingStage stageType )
{
    switch (stageType)
    {
        case RenderingStage::Main:    return "Main";
        case RenderingStage::R:       return "R";
        case RenderingStage::T:       return "T";
        case RenderingStage::RR:      return "RR";
        case RenderingStage::RT:      return "RT";
        case RenderingStage::TR:      return "TR";
        case RenderingStage::TT:      return "TT";
        case RenderingStage::RRR:     return "RRR";
        case RenderingStage::RRT:     return "RRT";
        case RenderingStage::RTR:     return "RTR";
        case RenderingStage::RTT:     return "RTT";
        case RenderingStage::TRR:     return "TRR";
        case RenderingStage::TRT:     return "TRT";
        case RenderingStage::TTR:     return "TTR";
        case RenderingStage::TTT:     return "TTT";
        case RenderingStage::RRRR:    return "RRRR";
        case RenderingStage::RRRT:    return "RRRT";
        case RenderingStage::RRTR:    return "RRTR";
        case RenderingStage::RRTT:    return "RRTT";
        case RenderingStage::RTRR:    return "RTRR";
        case RenderingStage::RTRT:    return "RTRT";
        case RenderingStage::RTTR:    return "RTTR";
        case RenderingStage::RTTT:    return "RTTT";
        case RenderingStage::TRRR:    return "TRRR";
        case RenderingStage::TRRT:    return "TRRT";
        case RenderingStage::TRTR:    return "TRTR";
        case RenderingStage::TRTT:    return "TRTT";
        case RenderingStage::TTRR:    return "TTRR";
        case RenderingStage::TTRT:    return "TTRT";
        case RenderingStage::TTTR:    return "TTTR";
        case RenderingStage::TTTT:    return "TTTT";
    }

    return "";
}

