#pragma once

#include <vector>
#include <memory>

class TextFile 
{
	public:
	
	static std::shared_ptr< std::vector<char> > load( const std::string& path );
};

