#include "Benchmark.h"

#include <sstream>
#include <iomanip>

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

        if ( !cameraAnimationPath.empty() ) {
            m_sceneManager.getCameraAnimator().loadAnimationFromFile( m_sceneManager.getCamera(), cameraAnimationPath );
        }

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

        resetFrameStats();
    }
}

void Benchmark::resetFrameStats()
{
    for ( int eventTypeIdx = 0; eventTypeIdx < (int)Profiler::GlobalEventType::MAX_VALUE; ++eventTypeIdx )
    {
        m_statsAccumulated.global[ eventTypeIdx ] = 0.0f;
        m_statsMaximal.global[ eventTypeIdx ]     = 0.0f;
        m_statsMinimal.global[ eventTypeIdx ]     = 0.0f;
    }

    for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) 
    {
        for ( int eventTypeIdx = 0; eventTypeIdx < (int)Profiler::EventTypePerStage::MAX_VALUE; ++eventTypeIdx ) 
        {
            m_statsAccumulated.perStage[ stageIdx ][ eventTypeIdx ] = 0.0f;
            m_statsMaximal.perStage[ stageIdx ][ eventTypeIdx ]     = 0.0f;
            m_statsMinimal.perStage[ stageIdx ][ eventTypeIdx ]     = 0.0f;
        }
    }

    for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) 
    {
        for ( int lightIdx = 0; lightIdx < Profiler::s_maxLightCount; ++lightIdx ) 
        {
            for ( int eventTypeIdx = 0; eventTypeIdx < (int)Profiler::EventTypePerStagePerLight::MAX_VALUE; ++eventTypeIdx ) 
            {
                m_statsAccumulated.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ] = 0.0f;
                m_statsMaximal.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ]     = 0.0f;
                m_statsMinimal.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ]     = 0.0f;
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

        if (eventDuration > 0.0f)
        {
            m_statsAccumulated.global[ eventTypeIdx ] += (eventDuration > 0.0f ? eventDuration : 0.0f);
            m_statsMaximal.global[ eventTypeIdx ] = std::max(m_statsMaximal.global[ eventTypeIdx ], eventDuration);
            m_statsMinimal.global[ eventTypeIdx ] = std::min(m_statsMinimal.global[ eventTypeIdx ], eventDuration);
        }
    }

    for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) 
    {
        const auto stage = static_cast< RenderingStage >( stageIdx );

        for ( int eventTypeIdx = 0; eventTypeIdx < (int)Profiler::EventTypePerStage::MAX_VALUE; ++eventTypeIdx ) 
        {
            const auto eventType = static_cast< Profiler::EventTypePerStage >( eventTypeIdx );

            const auto eventDuration = m_profiler.getEventDuration( stage, eventType );

            if (eventDuration > 0.0f)
            {
                m_statsAccumulated.perStage[ stageIdx ][ eventTypeIdx ] += (eventDuration > 0.0f ? eventDuration : 0.0f);
                m_statsMaximal.perStage[ stageIdx ][ eventTypeIdx ] = std::max(m_statsMaximal.perStage[ stageIdx ][ eventTypeIdx ], eventDuration);
                m_statsMinimal.perStage[ stageIdx ][ eventTypeIdx ] = std::min(m_statsMinimal.perStage[ stageIdx ][ eventTypeIdx ], eventDuration);
            }
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

                if (eventDuration > 0.0f)
                {
                    m_statsAccumulated.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ] += (eventDuration > 0.0f ? eventDuration : 0.0f);
                    m_statsMaximal.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ] = std::max(m_statsMaximal.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ], eventDuration);
                    m_statsMinimal.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ] = std::min(m_statsMinimal.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ], eventDuration);
                }
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
        testResult.statsMaximal.global[ eventTypeIdx ]  = m_statsMaximal.global[ eventTypeIdx ];
        testResult.statsMinimal.global[ eventTypeIdx ]  = m_statsMinimal.global[ eventTypeIdx ];
    }

    for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) 
    {
        for ( int eventTypeIdx = 0; eventTypeIdx < (int)Profiler::EventTypePerStage::MAX_VALUE; ++eventTypeIdx ) 
        {
            testResult.statsAveraged.perStage[ stageIdx ][ eventTypeIdx ] = m_statsAccumulated.perStage[ stageIdx ][ eventTypeIdx ] / (float)m_framesCollected;
            testResult.statsMaximal.perStage[ stageIdx ][ eventTypeIdx ]  = m_statsMaximal.perStage[ stageIdx ][ eventTypeIdx ];
            testResult.statsMinimal.perStage[ stageIdx ][ eventTypeIdx ]  = m_statsMinimal.perStage[ stageIdx ][ eventTypeIdx ];
        }
    }

    for ( int stageIdx = 0; stageIdx < (int)RenderingStage::MAX_VALUE; ++stageIdx ) 
    {
        for ( int lightIdx = 0; lightIdx < Profiler::s_maxLightCount; ++lightIdx ) 
        {
            for ( int eventTypeIdx = 0; eventTypeIdx < (int)Profiler::EventTypePerStagePerLight::MAX_VALUE; ++eventTypeIdx ) 
            {
                testResult.statsAveraged.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ] = m_statsAccumulated.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ] / (float)m_framesCollected;
                testResult.statsMaximal.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ]  = m_statsMaximal.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ];
                testResult.statsMinimal.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ]  = m_statsMinimal.perStagePerLight[ stageIdx ][ lightIdx ][ eventTypeIdx ];
            }
        }
    }
}

