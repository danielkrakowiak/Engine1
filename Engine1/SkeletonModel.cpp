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
: m_mesh( nullptr ) 
{}

SkeletonModel::SkeletonModel( const SkeletonModel& obj ) :
m_fileInfo( obj.m_fileInfo ),
m_mesh( obj.m_mesh ),
m_alphaTextures( obj.m_alphaTextures ),
m_emissionTextures( obj.m_emissionTextures ),
m_albedoTextures( obj.m_albedoTextures ),
m_metalnessTextures( obj.m_metalnessTextures ),
m_roughnessTextures( obj.m_roughnessTextures ),
m_normalTextures( obj.m_normalTextures ),
m_indexOfRefractionTextures( obj.m_indexOfRefractionTextures )
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

	for ( const ModelTexture2D< uchar4 >& texture : m_emissionTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_indexOfRefractionTextures )
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

	for ( const ModelTexture2D< uchar4 >& texture : m_emissionTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( texture.getTexture() ) subAssets.push_back( texture.getTexture() );

	for ( const ModelTexture2D< unsigned char >& texture : m_indexOfRefractionTextures )
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

        for ( ModelTexture2D< unsigned char >& texture : m_indexOfRefractionTextures )
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
        for ( ModelTexture2D< uchar4 >& texture : m_emissionTextures )
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
	if ( m_mesh )
		m_mesh->loadCpuToGpu( device );

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< uchar4 >& texture : m_emissionTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< uchar4 >& texture : m_albedoTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< unsigned char >& texture : m_metalnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< unsigned char >& texture : m_roughnessTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< uchar4 >& texture : m_normalTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->loadCpuToGpu( device, deviceContext );

	for ( const ModelTexture2D< unsigned char >& texture : m_indexOfRefractionTextures )
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
	if ( m_mesh )
		m_mesh->unloadFromCpu();

    for ( const ModelTexture2D< unsigned char >& texture : m_alphaTextures )
		if ( texture.getTexture() ) 
            texture.getTexture()->unloadFromCpu();

	for ( const ModelTexture2D< uchar4 >& texture : m_emissionTextures )
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

	for ( const ModelTexture2D< unsigned char >& texture : m_indexOfRefractionTextures )
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

	for ( const ModelTexture2D< uchar4 >& texture : m_emissionTextures )
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

	for ( const ModelTexture2D< unsigned char >& texture : m_indexOfRefractionTextures )
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

	for ( const ModelTexture2D< uchar4 >& texture : m_emissionTextures )
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

	for ( const ModelTexture2D< unsigned char >& texture : m_indexOfRefractionTextures )
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

	for ( const ModelTexture2D< uchar4 >& texture : m_emissionTextures )
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

	for ( const ModelTexture2D< unsigned char >& texture : m_indexOfRefractionTextures )
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

void SkeletonModel::addAlphaTexture( ModelTexture2D< unsigned char >& texture )
{
	m_alphaTextures.push_back( texture );
}

void SkeletonModel::addEmissionTexture( ModelTexture2D< uchar4 >& texture )
{
	m_emissionTextures.push_back( texture );
}

void SkeletonModel::addAlbedoTexture( ModelTexture2D< uchar4 >& texture )
{
	m_albedoTextures.push_back( texture );
}

void SkeletonModel::addMetalnessTexture( ModelTexture2D< unsigned char >& texture )
{
	m_metalnessTextures.push_back( texture );
}

void SkeletonModel::addRoughnessTexture( ModelTexture2D< unsigned char >& texture )
{
	m_roughnessTextures.push_back( texture );
}

void SkeletonModel::addNormalTexture( ModelTexture2D< uchar4 >& texture )
{
	m_normalTextures.push_back( texture );
}

void SkeletonModel::addIndexOfRefractionTexture( ModelTexture2D< unsigned char >& texture )
{
	m_indexOfRefractionTextures.push_back( texture );
}

void SkeletonModel::removeAllAlphaTextures()
{
    m_alphaTextures.clear();
}

void SkeletonModel::removeAllEmissionTextures()
{
    m_emissionTextures.clear();
}

void SkeletonModel::removeAllAlbedoTextures()
{
    m_albedoTextures.clear();
}

void SkeletonModel::removeAllMetalnessTextures()
{
    m_metalnessTextures.clear();
}

void SkeletonModel::removeAllRoughnessTextures()
{
    m_roughnessTextures.clear();
}

void SkeletonModel::removeAllNormalTextures()
{
    m_normalTextures.clear();
}

void SkeletonModel::removeAllIndexOfRefractionTextures()
{
    m_indexOfRefractionTextures.clear();
}

int SkeletonModel::getAlphaTexturesCount( ) const {
	return (int)m_alphaTextures.size();
}

std::vector< ModelTexture2D< unsigned char > > SkeletonModel::getAlphaTextures( ) const {
	return std::vector< ModelTexture2D< unsigned char > >( m_alphaTextures.begin( ), m_alphaTextures.end( ) );
}

ModelTexture2D< unsigned char > SkeletonModel::getAlphaTexture( int index ) const {
	if ( index >= (int)m_alphaTextures.size( ) ) throw std::exception( "SkeletonModel::getAlphaTexture: Trying to access texture at non-existing index" );

	return m_alphaTextures.at( index );
}

int SkeletonModel::getEmissionTexturesCount( ) const {
	return (int)m_emissionTextures.size();
}

std::vector< ModelTexture2D< uchar4 > > SkeletonModel::getEmissionTextures( ) const {
	return std::vector< ModelTexture2D< uchar4 > >( m_emissionTextures.begin( ), m_emissionTextures.end( ) );
}

ModelTexture2D< uchar4 > SkeletonModel::getEmissionTexture( int index ) const {
	if ( index >= (int)m_emissionTextures.size( ) ) throw std::exception( "SkeletonModel::getEmissionTexture: Trying to access texture at non-existing index" );

	return m_emissionTextures.at( index );
}

int SkeletonModel::getAlbedoTexturesCount( ) const {
	return (int)m_albedoTextures.size();
}

std::vector< ModelTexture2D< uchar4 > > SkeletonModel::getAlbedoTextures( ) const {
	return std::vector< ModelTexture2D< uchar4 > >( m_albedoTextures.begin( ), m_albedoTextures.end( ) );
}

ModelTexture2D< uchar4 > SkeletonModel::getAlbedoTexture( int index ) const {
	if ( index >= (int)m_albedoTextures.size( ) ) throw std::exception( "SkeletonModel::getAlbedoTexture: Trying to access texture at non-existing index" );

	return m_albedoTextures.at( index );
}

int SkeletonModel::getMetalnessTexturesCount( ) const {
	return (int)m_metalnessTextures.size();
}

std::vector< ModelTexture2D< unsigned char > > SkeletonModel::getMetalnessTextures( ) const {
	return std::vector< ModelTexture2D< unsigned char > >( m_metalnessTextures.begin( ), m_metalnessTextures.end( ) );
}

ModelTexture2D< unsigned char > SkeletonModel::getMetalnessTexture( int index ) const {
	if ( index >= (int)m_metalnessTextures.size( ) ) throw std::exception( "SkeletonModel::getMetalnessTexture: Trying to access texture at non-existing index" );

	return m_metalnessTextures.at( index );
}

int SkeletonModel::getRoughnessTexturesCount( ) const {
	return (int)m_roughnessTextures.size();
}

std::vector< ModelTexture2D< unsigned char > > SkeletonModel::getRoughnessTextures( ) const {
	return std::vector< ModelTexture2D< unsigned char > >( m_roughnessTextures.begin( ), m_roughnessTextures.end( ) );
}

ModelTexture2D< unsigned char > SkeletonModel::getRoughnessTexture( int index ) const {
	if ( index >= (int)m_roughnessTextures.size( ) ) throw std::exception( "SkeletonModel::getRoughnessTexture: Trying to access texture at non-existing index" );

	return m_roughnessTextures.at( index );
}

int SkeletonModel::getNormalTexturesCount( ) const {
	return (int)m_normalTextures.size();
}

std::vector< ModelTexture2D< uchar4 > > SkeletonModel::getNormalTextures( ) const {
	return std::vector< ModelTexture2D< uchar4 > >( m_normalTextures.begin( ), m_normalTextures.end( ) );
}

ModelTexture2D< uchar4 > SkeletonModel::getNormalTexture( int index ) const {
	if ( index >= (int)m_normalTextures.size( ) ) throw std::exception( "SkeletonModel::getNormalTexture: Trying to access texture at non-existing index" );

	return m_normalTextures.at( index );
}

int SkeletonModel::getIndexOfRefractionTexturesCount( ) const {
	return (int)m_indexOfRefractionTextures.size();
}

std::vector< ModelTexture2D< unsigned char > > SkeletonModel::getIndexOfRefractionTextures( ) const {
	return std::vector< ModelTexture2D< unsigned char > >( m_indexOfRefractionTextures.begin( ), m_indexOfRefractionTextures.end( ) );
}

ModelTexture2D< unsigned char > SkeletonModel::getIndexOfRefractionTexture( int index ) const {
	if ( index >= (int)m_indexOfRefractionTextures.size( ) ) throw std::exception( "SkeletonModel::getIndexOfRefractionTexture: Trying to access texture at non-existing index" );

	return m_indexOfRefractionTextures.at( index );
}