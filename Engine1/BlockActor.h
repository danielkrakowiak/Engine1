#pragma once

#include <memory>

#include "Actor.h"

#include "float43.h"

class BlockModel;

class BlockActor : public Actor
{
    public:

    BlockActor( std::shared_ptr<BlockModel> model, const float43& pose = float43::IDENTITY );
    ~BlockActor();

    Type getType( ) const;

    const float43&                    getPose() const;
    float43&                          getPose();
    std::shared_ptr<const BlockModel> getModel() const;
    std::shared_ptr<BlockModel>       getModel();

    void setPose( const float43& pose );
    void setModel( std::shared_ptr<BlockModel> model );

    private:

    float43 pose;
    std::shared_ptr<BlockModel> model;
};

