#include "RenderingTester.h"

#include "FreeCamera.h"
#include "FileUtil.h"
#include "StringUtil.h"
#include "Settings.h"
#include "Texture2D.h"
#include "StagingTexture2D.h"
#include "SceneManager.h"
#include "AssetManager.h"
#include "AssetPathManager.h"
#include "Direct3DRendererCore.h"
#include "Renderer.h"

#include <algorithm>
#include <experimental/filesystem>
#include <time.h>

using namespace Engine1;

RenderingTester::RenderingTester(
    SceneManager& sceneManager, 
    AssetManager& assetManager, 
    Direct3DRendererCore& rendererCore,
    Renderer& renderer)
    : m_sceneManager( sceneManager ),
    m_assetManager( assetManager ),
    m_rendererCore( rendererCore ),
    m_renderer( renderer )
{}

void RenderingTester::initialize()
{
    m_testPathManager.scanDirectory( settings().paths.renderingTests.testCases );
    m_testAssetsPathManager.scanDirectory( settings().paths.testAssets );

    loadTestCases();
}

void RenderingTester::addAndSaveTestCase( const std::string sceneName, const FreeCamera& camera )
{
    auto testCasesIt = m_testCases.find( sceneName );

    if (testCasesIt == m_testCases.end())
        m_testCases[ sceneName ] = {};

    auto& testCases = m_testCases[ sceneName ];

    const auto cameraName = std::to_string( testCases.size() );
    
    testCases.emplace_back( cameraName );

    const auto testCasePath = getTestCasePath( sceneName, cameraName );

     camera.saveToFile( testCasePath );
}

std::shared_ptr< StagingTexture2D< uchar4 > > 
RenderingTester::createStagingTexture()
{
    return std::make_shared< StagingTexture2D< uchar4 > >
        ( *m_renderer.getDevice().Get(), 
          settings().main.screenDimensions.x, 
          settings().main.screenDimensions.y, 
          DXGI_FORMAT_R8G8B8A8_UINT );
}

std::shared_ptr< StagingTexture2D< uchar4 > > 
RenderingTester::createDifferenceTexture()
{
    return createStagingTexture();
}

void RenderingTester::loadTestCases()
{
    for ( const auto& nameAndPath : m_testPathManager.getAllPaths() )
    {
        const auto filename  = FileUtil::getFileNameFromPath( nameAndPath.first, true );
        const auto path      = nameAndPath.second;
        const auto extension = FileUtil::getFileExtensionFromPath( filename );

        if ( extension.compare( "camera" ) == 0 )
        {
            auto sceneName = StringUtil::getSubstrBefore( filename, " (" );
            auto cameraName = StringUtil::getSubstrBetween( filename, " (", ")" );

            if (!sceneName.empty() && !cameraName.empty())
            {
                auto sceneIt = m_testCases.find( sceneName );

                if ( sceneIt != m_testCases.end() ) {
                    sceneIt->second.emplace_back( cameraName );
                } else {
                    m_testCases[ sceneName ] = { cameraName };
                }
            }
            else
            {
                throw std::exception( "Failed to parse test-case camera filename. Format: scene-name (camera-name).camera" );
            }
        }
    }
}

void RenderingTester::switchToUsingTestAssets()
{
    m_assetManager.unloadAll();
    m_sceneManager.unloadAll();

    AssetPathManager::get().scanDirectory( "TestAssets" );
}

