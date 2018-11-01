#pragma once

#include "PathManager.h"

namespace Engine1
{
    class AssetPathManager
    {
        public:

        static PathManager& get();

        private:

        static bool initialize();

        static bool        s_initialized;
        static PathManager s_assetPathManager;
    };
}