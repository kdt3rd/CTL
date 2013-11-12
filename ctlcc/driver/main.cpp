///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Academy of Motion Picture Arts and Sciences 
// ("A.M.P.A.S."). Portions contributed by others as indicated.
// All rights reserved.
// 
// A worldwide, royalty-free, non-exclusive right to copy, modify, create
// derivatives, and use, in source and binary forms, is hereby granted, 
// subject to acceptance of this license. Performance of any of the 
// aforementioned acts indicates acceptance to be bound by the following 
// terms and conditions:
//
//  * Copies of source code, in whole or in part, must retain the 
//    above copyright notice, this list of conditions and the 
//    Disclaimer of Warranty.
//
//  * Use in binary form must retain the above copyright notice, 
//    this list of conditions and the Disclaimer of Warranty in the
//    documentation and/or other materials provided with the distribution.
//
//  * Nothing in this license shall be deemed to grant any rights to 
//    trademarks, copyrights, patents, trade secrets or any other 
//    intellectual property of A.M.P.A.S. or any contributors, except 
//    as expressly stated herein.
//
//  * Neither the name "A.M.P.A.S." nor the name of any other 
//    contributors to this software may be used to endorse or promote 
//    products derivative of or based on this software without express 
//    prior written permission of A.M.P.A.S. or the contributors, as 
//    appropriate.
// 
// This license shall be construed pursuant to the laws of the State of 
// California, and any disputes related thereto shall be subject to the 
// jurisdiction of the courts therein.
//
// Disclaimer of Warranty: THIS SOFTWARE IS PROVIDED BY A.M.P.A.S. AND 
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, 
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS 
// FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT ARE DISCLAIMED. IN NO 
// EVENT SHALL A.M.P.A.S., OR ANY CONTRIBUTORS OR DISTRIBUTORS, BE LIABLE 
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, RESITUTIONARY, 
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
// THE POSSIBILITY OF SUCH DAMAGE.
//
// WITHOUT LIMITING THE GENERALITY OF THE FOREGOING, THE ACADEMY 
// SPECIFICALLY DISCLAIMS ANY REPRESENTATIONS OR WARRANTIES WHATSOEVER 
// RELATED TO PATENT OR OTHER INTELLECTUAL PROPERTY RIGHTS IN THE ACADEMY 
// COLOR ENCODING SYSTEM, OR APPLICATIONS THEREOF, HELD BY PARTIES OTHER 
// THAN A.M.P.A.S., WHETHER DISCLOSED OR UNDISCLOSED.
///////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <iomanip>

#include "file_io.h"

class Function
{
public:
	Function( void );
	virtual ~Function( void );
	virtual const std::string &getName( void ) const = 0;

	virtual const std::vector<std::string> &getArgumentFlags( void ) const = 0;
	virtual const std::vector<std::string> &getFloatArgs( void ) const = 0;

	virtual void setFlag( const std::string &flag ) const = 0;
	virtual void setArgument( const std::string &arg, double val ) const = 0;

	virtual void apply( ctl::dpx::fb<float> &pixels ) const = 0;
};

class Driver
{
public:
	Driver( void );
	~Driver( void );

	bool usesThreads( void ) const;
	void init( int threads );
	void shutdown( void );

	const std::vector<Function *> &getFunctions( void );
};

extern Driver theDriver;

