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


#include "CtlCodeCCommonLanguage.h"
#include "CtlCodeLContext.h"
#include "CtlCodeType.h"
#include "CtlCodeAddr.h"
#include "CtlCodeModule.h"
#include <iomanip>
#include <limits>
#include <half.h>
#include <deque>
#include <ctype.h>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>
#include <CtlErrors.h>
#include <CtlMessage.h>

////////////////////////////////////////


namespace Ctl
{


////////////////////////////////////////


CCommonLanguage::CCommonLanguage( void )
		: LanguageGenerator(),
		  myCurModule( NULL ),
		  myInElse( 0 ),
		  myInFunction( 0 ),
		  myInModuleInit( 0 ),
		  myDoForwardDecl( 0 ),
		  myExprLevel( 0 ),
		  myCurInitType( NONE ),
		  myCurModuleDefinition( NULL ),
		  myCurModuleUsage( NULL ),
		  myCurInitDestination( NULL )
{
}


////////////////////////////////////////


CCommonLanguage::~CCommonLanguage( void )
{
}


////////////////////////////////////////


void
CCommonLanguage::initStdLibrary( void )
{
	if ( ! myModules.empty() )
		return;

	std::string precSuffix = getPrecisionFunctionSuffix();

	ModuleDefinition stdMod;
	stdMod.module = NULL;
	stdMod.name = "ctlcc";
	std::stringstream prefixBuf;
	pushStream( prefixBuf );
	newlineAndIndent();
	curStream() << beginComment() << " Auto-generated standard library of CTL functions" << endComment();
	newlineAndIndent();
	if ( supportsNamespaces() )
	{
		curStream() << "namespace _ctlcc_ {";
		newlineAndIndent();
		stdMod.suffix = "\n} // namespace _ctlcc_\n";
	}
	else
		stdMod.call_prefix = "_ctlcc_";
	popStream();
	stdMod.prefix = prefixBuf.str();
	
	myStdMathFuncs["isinfinite_f"] = "isfinite";
	myStdMathFuncs["isnormal_f"] = "isnormal";
	myStdMathFuncs["isnan_f"] = "isnan";
	myStdMathFuncs["isinf_f"] = "isinf";
	myStdMathFuncs["acos"] = "acos" + precSuffix;
	myStdMathFuncs["asin"] = "asin" + precSuffix;
	myStdMathFuncs["atan"] = "atan" + precSuffix;
	myStdMathFuncs["atan2"] = "atan2" + precSuffix;
	myStdMathFuncs["cos"] = "cos" + precSuffix;
	myStdMathFuncs["sin"] = "sin" + precSuffix;
	myStdMathFuncs["tan"] = "tan" + precSuffix;
	myStdMathFuncs["cosh"] = "cosh" + precSuffix;
	myStdMathFuncs["sinh"] = "sinh" + precSuffix;
	myStdMathFuncs["tanh"] = "tanh" + precSuffix;
	myStdMathFuncs["exp"] = "exp" + precSuffix;
	myStdMathFuncs["log"] = "log" + precSuffix;
	myStdMathFuncs["log10"] = "log10" + precSuffix;
	myStdMathFuncs["pow"] = "pow" + precSuffix;
	myStdMathFuncs["sqrt"] = "sqrt" + precSuffix;
	myStdMathFuncs["fabs"] = "fabs" + precSuffix;
	myStdMathFuncs["floor"] = "floor" + precSuffix;
	myStdMathFuncs["fmod"] = "fmod" + precSuffix;
	myStdMathFuncs["hypot"] = "hypot" + precSuffix;

	std::string fltconstprefix, fltconstsuff, mathconstsuff;
	switch ( getPrecision() )
	{
		case FLOAT: fltconstprefix = "FLT"; fltconstsuff = "F"; break;
		case DOUBLE: fltconstprefix = "DBL"; break;
		case LONG_DOUBLE: fltconstprefix = "LDBL"; fltconstsuff = "L"; mathconstsuff = "l"; break;
	}
	myStdNames["M_E"] = "M_E" + mathconstsuff;
	myStdNames["M_PI"] = "M_PI" + mathconstsuff;
	myStdNames["FLT_MAX"] = fltconstprefix + "_MAX";
	myStdNames["FLT_MIN"] = fltconstprefix + "_MIN";
	myStdNames["FLT_EPSILON"] = fltconstprefix + "_EPSILON";
	myStdNames["FLT_POS_INF"] = "HUGE_VAL" + fltconstsuff;
	myStdNames["FLT_NEG_INF"] = "(-HUGE_VAL" + fltconstsuff + ")";
	myStdNames["FLT_NAN"] = "__builtin_nan" + precSuffix + "(\"\")";

	myStdNames["HALF_MAX"] = "HALF_MAX";
	myStdNames["HALF_MIN"] = "HALF_MIN";
	myStdNames["HALF_EPSILON"] = "HALF_EPSILON";
	myStdNames["HALF_POS_INF"] = "half::posInf()";
	myStdNames["HALF_NEG_INF"] = "half::negInf()";
	myStdNames["HALF_NAN"] = "half::qNan()";

	myStdNames["INT_MAX"] = "INT_MAX";
	myStdNames["INT_MIN"] = "INT_MIN";
	myStdNames["UINT_MAX"] = "UINT_MAX";

	initStandardLibraryBodies( stdMod );
	myModules.push_back( stdMod );
}


////////////////////////////////////////


void
CCommonLanguage::addStandardIncludes( void )
{
	curStream() <<
		"\n#include <math.h>"
		"\n#include <stdint.h>"
		"\n#include <limits.h>"
		"\n#include <float.h>";
	if ( supportsHalfType() )
		curStream() << "\n#include <half.h>";
}


////////////////////////////////////////


void
CCommonLanguage::initStandardLibraryBodies( ModuleDefinition &stdMod )
{
	std::string funcPref = getFunctionPrefix() + " " + getInlineKeyword() + "\n";
	std::string precSuffix = getPrecisionFunctionSuffix();

	defineStandardTypes( stdMod.types, funcPref, precSuffix );
	getStandardPrintBodies( stdMod.functions, funcPref, precSuffix );
	getStandardMathBodies( stdMod.functions, funcPref, precSuffix );
	getStandardHalfBodies( stdMod.functions, funcPref, precSuffix );
	getStandardColorBodies( stdMod.functions, funcPref, precSuffix );
	getStandardInterpBodies( stdMod.functions, funcPref, precSuffix );
}


////////////////////////////////////////


void
CCommonLanguage::defineStandardTypes( std::vector<TypeDefinition> &types, const std::string &funcPref, const std::string &precSuffix )
{
	if ( supportsStructOperators() )
		throw std::logic_error( "Subclass supports struct operators but does not define appropriate standard types" );

	types.push_back( TypeDefinition( "ctl_vec2f_t",
"struct ctl_vec2f_t\n"
"{\n"
"    union\n"
"    {\n"
"        ctl_number_t vals[2];\n"
"        struct\n"
"        {\n"
"            ctl_number_t x;\n"
"            ctl_number_t y;\n"
"        };\n"
"    };\n"
"};",
funcPref + "ctl_vec2f_t make_vec2f( ctl_number_t x, ctl_number_t y )\n"
"{ ctl_vec2f_t r; r.x = x; r.y = y; return r; }" ) );

	types.push_back( TypeDefinition( "ctl_vec2i_t",
"struct ctl_vec2i_t\n"
"{\n"
"    union\n"
"    {\n"
"        int vals[2];\n"
"        struct\n"
"        {\n"
"            int x;\n"
"            int y;\n"
"        };\n"
"    };\n"
"};",
funcPref + "ctl_vec2i_t make_vec2i( int x, int y )\n"
"{ ctl_vec2i_t r; r.x = x; r.y = y; return r; }" ) );

	types.push_back( TypeDefinition( "ctl_vec3f_t",
"struct ctl_vec3f_t\n"
"{\n"
"    union\n"
"    {\n"
"        ctl_number_t vals[3];\n"
"        struct\n"
"        {\n"
"            ctl_number_t x;\n"
"            ctl_number_t y;\n"
"            ctl_number_t z;\n"
"        };\n"
"    };\n"
"};",
funcPref + "ctl_vec3f_t make_vec3f( ctl_number_t x, ctl_number_t y, ctl_number_t z )\n"
"{ ctl_vec3f_t r; r.x = x; r.y = y; r.z = z; return r; }" ) );

	types.push_back( TypeDefinition( "ctl_vec3i_t",
"struct ctl_vec3i_t\n"
"{\n"
"    union\n"
"    {\n"
"        int vals[3];\n"
"        struct\n"
"        {\n"
"            int x;\n"
"            int y;\n"
"            int z;\n"
"        };\n"
"    };\n"
"};",
funcPref + "ctl_vec3i_t make_vec3i( int x, int y, int z )\n"
"{ ctl_vec3i_t r; r.x = x; r.y = y; r.z = z; return r; }" ) );

	types.push_back( TypeDefinition( "ctl_vec4f_t",
"struct ctl_vec4f_t\n"
"{\n"
"    union\n"
"    {\n"
"        ctl_number_t vals[4];\n"
"        struct\n"
"        {\n"
"            ctl_number_t x;\n"
"            ctl_number_t y;\n"
"            ctl_number_t z;\n"
"            ctl_number_t w;\n"
"        };\n"
"    };\n"
"};",
funcPref + "ctl_vec4f_t make_vec4f( ctl_number_t x, ctl_number_t y, ctl_number_t z, ctl_number_t w )\n"
"{ ctl_vec4f_t r; r.x = x; r.y = y; r.z = z; r.w = w; return r; }" ) );

	types.push_back( TypeDefinition( "ctl_vec4i_t",
"struct ctl_vec4i_t\n"
"{\n"
"    union\n"
"    {\n"
"        int vals[4];\n"
"        struct\n"
"        {\n"
"            int x;\n"
"            int y;\n"
"            int z;\n"
"            int w;\n"
"        };\n"
"    };\n"
"};",
funcPref + "ctl_vec4i_t make_vec4i( int x, int y, int z, int w )\n"
"{ ctl_vec4i_t r; r.x = x; r.y = y; r.z = z; r.w = w; return r; }" ) );

	types.push_back( TypeDefinition( "ctl_mat33f_t", "struct ctl_mat33f_t { ctl_number_t vals[3][3]; };",
funcPref + "ctl_mat33f_t make_mat33f( ctl_number_t m00, ctl_number_t m01, ctl_number_t m02, ctl_number_t m10, ctl_number_t m11, ctl_number_t m12, ctl_number_t m20, ctl_number_t m21, ctl_number_t m22 )\n"
"{\n"
"    ctl_mat33f_t r;\n"
"    r.vals[0][0] = m00;\n"
"    r.vals[0][1] = m01;\n"
"    r.vals[0][2] = m02;\n"
"    r.vals[1][0] = m10;\n"
"    r.vals[1][1] = m11;\n"
"    r.vals[1][2] = m12;\n"
"    r.vals[2][0] = m20;\n"
"    r.vals[2][1] = m21;\n"
"    r.vals[2][2] = m22;\n"
"    return r;\n"
"}" ) );
	types.push_back( TypeDefinition( "ctl_mat44f_t", "struct ctl_mat44f_t { ctl_number_t vals[4][4]; };",
funcPref + "ctl_mat44f_t make_mat44f( ctl_number_t m00, ctl_number_t m01, ctl_number_t m02, ctl_number_t m03, ctl_number_t m10, ctl_number_t m11, ctl_number_t m12, ctl_number_t m13, ctl_number_t m20, ctl_number_t m21, ctl_number_t m22, ctl_number_t m23, ctl_number_t m30, ctl_number_t m31, ctl_number_t m32, ctl_number_t m33 )\n"
"{\n"
"    ctl_mat44f_t r;\n"
"    r.vals[0][0] = m00;\n"
"    r.vals[0][1] = m01;\n"
"    r.vals[0][2] = m02;\n"
"    r.vals[0][3] = m03;\n"
"    r.vals[1][0] = m10;\n"
"    r.vals[1][1] = m11;\n"
"    r.vals[1][2] = m12;\n"
"    r.vals[1][3] = m13;\n"
"    r.vals[2][0] = m20;\n"
"    r.vals[2][1] = m21;\n"
"    r.vals[2][2] = m22;\n"
"    r.vals[2][3] = m23;\n"
"    r.vals[3][0] = m30;\n"
"    r.vals[3][1] = m31;\n"
"    r.vals[3][2] = m32;\n"
"    r.vals[3][3] = m33;\n"
"    return r;\n"
"}" ) );

	types.push_back( TypeDefinition( "Chromaticities", "struct Chromaticities { ctl_vec2f_t red; ctl_vec2f_t green; ctl_vec2f_t blue; ctl_vec2f_t white; };" ) );
	types.back().moduleUsage[NULL].types.insert( "ctl_vec2f_t" );
	types.push_back( TypeDefinition( "Box2i", "struct Box2i { ctl_vec2i_t min; ctl_vec2i_t max; };" ) );
	types.back().moduleUsage[NULL].types.insert( "ctl_vec2i_t" );
	types.push_back( TypeDefinition( "Box2f", "struct Box2f { ctl_vec2f_t min; ctl_vec2f_t max; };" ) );
	types.back().moduleUsage[NULL].types.insert( "ctl_vec2f_t" );
}


////////////////////////////////////////


void
CCommonLanguage::getStandardMathBodies( FuncDeclList &d, const std::string &funcPref, const std::string &precSuffix )
{
	std::string argmattype33 = "ctl_mat33f_t ";
	std::string argmattype44 = "ctl_mat44f_t ";
	std::string argvectype3 = "ctl_vec3f_t ";
	std::string argvectype4 = "ctl_vec4f_t ";
	if ( supportsReferences() )
	{
		argmattype33 = "const ctl_mat33f_t &";
		argmattype44 = "const ctl_mat44f_t &";
		argvectype3 = "const ctl_vec3f_t &";
		argvectype4 = "const ctl_vec4f_t &";
	}
	
	d.push_back( FunctionDefinition( "pow10", funcPref + "ctl_number_t pow10( ctl_number_t y ) { return pow" + precSuffix + "( (ctl_number_t)(10.0), y ); }" ) );
	// not really CTL functions, but other routines use these
	d.push_back( FunctionDefinition( "_min_i", funcPref + "int _min_i( int a, int b ) { return a < b ? a : b; }" ) );
	d.push_back( FunctionDefinition( "_clamp", funcPref + "ctl_number_t _clamp( ctl_number_t v, ctl_number_t low, ctl_number_t high ) { return fmax" + precSuffix + "( low, fmin" + precSuffix + "( v, high ) ); }" ) );
	d.push_back( FunctionDefinition( "_lerp", funcPref + "ctl_number_t _lerp( ctl_number_t a, ctl_number_t b, ctl_number_t mix ) { return a + mix * ( b - a ); }" ) );
	d.push_back( FunctionDefinition( "_lerp_f3", funcPref + "ctl_vec3f_t _lerp_f3( " + argvectype3 + "a, " + argvectype3 + "b, ctl_number_t mix )\n"
"{\n"
"    ctl_vec3f_t r;\n"
"    r.x = _lerp( a.x, b.x, mix );\n"
"    r.y = _lerp( a.y, b.y, mix );\n"
"    r.z = _lerp( a.z, b.z, mix );\n"
"    return r;\n"
"}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );
	d.back().moduleUsage[NULL].functions.insert( "_lerp" );

	d.push_back( FunctionDefinition( "mult_f33_f33", funcPref +
"ctl_mat33f_t mult_f33_f33( " + argmattype33 + "a, " + argmattype33 + "b )\n"
"{\n"
"    ctl_mat33f_t r;\n"
"    ctl_number_t t[3];\n"
"    t[0] = a.vals[0][0];\n"
"    t[1] = a.vals[0][1];\n"
"    t[2] = a.vals[0][2];\n"
"    r.vals[0][0] = t[0] * b.vals[0][0] + t[1] * b.vals[1][0] + t[2] * b.vals[2][0];\n"
"    r.vals[0][1] = t[0] * b.vals[0][1] + t[1] * b.vals[1][1] + t[2] * b.vals[2][1];\n"
"    r.vals[0][2] = t[0] * b.vals[0][2] + t[1] * b.vals[1][2] + t[2] * b.vals[2][2];\n"
"    t[0] = a.vals[1][0];\n"
"    t[1] = a.vals[1][1];\n"
"    t[2] = a.vals[1][2];\n"
"    r.vals[1][0] = t[0] * b.vals[0][0] + t[1] * b.vals[1][0] + t[2] * b.vals[2][0];\n"
"    r.vals[1][1] = t[0] * b.vals[0][1] + t[1] * b.vals[1][1] + t[2] * b.vals[2][1];\n"
"    r.vals[1][2] = t[0] * b.vals[0][2] + t[1] * b.vals[1][2] + t[2] * b.vals[2][2];\n"
"    t[0] = a.vals[2][0];\n"
"    t[1] = a.vals[2][1];\n"
"    t[2] = a.vals[2][2];\n"
"    r.vals[2][0] = t[0] * b.vals[0][0] + t[1] * b.vals[1][0] + t[2] * b.vals[2][0];\n"
"    r.vals[2][1] = t[0] * b.vals[0][1] + t[1] * b.vals[1][1] + t[2] * b.vals[2][1];\n"
"    r.vals[2][2] = t[0] * b.vals[0][2] + t[1] * b.vals[1][2] + t[2] * b.vals[2][2];\n"
"    return r;\n}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_mat33f_t" );
		
	d.push_back( FunctionDefinition( "mult_f_f33", funcPref +
"ctl_mat33f_t mult_f_f33( ctl_number_t a, " + argmattype33 + "b )\n"
"{\n"
"    ctl_mat33f_t r;\n"
"    r.vals[0][0] = a * b.vals[0][0];\n"
"    r.vals[0][1] = a * b.vals[0][1];\n"
"    r.vals[0][2] = a * b.vals[0][2];\n"
"    r.vals[1][0] = a * b.vals[1][0];\n"
"    r.vals[1][1] = a * b.vals[1][1];\n"
"    r.vals[1][2] = a * b.vals[1][2];\n"
"    r.vals[2][0] = a * b.vals[2][0];\n"
"    r.vals[2][1] = a * b.vals[2][1];\n"
"    r.vals[2][2] = a * b.vals[2][2];\n"
"    return r;\n}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_mat33f_t" );
		
	d.push_back( FunctionDefinition( "add_f33_f33", funcPref +
"ctl_mat33f_t add_f33_f33( " + argmattype33 + "a, " + argmattype33 + "b )\n"
"{\n"
"    ctl_mat33f_t r;\n"
"    r.vals[0][0] = a.vals[0][0] * b.vals[0][0];\n"
"    r.vals[0][1] = a.vals[0][1] * b.vals[0][1];\n"
"    r.vals[0][2] = a.vals[0][2] * b.vals[0][2];\n"
"    r.vals[1][0] = a.vals[1][0] * b.vals[1][0];\n"
"    r.vals[1][1] = a.vals[1][1] * b.vals[1][1];\n"
"    r.vals[1][2] = a.vals[1][2] * b.vals[1][2];\n"
"    r.vals[2][0] = a.vals[2][0] * b.vals[2][0];\n"
"    r.vals[2][1] = a.vals[2][1] * b.vals[2][1];\n"
"    r.vals[2][2] = a.vals[2][2] * b.vals[2][2];\n"
"    return r;\n}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_mat33f_t" );

	d.push_back( FunctionDefinition( "invert_f33", funcPref +
"ctl_mat33f_t invert_f33( " + argmattype33 + "a )\n"
"{\n"
"    ctl_mat33f_t r;\n"
"    ctl_number_t det = a.vals[0][0] * a.vals[1][1] * a.vals[2][2] + a.vals[0][1] * a.vals[1][2] * a.vals[2][0] + a.vals[0][2] * a.vals[1][0] * a.vals[2][1] - a.vals[2][0] * a.vals[1][1] * a.vals[0][2] - a.vals[2][1] * a.vals[1][2] * a.vals[0][0] - a.vals[2][2] * a.vals[1][0] * a.vals[0][1];\n"
"    if ( fabs" + precSuffix + "( det ) > " + myStdNames["FLT_EPSILON"] + " )\n"
"    {\n"
"        det = ctl_number_t(1.0) / det;\n"
"        r.vals[0][0] = ( a.vals[1][1] * a.vals[2][2] - a.vals[1][2] * a.vals[2][1] ) * det;\n"
"        r.vals[0][1] = ( a.vals[0][2] * a.vals[2][1] - a.vals[0][1] * a.vals[2][2] ) * det;\n"
"        r.vals[0][2] = ( a.vals[0][1] * a.vals[1][2] - a.vals[0][2] * a.vals[1][1] ) * det;\n"
"        r.vals[1][0] = ( a.vals[1][2] * a.vals[2][0] - a.vals[1][0] * a.vals[2][2] ) * det;\n"
"        r.vals[1][1] = ( a.vals[0][0] * a.vals[2][2] - a.vals[0][2] * a.vals[2][0] ) * det;\n"
"        r.vals[1][2] = ( a.vals[0][2] * a.vals[1][0] - a.vals[0][0] * a.vals[1][2] ) * det;\n"
"        r.vals[2][0] = ( a.vals[1][0] * a.vals[2][1] - a.vals[1][1] * a.vals[2][0] ) * det;\n"
"        r.vals[2][1] = ( a.vals[0][1] * a.vals[2][0] - a.vals[0][0] * a.vals[2][1] ) * det;\n"
"        r.vals[2][2] = ( a.vals[0][0] * a.vals[1][1] - a.vals[0][1] * a.vals[1][0] ) * det;\n"
"    } else { \n"
"        assert( false );\n"
"        r.vals[0][0] = r.vals[1][1] = r.vals[2][2] = (ctl_number_t)(1.0);\n"
"        r.vals[0][1] = ctl_number_t(0);\n"
"        r.vals[0][2] = ctl_number_t(0);\n"
"        r.vals[1][0] = ctl_number_t(0);\n"
"        r.vals[1][2] = ctl_number_t(0);\n"
"        r.vals[2][0] = ctl_number_t(0);\n"
"        r.vals[2][1] = ctl_number_t(0);\n"
"    }\n"
"    return r;\n}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_mat33f_t" );
	d.back().moduleUsage[NULL].functions.insert( "assert" );

	d.push_back( FunctionDefinition( "transpose_f33", funcPref +
"ctl_mat33f_t transpose_f33( " + argmattype33 + "a )\n"
"{\n"
"    ctl_mat33f_t r;\n"
"    r.vals[0][0] = a.vals[0][0];\n"
"    r.vals[0][1] = a.vals[1][0];\n"
"    r.vals[0][2] = a.vals[2][0];\n"
"    r.vals[1][0] = a.vals[0][1];\n"
"    r.vals[1][1] = a.vals[1][1];\n"
"    r.vals[1][2] = a.vals[2][1];\n"
"    r.vals[2][0] = a.vals[0][2];\n"
"    r.vals[2][1] = a.vals[1][2];\n"
"    r.vals[2][2] = a.vals[2][2];\n"
"    return r;\n}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_mat33f_t" );

	d.push_back( FunctionDefinition( "mult_f3_f33", funcPref +
"ctl_vec3f_t mult_f3_f33( " + argvectype3 + "a, " + argmattype33 + "b )\n"
"{\n"
"    ctl_vec3f_t r;\n"
"    r.x = a.x * b.vals[0][0] + a.y * b.vals[1][0] + a.z * b.vals[2][0];\n"
"    r.y = a.x * b.vals[0][1] + a.y * b.vals[1][1] + a.z * b.vals[2][1];\n"
"    r.z = a.x * b.vals[0][2] + a.y * b.vals[1][2] + a.z * b.vals[2][2];\n"
"    return r;\n}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );
	d.back().moduleUsage[NULL].types.insert( "ctl_mat33f_t" );

	d.push_back( FunctionDefinition( "mult_f_f3", funcPref +
"ctl_vec3f_t mult_f_f3( ctl_number_t a, " + argvectype3 + "b )\n"
"{\n"
"    ctl_vec3f_t r;\n"
"    r.x = a * b.x;\n"
"    r.y = a * b.y;\n"
"    r.z = a * b.z;\n"
"    return r;\n}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );

	d.push_back( FunctionDefinition( "add_f3_f3", funcPref +
"ctl_vec3f_t add_f3_f3( " + argvectype3 + "a, " + argvectype3 + "b )\n"
"{\n"
"    ctl_vec3f_t r;\n"
"    r.x = a.x + b.x;\n"
"    r.y = a.y + b.y;\n"
"    r.z = a.z + b.z;\n"
"    return r;\n}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );

	d.push_back( FunctionDefinition( "sub_f3_f3", funcPref +
"ctl_vec3f_t sub_f3_f3( " + argvectype3 + "a, " + argvectype3 + "b )\n"
"{\n"
"    ctl_vec3f_t r;\n"
"    r.x = a.x - b.x;\n"
"    r.y = a.y - b.y;\n"
"    r.z = a.z - b.z;\n"
"    return r;\n}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );

	d.push_back( FunctionDefinition( "cross_f3_f3", funcPref +
"ctl_vec3f_t cross_f3_f3( " + argvectype3 + "a, " + argvectype3 + "b )\n"
"{\n"
"    ctl_vec3f_t r;\n"
"    r.x = a.y * b.z - a.z * b.y;\n"
"    r.y = a.z * b.x - a.x * b.z;\n"
"    r.z = a.x * b.y - a.y * b.x;\n"
"    return r;\n}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );

	d.push_back( FunctionDefinition( "dot_f3_f3", funcPref +
"ctl_number_t dot_f3_f3( " + argvectype3 + "a, " + argvectype3 + "b )\n"
"{\n"
"    return a.x * b.x + a.y * b.y + a.z * b.z;\n"
"}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );

	d.push_back( FunctionDefinition( "length_f3", funcPref +
"ctl_number_t length_f3( " + argvectype3 + "a )\n"
"{\n"
"    return sqrt" + precSuffix + "( a.x * a.x + a.y * a.y + a.z * a.z );\n"
"}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );

	d.push_back( FunctionDefinition( "mult_f44_f44", funcPref +
"ctl_mat44f_t mult_f44_f44( " + argmattype44 + "a, " + argmattype44 + "b )\n"
"{\n"
"    ctl_mat44f_t r;\n"
"    ctl_number_t t[4];\n"
"    int i, j;\n"
"    for ( i = 0; i < 4; ++i )\n"
"    {\n"
"        for ( j = 0; j < 4; ++j )\n"
"            t[j] = a.vals[i][j];\n"
"        for ( j = 0; j < 4; ++j )\n"
"            r.vals[i][j] = t[0] * b.vals[0][j] + t[1] * b.vals[1][j] + t[2] * b.vals[2][j] + t[3] * b.vals[3][j];\n"
"    }\n"
"    return r;\n"
"}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_mat44f_t" );

	d.push_back( FunctionDefinition( "mult_f_f44", funcPref +
"ctl_mat44f_t mult_f_f44( ctl_number_t a, " + argmattype44 + "b )\n"
"{\n"
"    ctl_mat44f_t r;\n"
"    int i, j;\n"
"    for ( i = 0; i < 4; ++i )\n"
"        for ( j = 0; j < 4; ++j )\n"
"            r.vals[i][j] = a * b.vals[i][j];\n"
"    return r;\n"
"}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_mat44f_t" );

	d.push_back( FunctionDefinition( "add_f44_f44", funcPref +
"ctl_mat44f_t add_f44_f44( " + argmattype44 + "a, " + argmattype44 + "b )\n"
"{\n"
"    ctl_mat44f_t r;\n"
"    int i, j;\n"
"    for ( i = 0; i < 4; ++i )\n"
"        for ( j = 0; j < 4; ++j )\n"
"            r.vals[i][j] = a.vals[i][j] + b.vals[i][j];\n"
"    return r;\n"
"}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_mat44f_t" );

	// gauss-jordan form
	d.push_back( FunctionDefinition( "invert_f44", funcPref +
"ctl_mat44f_t invert_f44( " + argmattype44 + "a )\n"
"{\n"
"    ctl_mat44f_t s, t;\n"
"    int i, j, k;\n"
"    for ( i = 0; i < 4; ++i )\n"
"        for ( j = 0; j < 4; ++j )\n"
"            t.vals[i][j] = a.vals[i][j];\n"
"    for ( i = 0; i < 3 ; ++i )\n"
"    {\n"
"        int pivot = i;\n"
"        ctl_number_t pivotsize = fabs" + precSuffix + "( t.vals[i][i] );\n"
"        for ( j = i + 1; j < 4; ++j )\n"
"        {\n"
"            ctl_number_t tmp = fabs" + precSuffix + "( t.vals[j][i] );\n"
"            if ( tmp > pivotsize )\n"
"            {\n"
"                pivot = j;\n"
"                pivotsize = tmp;\n"
"            }\n"
"        }\n"
"        if ( pivotsize == 0 )\n"
"        {\n"
"            assert( false );\n" // not invertible
"            return a;\n"
"        }\n"
"        if ( pivot != i )\n"
"        {\n"
"            for ( j = 0; j < 4; ++j )\n"
"            {\n"
"                ctl_number_t tmp;\n"
"                tmp = t.vals[i][j];\n"
"                t.vals[i][j] = t.vals[pivot][j];\n"
"                t.vals[pivot][j] = tmp;\n"
"                tmp = s.vals[i][j];\n"
"                s.vals[i][j] = s.vals[pivot][j];\n"
"                s.vals[pivot][j] = tmp;\n"
"            }\n"
"        }\n"
"\n"
"        for ( j = i + 1; j < 4; ++j )\n"
"        {\n"
"            ctl_number_t f = t.vals[j][i] / t.vals[i][i];\n"
"            for ( k = 0; k < 4; ++k )\n"
"            {\n"
"                t.vals[j][k] -= f * t.vals[i][k];\n"
"                s.vals[j][k] -= f * s.vals[i][k];\n"
"            }\n"
"        }\n"
"    }\n"
"    for ( i = 3; i >= 0; --i )\n" // backward subst
"    {\n"
"        ctl_number_t f = t.vals[i][i];\n"
"        if ( f == 0 )\n"
"        {\n"
"            assert( false );\n" // not invertible
"            return a;\n"
"        }\n"
"        for ( j = 0; j < 4; ++j )\n"
"        {\n"
"            t.vals[i][j] /= f;\n"
"            s.vals[i][j] /= f;\n"
"        }\n"
"        for ( j = 0; j < i; ++j )\n"
"        {\n"
"            f = t.vals[j][i];\n"
"            for ( k = 0; k < 4; ++k )\n"
"            {\n"
"                t.vals[j][k] -= f * t.vals[i][k];\n"
"                s.vals[j][k] -= f * s.vals[i][k];\n"
"            }\n"
"        }\n"
"    }\n"
"    return s;\n"
"}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_mat44f_t" );
	d.back().moduleUsage[NULL].functions.insert( "assert" );

	d.push_back( FunctionDefinition( "transpose_f44", funcPref +
"ctl_mat44f_t transpose_f44( " + argmattype44 + "a )\n"
"{\n"
"    ctl_mat44f_t r;\n"
"    int i, j;\n"
"    for ( i = 0; i < 4; ++i )\n"
"        for ( j = 0; j < 4; ++j )\n"
"            r.vals[j][i] = a.vals[i][j];\n"
"    return r;\n"
"}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_mat44f_t" );

	d.push_back( FunctionDefinition( "mult_f3_f44", funcPref +
"ctl_vec3f_t mult_f3_f44( " + argvectype3 + "a, " + argmattype44 + "b )\n"
"{\n"
"    ctl_vec3f_t r;\n"
"    ctl_number_t w;\n"
"    r.x = a.x * b.vals[0][0] + a.y * b.vals[1][0] + a.z * b.vals[2][0] + b.vals[3][0];\n"
"    r.y = a.x * b.vals[0][1] + a.y * b.vals[1][1] + a.z * b.vals[2][1] + b.vals[3][1];\n"
"    r.z = a.x * b.vals[0][2] + a.y * b.vals[1][2] + a.z * b.vals[2][2] + b.vals[3][2];\n"
"    w = a.x * b.vals[0][3] + a.y * b.vals[1][3] + a.z * b.vals[2][3] + b.vals[3][3];\n"
"    r.x /= w;\n"
"    r.y /= w;\n"
"    r.z /= w;\n"
"    return r;\n"
"}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );
	d.back().moduleUsage[NULL].types.insert( "ctl_mat44f_t" );
}


////////////////////////////////////////


void
CCommonLanguage::getStandardHalfBodies( FuncDeclList &d, const std::string &funcPref, const std::string &precSuffix )
{
	if ( ! supportsHalfType() )
		return;

	d.push_back( FunctionDefinition( "exp_h", funcPref + "half exp_h( ctl_number_t v ) { return half( float( exp" + precSuffix + "( v ) ) ); }" ) );
	d.push_back( FunctionDefinition( "log_h", funcPref + "ctl_number_t log_h( half v ) { return log" + precSuffix + "( float( v ) ); }" ) );
	d.push_back( FunctionDefinition( "log10_h", funcPref + "ctl_number_t log10_h( half v ) { return log10" + precSuffix + "( float( v ) ); }" ) );
	d.push_back( FunctionDefinition( "pow_h", funcPref + "half pow_h( half x, ctl_number_t y ) { return half( float( pow" + precSuffix + "( float( x ), y ) ) ); }" ) );
	d.push_back( FunctionDefinition( "pow10_h", funcPref + "half pow10_h( ctl_number_t y ) { return half( float( pow" + precSuffix + "( 10.0, y ) ) ); }" ) );

	d.push_back( FunctionDefinition( "isfinite_h", funcPref + "bool isfinite_h( half v ) { return v.isFinite(); }" ) );
	d.push_back( FunctionDefinition( "isnormal_h", funcPref + "bool isnormal_h( half v ) { return v.isNormalized(); }" ) );
	d.push_back( FunctionDefinition( "isnan_h", funcPref + "bool isnan_h( half v ) { return v.isNan(); }" ) );
	d.push_back( FunctionDefinition( "isinf_h", funcPref + "bool isinf_h( half v ) { return v.isInfinity(); }" ) );
}


////////////////////////////////////////


void
CCommonLanguage::getStandardPrintBodies( FuncDeclList &d, const std::string &funcPref, const std::string &precSuffix )
{
	d.push_back( FunctionDefinition( "assert", funcPref + "void assert( bool v ) {}" ) );
	d.push_back( FunctionDefinition( "print_bool", funcPref + "void print_bool( bool v ) {}" ) );
	d.push_back( FunctionDefinition( "print_int", funcPref + "void print_int( int v ) {}" ) );
	d.push_back( FunctionDefinition( "print_unsigned_int", funcPref + "void print_unsigned_int( unsigned int v ) {}" ) );
	if ( supportsHalfType() )
		d.push_back( FunctionDefinition( "print_half", funcPref + "void print_half( half v ) {}" ) );
	d.push_back( FunctionDefinition( "print_float", funcPref + "void print_float( ctl_number_t v ) {}" ) );
	d.push_back( FunctionDefinition( "print_string", funcPref + "void print_string( const char *v ) {}" ) );
}


////////////////////////////////////////


void
CCommonLanguage::getStandardColorBodies( FuncDeclList &d, const std::string &funcPref, const std::string &precSuffix )
{
	std::string argvectype3 = "ctl_vec3f_t ";
	std::string argchrom = "Chromaticities ";
	std::string argmat44 = "ctl_mat44f_t ";
	if ( supportsReferences() )
	{
		argvectype3 = "const ctl_vec3f_t &";
		argchrom = "const Chromaticities &";
		argmat44 = "const ctl_mat44f_t &";
	}

	// These are a straight crib from IlmCtlSimd for the luv routines
	// to try to retain/ compatibility, but if we have higher
	// precision data types should we do more?
	d.push_back( FunctionDefinition( "_cspace_f", funcPref + "ctl_number_t _cspace_f( ctl_number_t x ) { if ( x > ctl_number_t( 0.008856 ) ) return pow" + precSuffix + "( x, ctl_number_t(1.0/3.0) ); return ctl_number_t(7.787) * x + ctl_number_t(16.0/116.0); }" ) );
	d.push_back( FunctionDefinition( "_cspace_fInverse", funcPref + "ctl_number_t _cspace_fInverse( ctl_number_t t ) { if ( t > ctl_number_t(0.206892) ) return t * t * t; return ctl_number_t(1.0/7.787) * ( t - ctl_number_t(16.0/116.0) ); }" ) );
	d.push_back( FunctionDefinition( "_cspace_uprime", funcPref + "ctl_number_t _cspace_uprime( " + argvectype3 +"XYZ ) { return ( XYZ.x * ctl_number_t(4) ) / ( XYZ.x + ctl_number_t(15) * XYZ.y + ctl_number_t(3) * XYZ.z ); }" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );
	d.push_back( FunctionDefinition( "_cspace_vprime", funcPref + "ctl_number_t _cspace_vprime( " + argvectype3 +"XYZ ) { return ( XYZ.y * ctl_number_t(9) ) / ( XYZ.x + ctl_number_t(15) * XYZ.y + ctl_number_t(3) * XYZ.z ); }" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );

	d.push_back( FunctionDefinition( "RGBtoXYZ", funcPref +
"ctl_mat44f_t RGBtoXYZ( " + argchrom + "chroma, ctl_number_t Y )\n"
"{\n"
"    const ctl_number_t one = ctl_number_t(1);\n"
"    ctl_number_t X = chroma.white.x * Y / chroma.white.y;\n"
"    ctl_number_t Z = (one - chroma.white.x - chroma.white.y) * Y / chroma.white.y;\n"
"    ctl_number_t d = chroma.red.x * (chroma.blue.y - chroma.green.y) + chroma.blue.x * (chroma.green.y - chroma.red.y) + chroma.green.x * (chroma.red.y - chroma.blue.y);\n"
"    ctl_number_t Sr = (X * (chroma.blue.y - chroma.green.y) - chroma.green.x * (Y * (chroma.blue.y - one) + chroma.blue.y * (X + Z)) + chroma.blue.x * (Y * (chroma.green.y - one) + chroma.green.y * (X + Z))) / d;\n"
"    ctl_number_t Sg = (X * (chroma.red.y - chroma.blue.y) + chroma.red.x * (Y * (chroma.blue.y - one) + chroma.blue.y * (X + Z)) - chroma.blue.x * (Y * (chroma.red.y - one) + chroma.red.y * (X + Z))) / d;\n"
"    ctl_number_t Sb = (X * (chroma.green.y - chroma.red.y) - chroma.red.x * (Y * (chroma.green.y - one) + chroma.green.y * (X + Z)) + chroma.green.x * (Y * (chroma.red.y - one) + chroma.red.y * (X + Z))) / d;\n"
"    ctl_mat44f_t M;\n"
"    M.vals[0][0] = Sr * chroma.red.x;\n"
"    M.vals[0][1] = Sr * chroma.red.y;\n"
"    M.vals[0][2] = Sr * (1 - chroma.red.x - chroma.red.y);\n"
"    M.vals[1][0] = Sg * chroma.green.x;\n"
"    M.vals[1][1] = Sg * chroma.green.y;\n"
"    M.vals[1][2] = Sg * (1 - chroma.green.x - chroma.green.y);\n"
"    M.vals[2][0] = Sb * chroma.blue.x;\n"
"    M.vals[2][1] = Sb * chroma.blue.y;\n"
"    M.vals[2][2] = Sb * (1 - chroma.blue.x - chroma.blue.y);\n"
"    return M;\n"
"}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_mat44f_t" );
	d.back().moduleUsage[NULL].types.insert( "Chromaticities" );

	d.push_back( FunctionDefinition( "XYZtoRGB", funcPref +
"ctl_mat44f_t XYZtoRGB( " + argchrom + "chroma, ctl_number_t Y ) { return invert_f44( RGBtoXYZ( chroma, Y ) ); }" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_mat44f_t" );
	d.back().moduleUsage[NULL].types.insert( "Chromaticities" );
	d.back().moduleUsage[NULL].functions.insert( "invert_f44" );
	d.back().moduleUsage[NULL].functions.insert( "RGBtoXYZ" );

	d.push_back( FunctionDefinition( "XYZtoLuv", funcPref +
"ctl_vec3f_t XYZtoLuv( " + argvectype3 + "XYZ, " + argvectype3 + "XYZn )\n"
"{\n"
"    ctl_vec3f_t r;\n"
"    r.x = ctl_number_t(116) * _cspace_f( XYZ.y / XYZn.y ) - ctl_number_t(16);\n"
"    r.y = ctl_number_t(13) * r.x * ( _cspace_uprime( XYZ ) - _cspace_uprime( XYZn ) );\n"
"    r.z = ctl_number_t(13) * r.x * ( _cspace_vprime( XYZ ) - _cspace_vprime( XYZn ) );\n"
"    return r;\n"
"}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );
	d.back().moduleUsage[NULL].functions.insert( "_cspace_f" );
	d.back().moduleUsage[NULL].functions.insert( "_cspace_uprime" );
	d.back().moduleUsage[NULL].functions.insert( "_cspace_vprime" );

	d.push_back( FunctionDefinition( "LuvtoXYZ", funcPref +
"ctl_vec3f_t LuvtoXYZ( " + argvectype3 + "Luv, " + argvectype3 + "XYZn )\n"
"{\n"
"    ctl_vec3f_t r;\n"
"    ctl_number_t unprime = _cspace_uprime( XYZn );\n"
"    ctl_number_t vnprime = _cspace_vprime( XYZn );\n"
"    ctl_number_t fY = (Luv.x + ctl_number_t(16)) / ctl_number_t(116);\n"
"    r.y = XYZn.y * _cspace_fInverse( fY );\n"
"    ctl_number_t d = ctl_number_t(4) * (ctl_number_t(13) * Luv.x * vnprime + Luv.z);\n"
"    r.x = ctl_number_t(9) * (ctl_number_t(13) * Luv.x * unprime + Luv.y) * r.y / d;\n"
"    r.z = -( ctl_number_t(3) * Luv.y + ctl_number_t(13) * Luv.x * ( ctl_number_t(-12) + ctl_number_t(3) * unprime + ctl_number_t(20) * vnprime ) + ctl_number_t(20) * Luv.z ) * r.y / d;\n"
"    return r;\n"
"}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );
	d.back().moduleUsage[NULL].functions.insert( "_cspace_fInverse" );
	d.back().moduleUsage[NULL].functions.insert( "_cspace_uprime" );
	d.back().moduleUsage[NULL].functions.insert( "_cspace_vprime" );
	
	d.push_back( FunctionDefinition( "XYZtoLab", funcPref +
"ctl_vec3f_t XYZtoLab( " + argvectype3 + "XYZ, " + argvectype3 + "XYZn )\n"
"{\n"
"    ctl_vec3f_t r;\n"
"    ctl_number_t tmpY = _cspace_f( XYZ.y / XYZn.y );\n"
"    r.x = ctl_number_t(116) * tmpY - ctl_number_t(16);\n"
"    r.y = ctl_number_t(500) * ( _cspace_f( XYZ.x / XYZn.x ) -  tmpY );\n"
"    r.z = ctl_number_t(200) * ( tmpY - _cspace_f( XYZ.z / XYZn.z ) );\n"
"    return r;\n"
"}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );
	d.back().moduleUsage[NULL].functions.insert( "_cspace_f" );

	d.push_back( FunctionDefinition( "LabtoXYZ", funcPref +
"ctl_vec3f_t LabtoXYZ( " + argvectype3 + "Lab, " + argvectype3 + "XYZn )\n"
"{\n"
"    ctl_vec3f_t r;\n"
"    ctl_number_t fY = (Lab.x + ctl_number_t(16)) / ctl_number_t(116);\n"
"    ctl_number_t fX = Lab.y / ctl_number_t(500) + fY;\n"
"    ctl_number_t fZ = fY - Lab.z / ctl_number_t(200);\n"
"    r.x = XYZn.x * _cspace_fInverse( fX );\n"
"    r.y = XYZn.y * _cspace_fInverse( fY );\n"
"    r.z = XYZn.z * _cspace_fInverse( fZ );\n"
"    return r;\n"
"}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );
	d.back().moduleUsage[NULL].functions.insert( "_cspace_fInverse" );
}


////////////////////////////////////////


void
CCommonLanguage::getStandardInterpBodies( FuncDeclList &d, const std::string &funcPref, const std::string &precSuffix )
{
	std::string argvectype3 = "ctl_vec3f_t ";
	std::string argvectype3i = "ctl_vec3i_t ";
	if ( supportsReferences() )
	{
		argvectype3 = "const ctl_vec3f_t &";
		argvectype3i = "const ctl_vec3i_t &";
	}

	d.push_back( FunctionDefinition( "lookup1D", funcPref +
"ctl_number_t lookup1D( const ctl_number_t table[], int size, const ctl_number_t pMin, const ctl_number_t pMax, ctl_number_t p )\n"
"{\n"
"    int iMax = size - 1;\n"
"    ctl_number_t r = ( _clamp( p, pMin, pMax ) - pMin ) / ( pMax - pMin ) * iMax;\n"
"    int i = (int)( r );\n"
"    ctl_number_t u = r - (ctl_number_t)( i );\n"
"    ctl_number_t t0 = table[i];\n"
"    ctl_number_t t1 = table[_min_i( i + 1, iMax )];\n"
"    return _lerp( t0, t1, u );\n"
"}" ) );
	d.back().moduleUsage[NULL].functions.insert( "_clamp" );
	d.back().moduleUsage[NULL].functions.insert( "_min_i" );
	d.back().moduleUsage[NULL].functions.insert( "_lerp" );

	d.push_back( FunctionDefinition( "lookupCubic1D", funcPref +
"ctl_number_t lookup1D( ctl_number_t table[], int size, ctl_number_t pMin, ctl_number_t pMax, ctl_number_t p )\n"
"{\n"
"    if ( size < 3 ) return lookup1D( table, size, pMin, pMax, p );\n"
"    int iMax = size - 1;\n"
"    ctl_number_t r = ( _clamp( p, pMin, pMax ) - pMin ) / ( pMax - pMin ) * iMax;\n"
"    if ( r >= iMax ) return table[iMax];\n"
"    int i = (int)( r );\n"
"    const ctl_number_t kHalf = (ctl_number_t)(0.5);\n"
"    const ctl_number_t kOne = (ctl_number_t)(1);\n"
"    const ctl_number_t kTwo = (ctl_number_t)(2);\n"
"    const ctl_number_t kThree = (ctl_number_t)(3);\n"
"    ctl_number_t dy = ( table[i+1] - table[i] );\n"
"    ctl_number_t m0, m1;\n"
"    if ( i < (iMax - 1) )\n"
"    {\n"
"        m1 = ( dy + ( table[i+2] - table[i+1] ) ) * kHalf;\n"
"        if ( i > 0 )\n"
"            m0 = ( dy + ( table[i] - table[i-1] ) ) * kHalf;\n"
"        else\n"
"            m0 = ( kThree * dy - m1 ) * kHalf;\n"
"    }\n"
"    else\n"
"    {\n"
"        m0 = ( dy + ( table[i] - table[i-1] ) ) * kHalf;\n"
"        m1 = ( kThree * dy - m0 ) * kHalf;\n"
"    }\n"
"    ctl_number_t t = r - (ctl_number_t)( i );\n"
"    ctl_number_t t2 = t * t;\n"
"    ctl_number_t t3 = t2 * t;\n"
"    return ( table[i] * (kTwo * t3 - kThree * t2 + kOne) +\n"
"             m0 * ( t3 - kTwo * t2 + t ) +\n"
"             table[i+1] * ( kThree * t2 - kTwo * t3 ) +\n"
"             m1 * ( t3 - t2 ) );\n"
"}" ) );
	d.back().moduleUsage[NULL].functions.insert( "_clamp" );
	d.back().moduleUsage[NULL].functions.insert( "lookup1D" );

	d.push_back( FunctionDefinition( "lookup3D_f3", funcPref +
"ctl_vec3f_t lookup3D_f3( const ctl_number_t table[], " + argvectype3i + "size, " + argvectype3 + "pMin, " + argvectype3 + "pMax, " + argvectype3 + "p )\n"
"{\n"
"    int iMax = size.x - 1;\n"
"    ctl_number_t r = ( _clamp( p.x, pMin.x, pMax.x ) - pMin.x ) / ( pMax.x - pMin.x ) * iMax;\n"
"    int jMax = size.y - 1;\n"
"    ctl_number_t s = ( _clamp( p.y, pMin.y, pMax.y ) - pMin.y ) / ( pMax.y - pMin.y ) * jMax;\n"
"    int kMax = size.z - 1;\n"
"    ctl_number_t t = ( _clamp( p.z, pMin.z, pMax.z ) - pMin.z ) / ( pMax.z - pMin.z ) * kMax;\n"
"    int i = (int)( r );\n"
"    int i1 = _min_i( i + 1, iMax );\n"
"    ctl_number_t u = r - (ctl_number_t)( i );\n"
"    int j = (int)( s );\n"
"    int j1 = _min_i( j + 1, jMax );\n"
"    ctl_number_t v = s - (ctl_number_t)( j );\n"
"    int k = (int)( t );\n"
"    int k1 = _min_i( k + 1, kMax );\n"
"    ctl_number_t w = t - (ctl_number_t)( k );\n"
"    const ctl_number_t *aP = table + (( i * size.y + j ) * size.z + k)*3;\n"
"    const ctl_number_t *bP = table + (( i1 * size.y + j ) * size.z + k)*3;\n"
"    const ctl_number_t *cP = table + (( i * size.y + j1 ) * size.z + k)*3;\n"
"    const ctl_number_t *dP = table + (( i1 * size.y + j1 ) * size.z + k)*3;\n"
"    const ctl_number_t *eP = table + (( i * size.y + j ) * size.z + k1)*3;\n"
"    const ctl_number_t *fP = table + (( i1 * size.y + j ) * size.z + k1)*3;\n"
"    const ctl_number_t *gP = table + (( i * size.y + j1 ) * size.z + k1)*3;\n"
"    const ctl_number_t *hP = table + (( i1 * size.y + j1 ) * size.z + k1)*3;\n"
"    const ctl_vec3f_t a = make_vec3f( aP[0], aP[1], aP[2] );\n"
"    const ctl_vec3f_t b = make_vec3f( bP[0], bP[1], bP[2] );\n"
"    const ctl_vec3f_t c = make_vec3f( cP[0], cP[1], cP[2] );\n"
"    const ctl_vec3f_t d = make_vec3f( dP[0], dP[1], dP[2] );\n"
"    const ctl_vec3f_t e = make_vec3f( eP[0], eP[1], eP[2] );\n"
"    const ctl_vec3f_t f = make_vec3f( fP[0], fP[1], fP[2] );\n"
"    const ctl_vec3f_t g = make_vec3f( gP[0], gP[1], gP[2] );\n"
"    const ctl_vec3f_t h = make_vec3f( hP[0], hP[1], hP[2] );\n"
"    return _lerp_f3( _lerp_f3( _lerp_f3( a, b, u ), _lerp_f3( c, d, u ), v ),\n"
"                     _lerp_f3( _lerp_f3( e, f, u ), _lerp_f3( g, h, u ), v ), w );\n"
"}" ) );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3i_t" );
	d.back().moduleUsage[NULL].functions.insert( "_clamp" );
	d.back().moduleUsage[NULL].functions.insert( "_min_i" );
	d.back().moduleUsage[NULL].functions.insert( "_lerp_f3" );

	if ( supportsReferences() )
	{
		d.push_back( FunctionDefinition( "lookup3D_f", funcPref + "void lookup3D_f( const ctl_number_t table[], const ctl_vec3i_t &size, const ctl_vec3f_t &pMin, const ctl_vec3f_t &pMax, ctl_number_t p0, ctl_number_t p1, ctl_number_t p2, ctl_number_t &o0, ctl_number_t &o1, ctl_number_t &o2 )\n"
"{\n"
"    ctl_vec3f_t out = lookup3D_f3( table, size, pMin, pMax, make_vec3f( p0, p1, p2 ) );\n"
"    o0 = out.x;\n"
"    o1 = out.y;\n"
"    o2 = out.z;\n"
"}" ) );
	}
	else
	{
		d.push_back( FunctionDefinition( "lookup3D_f", funcPref + "void lookup3D_f( const ctl_number_t table[], ctl_vec3i_t size, ctl_vec3f_t pMin, ctl_vec3f_t pMax, ctl_number_t p0, ctl_number_t p1, ctl_number_t p2, ctl_number_t *o0, ctl_number_t *o1, ctl_number_t *o2 )\n"
"{\n"
"    ctl_vec3f_t out = lookup3D_f3( table, size, pMin, pMax, make_vec3f( p0, p1, p2 ) );\n"
"    *o0 = out.x;\n"
"    *o1 = out.y;\n"
"    *o2 = out.z;\n"
"}" ) );
	}
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );
	d.back().moduleUsage[NULL].types.insert( "ctl_vec3i_t" );
	d.back().moduleUsage[NULL].functions.insert( "lookup3D_f3" );

	if ( supportsHalfType() )
	{
		if ( ! supportsReferences() )
			throw std::logic_error( "Support half type but not references???" );

		d.push_back( FunctionDefinition( "lookup3D_h", funcPref + "void lookup3D_h( const ctl_number_t table[], const ctl_vec3i_t &size, const ctl_vec3f_t &pMin, const ctl_vec3f_t &pMax, const half &p0, const half &p1, const half &p2, half &o0, half &o1, half &o2 )\n"
"{\n"
"    ctl_vec3f_t out = lookup3D_f3( table, size, pMin, pMax, make_vec3f( p0, p1, p2 ) );\n"
"    o0 = out.x;\n"
"    o1 = out.y;\n"
"    o2 = out.z;\n"
"}" ) );
		d.back().moduleUsage[NULL].types.insert( "ctl_vec3f_t" );
		d.back().moduleUsage[NULL].types.insert( "ctl_vec3i_t" );
		d.back().moduleUsage[NULL].functions.insert( "lookup3D_f3" );
	}

	d.push_back( FunctionDefinition( "interpolate1D", funcPref +
"ctl_number_t interpolate1D( const ctl_number_t table[][2], int size, ctl_number_t p )\n"
"{\n"
"    if ( size < 1 ) return (ctl_number_t)(0);\n"
"    if ( p < table[0][0] ) return table[0][1];\n"
"    if ( p >= table[size - 1][0] ) return table[size - 1][1];\n"
"    int i = 0;\n"
"    int j = size;\n"
"    while ( i < j - 1 )\n"
"    {\n"
"        int k = ( i + j ) / 2;\n"
"        if ( table[k][0] == p ) return table[k][1];\n"
"        else if ( table[k][0] < p ) i = k;\n"
"        else j = k;\n"
"    }\n"
"    ctl_number_t t = ( p - table[i][0] ) / ( table[i + 1][0] - table[i][0] );\n"
"    return _lerp( table[i][1], table[i + 1][1], t );\n"
"}" ) );
	d.back().moduleUsage[NULL].functions.insert( "_lerp" );

	d.push_back( FunctionDefinition( "interpolateCubic1D", funcPref +
"ctl_number_t interpolateCubic1D( const ctl_number_t table[][2], int size, ctl_number_t p )\n"
"{\n"
"    if ( size < 3 ) return interpolate1D( table, size, p );\n"
"    if ( p < table[0][0] ) return table[0][1];\n"
"    if ( p >= table[size - 1][0] ) return table[size - 1][1];\n"
"    int i = 0;\n"
"    int j = size;\n"
"    while ( i < j - 1 )\n"
"    {\n"
"        int k = ( i + j ) / 2;\n"
"        if ( table[k][0] == p ) return table[k][1];\n"
"        else if ( table[k][0] < p ) i = k;\n"
"        else j = k;\n"
"    }\n"
"    const ctl_number_t kHalf = ctl_number_t(0.5);\n"
"    const ctl_number_t kOne = ctl_number_t(1);\n"
"    const ctl_number_t kTwo = ctl_number_t(2);\n"
"    const ctl_number_t kThree = ctl_number_t(3);\n"
"    ctl_number_t dx = ( table[i+1][0] - table[i][0] );\n"
"    ctl_number_t dy = ( table[i+1][1] - table[i][1] );\n"
"    ctl_number_t m0, m1;\n"
"    if ( i > 0 ) m0 = kHalf * ( dy + dx * ( table[i][1] - table[i-1][1] ) / (table[i][0] - table[i-1][0]) );\n"
"    if ( i < (size - 2) ) m1 = kHalf * ( dy + dx * (table[i+2][1] - table[i+1][1]) / (table[i+2][0] - table[i+1][0]) );\n"
"    if ( i <= 0 ) m0 = kHalf * ( kThree * dy - m1 );\n"
"    if ( i >= (size - 2) ) m1 = kHalf * ( kThree * dy - m0 );\n"
"    ctl_number_t t = ( p - table[i][0] ) / dx;\n"
"    ctl_number_t t2 = t * t;\n"
"    ctl_number_t t3 = t2 * t;\n"
"    return ( table[i][1] * (kTwo * t3 - kThree * t2 + kOne) +\n"
"             m0 * ( t3 - kTwo * t2 + t ) +\n"
"             table[i+1][1] * ( kThree * t2 - kTwo * t3 ) +\n"
"             m1 * ( t3 - t2 ) );\n"
"}" ) );
	d.back().moduleUsage[NULL].functions.insert( "interpolate1D" );

}


////////////////////////////////////////


std::string
CCommonLanguage::getHeaderCode( bool calledOnly )
{
	std::stringstream retval;
	pushStream( retval );

	curStream() << beginComment() << " Auto generated function definitions" << endComment() << "\n";
	newlineAndIndent();
	curStream() << "typedef " << getPrecisionType() << " ctl_number_t;\n";
	newlineAndIndent();
	for ( size_t m = 0; m != myModules.size(); ++m )
	{
		std::string modFuncHeaders;
		const FuncDeclList &funcs = myModules[m].functions;
		for ( size_t f = 0; f != funcs.size(); ++f )
		{
			const FunctionDefinition &func = funcs[f];
			if ( ! func.header.empty() )
			{
				if ( ! modFuncHeaders.empty() )
					modFuncHeaders.push_back( '\n' );
				modFuncHeaders.append( func.header );
			}
		}

		bool needInit = false;
		if ( ! supportsModuleDynamicInitialization() )
		{
			const std::vector<GlobalVariableDefinition> &vars = myModules[m].variables;
			for ( size_t v = 0; v != vars.size(); ++v )
			{
				const GlobalVariableDefinition &var = vars[v];
				if ( ! var.delayedInit.empty() )
				{
					needInit = true;
					break;
				}
			}
		}

		if ( ! modFuncHeaders.empty() )
		{
			if ( supportsNamespaces() )
			{
				curStream() << "namespace " << myCurModuleName << " {";
				newlineAndIndent();
				if ( needInit )
				{
					curStream() << beginComment() << "Initialization routine must be called prior to usage or undefined results will occur" << endComment();
					newlineAndIndent();
					curStream() << "void init( void );";
					newlineAndIndent();
				}
			}
			else
			{
				if ( needInit )
				{
					newlineAndIndent();
					curStream() << beginComment() << "Initialization routine must be called prior to usage or undefined results will occur" << endComment();
					newlineAndIndent();
					curStream() << "void " << myCurModuleName << "_init( void );";
					newlineAndIndent();
				}
			}
			curStream() << modFuncHeaders;

			if ( supportsNamespaces() )
			{
				newlineAndIndent();
				curStream() << "} " << beginComment() << " namespace " << myCurModuleName << endComment();
			}
		}
	}
	// for all modules
	// if module has functions w/ header info

	popStream();
	return retval.str();
}


////////////////////////////////////////


std::string
CCommonLanguage::getCode( bool calledOnly )
{
	std::stringstream retval;
	pushStream( retval );

	if ( calledOnly )
	{
		std::map<const Module *, size_t> indexmap;
		const MainRoutineMap &mains = getMainRoutines();
		std::vector< std::pair<ModuleDefinition *, FunctionDefinition *> > theMains;
		for ( size_t md = 0, nMD = myModules.size(); md != nMD; ++md )
		{
			ModuleDefinition &curMod = myModules[md];
			if ( curMod.typeCode )
			{
				delete curMod.typeCode;
				curMod.typeCode = NULL;
			}
			if ( curMod.fwdCode )
			{
				delete curMod.fwdCode;
				curMod.fwdCode = NULL;
			}
			if ( curMod.varCode )
			{
				delete curMod.varCode;
				curMod.varCode = NULL;
			}
			if ( curMod.funCode )
			{
				delete curMod.funCode;
				curMod.funCode = NULL;
			}
			curMod.initCode.clear();
			indexmap[curMod.module] = md;
			for ( size_t t = 0, nT = curMod.types.size(); t != nT; ++t )
				curMod.types[t].emit = false;
			for ( size_t v = 0, nV = curMod.variables.size(); v != nV; ++v )
				curMod.variables[v].emit = false;
			for ( size_t f = 0, nF = curMod.functions.size(); f != nF; ++f )
			{
				curMod.functions[f].emit = false;
				curMod.functions[f].emitForward = false;
				MainRoutineMap::const_iterator mf = mains.begin();
				while ( mf != mains.end() )
				{
					if ( mf->first == curMod.functions[f].name )
						theMains.push_back( std::make_pair( &curMod, &( curMod.functions[f] ) ) );
					++mf;
				}
			}
		}

		size_t found = theMains.size();
		size_t defined = mains.size();
		if ( found != defined )
		{
			if ( found > defined )
				throw std::logic_error( "Found more main routines than were defined" );
			else
				throw std::logic_error( "Unable to find all main routine definitions" );
		}
		for ( size_t mf = 0; mf != found; ++mf )
		{
			ModuleDefinition *curMod = theMains[mf].first;
			FunctionDefinition *curFunc = theMains[mf].second;
			if ( ! curMod->funCode )
				curMod->funCode = new std::stringstream;
			pushStream( *(curMod->funCode) );
			traverseAndEmit( indexmap, *curFunc );
			popStream();
		}

		for ( size_t md = 0, nMD = myModules.size(); md != nMD; ++md )
		{
			ModuleDefinition &curMod = myModules[md];
			std::string tC, vC, fC, fnC;
			if ( curMod.typeCode )
				tC = curMod.typeCode->str();
			if ( curMod.varCode )
				vC = curMod.varCode->str();
			if ( curMod.fwdCode )
				fC = curMod.fwdCode->str();
			if ( curMod.funCode )
				fnC = curMod.funCode->str();
			if ( tC.empty() && vC.empty() && fC.empty() && fnC.empty() )
				continue;

			if ( ! curMod.prefix.empty() )
			{
				newlineAndIndent();
				curStream() << curMod.prefix;
			}
			if ( ! tC.empty() )
			{
				newlineAndIndent();
				curStream() << tC;
			}
			if ( ! fC.empty() )
			{
				newlineAndIndent();
				curStream() << fC;
			}
			if ( ! vC.empty() )
			{
				newlineAndIndent();
				curStream() << vC;
			}
			if ( ! fnC.empty() )
			{
				newlineAndIndent();
				curStream() << fnC;
			}

			if ( ! curMod.initCode.empty() )
				outputInitCode( curMod.name, curMod.initCode );

			if ( ! curMod.suffix.empty() )
			{
				newlineAndIndent();
				curStream() << curMod.suffix;
			}
		}
		newlineAndIndent();
	}
	else
	{
		for ( size_t md = 0, nMD = myModules.size(); md != nMD; ++md )
		{
			ModuleDefinition &curMod = myModules[md];

			if ( ! curMod.prefix.empty() )
			{
				newlineAndIndent();
				curStream() << curMod.prefix;
			}

			for ( size_t t = 0, nT = curMod.types.size(); t != nT; ++t )
			{
				newlineAndIndent();
				const TypeDefinition &curT = curMod.types[t];
				curStream() << curT.declare;
				if ( ! curT.constfunc.empty() )
				{
					newlineAndIndent();
					curStream() << curT.constfunc << '\n';
				}
			}

			for ( size_t f = 0, nF = curMod.functions.size(); f != nF; ++f )
			{
				if ( ! curMod.functions[f].forwardDecl.empty() )
				{
					newlineAndIndent();
					curStream() << curMod.functions[f].forwardDecl << '\n';
				}
			}

			std::string initCode;
			for ( size_t v = 0, nV = curMod.variables.size(); v != nV; ++v )
			{
				newlineAndIndent();
				const GlobalVariableDefinition &glob = curMod.variables[v];
				curStream() << glob.declare;
				if ( ! glob.delayedInit.empty() )
				{
					std::stringstream tmpInit;
					pushStream( tmpInit );
					newlineAndIndent();
					curStream() << beginComment() << " Initializing " << glob.name << "..." << endComment();
					newlineAndIndent();
					curStream() << glob.delayedInit;
					popStream();
					initCode.append( tmpInit.str() );
				}
			}
			
			for ( size_t f = 0, nF = curMod.functions.size(); f != nF; ++f )
			{
				newlineAndIndent();
				curStream() << curMod.functions[f].body << '\n';
			}

			if ( ! initCode.empty() )
				outputInitCode( curMod.name, initCode );

			if ( ! curMod.suffix.empty() )
			{
				newlineAndIndent();
				curStream() << curMod.suffix;
			}
		}
	}

	popStream();
	return retval.str();
}


////////////////////////////////////////


void
CCommonLanguage::outputInitCode( const std::string &modName, const std::string &initCode )
{
	if ( supportsModuleDynamicInitialization() )
	{
		newlineAndIndent();
		curStream() << "struct __ctlcc_InitVals_" << modName;
		pushBlock();
		newlineAndIndent();
		curStream() << "__ctlcc_InitVals_" << modName << "( void )";
		pushBlock();
		curStream() << initCode << '\n';
		newlineAndIndent();
		popBlock();
		popBlock();
		curStream() << ";";
		newlineAndIndent();
		curStream() << "static __ctlcc_InitVals_" << modName << " __ctlcc_GlobalInitializer_" << modName << ";\n\n";
	}
	else
	{
		newlineAndIndent();
		if ( supportsNamespaces() )
		{
			curStream() <<
				"void init( void )\n"
				"{\n"
						<< initCode <<
				"}\n";
		}
		else
		{
			curStream() <<
				"void " << modName << "_init( void )\n"
				"{\n"
						<< initCode <<
				"}\n";
		}
	}
}


////////////////////////////////////////


void
CCommonLanguage::recurseUsage( const std::map<const Module *, size_t> &indexmap, ModuleUsageMap &uMap, const Module *curModule )
{
	ModuleUsageMap::iterator i = uMap.begin();
	ModuleUsageMap::iterator e = uMap.end();
	for ( ; i != e; ++i )
	{
		std::map<const Module *, size_t>::const_iterator mi = indexmap.find( i->first );
		if ( mi == indexmap.end() )
			throw std::logic_error( "Unknown module pointer" );

		ModuleDefinition &curMod = myModules[mi->second];
		ModuleUsage &modUse = i->second;
		std::vector<TypeDefinition> &types = curMod.types;
		std::vector<GlobalVariableDefinition> &variables = curMod.variables;
		FuncDeclList &functions = curMod.functions;
		std::set<std::string>::iterator curUse;
		if ( ! modUse.types.empty() )
		{
			if ( ! curMod.typeCode )
				curMod.typeCode = new std::stringstream;
			pushStream( *(curMod.typeCode) );
			curUse = modUse.types.begin();
			while ( curUse != modUse.types.end() )
			{
				for ( size_t t = 0, nT = types.size(); t != nT; ++t )
				{
					if ( types[t].name == (*curUse) )
					{
						traverseAndEmit( indexmap, types[t] );
						break;
					}
				}
				++curUse;
			}
			popStream();
		}

		if ( ! modUse.variables.empty() )
		{
			if ( ! curMod.varCode )
				curMod.varCode = new std::stringstream;
			pushStream( *(curMod.varCode) );
			std::stringstream initDest;
			std::stringstream *curDest = myCurInitDestination;
			myCurInitDestination = &initDest;
			curUse = modUse.variables.begin();
			while ( curUse != modUse.variables.end() )
			{
				for ( size_t v = 0, nV = variables.size(); v != nV; ++v )
				{
					if ( variables[v].name == (*curUse) )
					{
						traverseAndEmit( indexmap, variables[v] );
						break;
					}
				}
				++curUse;
			}
			myCurInitDestination = curDest;
			curMod.initCode.append( initDest.str() );

			popStream();
		}

		if ( ! modUse.functions.empty() )
		{
			if ( curModule && curModule == curMod.module )
			{
				if ( ! curMod.fwdCode )
					curMod.fwdCode = new std::stringstream;
				pushStream( *(curMod.fwdCode) );
				curUse = modUse.functions.begin();
				while ( curUse != modUse.functions.end() )
				{
					for ( size_t f = 0, nF = functions.size(); f != nF; ++f )
					{
						if ( functions[f].name == (*curUse) )
						{
							traverseAndEmit( indexmap, functions[f], true );
							break;
						}
					}
					++curUse;
				}
				popStream();
			}

			if ( ! curMod.funCode )
				curMod.funCode = new std::stringstream;
			pushStream( *(curMod.funCode) );
			curUse = modUse.functions.begin();
			while ( curUse != modUse.functions.end() )
			{
				for ( size_t f = 0, nF = functions.size(); f != nF; ++f )
				{
					if ( functions[f].name == (*curUse) )
					{
						traverseAndEmit( indexmap, functions[f] );
						break;
					}
				}
				++curUse;
			}
			popStream();
		}
	}
}


////////////////////////////////////////


void
CCommonLanguage::traverseAndEmit( const std::map<const Module *, size_t> &indexmap, TypeDefinition &t )
{
	if ( t.emit )
		return;

	recurseUsage( indexmap, t.moduleUsage );

	if ( t.emit )
		return;

	newlineAndIndent();
	curStream() << t.declare;

	if ( ! t.constfunc.empty() )
	{
		newlineAndIndent();
		curStream() << t.constfunc << '\n';
	}
	t.emit = true;
}


////////////////////////////////////////


void
CCommonLanguage::traverseAndEmit( const std::map<const Module *, size_t> &indexmap, GlobalVariableDefinition &glob )
{
	if ( glob.emit )
		return;

	recurseUsage( indexmap, glob.moduleUsage, glob.module );

	if ( glob.emit )
		return;

	newlineAndIndent();
	curStream() << glob.declare;

	if ( ! glob.delayedInit.empty() )
	{
		if ( myCurInitDestination )
		{
			pushStream( *myCurInitDestination );
			newlineAndIndent();
			curStream() << beginComment() << " Initializing " << glob.name << "..." << endComment();
			newlineAndIndent();
			curStream() << glob.delayedInit;
			popStream();
		}
	}
	glob.emit = true;
}


////////////////////////////////////////


void
CCommonLanguage::traverseAndEmit( const std::map<const Module *, size_t> &indexmap, FunctionDefinition &func, bool fwdOnly )
{
	if ( fwdOnly )
	{
		if ( func.emitForward )
			return;
		recurseUsage( indexmap, func.moduleUsage );
		if ( func.emitForward )
			return;

		curStream() << func.forwardDecl;
		func.emitForward = true;
		return;
	}

	if ( func.emit )
		return;

	recurseUsage( indexmap, func.moduleUsage );

	if ( func.emit )
		return;

	newlineAndIndent();
	curStream() << func.body << '\n';
	func.emit = true;
}


////////////////////////////////////////


std::string
CCommonLanguage::stdLibraryAndSetup( void )
{
	std::stringstream libSetupB;
	pushStream( libSetupB );
	curStream() << beginComment() << " Autogenerated code from ctlcc" << endComment() << '\n';
	newlineAndIndent();
	addStandardIncludes();
	newlineAndIndent();
	curStream() << "\ntypedef " << getPrecisionType() << " ctl_number_t;\n";
	popStream();
	return libSetupB.str();
}


////////////////////////////////////////


void
CCommonLanguage::pushBlock( void )
{
	newlineAndIndent();
	curStream() << '{';
	pushIndent();
}


////////////////////////////////////////


void
CCommonLanguage::popBlock( void )
{
	popIndent();
	newlineAndIndent();
	curStream() << '}';
}


////////////////////////////////////////


void
CCommonLanguage::module( CodeLContext &ctxt, const CodeModuleNode &m )
{
	std::string oldModuleName = myCurModuleName;
	std::string oldModPrefix = myModPrefix;
	Module *oldModule = myCurModule;

	myCurModule = ctxt.module();
	myCurModuleName = cleanName( myCurModule->name() );
	myNamespaceTags[myCurModule] = constructNamespaceTag( myCurModuleName );

	extractLiteralConstants( m.constants, ctxt );

	myModules.push_back( ModuleDefinition() );

	ModuleDefinition &modDef = myModules.back();
	modDef.name = myCurModuleName;
	modDef.module = myCurModule;
	ModuleDefinition *oldDef = myCurModuleDefinition;
	myCurModuleDefinition = &modDef;

	std::stringstream modPrefix;
	pushStream( modPrefix );
	newlineAndIndent();
	curStream() << beginComment() << " Module " << ctxt.module()->name() << " ("
				<< ctxt.fileName() << ")" << endComment() << '\n';
	newlineAndIndent();

	// really only for c++, but you never know...
	if ( supportsNamespaces() )
	{
		curStream() << "namespace " << myCurModuleName << " {";
		newlineAndIndent();
		curStream() << "using namespace _ctlcc_;";
		newlineAndIndent();
	}
	else
	{
		myModPrefix = myNamespaceTags[myCurModule];
		modDef.call_prefix = myModPrefix;
	}

	popStream();
	modDef.prefix = modPrefix.str();

	const std::vector< std::pair< std::string, MemberVector > > &theStructs =
		ctxt.structs();

	if ( ! theStructs.empty() )
	{
		for ( size_t i = 0, N = theStructs.size(); i != N; ++i )
		{
			const std::pair< std::string, MemberVector > &s = theStructs[i];
			TypeDefinition curDef;
			curDef.name = removeNSQuals( s.first );
			ModuleUsageMap *curUsage = myCurModuleUsage;
			myCurModuleUsage = &(curDef.moduleUsage);
			std::stringstream curStruct;
			pushStream( curStruct );
			newlineAndIndent();
			// To support plain-old C, we are going to use the
			// typedef foo_XX {} XX; mechanism
			curStream() << "typedef struct ctl_typedef_" << myModPrefix << curDef.name;
			pushBlock();
			for ( size_t m = 0, M = s.second.size(); m != M; ++m )
			{
				newlineAndIndent();
				const Member &mem = s.second[m];
				variable( ctxt, mem.name, mem.type, false, false, false, false );
				curStream() << ';';
			}
			popBlock();
			curStream() << ' ' << myModPrefix << curDef.name << ";\n";
			popStream();
			curDef.declare = curStruct.str();
			myCurModuleUsage = curUsage;
			modDef.types.push_back( curDef );
		}
	}

	++myInModuleInit;
	StatementNodePtr consts = m.constants;
	while ( consts )
	{
		consts->generateCode( ctxt );
		consts = consts->next;
	}
	--myInModuleInit;

	FunctionNodePtr func = m.functions;
	while ( func )
	{
		func->generateCode( ctxt );
		func = func->next;
	}

	std::stringstream modSuff;
	pushStream( modSuff );
	newlineAndIndent();

	if ( supportsNamespaces() )
	{
		curStream() << "} // namespace " << myCurModuleName;
		newlineAndIndent();
	}

	popStream();
	modDef.suffix = modSuff.str();

	myCurModule = oldModule;
	myCurModuleName = oldModuleName;
	myModPrefix = oldModPrefix;
	myCurModuleDefinition = oldDef;
}


////////////////////////////////////////


void
CCommonLanguage::function( CodeLContext &ctxt, const CodeFunctionNode &f )
{
	CodeFunctionTypePtr functionType = f.info->functionType();
	const ParamVector &params = functionType->parameters();

	if ( functionType->returnVarying() )
	{
		MESSAGE_LE( ctxt, ERR_RETURN_CONV, f.lineNumber,
					"Code Generator does not yet implement return of a varying type from a function" );
		return;
	}

	FunctionDefinition funDecl;
	ModuleUsageMap *curUsage = myCurModuleUsage;
	myCurModuleUsage = &(funDecl.moduleUsage);

	myCurOutputVars.clear();
	std::string funcName = removeNSQuals( f.name );
	bool isMain = funcName == myCurModuleName;
	if ( funcName == "main" )
	{
		funcName = myCurModuleName;
		isMain = true;
	}

	if ( isMain )
	{
		std::string nsName = myCurModuleName + "::" + funcName;
		registerMainRoutine( funcName, nsName, f.info );

		// and put it in a header file in case someone cares
		// about that
		std::stringstream headerBuf;
		pushStream( headerBuf );

		variable( ctxt, std::string(), functionType->returnType(),
				  false, false, false, false );

		curStream() << ' ' << myModPrefix << funcName << "( ";
		bool notfirst = false;
		for ( size_t i = 0, N = params.size(); i != N; ++i )
		{
			const Param &parm = params[i];
			if ( notfirst )
				curStream() << ", ";
			else
				notfirst = true;

			variable( ctxt, parm.name, parm.type,
					  parm.access == RWA_READ, true,
					  parm.access == RWA_WRITE || parm.access == RWA_READWRITE,
					  false );

			checkNeedsSizeArgument( parm.type, parm.name );
		}
		curStream() << " );";

		popStream();
		funDecl.header = headerBuf.str();
	}

	// only need to generate forward decl if we are used
	// in initializing constants
	if ( myFuncsUsedInInit.find( f.name ) != myFuncsUsedInInit.end() )
	{
		std::stringstream fwdDeclBuf;
		pushStream( fwdDeclBuf );
		newlineAndIndent();
		curStream() << "static ";
		variable( ctxt, std::string(), functionType->returnType(),
				  false, false, false, false );

		curStream() << ' ' << funcName << "( ";
		bool notfirst = false;
		for ( size_t i = 0, N = params.size(); i != N; ++i )
		{
			const Param &parm = params[i];
			if ( notfirst )
				curStream() << ", ";
			else
				notfirst = true;

			variable( ctxt, parm.name, parm.type,
					  parm.access == RWA_READ, true,
					  parm.access == RWA_WRITE || parm.access == RWA_READWRITE,
					  false );

			checkNeedsSizeArgument( parm.type, parm.name );
		}
		curStream() << " );";
		popStream();

		funDecl.forwardDecl = fwdDeclBuf.str();
	}

	std::stringstream funcBodyStream;
	pushStream( funcBodyStream );

	newlineAndIndent();
	curStream() << getFunctionPrefix() << ' ';

	if ( ! isMain && myFuncsUsedInInit.find( f.name ) == myFuncsUsedInInit.end() )
		curStream() << getInlineKeyword() << ' ';

	variable( ctxt, std::string(), functionType->returnType(),
			  false, false, false, false );
	newlineAndIndent();

	curStream() << myModPrefix << funcName << "( ";
	bool notfirst = false;
	for ( size_t i = 0, N = params.size(); i != N; ++i )
	{
		const Param &parm = params[i];
		if ( notfirst )
			curStream() << ", ";
		else
			notfirst = true;

		variable( ctxt, parm.name, parm.type,
				  parm.access == RWA_READ, true,
				  parm.access == RWA_WRITE || parm.access == RWA_READWRITE,
				  false );

		if ( parm.access == RWA_WRITE || parm.access == RWA_READWRITE )
			myCurOutputVars.insert( parm.name );

		checkNeedsSizeArgument( parm.type, parm.name );

		if ( parm.defaultValue )
		{
			NameNodePtr n = parm.defaultValue.cast<NameNode>();
			if ( n )
			{
				NameNodePtr n = parm.defaultValue.cast<NameNode>();
				NameNodePtr dName = n->info->value().cast<NameNode>();
				std::string varVal;
				if ( dName )
				{
					varVal = dName->name;
				}
				else
				{
					std::stringstream varDeclB;
					pushStream( varDeclB );
					n->info->value()->generateCode( ctxt );
					popStream();
					varVal = varDeclB.str();
				}
				myDefaultMappings[n->name] = varVal;
			}
		}
	}
	curStream() << " )";
	pushBlock();
	++myInFunction;
	StatementNodePtr bodyNode = f.body;
	while ( bodyNode )
	{
		bodyNode->generateCode( ctxt );
		bodyNode = bodyNode->next;
	}
	--myInFunction;
	popBlock();
	curStream() << "\n\n";

	popStream();

	myCurModuleUsage = curUsage;
	funDecl.body = funcBodyStream.str();
	funDecl.name = funcName;
	if ( myCurModuleDefinition )
		myCurModuleDefinition->functions.push_back( funDecl );
	else
		throw std::logic_error( "Function declaration outside of module" );
}


////////////////////////////////////////


void
CCommonLanguage::variable( CodeLContext &ctxt, const CodeVariableNode &v )
{
	if ( myInModuleInit > 0 )
	{
		std::map<std::string, std::string>::const_iterator i = myGlobalLiterals.find( v.name );

		// We'll just put the literal in directly
		if ( i != myGlobalLiterals.end() )
			return;

		bool doConst = ! v.info->isWritable();
		bool overrideInit = false;
		if ( usesFunctionInitializers() )
		{
			// in C++11 we can use initializer lists or constructors
			// for everything. In old c++, some things have to be
			// initialized in a function. if we use one of those, we
			// can't initialize ourselves that quickly...
			if ( usesUninitLocalGlobals( v.initialValue ) )
			{
				doConst = false;
				overrideInit = true;
			}
		}
		GlobalVariableDefinition globDef;
		ModuleUsageMap *curUsage = myCurModuleUsage;
		myCurModuleUsage = &(globDef.moduleUsage);

		globDef.name = v.name;
		globDef.module = myCurModule;
		std::stringstream varDeclB;
		pushStream( varDeclB );
		const std::string &pref = getGlobalPrefix();
		if ( ! pref.empty() )
			curStream() << pref << ' ';
		InitType initT = variable( ctxt, v.name, v.info->type(),
								   doConst, false, false, true );

		if ( overrideInit )
			initT = FUNC;

		if ( v.name.find( '$' ) != std::string::npos )
		{
			std::string initVal;
			if ( v.initialValue )
			{
				if ( initT == FUNC )
				{
					if ( ! v.initialValue.cast<NameNode>() )
					{
						MESSAGE_LE( ctxt, ERR_TYPE, v.lineNumber,
									"Code Generator does not yet implement complex default value handling for functions" );
						throw std::logic_error( "NYI: complex default value handling for function type" );
					}
				}
				std::stringstream initB;
				pushStream( initB );
				myCurInitType = initT;
				v.initialValue->generateCode( ctxt );
				myCurInitType = NONE;
				popStream();
				initVal = initB.str();
			}

			myDefaultMappings[v.name] = initVal;
			// don't want to emit any declaration, but want to track
			// variable usage...
			popStream();
			globDef.declare.clear();
		}
		else
		{
			myGlobalInitType[v.name] = initT;
			myGlobalVariables.insert( v.name );
			globDef.delayedInit = doInit( initT, ctxt, v.info->type(),
										  v.initialValue, v.name );
			popStream();
			globDef.declare = varDeclB.str();
		}

		if ( myCurModuleDefinition )
			myCurModuleDefinition->variables.push_back( globDef );
		else
			throw std::logic_error( "in module global init, but no module definition" );
		myCurModuleUsage = curUsage;
	}
	else
	{
		newlineAndIndent();
		InitType initT = variable( ctxt, v.name, v.info->type(),
								   ! v.info->isWritable(), false, false, false );

		std::string delInit = doInit( initT, ctxt, v.info->type(),
									  v.initialValue, v.name );
		if ( ! delInit.empty() )
			throw std::logic_error( "Delayed init on normal variable" );
	}
}


////////////////////////////////////////


void
CCommonLanguage::assignment( CodeLContext &ctxt, const CodeAssignmentNode &v )
{
	newlineAndIndent();
	v.lhs->generateCode( ctxt );
	curStream() << " = ";
	v.lhs->type->generateCastFrom( v.rhs, ctxt );
	curStream() << ';';
}


////////////////////////////////////////


void
CCommonLanguage::expr( CodeLContext &ctxt, const CodeExprStatementNode &v )
{
	newlineAndIndent();
	v.expr->generateCode( ctxt );
	curStream() << ';';
}


////////////////////////////////////////


void CCommonLanguage::cond( CodeLContext &ctxt, const CodeIfNode &v )
{
	if ( myInElse > 0 )
		curStream() << ' ';
	else
		newlineAndIndent();
	curStream() << "if ( ";
	BoolTypePtr boolType = ctxt.newBoolType();
	boolType->generateCastFrom( v.condition, ctxt );
	curStream() << " )";

	StatementNodePtr tPath = v.truePath;
	StatementNodePtr fPath = v.falsePath;

	if ( tPath && ! tPath->next )
	{
		pushIndent();
		tPath->generateCode( ctxt );
		popIndent();
	}
	else
	{
		pushBlock();
		while ( tPath )
		{
			tPath->generateCode( ctxt );
			tPath = tPath->next;
		}
		popBlock();
	}

	if ( fPath )
	{
		newlineAndIndent();
		curStream() << "else";
		if ( fPath.cast<CodeIfNode>() && ! fPath->next )
		{
			++myInElse;
			fPath->generateCode( ctxt );
			--myInElse;
		}
		else if ( ! fPath->next )
		{
			pushIndent();
			fPath->generateCode( ctxt );
			popIndent();
		}
		else
		{
			int oldV = myInElse;
			myInElse = 0;
			pushBlock();
			while ( fPath )
			{
				fPath->generateCode( ctxt );
				fPath = fPath->next;
			}
			popBlock();
			myInElse = oldV;
		}
	}
}


////////////////////////////////////////


void
CCommonLanguage::retval( CodeLContext &ctxt, const CodeReturnNode &v )
{
	newlineAndIndent();
	curStream() << "return";
	if ( v.returnedValue )
	{
		curStream() << ' ';
		v.info->type()->generateCastFrom( v.returnedValue, ctxt );
	}
	curStream() << ';';
}


////////////////////////////////////////


void
CCommonLanguage::loop( CodeLContext &ctxt, const CodeWhileNode &v )
{
	newlineAndIndent();
	curStream() << "while ( ";
	BoolTypePtr boolType = ctxt.newBoolType();
	boolType->generateCastFrom( v.condition, ctxt );
	curStream() << " )";
	pushBlock();
	StatementNodePtr body = v.loopBody;
	while ( body )
	{
		body->generateCode( ctxt );
		body = body->next;
	}
	popBlock();
}


////////////////////////////////////////


void
CCommonLanguage::binaryOp( CodeLContext &ctxt, const CodeBinaryOpNode &v )
{
	// operator precedence in CTL is same as C++, but we will have
	// lost any parens during the parse stage, so we should
	// introduce them at all sub-levels just in case

	++myExprLevel;
	if ( myExprLevel > 1 )
		curStream() << "( ";

	v.operandType->generateCastFrom( v.leftOperand, ctxt );
	curStream() << ' ';
	v.operandType->generateCode( const_cast<CodeBinaryOpNode *>( &v ), ctxt );
	curStream() << ' ';
	v.operandType->generateCastFrom( v.rightOperand, ctxt );

	if ( myExprLevel > 1 )
		curStream() << " )";
	--myExprLevel;
}


////////////////////////////////////////


void
CCommonLanguage::unaryOp( CodeLContext &ctxt, const CodeUnaryOpNode &v )
{
	v.type->generateCode( const_cast<CodeUnaryOpNode *>( &v ), ctxt );
	v.type->generateCastFrom( v.operand, ctxt );
}


////////////////////////////////////////


void
CCommonLanguage::index( CodeLContext &ctxt, const CodeArrayIndexNode &v )
{
	IntTypePtr intType = ctxt.newIntType();

	ExprNodePtr root;
	ArrayIndexNode *parentArray = dynamic_cast<ArrayIndexNode *>( v.array.pointer() );
	std::vector<std::string> parSubs;
	while ( parentArray )
	{
		std::stringstream tmpbuf;
		pushStream( tmpbuf );
		intType->generateCastFrom( parentArray->index, ctxt );
		popStream();
		parSubs.push_back( tmpbuf.str() );

		ArrayIndexNode *nextPar = dynamic_cast<ArrayIndexNode *>( parentArray->array.pointer() );

		if ( ! nextPar )
			root = parentArray->array;
		parentArray = nextPar;
	}
	if ( ! root )
		root = v.array;

	const ArrayInfo &arrInfo = collapseArray( root->type.cast<ArrayType>() );
	root->generateCode( ctxt );
	if ( arrInfo.isCore && ! supportsStructOperators() )
		curStream() << ".vals";

	curStream() << '[';
	bool needAdd = false;
	if ( ! parSubs.empty() )
	{
		std::reverse( parSubs.begin(), parSubs.end() );
		const std::vector<int> &arr_sizes = arrInfo.sizes;
		if ( parSubs.size() != ( arr_sizes.size() - 1 ) )
		{
			std::cerr << "parent subs size: " << parSubs.size() << " arrinfo sizes: " << arr_sizes.size() << std::endl;
			MESSAGE_LE( ctxt, ERR_ARR_LEN, v.lineNumber,
						"Code Generator does not know how to interpret set of array subscripts" );
			throw std::logic_error( "Invalid set of array subscripts" );
		}

		for ( size_t pI = 0; pI != parSubs.size(); ++pI )
		{
			if ( arr_sizes[pI] < 0 )
			{
				curStream() << parSubs[pI] << "][";
				continue;
			}

			size_t nUnder = 1;
			for ( size_t x = pI + 1; x < arr_sizes.size(); ++x )
			{
				if ( arr_sizes[x] < 0 )
					break;

				nUnder *= arr_sizes[x];
			}
			
			if ( needAdd )
				curStream() << " + ";

			curStream() << "(" << parSubs[pI] << ") * " << nUnder;
			if ( arr_sizes[pI + 1] < 0 )
			{
				curStream() << "][";
				needAdd = false;
			}
			else
				needAdd = true;
		}
	}
	if ( needAdd )
		curStream() << " + ";
	intType->generateCastFrom( v.index, ctxt );
	curStream() << ']';
}


////////////////////////////////////////


void
CCommonLanguage::member( CodeLContext &ctxt, const CodeMemberNode &v )
{
	v.obj->generateCode( ctxt );
	curStream() << '.' << v.member;
}


////////////////////////////////////////


void
CCommonLanguage::size( CodeLContext &ctxt, const CodeSizeNode &v )
{
	std::cout << "size function for array - need to check if the array is a function parameter and retrieve <name>_size, or just extract the size value and inject it" << std::endl;
	MESSAGE_LE( ctxt, ERR_SIZE_SYNTAX, v.lineNumber,
				"Code Generator does not yet implement the size operator" );
	throw std::logic_error( "Function not yet implemented" );
}


////////////////////////////////////////


void
CCommonLanguage::name( CodeLContext &ctxt, const CodeNameNode &v )
{
	std::map<std::string, std::string>::const_iterator i = myGlobalLiterals.find( v.name );

	if ( i != myGlobalLiterals.end() )
	{
		curStream() << i->second;
		return;
	}
	
	std::string outname = removeNSQuals( v.name );

	if ( v.info->isFunction() )
	{
		const Module *m = v.info->module();
		if ( m == myCurModule )
			curStream() << myModPrefix << outname;
		else if ( m )
			curStream() << myNamespaceTags[m] << outname;
		else
		{
			std::map<std::string, std::string>::iterator i = myStdMathFuncs.find( outname );
			if ( i != myStdMathFuncs.end() )
				curStream() << i->second;
			else
				curStream() << outname;
		}

		if ( myCurModuleUsage )
			(*myCurModuleUsage)[m].functions.insert( outname );

		return;
	}

	if ( myGlobalVariables.find( v.name ) != myGlobalVariables.end() )
	{
		const Module *m = v.info->module();
		if ( m == myCurModule )
			curStream() << myModPrefix << outname;
		else if ( m )
			curStream() << myNamespaceTags[m] << outname;
		else
		{
			// these are currently all #defines, so no namespace
			std::map<std::string, std::string>::iterator i = myStdNames.find( outname );
			if ( i != myStdNames.end() )
				curStream() << i->second;
			else
				curStream() << outname;
		}

		if ( myCurModuleUsage )
			(*myCurModuleUsage)[m].variables.insert( outname );

		return;
	}

	i = myStdNames.find( outname );
	if ( i != myStdNames.end() )
		curStream() << i->second;
	else
	{
		if ( supportsReferences() )
			curStream() << outname;
		else
		{
			if ( myCurOutputVars.find( outname ) != myCurOutputVars.end() )
				curStream() << "*(" << outname << ")";
			else
				curStream() << outname;
		}
	}
}


////////////////////////////////////////


void
CCommonLanguage::boolLit( CodeLContext &ctxt, const CodeBoolLiteralNode &v )
{
	curStream() << getBoolLiteral( v.value );
}


////////////////////////////////////////


void
CCommonLanguage::intLit( CodeLContext &ctxt, const CodeIntLiteralNode &v )
{
	if ( v.literal.empty() )
		curStream() << v.value;
	else
		curStream() << v.literal;
}


////////////////////////////////////////


void
CCommonLanguage::uintLit( CodeLContext &ctxt, const CodeUIntLiteralNode &v )
{
	if ( v.literal.empty() )
		curStream() << v.value;
	else
		curStream() << v.literal;
}


////////////////////////////////////////


void
CCommonLanguage::halfLit( CodeLContext &ctxt, const CodeHalfLiteralNode &v )
{
	if ( ! supportsHalfType() )
	{
		MESSAGE_LE( ctxt, ERR_TYPE, v.lineNumber,
					"Language does not support the half data type" );
		throw std::logic_error( "Language does not support half" );
	}

	if ( v.literal.empty() )
		curStream() << "half( " << std::setprecision( std::numeric_limits<half>::digits ) << static_cast<number>( v.value ) << "F )";
	else
		curStream() << "half( " << v.literal << " )";
}


////////////////////////////////////////


void
CCommonLanguage::floatLit( CodeLContext &ctxt, const CodeFloatLiteralNode &v )
{
	if ( v.literal.empty() )
		curStream() << std::fixed << std::setprecision( std::numeric_limits<number>::digits ) << static_cast<number>( v.value ) << getPrecisionTypeSuffix();
	else
		curStream() << v.literal;
}


////////////////////////////////////////


void
CCommonLanguage::stringLit( CodeLContext &ctxt, const CodeStringLiteralNode &v )
{
	curStream() << '"' << escapeLiteral( v.value ) << '"';
}


////////////////////////////////////////


static bool
isStdPrintFunction( const std::string &n )
{
	return ( n == "::print_bool" ||
			 n == "::print_int" ||
			 n == "::print_unsigned_int" ||
			 n == "::print_half" ||
			 n == "::print_float" ||
			 n == "::print_string" );
}

////////////////////////////////////////


void
CCommonLanguage::call( CodeLContext &ctxt, const CodeCallNode &v )
{
	std::string baseFuncName = removeNSQuals( v.function->name );
	bool isPrint = isStdPrintFunction( baseFuncName );

	if ( isPrint )
		newlineAndIndent();

	bool needAddress = ! supportsReferences();
	v.function->generateCode( ctxt );
	if ( v.arguments.empty() )
		curStream() << "()";
	else
	{
		SymbolInfoPtr info = v.function->info;
		FunctionTypePtr functionType = info->functionType();
		const ParamVector &parameters = functionType->parameters();
		curStream() << "( ";
		size_t i = 0;
		for ( size_t N = v.arguments.size(); i != N; ++i )
		{
			if ( i >= parameters.size() )
			{
				MESSAGE_LE( ctxt, ERR_FUNC_ARG_NUM, v.lineNumber,
							"Too many arguments in function call" );
				throw std::logic_error( "Too many arguments in function call" );
			}

			if ( i > 0 )
				curStream() << ", ";

			if ( needAddress && parameters[i].access != RWA_READ )
			{
				curStream() << "&";
			}

			parameters[i].type->generateCastFrom( v.arguments[i], ctxt );
			if ( checkNeedsSizeArgument( parameters[i].type, std::string() ) )
				extractSizeAndAdd( v.arguments[i], parameters[i].type, ctxt );
		}
		for ( size_t N = parameters.size(); i < N; ++i )
		{
			const Param &parm = parameters[i];

			if ( i > 0 )
				curStream() << ", ";

			if ( ! parm.defaultValue )
			{
				MESSAGE_LE( ctxt, ERR_UNKNOWN_TYPE, v.lineNumber,
							"Missing argument in function call (no default value)" );
				throw std::logic_error( "Missing argument in function call (no default value)" );
			}

			NameNodePtr n = parm.defaultValue.cast<NameNode>();
			std::string defVal;
			std::string namesp;
			if ( n )
			{
				const Module *m = n->info->module();
				if ( m )
				{
					if ( m != myCurModule )
						namesp = myNamespaceTags[m];
					else
						namesp = myModPrefix;
				}
				defVal = n->name;

				if ( myCurModuleUsage )
					(*myCurModuleUsage)[m].variables.insert( defVal );
			}
			else
			{
				std::stringstream nameLookupB;
				pushStream( nameLookupB );
				parm.defaultValue->generateCode( ctxt );
				popStream();
				defVal = nameLookupB.str();
			}

			std::map<std::string, std::string>::const_iterator found = myDefaultMappings.find( defVal );

			if ( found != myDefaultMappings.end() )
			{
				std::map<std::string, std::string>::iterator i = myStdNames.find( defVal );
				if ( i != myStdNames.end() )
				{
					curStream() << i->second;
				}
				else
				{
					if ( isalpha( found->second[0] ) )
						curStream() << namesp;
					curStream() << found->second;
				}
			}
			else
				curStream() << defVal;

			if ( checkNeedsSizeArgument( parm.type, std::string() ) )
			{
				curStream() << ", ";
				extractSizeAndAdd( parm.defaultValue, parm.type, ctxt );
			}
		}
		curStream() << " )";
	}
	if ( isPrint )
		curStream() << ';';
}


////////////////////////////////////////


void
CCommonLanguage::value( CodeLContext &ctxt, const CodeValueNode &v )
{
	size_t idx = 0;
	valueRecurse( ctxt, v.elements, v.type, idx, "$$$$" );
}


////////////////////////////////////////


void
CCommonLanguage::startToBool( void )
{
	startCast( "bool" );
}


////////////////////////////////////////


void
CCommonLanguage::startToInt( void )
{
	startCast( "int" );
}


////////////////////////////////////////


void
CCommonLanguage::startToUint( void )
{
	startCast( "unsigned int" );
}


////////////////////////////////////////


void
CCommonLanguage::startToHalf( void )
{
	if ( ! supportsHalfType() )
		throw std::logic_error( "Language does not support the half type" );
	startCast( "half" );
}


////////////////////////////////////////


void
CCommonLanguage::startToFloat( void )
{
	startCast( "ctl_number_t" );
}


////////////////////////////////////////


void
CCommonLanguage::endCoersion( void )
{
	curStream() << " )";
}


////////////////////////////////////////


void
CCommonLanguage::emitToken( Token t )
{
	switch ( t )
	{
		case TK_AND: curStream() << "&&"; break;
		case TK_OR: curStream() << "||"; break;

		case TK_BITAND: curStream() << "&"; break;
		case TK_BITNOT: curStream() << "~"; break;
		case TK_BITOR: curStream() << "|"; break;
		case TK_BITXOR: curStream() << "^"; break;

		case TK_LEFTSHIFT: curStream() << "<<"; break;
		case TK_RIGHTSHIFT: curStream() << "<<"; break;

		case TK_DIV: curStream() << "/"; break;
		case TK_MINUS: curStream() << "-"; break;
		case TK_MOD: curStream() << "%"; break;
		case TK_PLUS: curStream() << "+"; break;
		case TK_TIMES: curStream() << "*"; break;

		case TK_EQUAL: curStream() << "=="; break;
		case TK_NOTEQUAL: curStream() << "!="; break;
		case TK_GREATER: curStream() << ">"; break;
		case TK_GREATEREQUAL: curStream() << ">="; break;
		case TK_LESS: curStream() << "<"; break;
		case TK_LESSEQUAL: curStream() << "<="; break;
		case TK_NOT: curStream() << "!"; break;

		default:
			break;
	}
}


////////////////////////////////////////


void
CCommonLanguage::valueRecurse( CodeLContext &ctxt, const ExprNodeVector &elements,
							const DataTypePtr &t, size_t &index,
							const std::string &varName,
							bool isSubItem )
{
	ArrayType *arrayType = dynamic_cast<ArrayType *>( t.pointer() );
	if ( arrayType )
	{
		const ArrayInfo &arrInfo = collapseArray( t.cast<ArrayType>() );
		const std::vector<int> &arr_sizes = arrInfo.sizes;
		const DataTypePtr &coreType = arrayType->coreType();

		size_t nSubScripts = arr_sizes.size();
		size_t nItems = 1;
		size_t nPerCollapseChunk = 1;
		for ( size_t i = 0; i < nSubScripts; ++i )
		{
			if ( arr_sizes[i] == 0 )
				throw std::logic_error( "Attempt to initialize unknown array size" );

			if ( arr_sizes[i] > 0 )
				nItems *= arr_sizes[i];
			else
				nPerCollapseChunk *= abs( arr_sizes[i] );
		}

		bool doCtor = myCurInitType != CTOR;
		if ( arrInfo.type == "int" || arrInfo.type == "ctl_number_t" )
			doCtor = false;

		if ( myCurInitType == FUNC )
		{
			pushIndent();
			for ( size_t i = 0; i < nItems; ++i )
			{
				newlineAndIndent();
				if ( nItems > 1 )
					curStream() << varName << '[' << i << "] = ";
				else
					curStream() << varName << " = ";

				if ( doCtor )
					curStream() << arrInfo.maker << "( ";

				for ( size_t x = 0; x < nPerCollapseChunk; ++x, ++index )
				{
					if ( x > 0 )
						curStream() << ", ";
					coreType->generateCastFrom( elements[index], ctxt );
				}
				if ( doCtor )
					curStream() << " );";
			}
			popIndent();
		}
		else
		{
			bool doBrace = false;
			if ( myCurInitType == ASSIGN )
			{
				newlineAndIndent();
				if ( ! isSubItem || ! doCtor )
					curStream() << "{ ";
				if ( nItems > 1 && nPerCollapseChunk > 1 && ! doCtor )
					doBrace = true;
			}
			pushIndent();

			bool lineEveryItem = nItems > 4;
			for ( size_t i = 0; i < nItems; ++i )
			{
				if ( doBrace )
				{
					if ( i > 0 )
						curStream() << ",\n";

					newlineAndIndent();
					curStream() << "{ ";
				}
				else if ( i > 0 )
				{
					if ( nPerCollapseChunk == 1 && ! lineEveryItem && i > 0 )
						curStream() << ", ";
					else
						curStream() << ',';
				}
				if ( doCtor )
				{
					if ( lineEveryItem )
						newlineAndIndent();

					curStream() << arrInfo.maker << "( ";
					for ( size_t x = 0; x < nPerCollapseChunk; ++x, ++index )
					{
						if ( x > 0 )
							curStream() << ", ";
					
						coreType->generateCastFrom( elements[index], ctxt );
					}
					curStream() << " )";
				}
				else
				{
					for ( size_t x = 0; x < nPerCollapseChunk; ++x, ++index )
					{
						if ( x > 0 )
							curStream() << ',';

						if ( lineEveryItem )
							newlineAndIndent();
						else if ( x > 0 )
							curStream() << ' ';

						coreType->generateCastFrom( elements[index], ctxt );
					}
				}

				if ( doBrace )
					curStream() << " }";
			}
			popIndent();

			if ( myCurInitType == ASSIGN )
			{
				if ( lineEveryItem )
				{
					newlineAndIndent();
					if ( ! isSubItem || ! doCtor )
						curStream() << "}";
				}
				else if ( ! isSubItem || ! doCtor )
					curStream() << " }";
			}
		}

		return;
	}

	StructType *structType = dynamic_cast<StructType *>( t.pointer() );
	if ( structType )
	{
		if ( myCurInitType == ASSIGN )
			curStream() << '{';

		if ( myCurInitType == FUNC )
		{
			for ( MemberVectorConstIterator it = structType->members().begin();
				 it != structType->members().end();
				 ++it )
			{
				std::string name = varName + "." + it->name;
				valueRecurse( ctxt, elements, it->type, index, name, true );
			}
		}
		else
		{
			pushIndent();
			for ( MemberVectorConstIterator it = structType->members().begin();
				 it != structType->members().end();
				 ++it )
			{
				if ( it != structType->members().begin() )
					curStream() << ", ";

				valueRecurse( ctxt, elements, it->type, index, varName, true );
			}
			popIndent();
		}
		if ( myCurInitType == ASSIGN )
		{
			newlineAndIndent();
			curStream() << "}";
		}
		return;
	}

	if ( myCurInitType == FUNC )
	{
		newlineAndIndent();
		curStream() << varName << " = ";
		t->generateCastFrom( elements[index], ctxt );
		curStream() << ';';
	}
	else
		t->generateCastFrom( elements[index], ctxt );
	++index;
}


////////////////////////////////////////


CCommonLanguage::InitType
CCommonLanguage::variable( CodeLContext &ctxt,
						   const std::string &name, const DataTypePtr &t,
						   bool isConst, bool isInput, bool isWritable,
						   bool isGlobal )
{
	InitType retval = ASSIGN;

	std::string postDecl;
	switch ( t->cDataType() )
	{
		case VoidTypeEnum:
			curStream() << "void";
			break;
		case BoolTypeEnum:
			if ( isConst )
				curStream() << getConstLiteral() << ' ';
			curStream() << "bool";
			break;
		case IntTypeEnum:
			if ( isConst )
				curStream() << getConstLiteral() << ' ';
			curStream() << "int";
			break;
		case UIntTypeEnum:
			if ( isConst )
				curStream() << getConstLiteral() << ' ';
			curStream() << "unsigned int";
			break;
		case HalfTypeEnum:
			if ( ! supportsHalfType() )
				throw std::logic_error( "Language does not support the half type" );
			if ( isConst )
				curStream() << getConstLiteral() << ' ';
			curStream() << "half";
			break;
		case FloatTypeEnum:
			if ( isConst )
				curStream() << getConstLiteral() << ' ';
			curStream() << "ctl_number_t";
			break;
		case StringTypeEnum:
			if ( isConst )
				curStream() << "const char *";
			else
				curStream() << "char *";
			retval = ASSIGN;
			break;
		case StructTypeEnum:
		{
			StructTypePtr structType = t.cast<StructType>();
			if ( usesFunctionInitializers() )
				retval = FUNC;
			else
				retval = ASSIGN;

			if ( retval == ASSIGN && isConst )
				curStream() << getConstLiteral() << ' ';

			std::string structName = removeNSQuals( structType->name() );
			if ( myCurModuleUsage )
			{
				// once we're at this level, no longer know the module,
				// but there shouldn't be too many struct types declared in CTL
				// so loop over all modules defined and find the module
				// (if not found, must be global / standard library struct
				// which we're using NULL for anyway)
				const Module *m = NULL;
				for ( size_t i = 0, N = myModules.size(); m == NULL && i != N; ++i )
				{
					const ModuleDefinition &curMod = myModules[i];
					for ( size_t s = 0, S = curMod.types.size(); s != S; ++s )
					{
						if ( curMod.types[s].name == structName )
						{
							m = curMod.module;
							break;
						}
					}
				}
				(*myCurModuleUsage)[m].types.insert( structName );
			}
			curStream() << structName;
			isWritable = isInput || isWritable;
			break;
		}
		case ArrayTypeEnum:
		{
			const ArrayInfo &arrInfo = collapseArray( t.cast<ArrayType>() );

			retval = ASSIGN;

			if ( myCurModuleUsage && arrInfo.isCore )
				(*myCurModuleUsage)[NULL].types.insert( arrInfo.type );

			std::stringstream postBuf;
			bool needOpen = true;
			int mulCnt = 0;
			for ( size_t i = 0, N = arrInfo.sizes.size(); i != N; ++i )
			{
				int sz = arrInfo.sizes[i];
				// built-in vec / matrix set size to negative in collapse
				if ( sz < 0 )
					break;

				if ( needOpen )
				{
					postBuf << '[';
					needOpen = false;
				}
				if ( sz == 0 )
				{
					postBuf << ']';
					needOpen = true;
					mulCnt = 0;
					continue;
				}
				if ( mulCnt > 0 )
					postBuf << " * ";

				postBuf << sz;
				++mulCnt;
			}
			if ( ! needOpen )
				postBuf << ']';
			postDecl = postBuf.str();

			if ( postDecl.empty() && supportsStructOperators() &&
				 ( arrInfo.type != "int" && arrInfo.type != "ctl_number_t" ) )
				retval = CTOR;

			if ( isConst )
				curStream() << getConstLiteral() << ' ';

			curStream() << arrInfo.type;
			isWritable = ( isInput || isWritable ) && postDecl.empty();
			break;
		}
	}

	if ( isWritable )
	{
		if ( supportsReferences() )
			curStream() << " &";
		else
			curStream() << " *";
	}

	if ( ! name.empty() )
	{
		if ( ! isWritable )
			curStream() << ' ';
		if ( isGlobal )
			curStream() << myModPrefix;
		curStream() << removeNSQuals( name );
	}
	curStream() << postDecl;

	return retval;
}


////////////////////////////////////////


std::string
CCommonLanguage::doInit( InitType initT, CodeLContext &ctxt,
						 const DataTypePtr &t,
						 const ExprNodePtr &initV, const std::string &varName )
{
	std::string delayedInit;
	myCurInitType = initT;
	if ( initV )
	{
		switch ( initT )
		{
			case CTOR:
				curStream() << "( ";
				initV->generateCode( ctxt );
				curStream() << " );";
				break;
			case FUNC:
			{
				curStream() << ';';
				std::stringstream varAssignB;
				pushStream( varAssignB );
				if ( ! initV.is_subclass<ValueNode>() )
				{
					pushIndent();
					addIndent();
					curStream() << "$$$$ = ";
					initV->generateCode( ctxt );
					curStream() << ';';
					popIndent();
				}
				else
					initV->generateCode( ctxt );
				popStream();

				std::string initVal = varAssignB.str();
				replaceInit( initVal, varName );

				if ( myInFunction )
				{
					pushBlock();
					curStream() << initVal;
					popBlock();
				}
				else if ( varName.find_first_of( '$' ) == std::string::npos )
					delayedInit = initVal;
				break;
			}
			case ASSIGN:
				curStream() << " = ";
				initV->generateCode( ctxt );
				curStream() << ";";
				break;
			case NONE:
				throw std::logic_error( "Invalid initialization type" );
				break;
		}
	}
	else
	{
		switch ( t->cDataType() )
		{
			case BoolTypeEnum:
				curStream() << " = " << getBoolLiteral( false ) << ";";
				break;
			case IntTypeEnum:
			case UIntTypeEnum:
			case HalfTypeEnum:
			case FloatTypeEnum:
				curStream() << " = 0;";
				break;
			case VoidTypeEnum:
			case StringTypeEnum:
			case StructTypeEnum:
			case ArrayTypeEnum:
			default:
				curStream() << ';';
				break;
		}
	}
	myCurInitType = NONE;
	return delayedInit;
}


////////////////////////////////////////


void
CCommonLanguage::replaceInit( std::string &initS, const std::string &name )
{
	std::string::size_type replB = initS.find( "$$$$" );
	while ( replB != std::string::npos )
	{
		initS.replace( replB, 4, name );
		replB = initS.find( "$$$$", replB + name.size() );
	}
}


////////////////////////////////////////


bool
CCommonLanguage::checkNeedInitInModuleInit( const ExprNodePtr &initV, bool deep )
{
	if ( ! initV )
		return false;

	if ( isAllLiterals( initV ) )
		return false;

	ValueNode *val = dynamic_cast<ValueNode *>( initV.pointer() );
	if ( val )
	{
		bool retval = false;
		// make sure we check every value for all possible functions
		for ( size_t i = 0, N = val->elements.size(); i != N; ++i )
		{
			bool a = checkNeedInitInModuleInit( val->elements[i] );
			if ( a && ! retval )
			{
				if ( ! deep )
					return true;
				retval = true;
			}
		}
		return true;
	}

	CallNode *c = dynamic_cast<CallNode *>( initV.pointer() );
	if ( c )
	{
		bool retval = false;
		SymbolInfoPtr info = c->function->info;
		if ( info->module() == myCurModule )
		{
			myFuncsUsedInInit.insert( c->function->name );
			if ( deep )
				retval = true;
			else
				return true;
		}

		if ( ! c->arguments.empty() )
		{
			// make sure we check every value for all possible functions
			for ( size_t i = 0, N = c->arguments.size(); i != N; ++i )
			{
				bool needed = checkNeedInitInModuleInit( c->arguments[i] );
				if ( needed && ! retval )
				{
					if ( ! deep )
						return true;
					retval = true;
				}
			}
		}
		return retval;
	}

	BinaryOpNodePtr bOp = initV.cast<BinaryOpNode>();
	if ( bOp )
	{
		// make sure both run to pick up
		// any functions used...
		bool l = checkNeedInitInModuleInit( bOp->leftOperand );
		bool r = checkNeedInitInModuleInit( bOp->rightOperand );
		return l || r;
	}
	UnaryOpNodePtr uOp = initV.cast<UnaryOpNode>();
	if ( uOp )
	{
		return checkNeedInitInModuleInit( uOp->operand );
	}

	return false;
}


////////////////////////////////////////


bool
CCommonLanguage::isAllLiterals( const ExprNodePtr &v )
{
	if ( ! v )
		return false;

	if ( v.is_subclass<LiteralNode>() )
		return true;

	if ( v.is_subclass<NameNode>() )
		return false;

	ValueNode *val = dynamic_cast<ValueNode *>( v.pointer() );
	if ( val )
	{
		for ( size_t i = 0, N = val->elements.size(); i != N; ++i )
		{
			if ( ! isAllLiterals( val->elements[i] ) )
				return false;
		}
		return true;
	}

	BinaryOpNodePtr bOp = v.cast<BinaryOpNode>();
	if ( bOp )
	{
		if ( ! isAllLiterals( bOp->leftOperand ) ||
			 ! isAllLiterals( bOp->rightOperand ) )
			return false;

		return true;
	}
	UnaryOpNodePtr uOp = v.cast<UnaryOpNode>();
	if ( uOp )
	{
		return isAllLiterals( uOp->operand );
	}

	return false;
}


////////////////////////////////////////


bool
CCommonLanguage::usesUninitLocalGlobals( const ExprNodePtr &v )
{
	if ( ! v )
		return false;

	if ( v.is_subclass<LiteralNode>() )
		return false;

	ValueNode *val = dynamic_cast<ValueNode *>( v.pointer() );
	if ( val )
	{
		for ( size_t i = 0, N = val->elements.size(); i != N; ++i )
		{
			if ( usesUninitLocalGlobals( val->elements[i] ) )
				return true;
		}
		return false;
	}

	NameNode *namePtr = dynamic_cast<NameNode *>( v.pointer() );
	if ( namePtr )
	{
		if ( namePtr->info->module() == myCurModule )
		{
			std::map<std::string, InitType>::const_iterator i = myGlobalInitType.find( namePtr->name );
			if ( i != myGlobalInitType.end() )
			{
				if ( i->second == FUNC )
					return true;
			}
		}

		return false;
	}

	CallNodePtr callPtr = v.cast<CallNode>();
	if ( callPtr )
	{
		for ( size_t i = 0, N = callPtr->arguments.size(); i != N; ++i )
		{
			if ( usesUninitLocalGlobals( callPtr->arguments[i] ) )
				return true;
		}

		return false;
	}

	BinaryOpNodePtr bOp = v.cast<BinaryOpNode>();
	if ( bOp )
	{
		if ( usesUninitLocalGlobals( bOp->leftOperand ) ||
			 usesUninitLocalGlobals( bOp->rightOperand ) )
			return true;

		return false;
	}
	UnaryOpNodePtr uOp = v.cast<UnaryOpNode>();
	if ( uOp )
	{
		return usesUninitLocalGlobals( uOp->operand );
	}

	return false;
}


////////////////////////////////////////


void
CCommonLanguage::extractLiteralConstants( const StatementNodePtr &consts,
										  CodeLContext &ctxt )
{
	StatementNodePtr curConst = consts;
	while ( curConst )
	{
		VariableNodePtr var = curConst.cast<VariableNode>();
		if ( isAllLiterals( var->initialValue ) )
		{
			if ( ! var->initialValue.cast<ValueNode>() )
			{
				std::stringstream x;
				pushStream( x );
				var->initialValue->generateCode( ctxt );
				popStream();
				myGlobalLiterals[var->name] = x.str();
			}
		}
		else if ( var->initialValue )
		{
			// don't care about the result, but need to get the function names
			// into the func used table
			checkNeedInitInModuleInit( var->initialValue, true );
		}
		curConst = curConst->next;
	}
}


////////////////////////////////////////


bool
CCommonLanguage::checkNeedsSizeArgument( const DataTypePtr &p, const std::string &name )
{
	int cnt = 0;
	if ( p->cDataType() == ArrayTypeEnum )
	{
		const ArrayInfo &arrInfo = collapseArray( p.cast<ArrayType>() );
		const std::vector<int> &sizes = arrInfo.sizes;

		for ( size_t i = 0, N = sizes.size(); i != N; ++i )
		{
			if ( sizes[i] == 0 )
				++cnt;
		}
	}

	if ( name.empty() )
		return cnt > 0;

	switch ( cnt )
	{
		case 0:
			break;
		case 1:
			curStream() << ", int " << name << "_size";
			break;
		case 2:
			if ( supportsReferences() )
				curStream() << ", const ctl_vec2i_t &" << name << "_size";
			else
				curStream() << ", ctl_vec2i_t " << name << "_size";
			break;
		case 3:
			if ( supportsReferences() )
				curStream() << ", const ctl_vec3i_t &" << name << "_size";
			else
				curStream() << ", ctl_vec3i_t " << name << "_size";
			break;
		default:
			throw std::logic_error( "Unimplemented size of variable array passing" );
			break;
	}
	
	return cnt > 0;
}


////////////////////////////////////////


void
CCommonLanguage::extractSizeAndAdd( const ExprNodePtr &p, const DataTypePtr &funcParm, CodeLContext &ctxt )
{
	if ( p->type->cDataType() == ArrayTypeEnum )
	{
		const ArrayInfo &arrInfo = collapseArray( p->type.cast<ArrayType>() );
		const std::vector<int> &arg_sizes = arrInfo.sizes;
		const ArrayInfo &funcInfo = collapseArray( funcParm.cast<ArrayType>() );
		const std::vector<int> &func_sizes = funcInfo.sizes;

		if ( arg_sizes.size() == func_sizes.size() )
		{
			int cnt = 0;
			SizeVector outSizes;
			for ( size_t i = 0, N = func_sizes.size(); i != N; ++i )
			{
				if ( func_sizes[i] == 0 )
				{
					++cnt;
					if ( arg_sizes[i] == 0 )
						throw std::logic_error( "Unknown argument size" );

					// take the absolute value since it could be a 'built-in' type
					// but we emit templates for lookup1D, et al. to translate
					// those...
					outSizes.push_back( abs( arg_sizes[i] ) );
				}
			}
			switch ( cnt )
			{
				case 0: throw std::logic_error( "Unhandled missing array size" );
				case 1: curStream() << ", (int)( "; break;
				case 2: curStream() << ", make_vec2i( "; break;
				case 3: curStream() << ", make_vec3i( "; break;
				default: throw std::logic_error( "Unimplemented size of variable array passing" );
			}
			for ( size_t i = 0, N = outSizes.size(); i != N; ++i )
			{
				if ( i > 0 )
					curStream() << ", ";
				curStream() << outSizes[i];
			}
			curStream() << " )";
		}
		else
		{
			throw std::logic_error( "Unhandled differing array sizes" );
		}
	}
	else
	{
		throw std::logic_error( "Unhandled array type coersion" );
	}
}


////////////////////////////////////////


const CCommonLanguage::ArrayInfo &
CCommonLanguage::collapseArray( const ArrayTypePtr &arrPtr )
{
	if ( ! arrPtr )
		throw std::logic_error( "Invalid array pointer passed" );

	ArrayInfoContainer::const_iterator ati = myArrayTypes.find( arrPtr.pointer() );
	if ( ati == myArrayTypes.end() )
	{
		SizeVector usizes;
		arrPtr->sizes( usizes );
		ArrayInfo newInfo;
		newInfo.isCore = false;
		newInfo.sizes.resize( usizes.size(), 0 );
		std::vector<int> &asize = newInfo.sizes;
		std::string &coreType = newInfo.type;

		for ( size_t i = 0, N = usizes.size(); i != N; ++i )
			asize[i] = static_cast<int>( usizes[i] );

		if ( asize.empty() )
			throw std::logic_error( "Empty array size list" );

		DataTypePtr coreTypePtr = arrPtr->coreType();
		if ( coreTypePtr.is_subclass<FloatType>() )
			coreType = "ctl_number_t";
		else if ( coreTypePtr.is_subclass<HalfType>() )
		{
			if ( ! supportsHalfType() )
				throw std::runtime_error( "Current language does not support the half data type, please promote to float" );

			coreType = "half";
		}
		else if ( coreTypePtr.is_subclass<IntType>() )
			coreType = "int";
		else
			throw std::logic_error( "Currently unhandled core data type for array" );

		if ( asize.size() == 2 && asize[0] == asize[1] &&
			 ( asize[0] == 3 || asize[0] == 4 ) &&
			 coreType != "int" )
		{
			if ( asize[0] == 3 )
			{
				coreType = "ctl_mat33f_t";
				if ( supportsStructOperators() )
					newInfo.maker = coreType;
				else
					newInfo.maker = "make_mat33f";
			}
			else
			{
				coreType = "ctl_mat44f_t";
				if ( supportsStructOperators() )
					newInfo.maker = coreType;
				else
					newInfo.maker = "make_mat44f";
			}
			newInfo.isCore = true;
			asize[0] = -asize[0];
			asize[1] = -asize[1];
		}
		else if ( asize.size() == 1 )
		{
			std::string &maker = newInfo.maker;
			// huh, if we do large tables with c++ objects, i.e. 3D luts,
			// both g++ and clang++ are pretty catatonic. gcc way more so
			// than clang++ but 3+ minutes for clang to compile and I never
			// could wait for g++ to finish a -O3 build...
			// so let's just do giant number table in those scenarios
			switch ( asize.back() )
			{
				case 2:
					if ( coreType == "int" )
					{
						coreType = "ctl_vec2i_t";
						if ( supportsStructOperators() )
							maker = coreType;
						else
							maker = "make_vec2i";
					}
					else
					{
						coreType = "ctl_vec2f_t";
						if ( supportsStructOperators() )
							maker = coreType;
						else
							maker = "make_vec2f";
					}
					newInfo.isCore = true;
					asize.back() *= -1;
					break;
				case 3:
					if ( coreType == "int" )
					{
						coreType = "ctl_vec3i_t";
						if ( supportsStructOperators() )
							maker = coreType;
						else
							maker = "make_vec3i";
					}
					else
					{
						coreType = "ctl_vec3f_t";
						if ( supportsStructOperators() )
							maker = coreType;
						else
							maker = "make_vec3f";
					}
					newInfo.isCore = true;
					asize.back() *= -1;
					break;
				case 4:
					if ( coreType == "int" )
					{
						coreType = "ctl_vec4i_t";
						if ( supportsStructOperators() )
							maker = coreType;
						else
							maker = "make_vec4i";
					}
					else
					{
						coreType = "ctl_vec4f_t";
						if ( supportsStructOperators() )
							maker = coreType;
						else
							maker = "make_vec4f";
					}
					newInfo.isCore = true;
					asize.back() *= -1;
					break;
				default:
					break;
			}
		}

		ati = myArrayTypes.insert( std::make_pair( arrPtr.pointer(), std::make_pair( arrPtr, newInfo ) ) ).first;
	}
	return ati->second.second;
}


////////////////////////////////////////


} // CtlCodeEmitter



