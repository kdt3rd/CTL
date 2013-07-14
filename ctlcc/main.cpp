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

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>

#include <CtlCodeInterpreter.h>

void usageAndExit( const char *argv0, int rv = -1 )
{
	std::cout << "ctlcc: " << argv0 << "<options> xxx.ctl ...\n\n"
			  << " Options:\n\n"
			  << "    -h|--help                        This message\n\n"
			  << "    --lang=[c,c++,c++11,opencl,cuda] Which language standard to generate\n\n"
			  << "    -I <path>|--include=<path>       Add an include path to search for CTL modules\n"
			  << "    -o <file>|--output=<file>        Send output to file\n"
			  << "    --header=<file>                  Send header definition to file\n"
			  << "    --threads                        Generate internal thread dispatch across all processors (default: off)\n"
			  << "    -p [f|d|l]|--precision=[f|d|l]   Generates specified floating point precision (some routines may be decimated)\n"
			  << std::endl;

	exit( rv );
}

int
main( int argc, const char *argv[] )
{
	try
	{
		Ctl::CodeInterpreter interpreter;
		std::vector< std::pair<std::string, std::string> > modList;

		// This will generate the default paths and add any from
		// the CTL_MODULE_PATH env. var, then we'll add the -I ones
		std::vector<std::string> modPaths = interpreter.modulePaths();

		std::string outputFN;
		std::string headerFN;
		bool genThreads = false;

		for ( int i = 1; i < argc; ++i )
		{
			if ( strcmp( argv[i], "-h" ) == 0 ||
				 strcmp( argv[i], "-help" ) == 0 ||
				 strcmp( argv[i], "--help" ) == 0 )
			{
				usageAndExit( argv[0], 0 );
			}

			if ( strcmp( argv[i], "--threads" ) == 0 )
			{
				genThreads = true;
				continue;
			}

			if ( strcmp( argv[i], "-I" ) == 0 )
			{
				++i;
				if ( i < argc )
					modPaths.push_back( std::string( argv[i] ) );
				else
				{
					std::cerr << "Missing argument for -I <path>" << std::endl;
					usageAndExit( argv[0] );
				}
				continue;
			}

			if ( strncmp( argv[i], "--include=", 10 ) == 0 )
			{
				std::string path( argv[i] + 10 );
				if ( ! path.empty() )
					modPaths.push_back( path );
				else
				{
					std::cerr << "Missing argument for --include=<path>" << std::endl;
					usageAndExit( argv[0] );
				}
				continue;
			}

			if ( strcmp( argv[i], "-o" ) == 0 )
			{
				++i;
				if ( i < argc )
					outputFN = argv[i];
				else
				{
					std::cerr << "Missing argument for -o <file>" << std::endl;
					usageAndExit( argv[0] );
				}
				continue;
			}

			if ( strncmp( argv[i], "--output=", 9 ) == 0 )
			{
				std::string path( argv[i] + 9 );
				if ( ! path.empty() )
					outputFN = path;
				else
				{
					std::cerr << "Missing argument for --include=<path>" << std::endl;
					usageAndExit( argv[0] );
				}
				continue;
			}

			if ( strncmp( argv[i], "--header=", 9 ) == 0 )
			{
				std::string path( argv[i] + 9 );
				if ( ! path.empty() )
					headerFN = path;
				else
				{
					std::cerr << "Missing argument for --header=<filename>" << std::endl;
					usageAndExit( argv[0] );
				}
				continue;
			}

			if ( strncmp( argv[i], "--lang=", 7 ) == 0 )
			{
				std::string lang( argv[i] + 7 );
				if ( ! lang.empty() )
				{
					if ( lang == "c++" )
						interpreter.setLanguage( Ctl::CodeInterpreter::CPP03 );
					else if ( lang == "c++11" )
						interpreter.setLanguage( Ctl::CodeInterpreter::CPP11 );
					else if ( lang == "opencl" )
						interpreter.setLanguage( Ctl::CodeInterpreter::OPENCL );
					else if ( lang == "cuda" )
						interpreter.setLanguage( Ctl::CodeInterpreter::CUDA );
				}
				else
				{
					std::cerr << "Missing argument for --lang=<language>" << std::endl;
					usageAndExit( argv[0] );
				}
				continue;
			}

			if ( strcmp( argv[i], "-p" ) == 0 )
			{
				++i;
				if ( i < argc )
				{
					if ( strcmp( argv[i], "f" ) == 0 )
						interpreter.setPrecision( Ctl::LanguageGenerator::FLOAT );
					else if ( strcmp( argv[i], "d" ) == 0 )
						interpreter.setPrecision( Ctl::LanguageGenerator::DOUBLE );
					else if ( strcmp( argv[i], "l" ) == 0 )
						interpreter.setPrecision( Ctl::LanguageGenerator::LONG_DOUBLE );
					else
					{
						std::cerr << "Unknown precision type specified" << std::endl;
						usageAndExit( argv[0] );
					}
				}
				else
				{
					std::cerr << "Missing argument for -p [f|d|l]" << std::endl;
					usageAndExit( argv[0] );
				}
				continue;
			}

			if ( strncmp( argv[i], "--precision=", 12 ) == 0 )
			{
				std::string precisionStr( argv[i] + 12 );
				if ( ! precisionStr.empty() )
				{
					if ( precisionStr == "f" )
						interpreter.setPrecision( Ctl::LanguageGenerator::FLOAT );
					else if ( precisionStr == "d" )
						interpreter.setPrecision( Ctl::LanguageGenerator::DOUBLE );
					else if ( precisionStr == "l" )
						interpreter.setPrecision( Ctl::LanguageGenerator::LONG_DOUBLE );
					else
					{
						std::cerr << "Unknown precision type specified" << std::endl;
						usageAndExit( argv[0] );
					}
				}
				else
				{
					std::cerr << "Missing argument for --precision=[f|d|l]" << std::endl;
					usageAndExit( argv[0] );
				}
				continue;
			}

			if ( argv[i][0] == '-' )
			{
				std::cerr << "Unknown argument '" << argv[i] << "'" << std::endl;
				usageAndExit( argv[0] );
			}

			std::string filename = argv[i];
			std::string module = filename;
			std::string::size_type mPos = module.find_last_of( '.' );
			if ( mPos != std::string::npos )
				module.erase( mPos );
			mPos = module.find_last_of( '/' );
			if ( mPos != std::string::npos )
				module.erase( 0, mPos + 1 );

			modList.push_back( std::make_pair( module, filename ) );
		}

		interpreter.setModulePaths( modPaths );

		if ( genThreads )
		{
			switch ( interpreter.getLanguage() )
			{
				case Ctl::CodeInterpreter::CPP03:
				case Ctl::CodeInterpreter::CPP11:
					break;
				default:
					std::cerr << "Thread generation does not work for specified language" << std::endl;
					usageAndExit( argv[0] );
					break;
			}
		}

		interpreter.initStdLibrary();

		for ( size_t i = 0, N = modList.size(); i != N; ++i )
			interpreter.loadFile( modList[i].second, modList[i].first );

		if ( outputFN.empty() || outputFN == "-" )
			interpreter.emitCode( std::cout );
		else
		{
			std::ofstream output( outputFN.c_str() );
			interpreter.emitCode( output );
		}

		if ( ! headerFN.empty() )
		{
			std::ofstream output( headerFN.c_str() );
			interpreter.emitHeader( output );
		}
	}
	catch ( std::exception &e )
	{
		std::cerr << "Unhandled exception:\n" << e.what() << std::endl;
		return -1;
	}

	return 0;
}