static bool kEnableThreads = false;
const char *argv0 = NULL;
static void
usageAndExit( void )
{
	const std::vector<Function *> &funcs = theDriver.getFunctions();
	size_t nFuncs = funcs.size();
	std::cout << argv0 << " usage:\n"
			  << "  " << argv0 << " [--force] [--format <outputfmt>] [--compression c]";
	if ( kEnableThreads )
		std::cout << " [--threads n]";
	if ( nFuncs > 1 )
		std::cout << " --function <func>";
	std::cout << " [--funcArgument [x] ...] <input file1> ... <output file|dir>\n\n"
		"  -h|--help         This message\n";
	if ( kEnableThreads )
		std::cout << "  -t|--threads N    Number of threads to use for computation (default: number of processors on system)\n";
	std::cout <<
		"  --skip-errors     If there is an error processing a particular frame, keep working instead of exiting\n"
		"  --in_scale S      Scale incoming pixels by amount S (default 1.0)\n"
		"  --out_scale S     Scale outgoing pixels by amount S (default 1.0)\n";
	std::cout <<
		"  --format <fmt>    Output file format (Available: ";
	const std::vector<file_format_t> &outputFmts = getAllowedFormats();
	for ( size_t f = 0; f != outputFmts.size(); ++f )
	{
		if ( f > 0 )
			std::cout << '|';
		std::cout << outputFmts[f].name;
	}
	std::cout << ")\n";
	std::cout <<
		"  --compression <c> Output file compression (Available: ";
	const std::vector<compression_t> &comp = getAvailableCompressionSchemes();
	for ( size_t c = 0; c != comp.size(); ++c )
	{
		if ( c > 0 )
			std::cout << '|';
		std::cout << comp[c].name;
	}
	std::cout << ")\n";
	if ( nFuncs > 1 )
	{
		std::cout << " -f|--func|--function ";
		for ( size_t i = 0; i != nFuncs; ++i )
		{
			if ( i > 0 )
				std::cout << '|';
			std::cout << funcs[i]->getName();
		}
		std::cout << " Which function to apply\n";
	}

	std::cout << '\n';
	for ( size_t i = 0; i != nFuncs; ++i )
	{
		const std::vector<std::string> &flags = funcs[i]->getArgumentFlags();
		const std::vector<std::string> &floatargs = funcs[i]->getFloatArgs();
		if ( flags.empty() && floatargs.empty() )
			continue;

		std::cout << "  Arguments defined for function '" << funcs[i]->getName() << "':\n";
		if ( ! flags.empty() )
		{
			std::cout << "\n   Toggle flags:\n";
			for ( size_t f = 0; f != flags.size(); ++f )
				std::cout << "      --" << flags[f] << "\n";
		}

		if ( ! floatargs.empty() )
		{
			std::cout << "\n   Floating point arguments:\n";
			for ( size_t f = 0; f != floatargs.size(); ++f )
				std::cout << "      --" << floatargs[f] << " <value>\n";
		}
	}
	std::cout << std::endl;
	exit( 0 );
}

static bool
isDirectory( const std::string &f )
{
	struct stat sb;

	if ( stat( f.c_str(), &sb ) == 0 )
	{
		if ( S_ISDIR( sb.st_mode ) )
			return true;
	}
	return false;
}

static double
extractFloatArg( const std::string &arg, int &c, int argc, char *argv[] )
{
	++c;
	if ( c == argc )
	{
		std::cerr << "ERROR: missing argument to --" << arg << ", expecting floating point number\n" << std::endl;
		usageAndExit();
	}

	char *endPtr = NULL;
	double x = strtod( argv[c], &endPtr );
	if ( *endPtr != '\0' )
	{
		std::cerr << "ERROR: Expecting floating point argument to '" << arg << "': Got '" << argv[c] << "'" << std::endl;
		usageAndExit();
	}
	return x;
}

static bool
exists( const std::string &f )
{
	struct stat sb;

	if ( stat( f.c_str(), &sb ) == 0 )
		return true;
	return false;
}

static std::string
basename( const std::string &path )
{
    std::string::size_type lastPos = path.rfind( '/' );

    if ( lastPos != std::string::npos )
        return path.substr( lastPos + 1 );

#ifdef WIN32
	lastPos = path.rfind( '\\' );
    if ( lastPos != std::string::npos )
        return path.substr( lastPos + 1 );
#endif
    return path;
}

static std::string
extension( const std::string &path )
{
    std::string::size_type lastPos = path.rfind( '.' );

    if ( lastPos != std::string::npos )
        return path.substr( lastPos + 1 );

    return std::string();
}

static void
reportTime( const char *tag, const struct timeval &start, const struct timeval &end )
{
	int64_t sec = int64_t(end.tv_sec) - int64_t(start.tv_sec);
	int64_t usec = int64_t(end.tv_usec) - int64_t(start.tv_usec);
	if ( usec < 0 )
	{
		usec += 1000000;
		sec -= 1;
	}
	std::cout << " (" << tag << ": " << sec << "." << std::setw(6) << std::setfill( '0' ) << usec << " secs)";
}

