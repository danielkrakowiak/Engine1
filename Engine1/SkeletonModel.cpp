#include "SkeletonModel.h"

#include "BinaryFile.h"
#include "SkeletonModelParser.h"

using namespace Engine1;

std::shared_ptr<SkeletonModel> SkeletonModel::createFromFile( const SkeletonModelFileInfo& fileInfo, const bool loadRecurrently, ID3D11Device& device )
{
	return createFromFile( fileInfo.getPath(), fileInfo.getFormat(), loadRecurrently, device );
}

std::shared_ptr<SkeletonModel> SkeletonModel::createFromFile( const std::string& path, const SkeletonModelFileInfo::Format format, bool loadRecurrently, ID3D11Device& device )
{
	std::shared_ptr< std::vector<char> > fileData = BinaryFile::load( path );

    std::shared_ptr<SkeletonModel> model = createFromMemory( fileData->cbegin(), format, loadRecurrently, device );

    model->getFileInfo().setPath( path );
    model->getFileInfo().setFormat( format );

    return model;
}

std::shared_ptr<SkeletonModel> SkeletonModel::createFromMemory( std::vector<char>::const_iterator dataIt, const SkeletonModelFileInfo::Format format, bool loadRecurrently, ID3D11Device& device )
{
	if ( SkeletonModelFileInfo::Format::SKELETONMODEL == format ) {
		return SkeletonModelParser::parseBinary( dataIt, loadRecurrently, device );
	}

	throw std::exception( "SkeletonModel::createFromMemory() - incorrect 'format' argument." );
}

SkeletonModel::SkeletonModel( )
: mesh( nullptr ) 
{}

SkeletonModel::~SkeletonModel( ) {}

Asset::Type SkeletonModel::getType( ) const
{
	return Asset::Type::SkeletonModel;
}

std::vector< std::shared_ptr<const Asset> > SkeletonModel::getSubAssets( ) const
{
	std::vector< std::shared_ptr<const Asset> > subAssets;

	if ( mesh )
		subAssets.push_back( mesh );

	for ( const ModelTexture2D< uchar4 >& texture : emissionTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : albedoTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : metalnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : roughnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : normalTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	return subAssets;
}

std::vector< std::shared_ptr<Asset> > SkeletonModel::getSubAssets( )
{
	std::vector< std::shared_ptr<Asset> > subAssets;

    if ( mesh )
		subAssets.push_back( mesh );

	for ( const ModelTexture2D< uchar4 >& texture : emissionTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : albedoTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : metalnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : roughnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : normalTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
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
	    if ( mesh == oldAsset ) 
        {
		    mesh = newMesh;
		    return;
        }

        throw std::exception( "SkeletonModel::swapSubAsset - tried to swap assets of different types." );
    }

    typedef Texture2DSpecBind< TexBind::ShaderResource, unsigned char > TextureU1;
    std::shared_ptr< TextureU1 > newTextureU1 = std::dynamic_pointer_cast< TextureU1 >( newAsset );

    if ( newTextureU1 )
    {
        for ( ModelTexture2D< unsigned char >& texture : metalnessTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU1 );
                return;
            }
        }

	    for ( ModelTexture2D< unsigned char >& texture : roughnessTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU1 );
                return;
            }
        }

        for ( ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU1 );
                return;
            }
        }

        throw std::exception( "SkeletonModel::swapSubAsset - tried to swap assets of different types." );
    }

    typedef Texture2DSpecBind< TexBind::ShaderResource, uchar4 > TextureU4;
    std::shared_ptr< TextureU4 > newTextureU4 = std::dynamic_pointer_cast< TextureU4 >( newAsset );

    if ( newTextureU4 )
    {
        for ( ModelTexture2D< uchar4 >& texture : emissionTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU4 );
                return;
            }
        }

	    for ( ModelTexture2D< uchar4 >& texture : albedoTextures )
        {
		    if ( texture.getTexture() == oldAsset ) {
                texture.setTexture( newTextureU4 );
                return;
            }
        }

        for ( ModelTexture2D< uchar4 >& texture : normalTextures ) 
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
	this->fileInfo = fileInfo;
}

