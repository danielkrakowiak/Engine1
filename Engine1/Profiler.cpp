#include "Profiler.h"

#include <d3d11.h>

#include <exception>
#include <string>

using Microsoft::WRL::ComPtr;

using namespace Engine1;

std::string Profiler::stageTypeToString( const StageType stageType )
{
    switch (stageType)
    {
        case StageType::Main: return "Main";
        case StageType::R:    return "R";
        case StageType::T:    return "T";
        case StageType::RR:    return "RR";
        case StageType::RT:    return "RT";
        case StageType::TR:    return "TR";
        case StageType::TT:    return "TT";
        case StageType::RRR:    return "RRR";
        case StageType::RRT:    return "RRT";
        case StageType::RTR:    return "RTR";
        case StageType::RTT:    return "RTT";
        case StageType::TRR:    return "TRR";
        case StageType::TRT:    return "TRT";
        case StageType::TTR:    return "TTR";
        case StageType::TTT:    return "TTT";
        case StageType::RRRR:    return "RRRR";
        case StageType::RRRT:    return "RRRT";
        case StageType::RRTR:    return "RRTR";
        case StageType::RRTT:    return "RRTT";
        case StageType::RTRR:    return "RTRR";
        case StageType::RTRT:    return "RTRT";
        case StageType::RTTR:    return "RTTR";
        case StageType::RTTT:    return "RTTT";
        case StageType::TRRR:    return "TRRR";
        case StageType::TRRT:    return "TRRT";
        case StageType::TRTR:    return "TRTR";
        case StageType::TRTT:    return "TRTT";
        case StageType::TTRR:    return "TTRR";
        case StageType::TTRT:    return "TTRT";
        case StageType::TTTR:    return "TTTR";
        case StageType::TTTT:    return "TTTT";
    }

    return "";
}

std::string Profiler::eventTypeToString( const GlobalEventType eventType )
{
    switch (eventType)
    {
        case GlobalEventType::Frame:                        return "Frame";
        case GlobalEventType::DeferredRendering:            return "DeferredRendering";
        case GlobalEventType::Bloom:                        return "Bloom";
        case GlobalEventType::ToneMapping:                  return "ToneMapping";
        case GlobalEventType::CopyFrameToFinalRenderTarget: return "CopyFrameToFinalRenderTarget";
    }

    return "";
}

std::string Profiler::eventTypeToString( const EventTypePerStage eventType )
{
    switch ( eventType ) 
    {
        case EventTypePerStage::MipmapGenerationForPositionAndNormals: return "MipmapGenerationForPositionAndNormals";
        case EventTypePerStage::EmissiveShading:                       return "EmissiveShading";
        case EventTypePerStage::ReflectionTransmissionShading:         return "ReflectionTransmissionShading";
        case EventTypePerStage::Raytracing:                            return "Raytracing";
        case EventTypePerStage::ShadingNoShadows:                      return "ShadingNoShadows";
        case EventTypePerStage::Shading:                               return "Shading";
        case EventTypePerStage::MipmapGenerationForShadedImage:        return "MipmapGenerationForShadedImage";
        case EventTypePerStage::CombiningWithMainImage:                return "CombiningWithMainImage";
    }

    return "";
}

std::string Profiler::eventTypeToString( const EventTypePerStagePerLight eventType, const int lightIdx )
{
    switch ( eventType ) 
    {
        case EventTypePerStagePerLight::ShadowsMapping:                                     return "ShadowMapping" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
        case EventTypePerStagePerLight::RaytracingShadows:                                  return "RaytracingShadows" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
        case EventTypePerStagePerLight::MipmapGenerationForPreillumination:                 return "MipmapGenerationForPreillumination" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
        case EventTypePerStagePerLight::MipmapGenerationForIllumination:                    return "MipmapGenerationForIllumination" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
        case EventTypePerStagePerLight::MipmapMinimumValueGenerationForDistanceToOccluder:  return "MipmapMinimumValueGenerationForDistanceToOccluder" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
        case EventTypePerStagePerLight::DistanceToOccluderSearch:                           return "DistanceToOccluderSearch" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
        case EventTypePerStagePerLight::BlurShadows:                                        return "BlurShadows" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
        case EventTypePerStagePerLight::Shadows:                                            return "Shadows" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
        case EventTypePerStagePerLight::Shading:                                            return "Shading" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
    }

    return "";
}

Profiler::Profiler() :
    m_profilingPaused( true ),
    m_currentSubmitQueryFrameIndex( -1 ),
    m_currentSaveResultsFrameIndex( -queryFrameCount + 1 ),
    m_currentReadResultsFrameIndex( -1 )
{}

Profiler::~Profiler()
{}

