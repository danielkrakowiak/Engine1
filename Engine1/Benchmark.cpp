#include "Benchmark.h"

#include "SceneManager.h"
#include "TextFile.h"
#include "SettingsHelper.h"

using namespace Engine1;

Benchmark::Benchmark( SceneManager& sceneManager, Profiler& profiler ) :
    m_sceneManager( sceneManager ),
    m_profiler( profiler )
{}

Benchmark::~Benchmark()
{}

void Benchmark::addSceneToTest( const std::string& scenePath, const std::string& cameraAnimationPath )
{
    SceneDescriptor desc;
    desc.scenePath           = scenePath;
    desc.cameraAnimationPath = cameraAnimationPath;

    m_scenesToTest.emplace_back( desc );
}

void Benchmark::addSettingsToTest( const Settings& settings )
{
    m_settingsToTest.emplace_back( settings );
}

void Benchmark::performTests( const float testDuration )
{
    if ( m_settingsToTest.empty() || m_scenesToTest.empty() )
        return;

    m_testDuration = testDuration;
    m_sceneIdx     = 0;
    m_settingsIdx  = 0;

    
    m_state = State::BREAK;

    m_lastFrameTick.reset();
    resetFrameStats();

    onFrameEnd(true);
}

void Benchmark::onFrameEnd(bool init)
{
    if ( m_state == State::FINISHED )
        return;

    Timer currTick;
    const auto timeSinceLastFrame = (float)( Timer::getElapsedTime( currTick, m_lastFrameTick ) / 1000.0 );

    if ( m_state == State::TESTING ) {
        m_testPassedTime += timeSinceLastFrame;
    } else if ( m_state == State::BREAK ) {
        m_breakPassedTime += timeSinceLastFrame;
    }

    m_lastFrameTick = currTick;

    if ( m_state == State::TESTING )
        collectFrameStats();

    const auto prevSettingsIdx = m_settingsIdx;
    const auto prevSceneIdx    = m_sceneIdx;

    // Switch to a next test if needed.
    if ( m_state == State::TESTING && m_testPassedTime >= m_testDuration )
    {
        m_testPassedTime  = 0.0f;
        m_breakPassedTime = 0.0f;
        m_state           = State::BREAK;

        saveSingleTestResults();
        resetFrameStats();

        if (m_settingsIdx < (int)m_settingsToTest.size() - 1) 
        {
            ++m_settingsIdx;
        } 
        else 
        {
            m_settingsIdx = 0;
            ++m_sceneIdx;
        }
    }

    // Check if all tests were performed.
    if ( m_sceneIdx >= (int)m_scenesToTest.size() ) 
    {
        m_sceneManager.getCameraAnimator().setPlaying( m_sceneManager.getCamera(), false );

        m_state = State::FINISHED;

        saveTestResultsToFile( "benchmark results.txt" );

        return;
    }
    
    if ( init || prevSettingsIdx != m_settingsIdx ) // Change settings if needed.
    {
        Settings::modify() = m_settingsToTest[ m_settingsIdx ];
    }
    
    if ( init || prevSceneIdx != m_sceneIdx ) // Change scene if needed.
    {
        const auto& scenePath           = m_scenesToTest[ m_sceneIdx ].scenePath;
        const auto& cameraAnimationPath = m_scenesToTest[ m_sceneIdx ].cameraAnimationPath;

        m_sceneManager.loadScene( scenePath );
        m_sceneManager.getCameraAnimator().loadAnimationFromFile( m_sceneManager.getCamera(), cameraAnimationPath );

        m_lastFrameTick.reset();
    }

    if ( init || prevSettingsIdx != m_settingsIdx || prevSceneIdx != m_sceneIdx )
    {
        m_sceneManager.getCameraAnimator().setPlaybackTime( m_sceneManager.getCamera(), 0.0f );
        m_sceneManager.getCameraAnimator().update(0.0001f); // Note: Needed to enforce new camera position.
        m_sceneManager.getCameraAnimator().setPlaying( m_sceneManager.getCamera(), false );
    }

    if ( m_state == State::BREAK && m_breakPassedTime >= m_breakDuration )
    {
        m_state = State::TESTING;
        m_testPassedTime = 0.0f;

        m_sceneManager.getCameraAnimator().setPlaying( m_sceneManager.getCamera(), true );
    }
}

