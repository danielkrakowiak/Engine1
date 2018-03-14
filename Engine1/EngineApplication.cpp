#include "EngineApplication.h"

#include <sstream>
#include <exception>
#include <iomanip>
#include <algorithm>

#include <Windows.h>

#include "Camera.h"

#include "MathUtil.h"
#include "StringUtil.h"
#include "TextureUtil.h"
#include "MeshUtil.h"
#include "ModelUtil.h"

#include "BlockMesh.h"
#include "BlockModel.h"
#include "SkeletonModel.h"
#include "SkeletonAnimation.h"

#include "BlockActor.h"
#include "SkeletonActor.h"

#include "PointLight.h"
#include "SpotLight.h"

#include "Scene.h"

#include "Timer.h"

#include "BVHTree.h"
#include "BVHTreeBuffer.h"

#include "AssetPathManager.h"

// Only for debugging.
#include "BlurShadowsComputeShader.h"
#include "HitDistanceSearchComputeShader.h"

#include "Settings.h"

using namespace Engine1;

EngineApplication::EngineApplication()
{}


EngineApplication::~EngineApplication()
{}

void EngineApplication::onStart( ) 
{
    __super::onStart();
}

void EngineApplication::onExit( ) 
{
    __super::onExit();
}

void EngineApplication::onResize( int newWidth, int newHeight ) 
{
    __super::onResize( newWidth, newHeight );
}

void EngineApplication::onMove( int newPosX, int newPosY )
{
    __super::onMove( newPosX, newPosY );
}

void EngineApplication::onFocusChange( bool windowFocused )
{
    __super::onFocusChange( windowFocused );

    Settings::modify().main.limitFPS = !windowFocused;
}