void Profiler::initialize( ComPtr< ID3D11Device > device, ComPtr< ID3D11DeviceContext > deviceContext )
{
    m_device = device;
    m_deviceContext = deviceContext;

    // Create timestamp queries.
    for ( int frameIdx = 0; frameIdx < queryFrameCount; ++frameIdx ) 
    {   
        // Create disjoint query.
        m_disjointQueries[ frameIdx ] = createDisjointQuery( *device.Get() );

        // Create timestamp queries for global events.
        for ( int eventTypeIdx = 0; eventTypeIdx < (int)GlobalEventType::MAX_VALUE; ++eventTypeIdx )
            m_globalEvents[ frameIdx ][ eventTypeIdx ] = createEvent( *m_device.Get() );

        // Create timestamp queries for events occurring at each rendering stage.
        for ( int stageIdx = 0; stageIdx < (int)StageType::MAX_VALUE; ++stageIdx ) 
        {
            for ( int eventTypeIdx = 0; eventTypeIdx < (int)EventTypePerStage::MAX_VALUE; ++eventTypeIdx )
                m_eventsPerStage[ frameIdx ][ stageIdx ][ eventTypeIdx ] = createEvent( *m_device.Get() );
        }

        // Create timestamp queries for events occurring at each rendering stage for each light source.
        for ( int stageIdx = 0; stageIdx < (int)StageType::MAX_VALUE; ++stageIdx ) 
        {
            for ( int lightIdx = 0; lightIdx < s_maxLightCount; ++lightIdx ) 
            {
                for ( int eventTypeIdx = 0; eventTypeIdx < (int)EventTypePerStagePerLight::MAX_VALUE; ++eventTypeIdx )
                    m_eventsPerStagePerLight[ frameIdx ][ stageIdx ][ lightIdx ][ eventTypeIdx ] = createEvent( *m_device.Get() );
            }
        }
    }
}

void Profiler::beginEvent( const GlobalEventType event )
{
    if ( m_profilingPaused )
        return;

    m_deviceContext->End( m_globalEvents[ m_currentSubmitQueryFrameIndex ][ (int)event ].queryBegin.Get() );
}

void Profiler::endEvent( const GlobalEventType event )
{
    if ( m_profilingPaused )
        return;

    m_globalEvents[ m_currentSubmitQueryFrameIndex ][ (int)event ].measured = true;

    m_deviceContext->End( m_globalEvents[ m_currentSubmitQueryFrameIndex ][ (int)event ].queryEnd.Get() );
}

void Profiler::beginEvent( const StageType stage, const EventTypePerStage event )
{
    if ( m_profilingPaused )
        return;

     m_deviceContext->End( m_eventsPerStage[ m_currentSubmitQueryFrameIndex ][ (int)stage ][ (int)event ].queryBegin.Get() );
}

void Profiler::endEvent( const StageType stage, const EventTypePerStage event )
{
    if ( m_profilingPaused )
        return;

    m_eventsPerStage[ m_currentSubmitQueryFrameIndex ][ (int)stage ][ (int)event ].measured = true;

    m_deviceContext->End( m_eventsPerStage[ m_currentSubmitQueryFrameIndex ][ (int)stage ][ (int)event ].queryEnd.Get() );
}

void Profiler::beginEvent( const StageType stage, const int lightIndex, const EventTypePerStagePerLight event )
{
    if ( m_profilingPaused )
        return;

    if ( lightIndex < 0 || lightIndex >= s_maxLightCount )
        return;

    m_deviceContext->End( m_eventsPerStagePerLight[ m_currentSubmitQueryFrameIndex ][ (int)stage ][ lightIndex ][ (int)event ].queryBegin.Get() );
}

void Profiler::endEvent( const StageType stage, const int lightIndex, const EventTypePerStagePerLight event )
{
    if ( m_profilingPaused )
        return;

    if ( lightIndex < 0 || lightIndex >= s_maxLightCount )
        return;

    m_eventsPerStagePerLight[ m_currentSubmitQueryFrameIndex ][ (int)stage ][ lightIndex ][ (int)event ].measured = true;

    m_deviceContext->End( m_eventsPerStagePerLight[ m_currentSubmitQueryFrameIndex ][ (int)stage ][ lightIndex ][ (int)event ].queryEnd.Get() );
}

void Profiler::beginFrameProfiling()
{
    m_profilingPaused = false;

    ++m_currentSubmitQueryFrameIndex;
    m_currentSubmitQueryFrameIndex = m_currentSubmitQueryFrameIndex % queryFrameCount;

    ++m_currentSaveResultsFrameIndex;
    m_currentSaveResultsFrameIndex = m_currentSaveResultsFrameIndex % queryFrameCount;

    resetEvents( m_currentSubmitQueryFrameIndex );

    m_deviceContext->Begin( m_disjointQueries[ m_currentSubmitQueryFrameIndex ].Get() );
}

