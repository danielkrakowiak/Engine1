#include "SceneParser.h"

#include "BinaryFile.h"
#include "FileInfo.h"
#include "Scene.h"
#include "Actor.h"
#include "BlockActor.h"
#include "BlockModel.h"
#include "BlockModelFileInfo.h"
#include "SkeletonActor.h"
#include "SkeletonModel.h"
#include "SkeletonModelFileInfo.h"

#include "PointLight.h"
#include "SpotLight.h"

#include <unordered_map>

using namespace Engine1;

std::tuple< std::shared_ptr<Scene>, std::shared_ptr<std::vector< std::shared_ptr<FileInfo> > > > SceneParser::parseBinary( const std::vector<char>& data )
{
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();

    std::vector<char>::const_iterator dataIt = data.begin();

    // Assigns each unique FileInfo an unique temporary id (index in the vector).
    std::shared_ptr < std::vector< std::shared_ptr< FileInfo > > > fileInfos = std::make_shared< std::vector<std::shared_ptr< FileInfo > > >();

    { // Read actors.
        // Read number of unique file infos.
        const int fileInfoCount = BinaryFile::readInt( dataIt );
        
        fileInfos->resize( fileInfoCount );

        // Read and collect all unique file infos.
        for ( int i = 0; i < fileInfoCount; ++i ) {
            const int                 fileInfoId = BinaryFile::readInt( dataIt );                              // Read unique file info id.
            const Asset::Type         assetType = static_cast<Asset::Type>(BinaryFile::readChar( dataIt )); // Read file info type.
            std::shared_ptr<FileInfo> fileInfo = nullptr;

            // Read file info.
            if ( assetType == Asset::Type::BlockModel )
                fileInfo = BlockModelFileInfo::createFromMemory( dataIt );
            else if ( assetType == Asset::Type::SkeletonModel )
                fileInfo = SkeletonModelFileInfo::createFromMemory( dataIt );

            fileInfos->at( fileInfoId ) = fileInfo;
        }

        // Read number of actors.
        const int actorsCount = BinaryFile::readInt( dataIt );

        // Read all actors.
        for ( int i = 0; i < actorsCount; ++i ) {
            float43   pose = BinaryFile::readFloat43( dataIt ); // Read actor's pose.
            const int fileInfoId = BinaryFile::readInt( dataIt );     // Read model's file info temporary id.

            const std::shared_ptr<FileInfo> fileInfo = fileInfos->at( fileInfoId );

            if ( fileInfo->getAssetType() == Asset::Type::BlockModel ) {
                std::shared_ptr<BlockModel> model = std::make_shared<BlockModel>();
                model->setFileInfo( *std::static_pointer_cast<BlockModelFileInfo>(fileInfo) );

                scene->addActor( std::make_shared<BlockActor>( model, pose ) );
            } else if ( fileInfo->getAssetType() == Asset::Type::SkeletonModel ) {
                std::shared_ptr<SkeletonModel> model = std::make_shared<SkeletonModel>();
                model->setFileInfo( *std::static_pointer_cast<SkeletonModelFileInfo>(fileInfo) );

                scene->addActor( std::make_shared<SkeletonActor>( model, pose ) );
            }
        }
    }

    { // Read lights.
        // Read number of point lights.
        const int pointLightCount = BinaryFile::readInt( dataIt );

        // Read point lights.
        for ( int i = 0; i < pointLightCount; ++i )
            scene->addLight( PointLight::createFromMemory( dataIt ) );

        // Read number of spot lights.
        const int spotLightCount = BinaryFile::readInt( dataIt );

        // Read spot lights.
        for ( int i = 0; i < spotLightCount; ++i )
            scene->addLight( SpotLight::createFromMemory( dataIt ) );
    }
    
    return std::make_pair( scene, fileInfos );
}

