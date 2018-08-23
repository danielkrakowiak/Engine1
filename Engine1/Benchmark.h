#pragma once

#include <vector>
#include <memory>
#include <map>

#include "Settings.h"
#include "Timer.h"
#include "Profiler.h"

namespace Engine1
{
    class Scene;
    class SceneManager;

    class Benchmark
    {
        public:

        Benchmark( SceneManager& sceneManager, Profiler& profiler );
        ~Benchmark();

        void addSceneToTest( const std::string& scenePath, const std::string& cameraAnimationPath );
        void addSettingsToTest( const Settings& settings );

        void performTests( const float testDuration );
        void onFrameEnd( bool init = false );

        private:

        struct SceneDescriptor
        {
            std::string scenePath;
            std::string cameraAnimationPath;
        };

        struct Statistics
        {
            std::array< 
                float, 
                (int)Profiler::GlobalEventType::MAX_VALUE 
            > global;

            std::array<
                std::array<
                    float,
                    (int)Profiler::EventTypePerStage::MAX_VALUE
                >,
                (int)RenderingStage::MAX_VALUE
            > perStage;

            std::array< 
                std::array< 
                    std::array< float, (int)Profiler::EventTypePerStagePerLight::MAX_VALUE >,
                    Profiler::s_maxLightCount
                >,
                (int)RenderingStage::MAX_VALUE 
            > perStagePerLight;
        };

        struct TestResult
        {
            std::string cameraAnimationPath;
            Settings    settings;
            int         framesCollected;
            Statistics  statsAveraged;
            Statistics  statsMaximal;
            Statistics  statsMinimal;
        };

        void resetFrameStats();
        void collectFrameStats();
        void saveSingleTestResults();
        
        void saveTestResultsToFile( const std::string& path );

        void writeGlobalEventTimings( 
            const std::vector< TestResult >& testResults, 
            std::string& text, 
            Profiler::GlobalEventType eventType, 
            const std::string& description 
        );

        void writeStageTimings( 
            const std::vector< TestResult >& testsResults, 
            std::string& text,
            RenderingStage stageType 
        );

        void writePerStageEventTimings( 
            const std::vector< TestResult >& testResults, 
            std::string& text, 
            RenderingStage stageType, 
            Profiler::EventTypePerStage eventType, 
            const std::string& description 
        );

        void writePerStagePerLightEventTimings( 
            const std::vector< TestResult >& testResults, 
            std::string& text, 
            RenderingStage stageType, 
            Profiler::EventTypePerStagePerLight eventType, 
            const std::string& description 
        );

        enum class State : int
        {
            TESTING = 0,
            BREAK,
            FINISHED,
        } m_state = State::FINISHED;

        float m_testDuration    = 0.0f;
        float m_breakDuration   = 5.0f;
        float m_testPassedTime  = 0.0f;
        float m_breakPassedTime = 0.0f;
        int   m_sceneIdx        = 0;
        int   m_settingsIdx     = 0;

        Timer m_lastFrameTick;

        std::vector< SceneDescriptor > m_scenesToTest;
        std::vector< Settings >        m_settingsToTest;

        SceneManager& m_sceneManager;
        Profiler&     m_profiler;

        int        m_framesCollected;
        Statistics m_statsAccumulated;
        Statistics m_statsMaximal;
        Statistics m_statsMinimal;

        // Key: scene path, value: test results for that scene.
        std::map< std::string, std::vector< TestResult > > m_testResults;
    };
}