void EngineApplication::onKeyPress( int key )
{
    __super::onKeyPress( key );

    const bool ctrlPressed  = m_inputManager.isKeyPressed( InputManager::Keys::ctrl );
    const bool shiftPressed = m_inputManager.isKeyPressed( InputManager::Keys::shift );

    // [\] - Hide/show text.
    if ( key == InputManager::Keys::backslash )
    {
        Settings::modify().debug.renderText = !settings().debug.renderText;
    }

    // [/] - Hide/show FPS counter.
    if ( key == InputManager::Keys::slash ) {
        Settings::modify().debug.renderFps = !settings().debug.renderFps;
    }

    // [ C + D + P ] - Create dynamic physics for a block actor.
    if ( m_inputManager.isKeyPressed( InputManager::Keys::c ) 
         && m_inputManager.isKeyPressed( InputManager::Keys::d )
         && m_inputManager.isKeyPressed( InputManager::Keys::p ) )
    {
        for ( auto& actor : m_sceneManager.getSelection().getBlockActors() )
        {
            if ( !actor->hasPhysics() )
                actor->createDynamicPhysics();
        }
    }

    // [ C + K + P ] - Create kinematic physics for a block actor.
    if ( m_inputManager.isKeyPressed( InputManager::Keys::c )
         && m_inputManager.isKeyPressed( InputManager::Keys::k )
         && m_inputManager.isKeyPressed( InputManager::Keys::p ) ) 
    {
        for ( auto& actor : m_sceneManager.getSelection().getBlockActors() ) {
            if ( !actor->hasPhysics() )
                actor->createKinematicPhysics();
        }
    }

    // [ L + P ] - Add point light.
    // [ L + S ] - Add spot light.
    if ( key == InputManager::Keys::l && m_inputManager.isKeyPressed( InputManager::Keys::p ) ) 
    {
        m_sceneManager.addPointLight();
        m_renderer.renderShadowMaps( m_sceneManager.getScene() );
    }
    else if ( key == InputManager::Keys::l && m_inputManager.isKeyPressed( InputManager::Keys::s ) )
    {
        m_sceneManager.addSpotLight();
        m_renderer.renderShadowMaps( m_sceneManager.getScene() );
    }
    
    // [Ctrl + S] - save scene or selected models.
    if ( ( key == InputManager::Keys::ctrl || key == InputManager::Keys::s ) &&
       ( ctrlPressed && m_inputManager.isKeyPressed( InputManager::Keys::s ) ) )
    {
        m_sceneManager.saveSceneOrSelectedModels();
    }

    // [Ctrl + A] - Select all actors and lights.
    if ( ( key == InputManager::Keys::ctrl || key == InputManager::Keys::a ) &&
       ( ctrlPressed && m_inputManager.isKeyPressed( InputManager::Keys::a ) ) )
    {
        m_sceneManager.selectAll();
    }

    // [Ctrl + D] - Unselect all.
    if ( ( key == InputManager::Keys::ctrl || key == InputManager::Keys::d ) &&
         ( ctrlPressed && m_inputManager.isKeyPressed( InputManager::Keys::d ) ) ) {
        m_sceneManager.clearSelection();
    }

    // [Shift + A] - Select all actors inside the selection volume.
    if ( ( key == InputManager::Keys::shift || key == InputManager::Keys::a ) &&
         ( shiftPressed && m_inputManager.isKeyPressed( InputManager::Keys::a ) ) ) {
        m_sceneManager.selectAllInsideSelectionVolume();
    }

    // [Delete] - Delete selected actors and lights.
    if ( key == InputManager::Keys::delete_ ) 
    {
        m_sceneManager.deleteSelected();
        m_renderer.renderShadowMaps( m_sceneManager.getScene() );
    }

    // [+] or [-] - Change light brightness.
    if ( !ctrlPressed && !shiftPressed 
         && !m_inputManager.isKeyPressed( InputManager::Keys::p ) )
    {
        if ( key == InputManager::Keys::plus || key == InputManager::Keys::minus ) 
        {
            const float3 colorChange =
                ( key == InputManager::Keys::plus ) ?
                float3( 0.05f, 0.05f, 0.05f ) :
                float3( -0.05f, -0.05f, -0.05f );

            m_sceneManager.modifySelectedLightsColor( colorChange );
        }
    }

    // [Shift] and ( [+] or [-] ) - Modify spot light cone angle.
    if ( !m_sceneManager.getSelectedLights().empty() && shiftPressed 
         && !ctrlPressed ) 
    {
        const float sensitivity = 0.01f;

        float change = 0.0f;
        if ( m_inputManager.isKeyPressed( InputManager::Keys::plus ) ) {
            change = sensitivity;
        } else if ( m_inputManager.isKeyPressed( InputManager::Keys::minus ) ) {
            change = -sensitivity;
        }

        if ( change != 0.0f ) {
            for ( auto& light : m_sceneManager.getSelectedLights() ) 
            {
                if ( light->getType() != Light::Type::SpotLight )
                    continue;

                auto& spotLight = static_cast<SpotLight&>( *light );

                spotLight.setConeAngle( std::min( MathUtil::pi, std::max( 0.01f, spotLight.getConeAngle() + change ) ) );
            }
        }
    }

    // [Shift] and [Ctrl] and ( [+] or [-] ) - Modify light emitter radius.
    if ( !m_sceneManager.getSelectedLights().empty() && shiftPressed 
        && ctrlPressed ) 
    {
        const float sensitivity = 0.005f;

        float change = 0.0f;
        if ( m_inputManager.isKeyPressed( InputManager::Keys::plus ) ) {
            change = sensitivity;
        } else if ( m_inputManager.isKeyPressed( InputManager::Keys::minus ) ) {
            change = -sensitivity;
        }

        if ( change != 0.0f ) 
        {
            for ( auto& light : m_sceneManager.getSelectedLights() ) {
                light->setEmitterRadius( std::min( 1.0f, std::max( 0.0f, light->getEmitterRadius() + change ) ) );
            }
        }
    }

    // [R] and [P/N/T/W] and ( [+] or [-] ) - Modify reflection dist-search position diff mul, 
    // normal diff mul or position/normal threshold or "min sample weight based on distance".
    if ( m_sceneManager.isSelectionEmpty() && m_inputManager.isKeyPressed( InputManager::Keys::r ) ) {
        if ( key == InputManager::Keys::plus || key == InputManager::Keys::minus ) {
            const float change =
                ( key == InputManager::Keys::plus ) ?
                0.05f : -0.05f;

            if ( m_inputManager.isKeyPressed( InputManager::Keys::p ) )
            {
                HitDistanceSearchComputeShader::s_positionDiffMul += change; 
            }
            else if ( m_inputManager.isKeyPressed( InputManager::Keys::n ) )
            {
                HitDistanceSearchComputeShader::s_normalDiffMul += change; 
            }
            else if ( m_inputManager.isKeyPressed( InputManager::Keys::t ) )
            {
                HitDistanceSearchComputeShader::s_positionNormalThreshold += change; 
            }
            else if ( m_inputManager.isKeyPressed( InputManager::Keys::w ) ) {
                HitDistanceSearchComputeShader::s_minSampleWeightBasedOnDistance += change;
            }
        }
    }

    // [C] and [P/N/T] and ( [+] or [-] ) - Modify combining shader position diff mul, normal diff mul or position/normal threshold.
    if ( m_sceneManager.isSelectionEmpty() && m_inputManager.isKeyPressed( InputManager::Keys::c ) ) {
        if ( key == InputManager::Keys::plus || key == InputManager::Keys::minus ) {
            const float change =
                ( key == InputManager::Keys::plus ) ?
                0.1f : -0.05f;

            if ( m_inputManager.isKeyPressed( InputManager::Keys::p ) ) {
                Settings::modify().rendering.combining.positionDiffMul  += change;
            } else if ( m_inputManager.isKeyPressed( InputManager::Keys::n ) ) {
                Settings::modify().rendering.combining.normalDiffMul  += change;
            } else if ( m_inputManager.isKeyPressed( InputManager::Keys::t ) ) {
                Settings::modify().rendering.combining.positionNormalThreshold  += change;
            }
        }
    }

    // [C] and [A] and ( [+] or [-] ) - Modify selected model albedo multiplier.
    if ( m_sceneManager.getSelectedBlockActors().size() >= 1 && m_inputManager.isKeyPressed( InputManager::Keys::c ) ) {
        if ( key == InputManager::Keys::plus || key == InputManager::Keys::minus ) {
            const float change =
                ( key == InputManager::Keys::plus ) ?
                0.05f : -0.05f;

            if ( m_inputManager.isKeyPressed( InputManager::Keys::a ) ) {
                auto& blockActors = m_sceneManager.getSelectedBlockActors();
                for ( auto blockActor : blockActors )
                {
                    if ( !blockActor->getModel()->getAlbedoTextures().empty() )
                    {
                        const float4 mul = blockActor->getModel()->getAlbedoTextures()[ 0 ].getColorMultiplier();
                        blockActor->getModel()->getAlbedoTextures()[ 0 ].setColorMultiplier( mul + float4( change ) );
                    }
                }
            }
        }
    }

    // [Enter] - Enable/disable light sources.
    if ( key == InputManager::Keys::enter && !shiftPressed && !ctrlPressed )
        m_sceneManager.enableDisableSelectedLights();

    // [Shift + Enter] - Enable/disable casting shadows for lights and actors.
    if ( key == InputManager::Keys::enter && shiftPressed )
    {
        m_sceneManager.enableDisableCastingShadowsForSelected();
        m_renderer.renderShadowMaps( m_sceneManager.getScene() );
    }

    // [K] - Add keyframe to spot light.
    if ( key == InputManager::Keys::k && m_sceneManager.getSelection().containsOnlyOneSpotLight() )
    {
        std::shared_ptr< SpotLight > spotlight = m_sceneManager.getSelection().getSpotLights().front();

        m_spotlightAnimator.addKeyframe( spotlight );
    }

    // [Space] - Play/pause animation on a spot light.
    if ( key == InputManager::Keys::spacebar && m_sceneManager.getSelection().containsOnlyOneSpotLight() ) 
    {
        std::shared_ptr< SpotLight > spotlight = m_sceneManager.getSelection().getSpotLights().front();

        m_spotlightAnimator.playPause( spotlight );
    }

    // [Shift + C] - Clone the actors, but share their models with the original actors.
    if ( key == InputManager::Keys::c && shiftPressed ) 
        m_sceneManager.cloneInstancesOfSelectedActors();

    // [Ctrl + C] - Clone the actors and clone their models or clone light sources.
    if ( key == InputManager::Keys::c && ctrlPressed ) 
        m_sceneManager.cloneSelectedActorsAndLights();

    // [Ctrl + M] - Merge selected actors/models/meshes etc.
    if ( key == InputManager::Keys::m && ctrlPressed ) 
        m_sceneManager.mergeSelectedActors();

    // [Enter] + [Ctrl] + [Shift] - Render alpha.
    if ( key == InputManager::Keys::enter && ctrlPressed && shiftPressed )
        Settings::modify().debug.debugRenderAlpha = !settings().debug.debugRenderAlpha;

    // [Page up/Page down] - Switch displayed mipmap.
    if ( key == InputManager::Keys::pageUp )
        Settings::modify().debug.debugDisplayedMipmapLevel = std::max( 0, settings().debug.debugDisplayedMipmapLevel - 1 );
    else if ( key == InputManager::Keys::pageDown )
        Settings::modify().debug.debugDisplayedMipmapLevel = settings().debug.debugDisplayedMipmapLevel + 1;

    // [Backspace] - Render in wireframe mode.
    if ( key == InputManager::Keys::backspace )
        Settings::modify().debug.debugWireframeMode = !settings().debug.debugWireframeMode;

    // [Caps Lock] - Enable slowmotion mode.
    if ( key == InputManager::Keys::capsLock )
        Settings::modify().debug.slowmotionMode = !settings().debug.slowmotionMode;

    // [Ctrl + B] - Rebuild bounding box and BVH.
    if ( key == InputManager::Keys::b && ctrlPressed )
        m_sceneManager.rebuildBoundingBoxAndBVH();

    // [Spacebar] - Enable/disable snapping when rotating/translating actors.
    if ( key == InputManager::Keys::spacebar )
        Settings::modify().debug.snappingMode = !settings().debug.snappingMode;

    // [left/right] - Select next/prev actor or light.
    if ( key == InputManager::Keys::right )
        m_sceneManager.selectNext();
    else if ( key == InputManager::Keys::left )
        m_sceneManager.selectPrev();

    // [E] and ( [+] or [-] ) - Change exposure.
    if ( !ctrlPressed && !shiftPressed
         && m_inputManager.isKeyPressed( InputManager::Keys::e ) ) {
        if ( key == InputManager::Keys::plus || key == InputManager::Keys::minus ) {
            const float valueChange =
                ( key == InputManager::Keys::plus ) ? 0.05f : -0.05f;

            m_renderer.setExposure( m_renderer.getExposure() + valueChange );
        }
    }

    /*static float normalThresholdChange = 0.01f;
    if ( inputManager.isKeyPressed( InputManager::Keys::ctrl ) && inputManager.isKeyPressed( InputManager::Keys::n ) )
    {
        if ( key == InputManager::Keys::plus )
            combiningRenderer.setNormalThreshold( combiningRenderer.getNormalThreshold() + normalThresholdChange );
        else if ( key == InputManager::Keys::minus )
            combiningRenderer.setNormalThreshold( combiningRenderer.getNormalThreshold() - normalThresholdChange );
    }

    static float positionThresholdChange = 0.01f;
    if ( inputManager.isKeyPressed( InputManager::Keys::ctrl ) && inputManager.isKeyPressed( InputManager::Keys::p ) )
    {
        if ( key == InputManager::Keys::plus )
            combiningRenderer.setPositionThreshold( combiningRenderer.getPositionThreshold() + positionThresholdChange );
        else if ( key == InputManager::Keys::minus )
            combiningRenderer.setPositionThreshold( combiningRenderer.getPositionThreshold() - positionThresholdChange );
    }*/

    const bool rKeyPressed = m_inputManager.isKeyPressed( InputManager::Keys::r );
    const bool sKeyPressed = m_inputManager.isKeyPressed( InputManager::Keys::s );

    if ( !rKeyPressed && !sKeyPressed )
    {
        if ( key == InputManager::Keys::tilde ) {
            m_renderer.setActiveViewType( Renderer::View::Final );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::one ) {
            m_renderer.setActiveViewType( Renderer::View::Shaded );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::two ) {
            m_renderer.setActiveViewType( Renderer::View::Depth );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::three ) {
            m_renderer.setActiveViewType( Renderer::View::Position );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::four ) {
            m_renderer.setActiveViewType( Renderer::View::Emissive );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::five ) {
            m_renderer.setActiveViewType( Renderer::View::Albedo );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::six ) {
            m_renderer.setActiveViewType( Renderer::View::Normal );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::seven ) {
            m_renderer.setActiveViewType( Renderer::View::Metalness );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::eight ) {
            m_renderer.setActiveViewType( Renderer::View::Roughness );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::nine ) {
            m_renderer.setActiveViewType( Renderer::View::IndexOfRefraction );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::f1 ) {
            m_renderer.setActiveViewType( Renderer::View::ShadedCombined );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::f2 ) {
            m_renderer.setActiveViewType( Renderer::View::RayDirections );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::f3 ) {
            m_renderer.setActiveViewType( Renderer::View::Contribution );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::f4 ) {
            m_renderer.setActiveViewType( Renderer::View::CurrentRefractiveIndex );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::f5 ) {
            m_renderer.setActiveViewType( Renderer::View::BloomBrightPixels );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        }
    }
    else if ( sKeyPressed )
    {
        if ( key == InputManager::Keys::one ) {
            m_renderer.setActiveViewType( Renderer::View::Preillumination );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::two ) {
            m_renderer.setActiveViewType( Renderer::View::HardShadow );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::three ) {
            m_renderer.setActiveViewType( Renderer::View::MediumShadow );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::four ) {
            m_renderer.setActiveViewType( Renderer::View::SoftShadow );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::five ) {
            m_renderer.setActiveViewType( Renderer::View::BlurredHardShadows );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::six ) {
            m_renderer.setActiveViewType( Renderer::View::BlurredMediumShadows );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::seven ) {
            m_renderer.setActiveViewType( Renderer::View::BlurredSoftShadows );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::eight ) {
            m_renderer.setActiveViewType( Renderer::View::BlurredShadows );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::f1 ) {
            m_renderer.setActiveViewType( Renderer::View::DistanceToOccluderHardShadow );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::f2 ) {
            m_renderer.setActiveViewType( Renderer::View::DistanceToOccluderMediumShadow );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::f3 ) {
            m_renderer.setActiveViewType( Renderer::View::DistanceToOccluderSoftShadow );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::f4 ) {
            m_renderer.setActiveViewType( Renderer::View::FinalDistanceToOccluderHardShadow );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::f5 ) {
            m_renderer.setActiveViewType( Renderer::View::FinalDistanceToOccluderMediumShadow );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::f6 ) {
            m_renderer.setActiveViewType( Renderer::View::FinalDistanceToOccluderSoftShadow );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        }
    }
    else if ( rKeyPressed ) 
    {
        if ( key == InputManager::Keys::one ) {
            m_renderer.setActiveViewType( Renderer::View::HitDistance );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::two ) {
            m_renderer.setActiveViewType( Renderer::View::HitDistanceBlurred );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        } else if ( key == InputManager::Keys::three ) {
            m_renderer.setActiveViewType( Renderer::View::HitDistanceToCamera );
            Settings::modify().debug.debugDisplayedMipmapLevel = 0;
        }
    }

    if ( m_sceneManager.isSelectionEmpty() )
    {
        if ( key == InputManager::Keys::up && ctrlPressed )
            Settings::modify().rendering.reflectionsRefractions.maxLevel++;
        else if ( key == InputManager::Keys::down && ctrlPressed )
            Settings::modify().rendering.reflectionsRefractions.maxLevel--;
        else if ( key == InputManager::Keys::plus && m_inputManager.isKeyPressed( InputManager::Keys::r ) )
            Settings::modify().rendering.reflectionsRefractions.activeView.push_back( true );
        else if ( key == InputManager::Keys::plus && m_inputManager.isKeyPressed( InputManager::Keys::t ) )
            Settings::modify().rendering.reflectionsRefractions.activeView.push_back( false );
        else if ( key == InputManager::Keys::minus && !Settings::modify().rendering.reflectionsRefractions.activeView.empty() )
            Settings::modify().rendering.reflectionsRefractions.activeView.pop_back();
        else if ( key == InputManager::Keys::b )
            Settings::modify().rendering.shadows.useSeparableShadowBlur = !settings().rendering.shadows.useSeparableShadowBlur;
    }
}

