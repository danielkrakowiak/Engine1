#pragma once

#include <vector>
#include <memory>

#include "Settings.h"
#include "Timer.h"

namespace Engine1
{
    class Scene;
    class SceneManager;

    class Benchmark
    {
        public:

        Benchmark( SceneManager& sceneManager );
        ~Benchmark();

        void addSceneToTest( const std::string& scenePath, const std::string& cameraAnimationPath );
        void addSettingsToTest( const Settings& settings );

        void performTests( const float testDuration );
        void onFrameEnd( bool init = false );

        private:

        bool  m_performingTests = false;
        float m_testDuration    = 0.0f;
        float m_testPassedTime  = 0.0f;
        int   m_sceneIdx        = 0;
        int   m_settingsIdx     = 0;

        Timer m_lastFrameTick;

        struct SceneDescriptor
        {
            std::string scenePath;
            std::string cameraAnimationPath;
        };

        std::vector< SceneDescriptor > m_sceneDescriptors;
        std::vector< Settings > m_settings;

        SceneManager& m_sceneManager;
    };
}

