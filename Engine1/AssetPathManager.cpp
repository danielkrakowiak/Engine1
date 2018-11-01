#include "AssetPathManager.h"

using namespace Engine1;

PathManager AssetPathManager::s_assetPathManager;
bool AssetPathManager::s_initialized = initialize();

bool AssetPathManager::initialize()
{
    s_assetPathManager.scanDirectory( "Assets" );

    return true;
}

PathManager& AssetPathManager::get()
{
    return s_assetPathManager;
}