void Profiler::endFrameProfiling()
{
    if ( m_profilingPaused )
        return;

    m_deviceContext->End( m_disjointQueries[ m_currentSubmitQueryFrameIndex ].Get() );

    // Save the results if they are ready - otherwise ignore the queries.
    if ( m_currentSaveResultsFrameIndex >= 0 && m_deviceContext->GetData( m_disjointQueries[ m_currentSaveResultsFrameIndex ].Get(), NULL, 0, 0) >= 0 )
    {
        // Check if GPU clock frequency changed during the frame - if so, ignore the queries' results.
        D3D10_QUERY_DATA_TIMESTAMP_DISJOINT disjointQueryResult;
        m_deviceContext->GetData(  m_disjointQueries[ m_currentSaveResultsFrameIndex ].Get(), &disjointQueryResult, sizeof( disjointQueryResult ), 0 );

        if ( !disjointQueryResult.Disjoint ) 
        {
            UINT64 begin, end;

            // Save global events duration.
            for ( int eventTypeIdx = 0; eventTypeIdx < (int)GlobalEventType::MAX_VALUE; ++eventTypeIdx )
            {
                if ( m_globalEvents[ m_currentSaveResultsFrameIndex ][ eventTypeIdx ].measured )
                {
                    m_deviceContext->GetData( m_globalEvents[ m_currentSaveResultsFrameIndex ][ eventTypeIdx ].queryBegin.Get(), &begin, sizeof( UINT64 ), 0 );
                    m_deviceContext->GetData( m_globalEvents[ m_currentSaveResultsFrameIndex ][ eventTypeIdx ].queryEnd.Get(), &end, sizeof( UINT64 ), 0 );

                    // Save event duration.
                    m_globalEvents[ m_currentSaveResultsFrameIndex ][ eventTypeIdx ].durationMilliseconds = float( end - begin ) / float( disjointQueryResult.Frequency ) * 1000.0f;
                }
            }

            // Save events per stage duration.
            for ( int stageIdx = 0; stageIdx < (int)StageType::MAX_VALUE; ++stageIdx ) 
            {
                for ( int eventTypeIdx = 0; eventTypeIdx < (int)EventTypePerStage::MAX_VALUE; ++eventTypeIdx ) 
                {
                    if ( m_eventsPerStage[ m_currentSaveResultsFrameIndex ][ stageIdx ][ eventTypeIdx ].measured )
                    {
                        m_deviceContext->GetData( m_eventsPerStage[ m_currentSaveResultsFrameIndex ][ stageIdx ][ eventTypeIdx ].queryBegin.Get(), &begin, sizeof( UINT64 ), 0 );
                        m_deviceContext->GetData( m_eventsPerStage[ m_currentSaveResultsFrameIndex ][ stageIdx ][ eventTypeIdx ].queryEnd.Get(), &end, sizeof( UINT64 ), 0 );

                        m_eventsPerStage[ m_currentSaveResultsFrameIndex ][ stageIdx ][ eventTypeIdx ].durationMilliseconds = float( end - begin ) / float( disjointQueryResult.Frequency ) * 1000.0f;
                    }
                }
            }

            // Save events per stage per light duration.
            for ( int stageIdx = 0; stageIdx < (int)StageType::MAX_VALUE; ++stageIdx ) 
            {
                for ( int lightIdx = 0; lightIdx < s_maxLightCount; ++lightIdx ) 
                {
                    for ( int eventTypeIdx = 0; eventTypeIdx < (int)EventTypePerStagePerLight::MAX_VALUE; ++eventTypeIdx ) 
                    {
                        if ( m_eventsPerStagePerLight[ m_currentSaveResultsFrameIndex ][ stageIdx ][ lightIdx ][ eventTypeIdx ].measured )
                        {
                            m_deviceContext->GetData( m_eventsPerStagePerLight[ m_currentSaveResultsFrameIndex ][ stageIdx ][ lightIdx ][ eventTypeIdx ].queryBegin.Get(), &begin, sizeof( UINT64 ), 0 );
                            m_deviceContext->GetData( m_eventsPerStagePerLight[ m_currentSaveResultsFrameIndex ][ stageIdx ][ lightIdx ][ eventTypeIdx ].queryEnd.Get(), &end, sizeof( UINT64 ), 0 );

                            m_eventsPerStagePerLight[ m_currentSaveResultsFrameIndex ][ stageIdx ][ lightIdx ][ eventTypeIdx ].durationMilliseconds = float( end - begin ) / float( disjointQueryResult.Frequency ) * 1000.0f;
                        }
                    }
                }
            }
        }
        else
        {
            resetEvents( m_currentSaveResultsFrameIndex );
        }
    }

    // Results are ready and can be read.
    m_currentReadResultsFrameIndex = m_currentSaveResultsFrameIndex;
}

