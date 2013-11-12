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

#include "CtlCodeCUDALanguage.h"
#include <stdexcept>

namespace Ctl
{

CUDAGenerator::CUDAGenerator( void )
		: CPPGenerator()
{
}


////////////////////////////////////////


CUDAGenerator::~CUDAGenerator( void )
{
}


////////////////////////////////////////


bool
CUDAGenerator::supportsPrecision( Precision p ) const
{
	switch ( p )
	{
		case FLOAT: return true;
		case DOUBLE: return true;
		case LONG_DOUBLE: return false;

		default:
			throw std::logic_error( "unhandled precision style" );
	}
}


////////////////////////////////////////


std::string
CUDAGenerator::getDriver( void )
{
	std::stringstream retval;

	return retval.str();
}


////////////////////////////////////////


void
CUDAGenerator::getStandardPrintBodies( FuncDeclList &d, const std::string &funcPref, const std::string &precSuffix )
{
	// just get the default ones, not the c++ ones so we have nothing,
	// at least for now
	CCommonLanguage::getStandardPrintBodies( d, funcPref, precSuffix );
}


////////////////////////////////////////


bool
CUDAGenerator::usesFunctionInitializers( void ) const
{
	return false;
}


////////////////////////////////////////


bool
CUDAGenerator::supportsModuleDynamicInitialization( void ) const
{
	return false;
}


////////////////////////////////////////


bool
CUDAGenerator::supportsNamespaces( void ) const
{
	return false;
}


////////////////////////////////////////


bool
CUDAGenerator::supportsHalfType( void ) const
{
	return false;
}


////////////////////////////////////////


std::string
CUDAGenerator::constructNamespaceTag( const std::string &modName )
{
	return modName + "_";
}


////////////////////////////////////////


const std::string &
CUDAGenerator::getInlineKeyword( void ) const
{
	static std::string kInline = "__inline__";
	return kInline;
}


////////////////////////////////////////


const std::string &
CUDAGenerator::getFunctionPrefix( void ) const
{
	static std::string kPrefix = "__device__";
	return kPrefix;
}


////////////////////////////////////////


const std::string &
CUDAGenerator::getGlobalPrefix( void ) const
{
	static std::string kGlobal = "__device__";
	return kGlobal;
}


////////////////////////////////////////


const std::string &
CUDAGenerator::getCTLNamespaceTag( void ) const
{
	static std::string kTag = "_ctlcc_";
	return kTag;
}


////////////////////////////////////////


const std::string &
CUDAGenerator::getBoolTypeName( void ) const
{
	static std::string kType = "bool";
	return kType;
}


////////////////////////////////////////


const std::string &
CUDAGenerator::getBoolLiteral( bool v ) const
{
	static std::string kBoolTrue = "true";
	static std::string kBoolFalse = "false";
	if ( v )
		return kBoolTrue;
	return kBoolFalse;
}


////////////////////////////////////////


const std::string &
CUDAGenerator::getConstLiteral( void ) const
{
	// do we need __constant__ support? I think const works in all
	// recent versions of cuda (>= 4 anyway)
	static std::string kConst = "const";
	return kConst;
}


////////////////////////////////////////


void
CUDAGenerator::startCast( const char *type )
{
	curStream() << '(' << type << ")( ";
}

} // namespace Ctl