void Benchmark::saveTestResultsToFile( const std::string& path )
{
    std::string text;
    text.reserve( 5000000 ); // reserve 5MB

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);

    if (!m_testResults.empty())
    {
        const auto& referenceSettings = m_testResults.begin()->second.front().settings;

        ss << "\"Scenes\\Settings\";";

        // Print all tested settings in a row.
        for ( const auto& testResults : m_testResults.begin()->second ) {
            ss << "\"" + SettingsHelper::compareSettings( testResults.settings, referenceSettings ) << "\"; ;";
        }

        ss << "\n";
        text += ss.str();
        ss.str("");

        for ( const auto& testResults : m_testResults )
        {
            // Print scene path.
            ss << "\"" << testResults.first << "\";";

            ss << "\n\"Av. frame\";";

            for ( const auto& testResult : testResults.second ) 
            {
                const auto frameTime = testResult.statsAveraged.global[ (int)Profiler::GlobalEventType::Frame ];

                ss << "\"" << frameTime << "\"; ; ";
            }

            ss << "\n\"Av. FPS\";";

            for ( const auto& testResult : testResults.second )
            {
                const auto frameTime = testResult.statsAveraged.global[ (int)Profiler::GlobalEventType::Frame ];

                ss << "\"" << (1000.0f / frameTime) << "\"; ; ";
            }

            ss << "\n\"Max. frame\";";

            for ( const auto& testResult : testResults.second ) 
            {
                const auto frameTime = testResult.statsMaximal.global[ (int)Profiler::GlobalEventType::Frame ];

                ss << "\"" << frameTime << "\"; ; ";
            }

            ss << "\n\"Min. FPS\";";

            for ( const auto& testResult : testResults.second )
            {
                const auto frameTime = testResult.statsMaximal.global[ (int)Profiler::GlobalEventType::Frame ];

                ss << "\"" << (1000.0f / frameTime) << "\"; ; ";
            }

            ss << "\n";

            text += ss.str();
            ss.str("");

            for ( int eventTypeIdx = 0; eventTypeIdx < static_cast<int>(Profiler::GlobalEventType::MAX_VALUE); ++eventTypeIdx )
            {
                const auto eventType = static_cast<Profiler::GlobalEventType>( eventTypeIdx );
                writeGlobalEventTimings( testResults.second, text, eventType, "Av. " + Profiler::eventTypeToString( eventType ) );
            }

            writeStageTimings( testResults.second, text, RenderingStage::Main );
            writeStageTimings( testResults.second, text, RenderingStage::R );
            writeStageTimings( testResults.second, text, RenderingStage::T );

            writeStageTimings( testResults.second, text, RenderingStage::RR );
            writeStageTimings( testResults.second, text, RenderingStage::RT );
            writeStageTimings( testResults.second, text, RenderingStage::TR );
            writeStageTimings( testResults.second, text, RenderingStage::TT );
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
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);

    ss << "\"" << description << "\";";

    for ( const auto& testResult : testResults ) 
    {
        const auto frameTime = testResult.statsAveraged.global[ (int)Profiler::GlobalEventType::Frame ];
        const auto eventTime = testResult.statsAveraged.global[ (int)eventType ];

        if ( eventTime > 0.0f ) {
            ss << "\"" << eventTime << "\"; \"" << (eventTime * 100.0f / frameTime) << "%\"; ";
        } else {
            ss << "; ; ";
        }
    }

    ss << "\n";

    text += ss.str();
}

