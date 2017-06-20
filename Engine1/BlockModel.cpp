#include "BlockModel.h"

#include <d3d11.h>
#include <tuple>

#include "BinaryFile.h"
#include "BlockModelParser.h"

using namespace Engine1;

std::shared_ptr<BlockModel> BlockModel::createFromFile( const BlockModelFileInfo& fileInfo, const bool loadRecurrently, ID3D11Device& device )
{
	return createFromFile( fileInfo.getPath(), fileInfo.getFormat(), loadRecurrently, device );
}

std::shared_ptr<BlockModel> BlockModel::createFromFile( const std::string& path, const BlockModelFileInfo::Format format, const bool loadRecurrently, ID3D11Device& device )
{
	std::shared_ptr< std::vector<char> > fileData = BinaryFile::load( path );

	std::shared_ptr<BlockModel> model = createFromMemory( fileData->cbegin(), format, loadRecurrently, device );

    model->getFileInfo( ).setPath( path );
    model->getFileInfo( ).setFormat( format );

    return model;
}

std::shared_ptr<BlockModel> BlockModel::createFromMemory( std::vector<char>::const_iterator dataIt, const BlockModelFileInfo::Format format, const bool loadRecurrently, ID3D11Device& device )
{
	if ( BlockModelFileInfo::Format::BLOCKMODEL == format ) {
		return BlockModelParser::parseBinary( dataIt, loadRecurrently, device );
	}

	throw std::exception( "BlockModel::createFromMemory() - incorrect 'format' argument." );
}

BlockModel::BlockModel( ) 
: m_mesh( nullptr ) 
{}

BlockModel::BlockModel( const BlockModel& obj ) :
m_fileInfo( obj.m_fileInfo ),
m_mesh( obj.m_mesh )
{}

BlockModel::~BlockModel( ) 
{}

Asset::Type BlockModel::getType() const
{
	return Asset::Type::BlockModel;
}

std::vector< std::shared_ptr< const Asset > > BlockModel::getSubAssets( ) const
{
	std::vector< std::shared_ptr< const Asset > > subAssets;

	if ( m_mesh )
		subAssets.push_back( m_mesh );

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	return subAssets;
}

std::vector< std::shared_ptr< Asset > > BlockModel::getSubAssets()
{
	std::vector< std::shared_ptr<Asset> > subAssets;

	if ( m_mesh )
		subAssets.push_back( m_mesh );

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	return subAssets;
}

void BlockModel::swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset )
{
	if ( !oldAsset || !newAsset )
		throw std::exception( "BlockModel::swapSubAsset - nullptr passed." );

    std::shared_ptr< BlockMesh > newMesh = std::dynamic_pointer_cast< BlockMesh >( newAsset );

    if ( newMesh )
    {
	    if ( m_mesh == oldAsset ) 
        {
		    m_mesh = newMesh;
		    return;
        }

        throw std::exception( "BlockModel::swapSubAsset - tried to swap assets of different types." );
    }

    typedef Texture2DSpecBind< TexBind::ShaderResource, unsigned char > TextureU1;
    std::shared_ptr< TextureU1 > newTextureU1 = std::dynamic_pointer_cast< TextureU1 >( newAsset );

    if ( newTextureU1 )
    {
        for ( ModelTexture2D< unsigned char >& texture : m_alphaTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU1 );
                return;
            }
        }

        for ( ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU1 );
                return;
            }
        }

	    for ( ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU1 );
                return;
            }
        }

        for ( ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU1 );
                return;
            }
        }

        throw std::exception( "BlockModel::swapSubAsset - tried to swap assets of different types." );
    }

    typedef Texture2DSpecBind< TexBind::ShaderResource, uchar4 > TextureU4;
    std::shared_ptr< TextureU4 > newTextureU4 = std::dynamic_pointer_cast< TextureU4 >( newAsset );

    if ( newTextureU4 )
    {
        for ( ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU4 );
                return;
            }
        }

	    for ( ModelTexture2D< uchar4 >& texture : m_albedoTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU4 );
                return;
            }
        }

        for ( ModelTexture2D< uchar4 >& texture : m_normalTextures ) 
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU4 );
                return;
            }
        }

        throw std::exception( "BlockModel::swapSubAsset - tried to swap assets of different types." );
    }

    throw std::exception( "BlockModel::swapSubAsset - tried to swap assets of different types." );
}

void BlockModel::setFileInfo( const BlockModelFileInfo& fileInfo )
{
	this->m_fileInfo = fileInfo;
}

const BlockModelFileInfo& BlockModel::getFileInfo() const
{
	return m_fileInfo;
}

BlockModelFileInfo& BlockModel::getFileInfo()
{
	return m_fileInfo;
}

void BlockModel::saveToFile( const std::string& path ) const
{
	std::vector<char> data;

	BlockModelParser::writeBinary( data, *this );

	BinaryFile::save( path, data );
}

void BlockModel::saveToMemory( std::vector<char>& data ) const
{
    BlockModelParser::writeBinary( data, *this );
}

void BlockModel::loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext, bool reload )
{
	if ( m_mesh )
		m_mesh->loadCpuToGpu( device, reload );

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( texture.getTexture() )
            texture.getTexture()->loadCpuToGpu( device, deviceContext, reload );

    for ( const ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext, reload );

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext, reload );

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext, reload );

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext, reload );

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext, reload );

	for ( const ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext, reload );
}

void BlockModel::loadGpuToCpu()
{
    throw std::exception( "BlockMesh::loadGpuToCpu - unimplemented method." );
	// #TODO: implement.

	/*if ( mesh )
		mesh->loadGpuToCpu();

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			texture.getTexture()->loadGpuToCpu();
	}*/
}

void BlockModel::unloadFromCpu()
{
	if ( m_mesh )
		m_mesh->unloadFromCpu();

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

    for ( const ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();
}

void BlockModel::unloadFromGpu()
{
	if ( m_mesh )
		m_mesh->unloadFromGpu();

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();
}

bool BlockModel::isInCpuMemory() const
{
	if ( !m_mesh || !m_mesh->isInCpuMemory() )
		return false;

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

    for ( const ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	return true;
}

bool BlockModel::isInGpuMemory() const
{
	if ( !m_mesh || !m_mesh->isInGpuMemory() )
		return false;

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

    for ( const ModelTexture2D< uchar4 >& texture : m_emissiveTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : m_refractiveIndexTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	return true;
}

void BlockModel::setMesh( std::shared_ptr<BlockMesh> mesh ) {
	this->m_mesh = mesh;
}

std::shared_ptr<const BlockMesh> BlockModel::getMesh() const {
	return std::static_pointer_cast<const BlockMesh>( m_mesh );
}

std::shared_ptr<BlockMesh> BlockModel::getMesh( ) {
	return m_mesh;
}