void EngineApplication::onMouseButtonPress( int button )
{
    __super::onMouseButtonPress( button );

    if ( button == 0 ) // On left button press.
    {
        // Calculate mouse pos relative to app window top-left corner.
        int2 mousePos = m_inputManager.getMousePos();
        mousePos -= m_windowPosition; 

        const float fieldOfView = m_sceneManager.getCamera().getFieldOfView();

        std::shared_ptr< Actor > pickedActor;
        std::shared_ptr< Light > pickedLight;
        std::tie( pickedActor, pickedLight ) = m_sceneManager.pickActorOrLight( float2( (float)mousePos.x, (float)mousePos.y ), (float)settings().main.screenDimensions.x, (float)settings().main.screenDimensions.y, fieldOfView );

        if ( pickedActor )
        {
            if ( m_inputManager.isKeyPressed( InputManager::Keys::shift ) ) {       // Add picked actor to selection.
                m_sceneManager.selectActor( pickedActor );
            } else if ( m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) ) { // Remove picked actor from selection.
                m_sceneManager.unselectActor( pickedActor );
            } else {                                                                // Clear selection and select the picked actor.
                m_sceneManager.clearSelection();
                m_sceneManager.selectActor( pickedActor );
            }

            onSelectionChanged();
        }
        else if ( pickedLight ) 
        {
            if ( m_inputManager.isKeyPressed( InputManager::Keys::shift ) ) {       // Add picked light to selection.
                m_sceneManager.selectLight( pickedLight );
            } else if ( m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) ) { // Remove picked light from selection.
                m_sceneManager.unselectLight( pickedLight );
            } else {                                                                // Clear selection and select the picked light.
                m_sceneManager.clearSelection();
                m_sceneManager.selectLight( pickedLight );
            }

            onSelectionChanged();
        }
    }
}