const SkeletonModelFileInfo& SkeletonModel::getFileInfo( ) const
{
	return fileInfo;
}

SkeletonModelFileInfo& SkeletonModel::getFileInfo( )
{
	return fileInfo;
}

void SkeletonModel::setMesh( std::shared_ptr<SkeletonMesh> mesh ) {
	this->mesh = mesh;
}

void SkeletonModel::saveToFile( const std::string& path )
{
	std::vector<char> data;

	SkeletonModelParser::writeBinary( data, *this );

	BinaryFile::save( path, data );
}

void SkeletonModel::saveToMemory( std::vector<char>& data ) const
{
    SkeletonModelParser::writeBinary( data, *this );
}

void SkeletonModel::loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext )
{
	if ( mesh )
		mesh->loadCpuToGpu( device );

	for ( const ModelTexture2D< uchar4 >& texture : emissionTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< uchar4 >& texture : albedoTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< unsigned char >& texture : metalnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< unsigned char >& texture : roughnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< uchar4 >& texture : normalTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );
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
	if ( mesh )
		mesh->unloadFromCpu();

	for ( const ModelTexture2D< uchar4 >& texture : emissionTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< uchar4 >& texture : albedoTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< unsigned char >& texture : metalnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< unsigned char >& texture : roughnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< uchar4 >& texture : normalTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();
}

void SkeletonModel::unloadFromGpu( )
{
	if ( mesh )
		mesh->unloadFromGpu();

	for ( const ModelTexture2D< uchar4 >& texture : emissionTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< uchar4 >& texture : albedoTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< unsigned char >& texture : metalnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< unsigned char >& texture : roughnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< uchar4 >& texture : normalTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();

	for ( const ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromGpu();
}

bool SkeletonModel::isInCpuMemory( ) const
{
	if ( !mesh || !mesh->isInCpuMemory() )
		return false;

	for ( const ModelTexture2D< uchar4 >& texture : emissionTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : albedoTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : metalnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : roughnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : normalTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
            return false;

	return true;
}

bool SkeletonModel::isInGpuMemory( ) const
{
	if ( !mesh || !mesh->isInGpuMemory() )
		return false;

	for ( const ModelTexture2D< uchar4 >& texture : emissionTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : albedoTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : metalnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : roughnessTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< uchar4 >& texture : normalTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	for ( const ModelTexture2D< unsigned char >& texture : indexOfRefractionTextures )
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
            return false;

	return true;
}

std::shared_ptr<const SkeletonMesh> SkeletonModel::getMesh( ) const {
	return std::static_pointer_cast<const SkeletonMesh>( mesh );
}

std::shared_ptr<SkeletonMesh> SkeletonModel::getMesh( ) {
	return mesh;
}

void SkeletonModel::addEmissionTexture( ModelTexture2D< uchar4 >& texture )
{
	emissionTextures.push_back( texture );
}

void SkeletonModel::addAlbedoTexture( ModelTexture2D< uchar4 >& texture )
{
	albedoTextures.push_back( texture );
}

void SkeletonModel::addMetalnessTexture( ModelTexture2D< unsigned char >& texture )
{
	metalnessTextures.push_back( texture );
}

void SkeletonModel::addRoughnessTexture( ModelTexture2D< unsigned char >& texture )
{
	roughnessTextures.push_back( texture );
}

void SkeletonModel::addNormalTexture( ModelTexture2D< uchar4 >& texture )
{
	normalTextures.push_back( texture );
}

void SkeletonModel::addIndexOfRefractionTexture( ModelTexture2D< unsigned char >& texture )
{
	indexOfRefractionTextures.push_back( texture );
}

int SkeletonModel::getEmissionTexturesCount( ) const {
	return (int)emissionTextures.size();
}

std::vector< ModelTexture2D< uchar4 > > SkeletonModel::getEmissionTextures( ) const {
	return std::vector< ModelTexture2D< uchar4 > >( emissionTextures.begin( ), emissionTextures.end( ) );
}

ModelTexture2D< uchar4 > SkeletonModel::getEmissionTexture( int index ) const {
	if ( index >= (int)emissionTextures.size( ) ) throw std::exception( "SkeletonModel::getEmissionTexture: Trying to access texture at non-existing index" );

	return emissionTextures.at( index );
}

int SkeletonModel::getAlbedoTexturesCount( ) const {
	return (int)albedoTextures.size();
}

std::vector< ModelTexture2D< uchar4 > > SkeletonModel::getAlbedoTextures( ) const {
	return std::vector< ModelTexture2D< uchar4 > >( albedoTextures.begin( ), albedoTextures.end( ) );
}

ModelTexture2D< uchar4 > SkeletonModel::getAlbedoTexture( int index ) const {
	if ( index >= (int)albedoTextures.size( ) ) throw std::exception( "SkeletonModel::getAlbedoTexture: Trying to access texture at non-existing index" );

	return albedoTextures.at( index );
}

int SkeletonModel::getMetalnessTexturesCount( ) const {
	return (int)metalnessTextures.size();
}

std::vector< ModelTexture2D< unsigned char > > SkeletonModel::getMetalnessTextures( ) const {
	return std::vector< ModelTexture2D< unsigned char > >( metalnessTextures.begin( ), metalnessTextures.end( ) );
}

ModelTexture2D< unsigned char > SkeletonModel::getMetalnessTexture( int index ) const {
	if ( index >= (int)metalnessTextures.size( ) ) throw std::exception( "SkeletonModel::getMetalnessTexture: Trying to access texture at non-existing index" );

	return metalnessTextures.at( index );
}

int SkeletonModel::getRoughnessTexturesCount( ) const {
	return (int)roughnessTextures.size();
}

std::vector< ModelTexture2D< unsigned char > > SkeletonModel::getRoughnessTextures( ) const {
	return std::vector< ModelTexture2D< unsigned char > >( roughnessTextures.begin( ), roughnessTextures.end( ) );
}

ModelTexture2D< unsigned char > SkeletonModel::getRoughnessTexture( int index ) const {
	if ( index >= (int)roughnessTextures.size( ) ) throw std::exception( "SkeletonModel::getRoughnessTexture: Trying to access texture at non-existing index" );

	return roughnessTextures.at( index );
}

int SkeletonModel::getNormalTexturesCount( ) const {
	return (int)normalTextures.size();
}

std::vector< ModelTexture2D< uchar4 > > SkeletonModel::getNormalTextures( ) const {
	return std::vector< ModelTexture2D< uchar4 > >( normalTextures.begin( ), normalTextures.end( ) );
}

ModelTexture2D< uchar4 > SkeletonModel::getNormalTexture( int index ) const {
	if ( index >= (int)normalTextures.size( ) ) throw std::exception( "SkeletonModel::getNormalTexture: Trying to access texture at non-existing index" );

	return normalTextures.at( index );
}

int SkeletonModel::getIndexOfRefractionTexturesCount( ) const {
	return (int)indexOfRefractionTextures.size();
}

std::vector< ModelTexture2D< unsigned char > > SkeletonModel::getIndexOfRefractionTextures( ) const {
	return std::vector< ModelTexture2D< unsigned char > >( indexOfRefractionTextures.begin( ), indexOfRefractionTextures.end( ) );
}

ModelTexture2D< unsigned char > SkeletonModel::getIndexOfRefractionTexture( int index ) const {
	if ( index >= (int)indexOfRefractionTextures.size( ) ) throw std::exception( "SkeletonModel::getIndexOfRefractionTexture: Trying to access texture at non-existing index" );

	return indexOfRefractionTextures.at( index );
}