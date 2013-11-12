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


//-----------------------------------------------------------------------------
//
//	class SimdInterpreter
//
//-----------------------------------------------------------------------------

#include "CtlCodeInterpreter.h"
#include "CtlCodeLContext.h"
#include "CtlCodeModule.h"
#include "CtlCodeFunctionCall.h"
#include "CtlCodeStdLibrary.h"

#include "CtlCodeCPPLanguage.h"
#include "CtlCodeCUDALanguage.h"
#include "CtlCodeOPENCLLanguage.h"
#include "CtlCodeNukeLanguage.h"

#include <cassert>
#include <sstream>
#include <stdexcept>

namespace Ctl {


////////////////////////////////////////


CodeInterpreter::CodeInterpreter( void )
		: Interpreter(), myLanguage( CPP03 ), myLanguageGenerator( NULL ),
		  myCalledOnly( false )
{
	myLanguageGenerator = new CPPGenerator( false );
}


////////////////////////////////////////


CodeInterpreter::~CodeInterpreter( void )
{
	delete myLanguageGenerator;
}


////////////////////////////////////////


size_t
CodeInterpreter::maxSamples( void ) const
{
    return 1;
}


////////////////////////////////////////


void	
CodeInterpreter::setMaxInstCount( unsigned long count )
{
}


////////////////////////////////////////


void	
CodeInterpreter::abortAllPrograms( void )
{
}


////////////////////////////////////////


void
CodeInterpreter::setLanguage( Language l )
{
	if ( myLanguage == l )
		return;

	myLanguage = l;
	LanguageGenerator::Precision curPrec = myLanguageGenerator->getPrecision();
	delete myLanguageGenerator;
	myLanguageGenerator = NULL;
	switch ( l )
	{
		case C89:
		case C99:
			throw std::logic_error( "Sorry, not implemented yet" );
		case CPP03:
			myLanguageGenerator = new CPPGenerator( false );
			break;
		case CPP11:
			myLanguageGenerator = new CPPGenerator( true );
			break;
		case OPENCL:
		case GLSL:
			throw std::logic_error( "Sorry, not implemented yet" );
//			myLanguageGenerator = new OPENCLGenerator;
//			break;
		case CUDA:
			myLanguageGenerator = new CUDAGenerator;
			break;
		case NUKE:
			myLanguageGenerator = new NukeGenerator;
			break;
	}
	myLanguageGenerator->setPrecision( curPrec );
}


////////////////////////////////////////


bool
CodeInterpreter::isDriverEnabledForLanguage( void ) const
{
	switch ( myLanguage )
	{
		case CPP03:
		case CPP11:
			return true;

		case NUKE:
			return true;

		case C89:
		case C99:
		case OPENCL:
		case GLSL:
		case CUDA:
			break;
	}

	return false;
}


////////////////////////////////////////


void
CodeInterpreter::setPrecision( LanguageGenerator::Precision p )
{
	myLanguageGenerator->setPrecision( p );
}


////////////////////////////////////////


void
CodeInterpreter::initStdLibrary( void )
{
	if ( ! myLanguageGenerator )
		throw std::logic_error( "No Language generator yet" );

    //
    // Create a dummy LContext and load the standard library
    //
    CodeModule module( "none", "none" );
	std::stringstream file;
    CodeLContext lcontext( *myLanguageGenerator, file, &module, symtab() );

    declareCodeStdLibrary( lcontext );
	myLanguageGenerator->initStdLibrary();
}


////////////////////////////////////////


void
CodeInterpreter::emitHeader( std::ostream &out )
{
	out << myLanguageGenerator->getHeaderPrefix()
		<< myLanguageGenerator->getHeaderCode( myCalledOnly )
		<< myLanguageGenerator->getHeaderSuffix()
		<< std::endl;
}


////////////////////////////////////////


void
CodeInterpreter::emitCode( std::ostream &out )
{
	out << myLanguageGenerator->stdLibraryAndSetup()
		<< myLanguageGenerator->getCode( myCalledOnly );
}


////////////////////////////////////////


void
CodeInterpreter::emitDriverCode( std::ostream &out )
{
	out << myLanguageGenerator->getDriver();
}


////////////////////////////////////////


Module *
CodeInterpreter::newModule( const std::string &moduleName,
							const std::string &fileName )
{
    return new CodeModule( moduleName, fileName );
}


////////////////////////////////////////


FunctionCallPtr
CodeInterpreter::newFunctionCallInternal( const SymbolInfoPtr info, 
										  const std::string &functionName )
{
    assert(info);

    return new CodeFunctionCall( *this, functionName,
								 info->type(), info->addr(), symtab() );
}


////////////////////////////////////////


LContext *
CodeInterpreter::newLContext( std::istream &file,
							  Module *module,
							  SymbolTable &symtab ) const
{
	if ( ! myLanguageGenerator )
		throw std::logic_error( "No Language generator yet" );

    return new CodeLContext( *myLanguageGenerator, file, module, symtab );
}

} // namespace Ctl
