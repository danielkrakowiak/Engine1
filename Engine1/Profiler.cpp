#include "Profiler.h"

#include <d3d11_3.h>

#include <exception>
#include <string>
#include <assert.h>

#include "MathUtil.h"

using Microsoft::WRL::ComPtr;

using namespace Engine1;

std::string Profiler::eventTypeToString( const GlobalEventType eventType )
{
    switch (eventType)
    {
        case GlobalEventType::Frame:                        return "Frame";
        case GlobalEventType::RenderSceneToFrame:           return "RenderSceneToFrame";
        case GlobalEventType::RenderFrameToScreen:          return "RenderFrameToScreen";
        case GlobalEventType::RenderTextToFrame:            return "RenderTextToFrame";
        case GlobalEventType::RenderTextFrameToScreen:      return "RenderTextFrameToScreen";
        case GlobalEventType::RenderControlPanelToScreen:   return "RenderControlPanelToScreen";
        case GlobalEventType::DeferredRendering:            return "DeferredRendering";
        case GlobalEventType::PostProcess:                  return "PostProcess";
        case GlobalEventType::ASSAO:                        return "ASSAO";
        case GlobalEventType::Bloom:                        return "Bloom";
        case GlobalEventType::ToneMapping:                  return "ToneMapping";
        case GlobalEventType::CalculateLuminance:           return "CalculateLuminance";
        case GlobalEventType::Antialiasing:                 return "Antialiasing";
    }

    return "";
}

std::string Profiler::eventTypeToString( const EventTypePerStage eventType )
{
    switch ( eventType ) 
    {
        case EventTypePerStage::Total_WO_Combining:                    return "Total_WO_Combining";
        case EventTypePerStage::MipmapGenerationForPositionAndNormals: return "MipmapGenerationForPositionAndNormals";
        case EventTypePerStage::EmissiveShading:                       return "EmissiveShading";
        case EventTypePerStage::ReflectionTransmissionShading:         return "ReflectionTransmissionShading";
        case EventTypePerStage::RaytracingReflectedRefractedRays:      return "RaytracingReflectedRefractedRays";
        case EventTypePerStage::ShadingNoShadows:                      return "ShadingNoShadows";
        case EventTypePerStage::Shading:                               return "Shading";
        case EventTypePerStage::MipmapGenerationForShadedLayer:        return "MipmapGenerationForShadedLayer";
        case EventTypePerStage::CombiningWithPreviousLayer:            return "CombiningWithPreviousLayer";
        case EventTypePerStage::MipmapGenerationForHitDistance:        return "MipmapGenerationForHitDistance";
        case EventTypePerStage::HitDistanceSearch:                     return "HitDistanceSearch";
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
        case EventTypePerStagePerLight::MipmapGenerationForShadows:                         return "MipmapGenerationForShadows" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
        case EventTypePerStagePerLight::MipmapGenerationForDistanceToOccluder:              return "MipmapGenerationForDistanceToOccluder" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
        case EventTypePerStagePerLight::DistanceToOccluderSearch:                           return "DistanceToOccluderSearch" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
        case EventTypePerStagePerLight::BlurShadowPattern:                                  return "BlurShadowPattern" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
        case EventTypePerStagePerLight::MipmapGenerationForSmoothedShadowPattern:           return "MipmapGenerationForSmoothedShadowPattern" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
        case EventTypePerStagePerLight::BlurShadows:                                        return "BlurShadows" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
        case EventTypePerStagePerLight::CombineShadowLayers:                                return "CombineShadowLayers" + std::string( lightIdx >= 0 ? " Light" + std::to_string( lightIdx ) : "" );
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

void Profiler::initialize( ComPtr< ID3D11Device3 > device, ComPtr< ID3D11DeviceContext3 > deviceContext )
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
        for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) 
        {
            for ( int eventTypeIdx = 0; eventTypeIdx < (int)EventTypePerStage::MAX_VALUE; ++eventTypeIdx )
                m_eventsPerStage[ frameIdx ][ stageIdx ][ eventTypeIdx ] = createEvent( *m_device.Get() );
        }

        // Create timestamp queries for events occurring at each rendering stage for each light source.
        for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) 
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