void EngineApplication::onDragAndDropFile( std::string filePath, bool replaceSelected )
{
    __super::onDragAndDropFile( filePath, replaceSelected );

	std::string currentPath;
	{
		const DWORD charCount = GetCurrentDirectoryW( 0, nullptr );
		std::vector<wchar_t> currentPathBufferW;
		currentPathBufferW.resize( charCount );
		GetCurrentDirectoryW( charCount, (LPWSTR)currentPathBufferW.data( ) );
		std::wstring currentPathW( currentPathBufferW.data( ), charCount - 1 );
		currentPath = StringUtil::narrow( currentPathW );
	}

	// Transform absolute path into relative path.
	if ( filePath.find( currentPath ) == 0 )
		filePath = filePath.substr( currentPath.size( ) );

	// Remove "\\" from the beginning of the path.
	if ( filePath.find( "\\" ) == 0 )
		filePath = filePath.substr( 1 );

    // Temporarily always replace assets. Holding Ctrl is too hard...
    const bool invertZ = true;//!m_inputManager.isKeyPressed( InputManager::Keys::shift );
    const bool invertVertexWindingOrder = true;
    const bool invertUVs = false;

    m_sceneManager.loadAsset( filePath, replaceSelected, invertZ );

    m_renderer.renderShadowMaps( m_sceneManager.getScene() );
}

