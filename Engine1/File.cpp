#include "File.h"

using namespace Engine1;

long long File::getFileSize( const std::string& path )
{
    struct _stat64 stat_buf;
    
    const int result = _stat64( path.c_str(), &stat_buf );

    if ( result == 0 )
        return stat_buf.st_size;
    else
        throw std::exception( "File::getFileSize - failed to deduce file size. File may be missing." );
}