void Profiler::beginEvent( const RenderingStage stage, const EventTypePerStage event )
{
    if ( m_profilingPaused || (int)stage >= (int)RenderingStage::MAX_VALUE )
        return;

     m_deviceContext->End( m_eventsPerStage[ m_currentSubmitQueryFrameIndex ][ (int)stage ][ (int)event ].queryBegin.Get() );
}

void Profiler::endEvent( const RenderingStage stage, const EventTypePerStage event )
{
    if ( m_profilingPaused || (int)stage >= (int)RenderingStage::MAX_VALUE )
        return;

    m_eventsPerStage[ m_currentSubmitQueryFrameIndex ][ (int)stage ][ (int)event ].measured = true;

    m_deviceContext->End( m_eventsPerStage[ m_currentSubmitQueryFrameIndex ][ (int)stage ][ (int)event ].queryEnd.Get() );
}

void Profiler::beginEvent( const RenderingStage stage, const int lightIndex, const EventTypePerStagePerLight event )
{
    if ( m_profilingPaused || (int)stage >= (int)RenderingStage::MAX_VALUE )
        return;

    if ( lightIndex < 0 || lightIndex >= s_maxLightCount )
        return;

    m_deviceContext->End( m_eventsPerStagePerLight[ m_currentSubmitQueryFrameIndex ][ (int)stage ][ lightIndex ][ (int)event ].queryBegin.Get() );
}

void Profiler::endEvent( const RenderingStage stage, const int lightIndex, const EventTypePerStagePerLight event )
{
    if ( m_profilingPaused || (int)stage >= (int)RenderingStage::MAX_VALUE )
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
            for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) 
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
            for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) 
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
float Profiler::getEventDuration( const RenderingStage stage, const EventTypePerStage event )
{
    if ( m_currentReadResultsFrameIndex < 0 )
        return 0.0f; // Results are not ready yet in the first few frames.

    return m_eventsPerStage[ m_currentReadResultsFrameIndex ][ (int)stage ][ (int)event ].durationMilliseconds;
}

// Negative value means that event hasn't occurred.
float Profiler::getEventDuration( const RenderingStage stage, const int lightIndex, const EventTypePerStagePerLight event )
{
    if ( lightIndex >= s_maxLightCount )
        throw std::exception( "Profiler::getEventDuration - given lightIndex exceeds the number of profiled lights." );

    if ( m_currentReadResultsFrameIndex < 0 )
        return 0.0f; // Results are not ready yet in the first few frames.

    return m_eventsPerStagePerLight[ m_currentReadResultsFrameIndex ][ (int)stage ][ (int)lightIndex ][ (int)event ].durationMilliseconds;
}

Profiler::Event Profiler::createEvent( ID3D11Device3& device )
{
    Event event;
    event.measured             = false;
    event.queryBegin           = createTimestampQuery( device );
    event.queryEnd             = createTimestampQuery( device );
    event.durationMilliseconds = 0.0f;

    return event;
}

ComPtr< ID3D11Query > Profiler::createTimestampQuery( ID3D11Device3& device )
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

Microsoft::WRL::ComPtr< ID3D11Query > Profiler::createDisjointQuery( ID3D11Device3& device )
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
    for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) {
        for ( int eventTypeIdx = 0; eventTypeIdx < (int)EventTypePerStage::MAX_VALUE; ++eventTypeIdx ) 
        {
            m_eventsPerStage[ frameIdx ][ stageIdx ][ eventTypeIdx ].measured             = false;
            m_eventsPerStage[ frameIdx ][ stageIdx ][ eventTypeIdx ].durationMilliseconds = -1.0f;
        }
    }

    // Reset events per stage per light duration.
    for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) {
        for ( int lightIdx = 0; lightIdx < s_maxLightCount; ++lightIdx ) {
            for ( int eventTypeIdx = 0; eventTypeIdx < (int)EventTypePerStagePerLight::MAX_VALUE; ++eventTypeIdx )
            {
                m_eventsPerStagePerLight[ frameIdx ][ stageIdx ][ lightIdx ][ eventTypeIdx ].measured             = false;
                m_eventsPerStagePerLight[ frameIdx ][ stageIdx ][ lightIdx ][ eventTypeIdx ].durationMilliseconds = -1.0f;
            }
        }
    }
}