#pragma once

#include <vector>
#include <memory>
#include <tuple>
#include <wrl.h>

#include "FreeCamera.h"
#include "BoundingBox.h"

#include "SpotLight.h" 
#include "FreeCamera.h"
#include "BlockActor.h"
#include "BlockModel.h"

#include "Texture2DGeneric.h"

#include "Selection.h"
#include "Animator.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class Light;
    class SkeletonActor;
    class AssetManager;
    class PathManager;
    class Scene;
    class Actor;
    class BlockMesh;

    class SceneManager
    {
        public:

        SceneManager( AssetManager& assetManager );

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 > device, Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        std::shared_ptr< FreeCamera >& getCamera();
        std::shared_ptr< Scene >&      getScene();

        const Selection&                                       getSelection();
        const std::vector< std::shared_ptr< Light > >&         getSelectedLights();
        const std::vector< std::shared_ptr< BlockActor > >&    getSelectedBlockActors();
        const std::vector< std::shared_ptr< SkeletonActor > >& getSelectedSkeletonActors();

        std::shared_ptr< BlockMesh > getSelectionVolumeMesh();

        void loadCamera( std::string path );
        void saveCamera( std::string path );

        void loadScene( std::string path );
        void saveScene( std::string path );

        void loadAsset( std::string fileName, const bool replaceSelected = false, const bool invertZ = true, const bool invertVertexWindingOrder = true, const bool invertUVs = true );

        void unloadAll();

        // Returns: tuple< picked actor, picked light, hit-distance >
        std::tuple< std::shared_ptr< Actor >, std::shared_ptr< Light >, float >
            pickActorOrLight( const float2& targetPixel, const float screenWidth, const float screenHeight, const float fieldOfView );

        void mergeSelectedActors();
        void saveSelectedModels();

        std::tuple< int, int > getSelectedActorsVertexAndTriangleCount();
        std::tuple< int, int > getSceneVertexAndTriangleCount();

        void addPointLight();
        void addSpotLight();

        void modifySelectedLightsColor( float3 colorChange );
        void enableDisableSelectedLights();

        void saveSceneOrSelectedModels();
        
        bool isSelectionEmpty();
        void selectAll();
        void clearSelection();
        bool isActorSelected( const std::shared_ptr< Actor >& actor );
        void selectActor( const std::shared_ptr< Actor >& actor );
        void unselectActor( const std::shared_ptr< Actor >& actor );
        bool isLightSelected( const std::shared_ptr< Light >& light );
        void selectLight( const std::shared_ptr< Light >& light );
        void unselectLight( const std::shared_ptr< Light >& light );
        void selectNext();
        void selectPrev();

        void deleteSelected();

        void enableDisableCastingShadowsForSelected();

        void cloneInstancesOfSelectedActors();
        void cloneSelectedActorsAndLights();

        void recreateSelectionVolumeMesh();
        void recalculateSelectionVolume();
        void modifySelectionVolume( const float3 minChange, const float3 maxChange );
        void selectAllInsideSelectionVolume();

        Animator< SpotLight >&  getLightAnimator();
        Animator< FreeCamera >& getCameraAnimator();
        Animator< BlockActor >& getActorAnimator();
        Animator< BlockModel >& getModelAnimator();

        void rebuildBoundingBoxAndBVH();

        void flipTexcoordsVerticallyAndResaveMesh();
        void flipTangentsAndResaveMesh();
        void flipNormalsAndResaveMesh();
        void invertVertexWindingOrderAndResaveMesh();

        private:

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        AssetManager& m_assetManager;

        std::string m_cameraPath;
        
        std::shared_ptr< FreeCamera > m_camera;

        std::string m_scenePath;
        std::shared_ptr< Scene > m_scene;

        Selection m_selection;

        BoundingBox                   m_selectionVolume;
        std::shared_ptr< BlockMesh > m_selectionVolumeMesh;

        Animator< SpotLight >  m_spotlightAnimator;
        Animator< FreeCamera > m_cameraAnimator;
        Animator< BlockActor > m_actorAnimator;
        Animator< BlockModel > m_modelAnimator;

        std::vector< std::shared_ptr< Texture2DGeneric< unsigned char > > > m_texturesToMerge;
        std::vector< std::shared_ptr< BlockMesh > >                         m_meshesToMerge;
    };
};

