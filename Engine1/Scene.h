#pragma once

#include <unordered_set>
#include <memory>

namespace Engine1
{
    class Actor;
    class Light;
    class SceneParser;
    class FileInfo;

    class Scene
    {
        friend class SceneParser;

        public:

        // Returns the parsed scene (with models containing only file info) and a vector of unique models (their file infos) in that scene.
        static std::tuple< std::shared_ptr<Scene>, std::shared_ptr<std::vector< std::shared_ptr<FileInfo> > > > createFromFile( std::string path );

        Scene();
        ~Scene();

        void addActor( std::shared_ptr<Actor> actor );
        void removeActor( std::shared_ptr<Actor> actor );
        void removeAllActors();

        void addLight( std::shared_ptr<Light> light );
        void removeLight( std::shared_ptr<Light> light );
        void removeAllLights( );

        const std::unordered_set< std::shared_ptr<Actor> >& getActors() const;
        const std::unordered_set< std::shared_ptr<Light> >& getLights( ) const;

        void saveToFile( const std::string& path ) const;

        private:

        std::unordered_set< std::shared_ptr<Actor> > m_actors;
        std::unordered_set< std::shared_ptr<Light> > m_lights;
    };
}

