#pragma once

#include <vector>
#include <memory>

class File {
	public:
	
	static std::shared_ptr< std::vector<unsigned char> > loadBinary( const std::string& path );
	static std::shared_ptr< std::vector<char> >          loadText( const std::string& path );
};

