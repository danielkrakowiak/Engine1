#pragma once

#include <string>

namespace Engine1
{
    class Settings;

    class SettingsHelper
    {
        public:
    
        static std::string compareSettings( const Settings& settings1, const Settings& settings2 );
    };
}