void Profiler::pauseProfiling()
{
    m_profilingPaused = true;
}

// Negative value means that event hasn't occurred.
float Profiler::getEventDuration( const GlobalEventType event )
{
    if ( m_currentReadResultsFrameIndex < 0 )
        return 0.0f; // Results are not ready yet in the first few frames.

    return m_globalEvents[ m_currentReadResultsFrameIndex ][ (int)event ].durationMilliseconds;
}

// Negative value means that event hasn't occurred.
float Profiler::getEventDuration( const StageType stage, const EventTypePerStage event )
{
    if ( m_currentReadResultsFrameIndex < 0 )
        return 0.0f; // Results are not ready yet in the first few frames.

    return m_eventsPerStage[ m_currentReadResultsFrameIndex ][ (int)stage ][ (int)event ].durationMilliseconds;
}

// Negative value means that event hasn't occurred.
float Profiler::getEventDuration( const StageType stage, const int lightIndex, const EventTypePerStagePerLight event )
{
    if ( lightIndex >= s_maxLightCount )
        throw std::exception( "Profiler::getEventDuration - given lightIndex exceeds the number of profiled lights." );

    if ( m_currentReadResultsFrameIndex < 0 )
        return 0.0f; // Results are not ready yet in the first few frames.

    return m_eventsPerStagePerLight[ m_currentReadResultsFrameIndex ][ (int)stage ][ (int)lightIndex ][ (int)event ].durationMilliseconds;
}

Profiler::Event Profiler::createEvent( ID3D11Device& device )
{
    Event event;
    event.measured             = false;
    event.queryBegin           = createTimestampQuery( device );
    event.queryEnd             = createTimestampQuery( device );
    event.durationMilliseconds = 0.0f;

    return event;
}

ComPtr< ID3D11Query > Profiler::createTimestampQuery( ID3D11Device& device )
{
    D3D11_QUERY_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Query = D3D11_QUERY_TIMESTAMP;

    ComPtr< ID3D11Query > query;
    HRESULT result = device.CreateQuery( &desc, query.ReleaseAndGetAddressOf() );
    if ( result < 0 )
        throw std::exception( "Failed to create a timestamp query." );

    return query;
}

Microsoft::WRL::ComPtr< ID3D11Query > Profiler::createDisjointQuery( ID3D11Device& device )
{
    D3D11_QUERY_DESC desc;
    ZeroMemory( &desc, sizeof( desc ) );
    desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;

    ComPtr< ID3D11Query > query;
    HRESULT result = device.CreateQuery( &desc, query.ReleaseAndGetAddressOf() );
    if ( result < 0 )
        throw std::exception( "Failed to create a disjoint query." );

    return query;
}

void Profiler::resetEvents( const int frameIdx )
{
    // Reset global events duration.
    for ( int eventTypeIdx = 0; eventTypeIdx < (int)GlobalEventType::MAX_VALUE; ++eventTypeIdx ) 
    {
        m_globalEvents[ frameIdx ][ eventTypeIdx ].measured             = false;
        m_globalEvents[ frameIdx ][ eventTypeIdx ].durationMilliseconds = -1.0f;
    }

    // Reset events per stage duration.
    for ( int stageIdx = 0; stageIdx < (int)StageType::MAX_VALUE; ++stageIdx ) {
        for ( int eventTypeIdx = 0; eventTypeIdx < (int)EventTypePerStage::MAX_VALUE; ++eventTypeIdx ) 
        {
            m_eventsPerStage[ frameIdx ][ stageIdx ][ eventTypeIdx ].measured             = false;
            m_eventsPerStage[ frameIdx ][ stageIdx ][ eventTypeIdx ].durationMilliseconds = -1.0f;
        }
    }

    // Reset events per stage per light duration.
    for ( int stageIdx = 0; stageIdx < (int)StageType::MAX_VALUE; ++stageIdx ) {
        for ( int lightIdx = 0; lightIdx < s_maxLightCount; ++lightIdx ) {
            for ( int eventTypeIdx = 0; eventTypeIdx < (int)EventTypePerStagePerLight::MAX_VALUE; ++eventTypeIdx )
            {
                m_eventsPerStagePerLight[ frameIdx ][ stageIdx ][ lightIdx ][ eventTypeIdx ].measured             = false;
                m_eventsPerStagePerLight[ frameIdx ][ stageIdx ][ lightIdx ][ eventTypeIdx ].durationMilliseconds = -1.0f;
            }
        }
    }
}