bool EngineApplication::onFrame( const double frameTimeMs, const bool lockCursor )
{
    __super::onFrame( frameTimeMs, lockCursor );

    bool modifyingScene = false;

    m_spotlightAnimator.update( (float)(frameTimeMs / 1000.0) );

    // Set renderer exposure from settings.
    m_renderer.setExposure( settings().rendering.exposure );

    // Translate / rotate the selected actors.
    if ( m_windowFocused && ( !m_sceneManager.getSelectedBlockActors().empty() || !m_sceneManager.getSelectedSkeletonActors().empty() ) ) {
        const int2 mouseMove = m_inputManager.getMouseMove();

        const float3 sensitivity(
            m_inputManager.isKeyPressed( InputManager::Keys::x ) ? 1.0f : 0.0f,
            m_inputManager.isKeyPressed( InputManager::Keys::y ) ? 1.0f : 0.0f,
            m_inputManager.isKeyPressed( InputManager::Keys::z ) ? 1.0f : 0.0f
        );

        // Move along horizontal and vertical axes added together.
        float mouseTotalMove = (float)( mouseMove.x - mouseMove.y );

        if ( settings().debug.snappingMode ) {
            const float rotationSnapAngleDegrees = settings().debug.slowmotionMode ? 1.0f : 5.0f;
            const float translationSnapDist = settings().debug.slowmotionMode ? 0.01f : 0.1f;

            if ( fabs( mouseTotalMove ) > 1.0f ) {
                if ( m_inputManager.isKeyPressed( InputManager::Keys::r ) ) {
                    for ( auto& actor : m_sceneManager.getSelectedBlockActors() )
                        actor->getPose().rotate( MathUtil::sign( mouseTotalMove ) * sensitivity * ( rotationSnapAngleDegrees / 360.0f ) * MathUtil::piTwo );

                    for ( auto& actor : m_sceneManager.getSelectedSkeletonActors() )
                        actor->getPose().rotate( MathUtil::sign( mouseTotalMove ) * sensitivity * ( rotationSnapAngleDegrees / 360.0f ) * MathUtil::piTwo );

                    modifyingScene = true;
                } else if ( m_inputManager.isKeyPressed( InputManager::Keys::t ) ) {
                    for ( auto& actor : m_sceneManager.getSelectedBlockActors() )
                        actor->getPose().translate( mouseTotalMove * translationSnapDist * sensitivity );

                    for ( auto& actor : m_sceneManager.getSelectedSkeletonActors() )
                        actor->getPose().translate( mouseTotalMove * translationSnapDist * sensitivity );

                    modifyingScene = true;
                }
            }
        } else {
            const float translationSensitivity = settings().debug.slowmotionMode ? 0.00001f : 0.0002f;
            const float rotationSensitivity = settings().debug.slowmotionMode ? 0.00001f : 0.0001f;

            if ( m_inputManager.isKeyPressed( InputManager::Keys::r ) ) {
                for ( auto& actor : m_sceneManager.getSelectedBlockActors() )
                    actor->getPose().rotate( mouseTotalMove * (float)frameTimeMs * sensitivity * rotationSensitivity );

                for ( auto& actor : m_sceneManager.getSelectedSkeletonActors() )
                    actor->getPose().rotate( mouseTotalMove * (float)frameTimeMs * sensitivity * rotationSensitivity );

                modifyingScene = true;
            } else if ( m_inputManager.isKeyPressed( InputManager::Keys::t ) ) {
                for ( auto& actor : m_sceneManager.getSelectedBlockActors() )
                    actor->getPose().translate( mouseTotalMove * (float)frameTimeMs * sensitivity * translationSensitivity );

                for ( auto& actor : m_sceneManager.getSelectedSkeletonActors() )
                    actor->getPose().translate( mouseTotalMove * (float)frameTimeMs * sensitivity * translationSensitivity );

                modifyingScene = true;
            }
        }
    }

    // Translate / rotate the selected light.
    if ( m_windowFocused && !m_sceneManager.getSelectedLights().empty() ) {
        const float   translationSensitivity = settings().debug.slowmotionMode ? 0.00005f : 0.0002f;
        const float   rotationSensitivity = settings().debug.slowmotionMode ? 0.00001f : 0.0001f;
        const int2    mouseMove = m_inputManager.getMouseMove();

        float mouseTotalMove = (float)( mouseMove.x - mouseMove.y );

        const float3 sensitivity(
            m_inputManager.isKeyPressed( InputManager::Keys::x ) ? 1.0f : 0.0f,
            m_inputManager.isKeyPressed( InputManager::Keys::y ) ? 1.0f : 0.0f,
            m_inputManager.isKeyPressed( InputManager::Keys::z ) ? 1.0f : 0.0f
        );

        if ( m_inputManager.isKeyPressed( InputManager::Keys::t ) ) {
            for ( auto& light : m_sceneManager.getSelectedLights() )
                light->setPosition( light->getPosition() + ( mouseTotalMove * (float)frameTimeMs * sensitivity * translationSensitivity ) );

            modifyingScene = true;
        } else if ( m_inputManager.isKeyPressed( InputManager::Keys::r ) ) {
            for ( auto& light : m_sceneManager.getSelectedLights() ) {
                if ( light->getType() != Light::Type::SpotLight )
                    continue;

                auto& spotLight = static_cast<SpotLight&>( *light );

                float3 direction = spotLight.getDirection();
                direction.rotate( mouseTotalMove * (float)frameTimeMs * sensitivity * rotationSensitivity );

                spotLight.setDirection( direction );
            }

            modifyingScene = true;
        }
    }

    // Set camera to align with a spot light.
    if ( m_windowFocused && m_sceneManager.getSelectedLights().size() == 1 && m_inputManager.isKeyPressed( InputManager::Keys::ctrl ) && m_inputManager.isKeyPressed( InputManager::Keys::l ) ) {
        const Light& light = *m_sceneManager.getSelectedLights()[ 0 ];

        if ( light.getType() == Light::Type::SpotLight ) {
            const SpotLight& spotLight = static_cast<const SpotLight&>( light );

            m_sceneManager.getCamera().setFieldOfView( spotLight.getConeAngle() );
            m_sceneManager.getCamera().setPosition( spotLight.getPosition() );
            m_sceneManager.getCamera().setDirection( spotLight.getDirection() );
        }
    }
    else
    {
        // Update camera FOV from settings.
        m_sceneManager.getCamera().setFieldOfView( MathUtil::degreesToRadians( settings().rendering.fieldOfViewDegress ) );
    }

    // Update the camera.
    if ( m_windowFocused && !modifyingScene && m_inputManager.isMouseButtonPressed( InputManager::MouseButtons::right ) ) {
        const float cameraRotationSensitivity = settings().debug.slowmotionMode ? 0.00002f : 0.0001f;
        const float acceleration = settings().debug.slowmotionMode ? 0.02f : 0.25f;

        if ( m_inputManager.isKeyPressed( InputManager::Keys::w ) )      m_sceneManager.getCamera().accelerateForward( (float)frameTimeMs * acceleration );
        else if ( m_inputManager.isKeyPressed( InputManager::Keys::s ) ) m_sceneManager.getCamera().accelerateReverse( (float)frameTimeMs * acceleration );
        if ( m_inputManager.isKeyPressed( InputManager::Keys::d ) )      m_sceneManager.getCamera().accelerateRight( (float)frameTimeMs * acceleration );
        else if ( m_inputManager.isKeyPressed( InputManager::Keys::a ) ) m_sceneManager.getCamera().accelerateLeft( (float)frameTimeMs * acceleration );
        if ( m_inputManager.isKeyPressed( InputManager::Keys::e ) )      m_sceneManager.getCamera().accelerateUp( (float)frameTimeMs * acceleration );
        else if ( m_inputManager.isKeyPressed( InputManager::Keys::q ) ) m_sceneManager.getCamera().accelerateDown( (float)frameTimeMs * acceleration );

        int2 mouseMove = m_inputManager.getMouseMove();
        m_sceneManager.getCamera().rotate( float3( -(float)mouseMove.y, -(float)mouseMove.x, 0.0f ) * (float)frameTimeMs * cameraRotationSensitivity );
    }

    m_inputManager.lockCursor( lockCursor );
    m_inputManager.updateMouseState();

    m_sceneManager.getCamera().updateState( (float)frameTimeMs );

    { // Update animations.
        const std::unordered_set< std::shared_ptr<Actor> >& sceneActors = m_sceneManager.getScene().getActors();

        for ( const std::shared_ptr<Actor>& actor : sceneActors ) {
            if ( actor->getType() != Actor::Type::SkeletonActor )
                continue;

            const std::shared_ptr< SkeletonActor > skeletonActor = std::dynamic_pointer_cast<SkeletonActor>( actor );

            skeletonActor->updateAnimation( (float)frameTimeMs / 1000.0f );
        }
    }

    { // Update color multipliers from Settings.
        auto& selectedBlockActors = m_sceneManager.getSelectedBlockActors();
        auto& selectedSkeletonActors = m_sceneManager.getSelectedSkeletonActors();

        for ( auto& actor : selectedBlockActors )
        {
            if ( actor->getModel() )
                setModelTextureMultipliersFromSettings( *actor->getModel() );
        } 
    
        for ( auto& actor : selectedSkeletonActors )
        {
            if ( actor->getModel() )
                setModelTextureMultipliersFromSettings( *actor->getModel() );
        }

        // Apply modified multipliers only once after they have been changed in Control Panel.
        Settings::s_settings.debug.alphaMulChanged           = false;
        Settings::s_settings.debug.emissiveMulChanged        = false;
        Settings::s_settings.debug.albedoMulChanged          = false;
        Settings::s_settings.debug.metalnessMulChanged       = false;
        Settings::s_settings.debug.roughnessMulChanged       = false;
        Settings::s_settings.debug.refractiveIndexMulChanged = false;
    }

    { // Update light setup from Settings.
        auto& selectedLights = m_sceneManager.getSelectedLights();

        for ( auto& light : selectedLights ) {
            setLightFromSettings( *light );
        } 
    }

    return modifyingScene;
}

