#pragma once

#include <string>
#include <vector>
#include <memory>

class Texture2DFileInfo
{
	public:

	enum class Format : char
	{
		BMP  = 0,
		DDS  = 1,
		JPEG = 2,
		PNG  = 3,
		RAW  = 4,
		TIFF = 5,
		TGA  = 6
	};

	static std::shared_ptr<Texture2DFileInfo> parseBinary( std::vector<unsigned char>::const_iterator& dataIt );

	Texture2DFileInfo( );
	Texture2DFileInfo::Texture2DFileInfo( std::string path, Format format );
	~Texture2DFileInfo( );

	void writeBinary( std::vector<unsigned char>& data ) const;

	void setPath( std::string path );
	void setFormat( Format format );

	std::string getPath() const;
	Format      getFormat() const;

	private:

	std::string path;
	Format format;
};