void RenderingTester::generateReference()
{
    switchToUsingTestAssets();

    const auto prevSettings = settings();

    Settings::modify().debug.renderLightSources = false;

    auto stagingTexture = createStagingTexture();

    const auto dateString = getCurrentDateString();
    const auto referenceDirectoryPath = getDatedReferenceDirectory( dateString );

    const auto referenceDirectoryExists = std::experimental::filesystem::is_directory( referenceDirectoryPath );

    if ( !referenceDirectoryExists && !std::experimental::filesystem::create_directories( referenceDirectoryPath ) ) {
        throw std::exception( ("Failed to create directory for test reference. Directory path: " + referenceDirectoryPath).c_str() );
    }

    for ( auto& testScene : m_testCases )
    {
        const auto& sceneName = testScene.first;

        m_sceneManager.unloadAll();
        m_assetManager.unloadAll();

        m_sceneManager.loadScene( getTestScenePath( sceneName ) );

        for ( auto& cameraName : testScene.second )
        {
            m_sceneManager.loadCamera( getTestCameraPath( sceneName, cameraName ) );

            const auto output = m_renderer.renderScene( *m_sceneManager.getScene(), *m_sceneManager.getCamera(), false, Selection(), nullptr );

            // Transfer output image from GPU to CPU.
            m_rendererCore.copyTextureGpu(*stagingTexture, *output.uchar4Image);
            stagingTexture->loadGpuToCpu(*m_renderer.getDeviceContext().Get());

            const bool flipRedAndBlue = true;
            const bool removeAlpha    = true;
            const bool flipVertically = true;

            // Save test result to file.
            stagingTexture->saveToFile( 
                getDatedReferencePath( sceneName, cameraName, dateString ), 
                Texture2DFileInfo::Format::PNG, 
                flipRedAndBlue, removeAlpha, flipVertically );
        }
    }

    Settings::modify() = prevSettings;
}

std::string RenderingTester::runTests()
{
    const auto prevSettings = settings();

    Settings::modify().debug.renderLightSources = false;

    auto stagingTexture = createStagingTexture();
    auto differenceTexture = createDifferenceTexture();

    const auto dateString = getCurrentDateString();
    const auto resultDirectoryPath = getDatedResultDirectory( dateString );

    const auto resultsDirectoryExists = std::experimental::filesystem::is_directory(resultDirectoryPath);

    if ( !resultsDirectoryExists && !std::experimental::filesystem::create_directories( resultDirectoryPath ) ) {
        throw std::exception( ("Failed to create directory for test results. Directory path: " + resultDirectoryPath).c_str() );
    }

    std::string testLog;

    for ( auto& testScene : m_testCases )
    {
        const auto& sceneName = testScene.first;

        m_sceneManager.unloadAll();
        m_assetManager.unloadAll();

        m_sceneManager.loadScene( getTestScenePath( sceneName ) );

        for ( auto& cameraName : testScene.second )
        {
            m_sceneManager.loadCamera( getTestCameraPath( sceneName, cameraName ) );

            const auto output = m_renderer.renderScene( *m_sceneManager.getScene(), *m_sceneManager.getCamera(), false, Selection(), nullptr );

            // Transfer output image from GPU to CPU.
            m_rendererCore.copyTextureGpu(*stagingTexture, *output.uchar4Image);
            stagingTexture->loadGpuToCpu(*m_renderer.getDeviceContext().Get());

            const bool flipRedAndBlue = true;
            const bool removeAlpha    = true;
            const bool flipVertically = true;

            // Save test result to file.
            stagingTexture->saveToFile( 
                getDatedResultPath( sceneName, cameraName, dateString ), 
                Texture2DFileInfo::Format::PNG, 
                flipRedAndBlue, removeAlpha, flipVertically );

            const auto referenceTexture = loadTestCaseReference( sceneName, cameraName );

            const auto width  = referenceTexture->getWidth();
            const auto height = referenceTexture->getHeight();

            uchar4 zeroDiff(128, 128, 128, 255);
            bool nonZeroDifference = false;

            // Calculate difference between reference and test image.
            for (auto y = 0; y < height; ++y )
            {
                for (auto x = 0; x < width; ++x )
                {
                    int2 coords(x, y);
                    int2 refCoords(x, height - y - 1); // Need to be vertically flipped, because staging texture is flipped.

                    const auto pixel    = stagingTexture->getPixel(coords);
                    const auto refPixel = referenceTexture->getDataPixel(refCoords);
                    
                    // Note: account for the fact that staging texture is BGR, while reference is RGB.
                    uchar4 diff;
                    diff.x = static_cast<unsigned char>(128 + std::min(127, std::max(-128, pixel.z - refPixel.x)));
                    diff.y = static_cast<unsigned char>(128 + std::min(127, std::max(-128, pixel.y - refPixel.y)));
                    diff.z = static_cast<unsigned char>(128 + std::min(127, std::max(-128, pixel.x - refPixel.z)));
                    diff.w = 255;

                    if (diff != zeroDiff) {
                        nonZeroDifference = true;
                    }

                    differenceTexture->setPixel(diff, coords);
                }
            }

            if (nonZeroDifference)
            {
                const bool diffFlipRedAndBlue = false;
                const bool diffRemoveAlpha    = true;
                const bool diffFlipVertically = false;

                // Save difference image to file.
                differenceTexture->saveToFile( 
                    getDatedDifferencePath( sceneName, cameraName, dateString ), 
                    Texture2DFileInfo::Format::PNG, 
                    flipRedAndBlue, removeAlpha, flipVertically );

                testLog += "\nFailed: " + sceneName + " (" + cameraName + ").";
            }
        }
    }

    // #TODO: How to deal with different settings? Should settings be savable to file? Comparable?

    Settings::modify() = prevSettings;

    return testLog;
}