void EngineApplication::onSelectionChanged()
{
    __super::onSelectionChanged();

    if ( !m_sceneManager.getSelectedBlockActors().empty() )
    {
        auto actor = m_sceneManager.getSelectedBlockActors()[ 0 ];
        if ( actor->getModel() )
            setTextureMultipliersInSettingsFromModel( *actor->getModel() );
    }

    if ( !m_sceneManager.getSelectedLights().empty() )
    {
        auto light = m_sceneManager.getSelectedLights()[ 0 ];

        setSettingsFromLight( *light );
    }
}

void EngineApplication::setTextureMultipliersInSettingsFromModel( const Model& model )
{
    auto& alphaTextures = model.getAlphaTextures();
    auto& emissiveTextures = model.getEmissiveTextures();
    auto& albedoTextures = model.getAlbedoTextures();
    auto& metalnessTextures = model.getMetalnessTextures();
    auto& roughnessTextures = model.getRoughnessTextures();
    auto& refractiveIndexTextures = model.getRefractiveIndexTextures();

    if ( !alphaTextures.empty() )
        Settings::modify().debug.alphaMul = alphaTextures[ 0 ].getColorMultiplier().x;

    if ( !emissiveTextures.empty() )
    {
        auto emissiveMul    = emissiveTextures[ 0 ].getColorMultiplier();
        auto maxEmissiveMul = std::max(std::max(emissiveMul.x, emissiveMul.y), emissiveMul.z);

        Settings::modify().debug.emissiveMul     = maxEmissiveMul;
        Settings::modify().debug.emissiveBaseMul = emissiveTextures[ 0 ].getColorMultiplier() / maxEmissiveMul;
    }

    if ( !albedoTextures.empty() )
        Settings::modify().debug.albedoMul = albedoTextures[ 0 ].getColorMultiplier();

    if ( !metalnessTextures.empty() )
        Settings::modify().debug.metalnessMul = metalnessTextures[ 0 ].getColorMultiplier().x;

    if ( !roughnessTextures.empty() )
        Settings::modify().debug.roughnessMul = roughnessTextures[ 0 ].getColorMultiplier().x;

    if ( !refractiveIndexTextures.empty() )
        Settings::modify().debug.refractiveIndexMul = refractiveIndexTextures[ 0 ].getColorMultiplier().x;
}