void Benchmark::writeStageTimings( 
    const std::vector< TestResult >& testsResults, 
    std::string& text,
    RenderingStage stageType )
{
    const auto stageName = renderingStageToString( stageType );

    for ( int eventTypeIdx = 0; eventTypeIdx < static_cast<int>(Profiler::EventTypePerStage::MAX_VALUE); ++eventTypeIdx )
    {
        const auto eventType = static_cast<Profiler::EventTypePerStage>( eventTypeIdx );

        writePerStageEventTimings( testsResults, text, stageType, eventType, stageName + " Av. " + Profiler::eventTypeToString( eventType ) );
    }

    for ( int eventTypeIdx = 0; eventTypeIdx < static_cast<int>(Profiler::EventTypePerStagePerLight::MAX_VALUE); ++eventTypeIdx )
    {
        const auto eventType = static_cast<Profiler::EventTypePerStagePerLight>( eventTypeIdx );

        writePerStagePerLightEventTimings( testsResults, text, stageType, eventType, stageName + " Av. " + Profiler::eventTypeToString( eventType ) );
    }
}

void Benchmark::writePerStageEventTimings( 
    const std::vector< TestResult >& testResults, 
    std::string& text, 
    RenderingStage stageType, 
    Profiler::EventTypePerStage eventType, 
    const std::string& description )
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);

    ss << "\"" << description << "\";";

    for ( const auto& testResult : testResults ) 
    {
        const auto frameTime = testResult.statsAveraged.global[ (int)Profiler::GlobalEventType::Frame ];
        const auto eventTime = testResult.statsAveraged.perStage[ (int)stageType ][ (int)eventType ];

        if ( eventTime > 0.0f ) {
            ss << "\"" << eventTime << " \"; \"" << (eventTime * 100.0f / frameTime) << "%\"; ";
        } else {
            ss << "; ; ";
        }
    }

    ss << "\n";

    text += ss.str();
}

void Benchmark::writePerStagePerLightEventTimings( 
    const std::vector< TestResult >& testResults, 
    std::string& text, 
    RenderingStage stageType, 
    Profiler::EventTypePerStagePerLight eventType, 
    const std::string& description 
)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);

    ss << "\"" << description << "\";";

    for ( const auto& testResult : testResults ) 
    {
        const auto frameTime = testResult.statsAveraged.global[ (int)Profiler::GlobalEventType::Frame ];

        float accumulatedEventTime = 0.0f;

        for ( int lightIdx = 0; lightIdx < Profiler::s_maxLightCount; ++lightIdx )
        {
            const auto eventTime = testResult.statsAveraged.perStagePerLight[ (int)stageType ][ lightIdx ][ (int)eventType ];

            if ( eventTime > 0.0f ) {
                accumulatedEventTime += eventTime;
            }
        }

        if ( accumulatedEventTime > 0.0f ) {
            ss << "\"" << accumulatedEventTime << "\"; \"" << (accumulatedEventTime * 100.0f / frameTime) << "%\"; ";
        } else {
            ss << "; ; ";
        }
    }

    ss << "\n";

    text += ss.str();
}