void SceneParser::writeBinary( std::vector<char>& data, const Scene& scene )
{
    { // Save actors.
        // Assigns each unique FileInfo an unique temporary id.
        std::unordered_map<std::shared_ptr<FileInfo>, int, FileInfoHasher, FileInfoComparator> fileInfos;

        int unsavableActorsCount = 0;

        // Collect all unique file infos for each model in the scene.
        for ( const std::shared_ptr<Actor>& actor : scene.m_actors ) {
            if ( actor->getType() == Actor::Type::BlockActor ) {
                const std::shared_ptr<BlockActor>& blockActor = std::static_pointer_cast<BlockActor>(actor);
                if ( blockActor->getModel() && !blockActor->getModel()->getFileInfo().getPath().empty() ) {
                    fileInfos.insert( std::make_pair(
                        std::make_shared<BlockModelFileInfo>( blockActor->getModel()->getFileInfo() ),
                        (int)fileInfos.size() )
                        );
                } else {
                    ++unsavableActorsCount;
                }
            } else if ( actor->getType() == Actor::Type::SkeletonActor ) {
                const std::shared_ptr<SkeletonActor>& skeletonActor = std::static_pointer_cast<SkeletonActor>(actor);
                if ( skeletonActor->getModel() && !skeletonActor->getModel()->getFileInfo().getPath().empty() ) {
                    fileInfos.insert( std::make_pair(
                        std::make_shared<SkeletonModelFileInfo>( skeletonActor->getModel()->getFileInfo() ),
                        (int)fileInfos.size() )
                        );
                } else {
                    ++unsavableActorsCount;
                }
            }
        }

        // Save number of unique file infos.
        BinaryFile::writeInt( data, (int)fileInfos.size() );

        // Save unique file infos.
        for ( const std::pair<std::shared_ptr<FileInfo>, int>& fileInfo : fileInfos ) {
            BinaryFile::writeInt( data, fileInfo.second );                                    // Save unique file info id.
            BinaryFile::writeChar( data, static_cast<char>(fileInfo.first->getAssetType()) ); // Save file info type.
            fileInfo.first->saveToMemory( data );                                              // Save file info.
        }

        // Save number of actors.
        BinaryFile::writeInt( data, (int)scene.m_actors.size() - unsavableActorsCount );

        // Save all actors (pose and file info id).
        for ( const std::shared_ptr<Actor>& actor : scene.m_actors ) {
            if ( actor->getType() == Actor::Type::BlockActor ) {
                const std::shared_ptr<BlockActor>& blockActor = std::static_pointer_cast<BlockActor>(actor);
                if ( blockActor->getModel() && !blockActor->getModel()->getFileInfo().getPath().empty() ) {
                    auto it = fileInfos.find( std::make_shared<BlockModelFileInfo>( blockActor->getModel()->getFileInfo() ) );
                    if ( it == fileInfos.end() )
                        throw std::exception( "SceneParser::writeBinary - unknown error." );

                    BinaryFile::writeFloat43( data, blockActor->getPose() );    // Save actor's pose.
                    BinaryFile::writeInt( data, it->second );                   // Save model's file info temporary id.
                }
            } else if ( actor->getType() == Actor::Type::SkeletonActor ) {
                const std::shared_ptr<SkeletonActor>& skeletonActor = std::static_pointer_cast<SkeletonActor>(actor);
                if ( skeletonActor->getModel() && !skeletonActor->getModel()->getFileInfo().getPath().empty() ) {
                    auto it = fileInfos.find( std::make_shared<SkeletonModelFileInfo>( skeletonActor->getModel()->getFileInfo() ) );
                    if ( it == fileInfos.end() )
                        throw std::exception( "SceneParser::writeBinary - unknown error." );

                    BinaryFile::writeFloat43( data, skeletonActor->getPose() ); // Save actor's pose.
                    BinaryFile::writeInt( data, it->second );                   // Save model's file info temporary id.
                }
            }
        }
    }

    { // Save lights.
        int pointLightCount = 0;
        int spotLightCount  = 0;

        // Count lights of each type.
        for ( const std::shared_ptr< Light >& light : scene.m_lights ) 
        {
            if ( light->getType() == Light::Type::PointLight )
                ++pointLightCount;
            else if ( light->getType() == Light::Type::SpotLight )
                ++spotLightCount;
        }

        // Save number of point lights.
        BinaryFile::writeInt( data, pointLightCount );

        // Save point lights.
        for ( const auto& light : scene.m_lights ) 
        {
            if ( light->getType() == Light::Type::PointLight )
            {
                const auto& pointLight = std::static_pointer_cast< PointLight >( light );
                pointLight->saveToMemory( data );
            }
        }

        // Save number of spot lights.
        BinaryFile::writeInt( data, spotLightCount );

        // Save spot lights.
        for ( const auto& light : scene.m_lights ) 
        {
            if ( light->getType() == Light::Type::SpotLight ) 
            {
                const auto& spotLight = std::static_pointer_cast< SpotLight >( light );
                spotLight->saveToMemory( data );
            }
        }
    }
}

std::size_t SceneParser::FileInfoHasher::operator( )( const std::shared_ptr<FileInfo>& fileInfo ) const
{
    std::hash<std::string> hasher;

    return hasher(
        std::to_string( static_cast<char>(fileInfo->getFileType()) )
        + std::to_string( static_cast<char>(fileInfo->getAssetType()) )
        + fileInfo->getPath()
        + std::to_string( fileInfo->getIndexInFile() )
        );
}

bool SceneParser::FileInfoComparator::operator()( const std::shared_ptr<FileInfo>& fileInfo1, const std::shared_ptr<FileInfo>& fileInfo2 ) const
{
    return fileInfo1->getAssetType() == fileInfo2->getAssetType()
        && fileInfo1->getFileType() == fileInfo2->getFileType()
        && fileInfo1->getIndexInFile() == fileInfo2->getIndexInFile()
        && fileInfo1->getPath().compare( fileInfo2->getPath() ) == 0;
}


