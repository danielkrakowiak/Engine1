#pragma once

#include <vector>
#include <memory>
#include <tuple>
#include <wrl.h>

#include "FreeCamera.h"
#include "BoundingBox.h"

#include "Texture2DGeneric.h"

#include "Selection.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class Light;
    class BlockActor;
    class SkeletonActor;
    class AssetManager;
    class AssetPathManager;
    class Scene;
    class Actor;
    class BlockMesh;
    class BlockModel;

    class SceneManager
    {
        public:

        SceneManager( AssetManager& assetManager );

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device > device, Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        FreeCamera& getCamera();
        Scene&     getScene();

        const std::vector< std::shared_ptr< Light > >&         getSelectedLights();
        const std::vector< std::shared_ptr< BlockActor > >&    getSelectedBlockActors();
        const std::vector< std::shared_ptr< SkeletonActor > >& getSelectedSkeletonActors();

        std::shared_ptr< BlockMesh > getSelectionVolumeMesh();

        void loadCamera( std::string path );
        void saveCamera( std::string path );

        void loadScene( std::string path );
        void saveScene( std::string path );

        void loadAsset( std::string fileName, const bool replaceSelected = false, const bool invertZ = true, const bool invertVertexWindingOrder = true, const bool invertUVs = true );

        std::tuple< std::shared_ptr< Actor >, std::shared_ptr< Light > >
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

        void rebuildBoundingBoxAndBVH();

        private:

        Microsoft::WRL::ComPtr< ID3D11Device >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_deviceContext;

        AssetManager& m_assetManager;

        std::string m_cameraPath;
        FreeCamera m_camera;

        std::string m_scenePath;
        std::shared_ptr< Scene > m_scene;

        Selection m_selection;

        BoundingBox                   m_selectionVolume;
        std::shared_ptr< BlockMesh > m_selectionVolumeMesh;

        std::vector< std::shared_ptr< Texture2DGeneric< unsigned char > > > m_texturesToMerge;
        std::vector< std::shared_ptr< BlockMesh > >                         m_meshesToMerge;
    };
};

