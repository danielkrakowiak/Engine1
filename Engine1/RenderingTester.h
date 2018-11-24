#pragma once

#include "PathManager.h"

#include <map>
#include <memory>

namespace Engine1
{
    class FreeCamera;
    class Scene;
    class uchar4;
    template< typename PixelType > class Texture2D;
    template< typename PixelType > class StagingTexture2D;
    class SceneManager;
    class AssetManager;
    class Renderer;
    class Direct3DRendererCore;

    class RenderingTester
    {
        public:

        RenderingTester(
            SceneManager& sceneManager, 
            AssetManager& assetManager, 
            Direct3DRendererCore& rendererCore,
            Renderer& renderer);

        void initialize();

        void addAndSaveTestCase( const std::string sceneName, const FreeCamera& camera );

        void switchToUsingTestAssets();
        void generateReference();

        // Returns test log - listing test cases that failed.
        std::string runTests();

        private:

        std::shared_ptr< StagingTexture2D< uchar4 > > 
        createStagingTexture();

        std::shared_ptr< StagingTexture2D< uchar4 > > 
        createDifferenceTexture();

        void loadTestCases();

        std::shared_ptr< Texture2D< uchar4 > > 
        loadTestCaseReference( const std::string& sceneName, const std::string& cameraName );

        std::string getTestScenePath( const std::string& sceneName );
        std::string getTestCameraPath( const std::string& sceneName, const std::string& cameraName );
        std::string getTestCasePath( const std::string& sceneName, const std::string& cameraName );
        std::string getReferencePath( const std::string& sceneName, const std::string& cameraName );
        
        std::string getDatedReferencePath(
            const std::string& sceneName, 
            const std::string& cameraName, 
            const std::string& dateString );

        std::string getDatedReferenceDirectory( const std::string& dateString );
        std::string getDatedResultDirectory( const std::string& dateString );
        
        std::string getDatedResultPath( 
            const std::string& sceneName, 
            const std::string& cameraName, 
            const std::string& dateString );
        
        std::string getDatedDifferencePath( 
            const std::string& sceneName, 
            const std::string& cameraName, 
            const std::string& dateString );

        std::string getCurrentDateString();

        PathManager m_testPathManager;
        PathManager m_testAssetsPathManager;
        SceneManager& m_sceneManager;
        AssetManager& m_assetManager;
        Direct3DRendererCore& m_rendererCore;
        Renderer& m_renderer;

        // map< scene-name, vector< camera-name > >
        std::map< std::string, std::vector< std::string > > m_testCases;
    };
}


