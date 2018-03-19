#include "Benchmark.h"

#include "SceneManager.h"

using namespace Engine1;

Benchmark::Benchmark( SceneManager& sceneManager ) :
    m_sceneManager( sceneManager )
{}

Benchmark::~Benchmark()
{}

void Benchmark::addSceneToTest( const std::string& scenePath, const std::string& cameraAnimationPath )
{
    SceneDescriptor desc;
    desc.scenePath           = scenePath;
    desc.cameraAnimationPath = cameraAnimationPath;

    m_sceneDescriptors.emplace_back( desc );
}

void Benchmark::addSettingsToTest( const Settings& settings )
{
    m_settings.emplace_back( settings );
}

void Benchmark::performTests( const float testDuration )
{
    if ( m_settings.empty() || m_sceneDescriptors.empty() )
        return;

    m_testDuration = testDuration;
    m_sceneIdx     = 0;
    m_settingsIdx  = 0;

    
    m_performingTests = true;

    m_lastFrameTick.reset();

    onFrameEnd(true);
}

void Benchmark::onFrameEnd(bool init)
{
    if ( !m_performingTests )
        return;

    Timer currTick;
    m_testPassedTime += (float)( Timer::getElapsedTime( currTick, m_lastFrameTick ) / 1000.0 );
    m_lastFrameTick = currTick;

    const auto prevSettingsIdx = m_settingsIdx;
    const auto prevSceneIdx    = m_sceneIdx;

    // Switch to a next test if needed.
    if (m_testPassedTime >= m_testDuration)
    {
        m_testPassedTime = 0.0f;

        if (m_settingsIdx < (int)m_settings.size() - 1) 
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
    if ( m_sceneIdx >= (int)m_sceneDescriptors.size() ) 
    {
        m_sceneManager.getCameraAnimator().setPlaying( m_sceneManager.getCamera(), false );

        m_performingTests = false;
        return;
    }
    
    if ( init || prevSettingsIdx != m_settingsIdx ) // Change settings if needed.
    {
        Settings::modify() = m_settings[ m_settingsIdx ];
    }
    
    if ( init || prevSceneIdx != m_sceneIdx ) // Change scene if needed.
    {
        const auto& scenePath           = m_sceneDescriptors[ m_sceneIdx ].scenePath;
        const auto& cameraAnimationPath = m_sceneDescriptors[ m_sceneIdx ].cameraAnimationPath;

        m_sceneManager.loadScene( scenePath );
        m_sceneManager.getCameraAnimator().loadAnimationFromFile( m_sceneManager.getCamera(), cameraAnimationPath );

        m_lastFrameTick.reset();
    }

    if ( init || prevSettingsIdx != m_settingsIdx || prevSceneIdx != m_sceneIdx )
    {
        m_sceneManager.getCameraAnimator().setPlaybackTime( m_sceneManager.getCamera(), 0.0f );
        m_sceneManager.getCameraAnimator().setPlaying( m_sceneManager.getCamera(), true );
    }
}
