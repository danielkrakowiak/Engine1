#include "SkeletonModel.h"

#include "BinaryFile.h"
#include "SkeletonModelParser.h"

using namespace Engine1;

std::shared_ptr<SkeletonModel> SkeletonModel::createFromFile( const SkeletonModelFileInfo& fileInfo, const bool loadRecurrently, ID3D11Device3& device )
{
	return createFromFile( fileInfo.getPath(), fileInfo.getFormat(), loadRecurrently, device );
}

std::shared_ptr<SkeletonModel> SkeletonModel::createFromFile( const std::string& path, const SkeletonModelFileInfo::Format format, bool loadRecurrently, ID3D11Device3& device )
{
	std::shared_ptr< std::vector<char> > fileData = BinaryFile::load( path );

    std::shared_ptr<SkeletonModel> model = createFromMemory( fileData->cbegin(), format, loadRecurrently, device );

    model->getFileInfo().setPath( path );
    model->getFileInfo().setFormat( format );

    return model;
}

std::shared_ptr<SkeletonModel> SkeletonModel::createFromMemory( std::vector<char>::const_iterator dataIt, const SkeletonModelFileInfo::Format format, bool loadRecurrently, ID3D11Device3& device )
{
	if ( SkeletonModelFileInfo::Format::SKELETONMODEL == format ) {
		return SkeletonModelParser::parseBinary( dataIt, loadRecurrently, device );
	}

	throw std::exception( "SkeletonModel::createFromMemory() - incorrect 'format' argument." );
}

SkeletonModel::SkeletonModel( )
: m_mesh( nullptr ) 
{}

SkeletonModel::SkeletonModel( const SkeletonModel& obj ) :
m_fileInfo( obj.m_fileInfo ),
m_mesh( obj.m_mesh )
{}

SkeletonModel::~SkeletonModel( ) {}

Asset::Type SkeletonModel::getType( ) const
{
	return Asset::Type::SkeletonModel;
}

std::vector< std::shared_ptr<const Asset> > SkeletonModel::getSubAssets( ) const
{
	std::vector< std::shared_ptr<const Asset> > subAssets;

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

std::vector< std::shared_ptr<Asset> > SkeletonModel::getSubAssets( )
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

void SkeletonModel::swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset )
{
	if ( !oldAsset || !newAsset )
		throw std::exception( "SkeletonModel::swapSubAsset - nullptr passed." );

    std::shared_ptr< SkeletonMesh > newMesh = std::dynamic_pointer_cast< SkeletonMesh >( newAsset );

    if ( newMesh )
    {
	    if ( m_mesh == oldAsset ) 
        {
		    m_mesh = newMesh;
		    return;
        }

        throw std::exception( "SkeletonModel::swapSubAsset - tried to swap assets of different types." );
    }

    typedef Texture2D< unsigned char > TextureU1;
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

        throw std::exception( "SkeletonModel::swapSubAsset - tried to swap assets of different types." );
    }

    typedef Texture2D< uchar4 > TextureU4;
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

        throw std::exception( "SkeletonModel::swapSubAsset - tried to swap assets of different types." );
    }

    throw std::exception( "SkeletonModel::swapSubAsset - tried to swap assets of different types." );
}

void SkeletonModel::setFileInfo( const SkeletonModelFileInfo& fileInfo )
{
	this->m_fileInfo = fileInfo;
}

const SkeletonModelFileInfo& SkeletonModel::getFileInfo( ) const
{
	return m_fileInfo;
}

SkeletonModelFileInfo& SkeletonModel::getFileInfo( )
{
	return m_fileInfo;
}

void SkeletonModel::setMesh( std::shared_ptr<SkeletonMesh> mesh ) {
	this->m_mesh = mesh;
}

void SkeletonModel::saveToFile( const std::string& path ) const
{
	std::vector<char> data;

	SkeletonModelParser::writeBinary( data, *this );

	BinaryFile::save( path, data );
}

void SkeletonModel::saveToMemory( std::vector<char>& data ) const
{
    SkeletonModelParser::writeBinary( data, *this );
}

void SkeletonModel::loadCpuToGpu( ID3D11Device3& device, ID3D11DeviceContext3& deviceContext, bool reload )
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

void SkeletonModel::loadGpuToCpu( )
{
    throw std::exception( "SkeletonModel::loadGpuToCpu - unimplemented method." );
	// #TODO: implement.

	/*if ( mesh )
		mesh->loadGpuToCpu();

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			texture.getTexture()->loadGpuToCpu();
	}*/
}

void SkeletonModel::unloadFromCpu( )
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

void SkeletonModel::unloadFromGpu( )
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

bool SkeletonModel::isInCpuMemory( ) const
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

bool SkeletonModel::isInGpuMemory( ) const
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

std::shared_ptr<const SkeletonMesh> SkeletonModel::getMesh( ) const {
	return std::static_pointer_cast<const SkeletonMesh>( m_mesh );
}

std::shared_ptr<SkeletonMesh> SkeletonModel::getMesh( ) {
	return m_mesh;
}