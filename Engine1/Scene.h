#pragma once

#include <unordered_set>
#include <memory>

class Actor;

class Scene
{
    public:

    Scene();
    ~Scene();

    void addActor( std::shared_ptr<Actor> actor );
    void removeActor( std::shared_ptr<Actor> actor );
    void removeAllActors();

    const std::unordered_set< std::shared_ptr<Actor> >& getActors( );

    private:

    std::unordered_set< std::shared_ptr<Actor> > actors;
};