void Benchmark::resetFrameStats()
{
    for ( int eventTypeIdx = 0; eventTypeIdx < (int)Profiler::GlobalEventType::MAX_VALUE; ++eventTypeIdx )
    {
        m_statsAccumulated.global[ eventTypeIdx ] = 0.0f;
    }

    for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) 
    {
        for ( int eventTypeIdx = 0; eventTypeIdx < (int)Profiler::EventTypePerStage::MAX_VALUE; ++eventTypeIdx ) 
        {
            m_statsAccumulated.perStage[ stageIdx ][ eventTypeIdx ] = 0.0f;
        }
    }

    for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) 
    {
        for ( int lightIdx = 0; lightIdx < Profiler::s_maxLightCount; ++lightIdx ) 
        {
            for ( int eventTypeIdx = 0; eventTypeIdx < (int)Profiler::EventTypePerStagePerLight::MAX_VALUE; ++eventTypeIdx ) 
            {
                m_statsAccumulated.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ] = 0.0f;
            }
        }
    }

    m_framesCollected = 0;
}

void Benchmark::collectFrameStats()
{
    for ( int eventTypeIdx = 0; eventTypeIdx < (int)Profiler::GlobalEventType::MAX_VALUE; ++eventTypeIdx )
    {
        const auto eventType = static_cast< Profiler::GlobalEventType >( eventTypeIdx );

        const auto eventDuration = m_profiler.getEventDuration( eventType );

        m_statsAccumulated.global[ eventTypeIdx ] += (eventDuration > 0.0f ? eventDuration : 0.0f);
    }

    for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) 
    {
        const auto stage = static_cast< RenderingStage >( stageIdx );

        for ( int eventTypeIdx = 0; eventTypeIdx < (int)Profiler::EventTypePerStage::MAX_VALUE; ++eventTypeIdx ) 
        {
            const auto eventType = static_cast< Profiler::EventTypePerStage >( eventTypeIdx );

            const auto eventDuration = m_profiler.getEventDuration( stage, eventType );

            m_statsAccumulated.perStage[ stageIdx ][ eventTypeIdx ] += (eventDuration > 0.0f ? eventDuration : 0.0f);
        }
    }

    for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) 
    {
        const auto stage = static_cast< RenderingStage >( stageIdx );

        for ( int lightIdx = 0; lightIdx < Profiler::s_maxLightCount; ++lightIdx ) 
        {
            for ( int eventTypeIdx = 0; eventTypeIdx < (int)Profiler::EventTypePerStagePerLight::MAX_VALUE; ++eventTypeIdx ) 
            {
                const auto eventType = static_cast< Profiler::EventTypePerStagePerLight >( eventTypeIdx );
                
                const auto eventDuration = m_profiler.getEventDuration( stage, lightIdx, eventType );

                m_statsAccumulated.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ] += (eventDuration > 0.0f ? eventDuration : 0.0f);
            }
        }
    }

    ++m_framesCollected;
}

void Benchmark::saveSingleTestResults()
{
    const auto& scenePath = m_scenesToTest[ m_sceneIdx ].scenePath;

    auto& testResults = m_testResults[ scenePath ];
    testResults.emplace_back();
    auto& testResult = testResults.back();

    testResult.cameraAnimationPath = m_scenesToTest[ m_sceneIdx ].cameraAnimationPath;
    testResult.settings            = m_settingsToTest[ m_settingsIdx ];
    testResult.framesCollected     = m_framesCollected;

    for ( int eventTypeIdx = 0; eventTypeIdx < (int)Profiler::GlobalEventType::MAX_VALUE; ++eventTypeIdx )
    {
        testResult.statsAveraged.global[ eventTypeIdx ] = m_statsAccumulated.global[ eventTypeIdx ] / (float)m_framesCollected;
    }

    for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) 
    {
        for ( int eventTypeIdx = 0; eventTypeIdx < (int)Profiler::EventTypePerStage::MAX_VALUE; ++eventTypeIdx ) 
        {
            testResult.statsAveraged.perStage[ stageIdx ][ eventTypeIdx ] = m_statsAccumulated.perStage[ stageIdx ][ eventTypeIdx ] / (float)m_framesCollected;
        }
    }

    for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) 
    {
        for ( int lightIdx = 0; lightIdx < Profiler::s_maxLightCount; ++lightIdx ) 
        {
            for ( int eventTypeIdx = 0; eventTypeIdx < (int)Profiler::EventTypePerStagePerLight::MAX_VALUE; ++eventTypeIdx ) 
            {
                testResult.statsAveraged.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ] = m_statsAccumulated.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ] / (float)m_framesCollected;
            }
        }
    }
}

