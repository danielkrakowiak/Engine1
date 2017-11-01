#pragma once

#include "Application.h"

namespace Engine1
{
    class EngineApplication : public Application
    {
        public:

        EngineApplication();
        ~EngineApplication();

        private:

        void onStart() override;
        void onExit() override;
        void onResize( int newWidth, int newHeight ) override;
        void onMove( int newPosX, int newPosY ) override;
        void onFocusChange( bool windowFocused ) override;
        void onKeyPress( int key ) override;
        void onMouseButtonPress( int button ) override;
        void onDragAndDropFile( std::string filePath, bool replaceSelected ) override;
        bool onFrame( const double frameTimeMs, const bool lockCursor ) override; // returns: modifyingScene
        void onSelectionChanged() override;

        void setTextureMultipliersInSettingsFromModel( const Model& model );
        void setModelTextureMultipliersFromSettings( Model& model );

        void setSettingsFromLight( const Light& light );
        void setLightFromSettings( Light& light );

        
    };
}