void EngineApplication::setModelTextureMultipliersFromSettings( Model& model )
{
    auto& alphaTextures = model.getAlphaTextures();
    auto& emissiveTextures = model.getEmissiveTextures();
    auto& albedoTextures = model.getAlbedoTextures();
    auto& metalnessTextures = model.getMetalnessTextures();
    auto& roughnessTextures = model.getRoughnessTextures();
    auto& refractiveIndexTextures = model.getRefractiveIndexTextures();

    if ( !alphaTextures.empty() && settings().debug.alphaMulChanged )
        alphaTextures[ 0 ].setColorMultiplier( float4( settings().debug.alphaMul ) );

    if ( !emissiveTextures.empty() && settings().debug.emissiveMulChanged )
        emissiveTextures[ 0 ].setColorMultiplier( float4( settings().debug.emissiveMul * settings().debug.emissiveBaseMul, 0.0f ) );

    if ( !albedoTextures.empty() && settings().debug.albedoMulChanged )
        albedoTextures[ 0 ].setColorMultiplier( float4( settings().debug.albedoMul, 0.0f ) );

    if ( !metalnessTextures.empty() && settings().debug.metalnessMulChanged )
        metalnessTextures[ 0 ].setColorMultiplier( float4( settings().debug.metalnessMul ) );

    if ( !roughnessTextures.empty() && settings().debug.roughnessMulChanged )
        roughnessTextures[ 0 ].setColorMultiplier( float4( settings().debug.roughnessMul ) );

    if ( !refractiveIndexTextures.empty() && settings().debug.refractiveIndexMulChanged )
        refractiveIndexTextures[ 0 ].setColorMultiplier( float4( settings().debug.refractiveIndexMul ) );

    auto& settings = Settings::modify();
    settings.debug.alphaMulChanged           = false;
    settings.debug.emissiveMulChanged        = false;
    settings.debug.albedoMulChanged          = false;
    settings.debug.metalnessMulChanged       = false;
    settings.debug.roughnessMulChanged       = false;
    settings.debug.refractiveIndexMulChanged = false;
}

