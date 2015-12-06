#pragma once

#include <unordered_set>
#include <memory>

namespace Engine1
{
    class Actor;
    class SceneParser;
    class FileInfo;

    class CScene
    {
        friend class SceneParser;

        public:

        // Returns the parsed scene (with models containing only file info) and a vector of unique models (their file infos) in that scene.
        static std::tuple< std::shared_ptr<CScene>, std::shared_ptr<std::vector< std::shared_ptr<FileInfo> > > > createFromFile( std::string path );

        CScene();
        ~CScene();

        void addActor( std::shared_ptr<Actor> actor );
        void removeActor( std::shared_ptr<Actor> actor );
        void removeAllActors();

        const std::unordered_set< std::shared_ptr<Actor> >& getActors();

        void saveToFile( const std::string& path );

        private:

        std::unordered_set< std::shared_ptr<Actor> > actors;
    };
}