static int
safeMain( int argc, char *argv[] )
{
	kEnableThreads = theDriver.usesThreads();

	const std::vector<Function *> &funcs = theDriver.getFunctions();
	if ( funcs.empty() )
	{
		std::cerr << "ERROR: Internal error in driver generator, no main function found" << std::endl;
		return -1;
	}

	std::vector<std::string> fileArgs;

	compression_t outComp = findCompressionByName( "NONE" );
	file_format_t outFmt;

	float inScale = 1.F, outScale = 1.F;
	argv0 = argv[0];
	std::string chosenFunc;
	size_t funcIdx = 0, nFuncs = funcs.size();
	bool fileOnly = false;
	bool force = false;
	int nThreads = -1;
	bool stopOnErrors = true;
	for ( int c = 1; c < argc; ++c )
	{
		const char *curArg = argv[c];
		bool hasDash = false;
		if ( fileOnly )
		{
			fileArgs.push_back( std::string( curArg ) );
			continue;
		}

		if ( curArg[0] == '-' )
		{
			hasDash = true;
			++curArg;
		}
		if ( curArg[0] == '-' )
			++curArg;

		if ( curArg[0] == '\0' )
		{
			fileOnly = true;
			continue;
		}

		std::string arg;
		arg = curArg;

		if ( ! hasDash )
		{
			fileArgs.push_back( arg );
			continue;
		}

		if ( arg == "h" || arg == "help" || arg == "?" )
			usageAndExit();

		if ( kEnableThreads )
		{
			if ( arg == "t" || arg == "threads" )
			{
				++c;
				if ( c == argc )
				{
					std::cerr << "ERROR: missing argument to function argument, expecting function name\n" << std::endl;
					usageAndExit();
				}

				char *endPtr = NULL;
				nThreads = strtol( argv[c], &endPtr, 10 );
				if ( *endPtr != '\0' )
				{
					std::cerr << "ERROR: Unable to fully parse the number of threads: " << argv[c] << std::endl;
					usageAndExit();
				}

				continue;
			}
		}

		if ( arg == "skip-errors" )
		{
			stopOnErrors = false;
			continue;
		}

		if ( arg == "force" )
		{
			force = true;
			continue;
		}

		if ( arg == "in_scale" )
		{
			inScale = extractFloatArg( arg, c, argc, argv );
			continue;
		}

		if ( arg == "out_scale" )
		{
			outScale = extractFloatArg( arg, c, argc, argv );
			continue;
		}

		if ( arg == "format" )
		{
			++c;
			if ( c == argc )
			{
				std::cerr << "ERROR: missing argument to format, expecting format tag\n" << std::endl;
				usageAndExit();
			}
			outFmt = findFormatByName( argv[c] );
			if ( outFmt.name.empty() )
			{
				std::cerr << "ERROR: Unknown format tag specified\n" << std::endl;
				usageAndExit();
			}
			continue;
		}

		if ( arg == "compression" )
		{
			++c;
			if ( c == argc )
			{
				std::cerr << "ERROR: missing argument to compression, expecting compression name\n" << std::endl;
				usageAndExit();
			}
			outComp = findCompressionByName( argv[c] );
			if ( outComp.name.empty() )
			{
				std::cerr << "ERROR: Unknown compression name specified\n" << std::endl;
				usageAndExit();
			}
			continue;
		}

		if ( arg == "f" || arg == "func" || arg == "function" )
		{
			++c;
			if ( c == argc )
			{
				std::cerr << "ERROR: missing argument to function argument, expecting function name\n" << std::endl;
				usageAndExit();
			}
			chosenFunc = argv[c];
			bool found = false;
			for ( size_t i = 0; i != nFuncs; ++i )
			{
				if ( funcs[i]->getName() == chosenFunc )
				{
					found = true;
					funcIdx = i;
					break;
				}
			}

			if ( ! found )
			{
				std::cerr << "ERROR: Unknown function specified: '" << chosenFunc << "', please check your spelling" << std::endl;
				usageAndExit();
			}
			continue;
		}

		bool foundFuncArg = false;
		for ( size_t i = 0; i != nFuncs; ++i )
		{
			const std::vector<std::string> &flags = funcs[i]->getArgumentFlags();
			const std::vector<std::string> &floatargs = funcs[i]->getFloatArgs();

			for ( size_t f = 0, nF = flags.size(); f != nF; ++f )
			{
				if ( flags[f] == arg )
				{
					funcs[i]->setFlag( arg );
					foundFuncArg = true;
					break;
				}
			}

			for ( size_t f = 0, nF = floatargs.size(); f != nF; ++f )
			{
				if ( floatargs[f] == arg )
				{
					funcs[i]->setArgument( arg, extractFloatArg( arg, c, argc, argv ) );
					foundFuncArg = true;
					break;
				}
			}
		}

		if ( foundFuncArg )
			continue;

		std::cerr << "ERROR: Unknown argument '" << argv[c] << "'\n" << std::endl;
		usageAndExit();
	}

	if ( chosenFunc.empty() && nFuncs > 1 )
	{
		std::cerr << "ERROR: There were multiple CTL routines embedded, please specify which to use using the function argument" << std::endl;
		usageAndExit();
	}

	if ( fileArgs.size() < 2 )
	{
		std::cerr << "ERROR: Need to specify at least 2 file arguments (or an output directory)\n" << std::endl;
		usageAndExit();
	}
	std::string outputPath = fileArgs.back();
	fileArgs.pop_back();

	std::vector< std::pair<std::string, std::string> > filePairs;
	filePairs.reserve( fileArgs.size() );
	if ( isDirectory( outputPath ) )
	{
		if ( *(outputPath.rbegin()) != '/' )
			outputPath.push_back( '/' );
		for ( size_t a = 0; a != fileArgs.size(); ++a )
		{
			std::string outPath = outputPath + basename( fileArgs[a] );
			if ( ! force )
			{
				if ( exists( outPath ) )
				{
					std::cerr << "ERROR: Output path '" << outPath << "' exists, skipping (use -force to overwrite)" << std::endl;
					continue;
				}
			}
			filePairs.push_back( std::make_pair( fileArgs[a], outPath ) );
		}
	}
	else
	{
		if ( fileArgs.size() > 1 )
		{
			std::cerr << "ERROR: More than one input file specified, but last argument does not specify an existing directory" << std::endl;
			usageAndExit();
		}
		if ( ! force && exists( outputPath ) )
			std::cerr << "ERROR: Output path '" << outputPath << "' exists, skipping (use -force to overwrite)" << std::endl;
		else
			filePairs.push_back( std::make_pair( fileArgs[0], outputPath ) );
	}

	if ( filePairs.empty() )
		return -1;

	theDriver.init( nThreads );
	const Function &theFunc = *(funcs[funcIdx]);

	ctl::dpx::fb<float> pixels;
	for ( size_t f = 0; f != filePairs.size(); ++f )
	{
		const std::string &inPath = filePairs[f].first;
		const std::string &outPath = filePairs[f].second;

		struct timeval startTime, readTime, writeTime, procTime;
		format_t curFmt;
		gettimeofday( &startTime, NULL );
		if ( readImage( inPath, inScale, pixels, curFmt ) )
		{
			gettimeofday( &readTime, NULL );
			theFunc.apply( pixels );
			gettimeofday( &procTime, NULL );
			if ( ! outFmt.name.empty() )
				curFmt = outFmt.output_fmt;
			else
			{
				curFmt.ext = extension( inPath );
				curFmt.bps = curFmt.src_bps;
			}

			if ( ! writeImage( outPath, outScale, pixels, curFmt, outComp ) )
			{
				if ( stopOnErrors )
				{
					std::cerr << "ERROR writing output file '" << outPath << "', exiting..." << std::endl;
					break;
				}
			}
			else
			{
				gettimeofday( &writeTime, NULL );
				std::cout << inPath << " ==> " << outPath << ":";
				reportTime( "Read", startTime, readTime );
				reportTime( "Process", readTime, procTime );
				reportTime( "Write", procTime, writeTime );
				reportTime( "Total", startTime, writeTime );
				std::cout << std::endl;
			}
		}
		else if ( stopOnErrors )
		{
			std::cerr << "ERROR reading file '" << inPath << "', exiting..." << std::endl;
			break;
		}
	}

	theDriver.shutdown();
	std::cout << "Finished!" << std::endl;
	return 0;
}

int
main( int argc, char *argv[] )
{
	int retval = 0;
	try
	{
		retval = safeMain( argc, argv );
	}
	catch ( std::exception &e )
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
		retval = -1;
	}
	return retval;
}