std::shared_ptr< Texture2DGeneric< uchar4 > > 
RenderingTester::loadTestCaseReference( const std::string& sceneName, const std::string& cameraName )
{
    const auto referencePath = getReferencePath( sceneName, cameraName );

    Texture2DFileInfo fileInfo( referencePath, Texture2DFileInfo::Format::PNG, Texture2DFileInfo::PixelType::UCHAR4 );

    return std::make_shared< Texture2D< TexUsage::Immutable, TexBind::ShaderResource, uchar4 > >
        ( *m_renderer.getDevice().Get(), fileInfo, true, false, false, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UINT );
}

std::string RenderingTester::getTestScenePath( const std::string& sceneName )
{
    return m_testAssetsPathManager.getPathForFileName( sceneName + ".scene" );
}

std::string RenderingTester::getTestCameraPath( const std::string& sceneName, const std::string& cameraName )
{
    return m_testPathManager.getPathForFileName( sceneName + " (" + cameraName + ").camera" );
}

std::string RenderingTester::getTestCasePath( const std::string& sceneName, const std::string& cameraName )
{
    return settings().paths.renderingTests.testCases + "//" + sceneName + " (" + cameraName + ").camera";
}

std::string RenderingTester::getReferencePath( const std::string& sceneName, const std::string& cameraName )
{
    return settings().paths.renderingTests.references + "//" + sceneName + " (" + cameraName + ").png";
}

std::string RenderingTester::getDatedReferencePath( 
    const std::string& sceneName, 
    const std::string& cameraName, 
    const std::string& dateString )
{
    return getDatedReferenceDirectory( dateString ) + "//" + sceneName + " (" + cameraName + ").png";
}

std::string RenderingTester::getDatedReferenceDirectory( const std::string& dateString )
{
    return settings().paths.renderingTests.references + " " + dateString;
}

std::string RenderingTester::getDatedResultDirectory( const std::string& dateString )
{
    return settings().paths.renderingTests.results + " " + dateString;
}

std::string RenderingTester::getDatedResultPath( 
    const std::string& sceneName, 
    const std::string& cameraName, 
    const std::string& dateString )
{
    return getDatedResultDirectory( dateString ) + "//" + sceneName + " (" + cameraName + ") result " + dateString + ".png";
}

std::string RenderingTester::getDatedDifferencePath( 
    const std::string& sceneName, 
    const std::string& cameraName, 
    const std::string& dateString )
{
    return getDatedResultDirectory( dateString ) + "//" + sceneName + " (" + cameraName + ") diff " + dateString + ".png";
}

std::string RenderingTester::getCurrentDateString()
{
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    char buf[64] = {0};
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&now));

    return std::string( buf );
}

        

