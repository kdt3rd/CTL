///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2013 Academy of Motion Picture Arts and Sciences
// ("A.M.P.A.S."). Portions contributed by others as indicated.
// All rights reserved.
// 
// A world-wide, royalty-free, non-exclusive right to distribute, copy,
// modify, create derivatives, and use, in source and binary forms, is
// hereby granted, subject to acceptance of this license. Performance of
// any of the aforementioned acts indicates acceptance to be bound by the
// following terms and conditions:
// 
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the Disclaimer of Warranty.
// 
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the Disclaimer of Warranty
//     in the documentation and/or other materials provided with the
//     distribution.
// 
//   * Nothing in this license shall be deemed to grant any rights to
//     trademarks, copyrights, patents, trade secrets or any other
//     intellectual property of A.M.P.A.S. or any contributors, except
//     as expressly stated herein, and neither the name of A.M.P.A.S.
//     nor of any other contributors to this software, may be used to
//     endorse or promote products derived from this software without
//     specific prior written permission of A.M.P.A.S. or contributor,
//     as appropriate.
// 
// This license shall be governed by the laws of the State of California,
// and subject to the jurisdiction of the courts therein.
// 
// Disclaimer of Warranty: THIS SOFTWARE IS PROVIDED BY A.M.P.A.S. AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT ARE DISCLAIMED. IN NO
// EVENT SHALL A.M.P.A.S., ANY CONTRIBUTORS OR DISTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

#include "CtlCodeLanguageGenerator.h"
#include <iomanip>
#include <stdexcept>

namespace Ctl
{

LanguageGenerator::LanguageGenerator( void )
		: myPrecision( FLOAT ), myCurIndent( 0 ), myIndent( "    " )
{
}


////////////////////////////////////////


LanguageGenerator::~LanguageGenerator( void )
{
}


////////////////////////////////////////


void
LanguageGenerator::initStdLibrary( void )
{
}


////////////////////////////////////////


void
LanguageGenerator::setPrecision( Precision p )
{
	if ( ! supportsPrecision( p ) )
		throw std::logic_error( "Attempt to set a precision not supported by this generator" );

	myPrecision = p;
}


////////////////////////////////////////


std::string
LanguageGenerator::getHeaderPrefix( void ) const
{
	return std::string();
}


////////////////////////////////////////


std::string
LanguageGenerator::getHeaderSuffix( void ) const
{
	return std::string();
}


////////////////////////////////////////


std::string
LanguageGenerator::getDriver( void )
{
	return std::string();
}


////////////////////////////////////////


void
LanguageGenerator::addIndent( void )
{
	for ( int i = 0; i < myCurIndent; ++i )
		curStream() << myIndent;
}


////////////////////////////////////////


void
LanguageGenerator::newlineAndIndent( void )
{
	curStream() << '\n';
	addIndent();
}


////////////////////////////////////////


void
LanguageGenerator::pushIndent( void )
{
	++myCurIndent;
}


////////////////////////////////////////


void
LanguageGenerator::popIndent( void )
{
	--myCurIndent;
	if ( myCurIndent < 0 )
		throw std::logic_error( "Invalid push/pop pairs for indent level" );
}


////////////////////////////////////////


void
LanguageGenerator::pushStream( std::ostream &os )
{
	myStreamStack.push( &os );
}


////////////////////////////////////////


void
LanguageGenerator::popStream( void )
{
	if ( myStreamStack.empty() )
		throw std::logic_error( "Invalid push/pop pairs for stream stack" );

	myStreamStack.pop();
}


////////////////////////////////////////


std::string
LanguageGenerator::cleanName( const std::string &x )
{
	std::string retval = x;
	for ( size_t i = 0, N = retval.size(); i != N; ++i )
	{
		char &curC = retval[i];
		if ( i == 0 )
		{
			if ( curC != '_' && ! isalpha( curC ) )
				curC = '_';
		}
		else if ( curC != '_' && ! isalnum( curC ) )
			curC = '_';
	}
	return retval;
}


////////////////////////////////////////


std::string
LanguageGenerator::escapeLiteral( const std::string &s )
{
	std::string retval;
	retval.reserve( s.size() );
	for ( size_t i = 0, N = s.size(); i != N; ++i )
	{
		switch ( s[i] )
		{
			case '\n':
				retval.push_back( '\\' );
				retval.push_back( 'n' );
				break;
			case '\r':
				retval.push_back( '\\' );
				retval.push_back( 'r' );
				break;
			case '\t':
				retval.push_back( '\\' );
				retval.push_back( 't' );
				break;
			case '"':
				retval.push_back( '\\' );
				retval.push_back( '"' );
				break;
			default:
				retval.push_back( s[i] );
				break;
		}

	}
	return retval;
}


////////////////////////////////////////


std::string
LanguageGenerator::removeNSQuals( const std::string &x )
{
	std::string retval = x;
	std::string::size_type firstP = retval.find( "::" );
	if ( firstP == 0 )
		retval.erase( 0, firstP + 2 );
	else if ( firstP != std::string::npos )
	{
		do
		{
			retval.replace( firstP, 2, "__" );
			firstP = retval.find( "::" );
		} while ( firstP != std::string::npos );
	}
	return cleanName( retval );
}


////////////////////////////////////////


std::string
LanguageGenerator::getPrecisionFunctionSuffix( void ) const
{
	switch ( myPrecision )
	{
		case FLOAT: return std::string( "f" );
		case LONG_DOUBLE: return std::string( "l" );
		case DOUBLE: break;
	}

	return std::string();
}


////////////////////////////////////////


std::string
LanguageGenerator::getPrecisionType( void ) const
{
	switch ( myPrecision )
	{
		case FLOAT: return std::string( "float" );
		case DOUBLE: return std::string( "double" );
		case LONG_DOUBLE: return std::string( "long double" );
	}

	return std::string( "unknown" );
}


////////////////////////////////////////


std::string
LanguageGenerator::getPrecisionTypeSuffix( void ) const
{
	switch ( myPrecision )
	{
		case FLOAT: return std::string( "F" );
		case LONG_DOUBLE: return std::string( "L" );
		case DOUBLE: break;
	}

	return std::string();
}


////////////////////////////////////////


void
LanguageGenerator::registerMainRoutine( const std::string &name,
										const std::string &nsName,
										const SymbolInfoPtr &fnInfo )
{
	if ( myMainRoutines.find( name ) != myMainRoutines.end() )
		throw std::logic_error( "Attempt to register main routine '" + name + "' twice" );

	myMainRoutines[name] = std::make_pair( nsName, fnInfo );
}

} // namespace Ctl