void Benchmark::saveTestResultsToFile( const std::string& path )
{
    std::string text;
    text.reserve( 50000 );

    if (!m_testResults.empty())
    {
        const auto& referenceSettings = m_testResults.begin()->second.front().settings;

        text += "\"Scenes\\Settings\";";

        // Print all tested settings in a row.
        for ( const auto& testResults : m_testResults.begin()->second ) {
            text += "\"" + SettingsHelper::compareSettings( testResults.settings, referenceSettings ) + "\";";
        }

        text += "\n";

        for ( const auto& testResults : m_testResults )
        {
            // Print scene path.
            text += "\"" + testResults.first + "\";";

            text += "\n\"Av. frame\";";

            for ( const auto& testResult : testResults.second ) 
            {
                const auto frameTime = testResult.statsAveraged.global[ (int)Profiler::GlobalEventType::Frame ];

                text += "\"" + std::to_string( frameTime ) + "\"; ";
            }

            text += "\n\"Av. FPS\";";

            for ( const auto& testResult : testResults.second )
            {
                const auto frameTime = testResult.statsAveraged.global[ (int)Profiler::GlobalEventType::Frame ];

                text += "\"" + std::to_string( 1000.0f / frameTime ) + "\"; ";
            }

            text += "\n";

            writeGlobalEventTimings( testResults.second, text, Profiler::GlobalEventType::DeferredRendering, "Av. deferred" );
            writeGlobalEventTimings( testResults.second, text, Profiler::GlobalEventType::Bloom, "Av. bloom" );
            writeGlobalEventTimings( testResults.second, text, Profiler::GlobalEventType::CalculateLuminance, "Av. calculate luminance" );
            writeGlobalEventTimings( testResults.second, text, Profiler::GlobalEventType::ToneMapping, "Av. tone mapping" );
            writeGlobalEventTimings( testResults.second, text, Profiler::GlobalEventType::Antialiasing, "Av. antialiasing" );

            writePerStageEventTimings( testResults.second, text, RenderingStage::R ,Profiler::EventTypePerStage::RaytracingReflectedRefractedRays, "Av. raytracing refl. rays" );
            writePerStageEventTimings( testResults.second, text, RenderingStage::T ,Profiler::EventTypePerStage::RaytracingReflectedRefractedRays, "Av. raytracing trans. rays" );
        }
    }

    std::vector< char > data( text.begin(), text.end() );

    TextFile::save( path, data );
}

void Benchmark::writeGlobalEventTimings( 
    const std::vector< TestResult >& testResults, 
    std::string& text, 
    Profiler::GlobalEventType eventType, 
    const std::string& description )
{
    text += "\"" + description + "\";";

    for ( const auto& testResult : testResults ) 
    {
        const auto frameTime = testResult.statsAveraged.global[ (int)Profiler::GlobalEventType::Frame ];
        const auto eventTime = testResult.statsAveraged.global[ (int)eventType ];

        text += "\"" + std::to_string( eventTime ) + " (" + std::to_string( eventTime * 100.0f / frameTime  ) + "%)\"; ";
    }

    text += "\n";
}

void Benchmark::writePerStageEventTimings( 
    const std::vector< TestResult >& testResults, 
    std::string& text, 
    RenderingStage stageType, 
    Profiler::EventTypePerStage eventType, 
    const std::string& description )
{
    text += "\"" + description + "\";";

    for ( const auto& testResult : testResults ) 
    {
        const auto frameTime = testResult.statsAveraged.global[ (int)Profiler::GlobalEventType::Frame ];
        const auto eventTime = testResult.statsAveraged.perStage[ (int)stageType ][ (int)eventType ];

        text += "\"" + std::to_string( eventTime ) + " (" + std::to_string( eventTime * 100.0f / frameTime  ) + "%)\"; ";
    }

    text += "\n";
}
