#include <string>
#include <iostream>
#include <sstream>
#include <cstdio>

/* Create a MINC file using rawtominc.
 * The "args" argument contains arguments passed to "rawtominc".
 * Files with 2, 3, or 4 dimensions may be created.
 */


namespace detail {

    std::string createMincFile( const std::string& args, 
				unsigned int sampleCount )
    {
	std::string command( "rawtominc -clobber -2 -unsigned -byte " );
	command += args;
	//std::cout << "Executing [" << command << "]\n";

	FILE* rawtominc = popen( command.c_str(), "w" );

	unsigned char value = 0;
	for( unsigned int i = 0; i < sampleCount; ++i )
	{
	    fwrite( &value, sizeof(value), 1, rawtominc );
	    value += 1;
	}
	
	pclose( rawtominc );

	return command;
    }
}


std::string createMincFile( const std::string& extra_args, 
			    unsigned int size1,
			    unsigned int size2 )
{
    std::stringstream args;
    args << extra_args << " " << size1 << " " << size2;
    return detail::createMincFile( args.str(), size1 * size2 );
}


std::string createMincFile( const std::string& extra_args, 
			    unsigned int size1,
			    unsigned int size2,
			    unsigned int size3 )
{
    std::stringstream args;
    args << extra_args << " " << size1 << " " << size2 << " " << size3;
    return detail::createMincFile( args.str(), size1 * size2 * size3 );
}