void EngineApplication::setSettingsFromLight( const Light& light )
{
    auto& settings = Settings::modify();

    settings.debug.lightEnabled                    = light.isEnabled();
    settings.debug.lightCastShadows                = light.isCastingShadows();
    settings.debug.lightColor                      = light.getColor();
    settings.debug.lightColor.normalize();
    settings.debug.lightIntensity                  = light.getColor().length();
    settings.debug.lightEmitterRadius              = light.getEmitterRadius();
    settings.debug.lightLinearAttenuationFactor    = light.getLinearAttenuationFactor();
    settings.debug.lightQuadraticAttenuationFactor = light.getQuadraticAttenuationFactor();
}

void EngineApplication::setLightFromSettings( Light& light )
{
    if ( settings().debug.lightEnabledChanged )
        light.setEnabled( settings().debug.lightEnabled ); 

    if ( settings().debug.lightCastShadowsChanged )
        light.setCastingShadows( settings().debug.lightCastShadows ); 

    if ( settings().debug.lightIntensityChanged || settings().debug.lightColorChanged )
        light.setColor( settings().debug.lightColor * settings().debug.lightIntensity ); 

    if ( settings().debug.lightEmitterRadiusChanged )
        light.setEmitterRadius( settings().debug.lightEmitterRadius );    

    //if ( settings().debug.lightLinearAttenuationFactorChanged )
    //    light.setLinearAttenuationFactor( settings().debug.lightLinearAttenuationFactor ); //#TODO: Uncomment once the member has been added.

    //if ( settings().debug.lightQuadraticAttenuationFactorChanged )
    //    light.setQuadraticAttenuationFactor( settings().debug.lightQuadraticAttenuationFactor ); //#TODO: Uncomment once the member has been added.

    auto& settings = Settings::modify();

    settings.debug.lightEnabledChanged                    = false;
    settings.debug.lightCastShadowsChanged                = false;
    settings.debug.lightIntensityChanged                  = false;
    settings.debug.lightEmitterRadiusChanged              = false;
    settings.debug.lightLinearAttenuationFactorChanged    = false;
    settings.debug.lightQuadraticAttenuationFactorChanged = false;
}
