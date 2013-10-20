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

#include "CtlCodeCPPLanguage.h"
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

namespace Ctl
{

CPPGenerator::CPPGenerator( bool cpp11 )
		: CCommonLanguage(),
		  myCPP11Mode( cpp11 )
{
}


////////////////////////////////////////


CPPGenerator::~CPPGenerator( void )
{
}


////////////////////////////////////////


bool
CPPGenerator::supportsPrecision( Precision p ) const
{
	return true;
}


////////////////////////////////////////


std::string
CPPGenerator::stdLibraryAndSetup( void )
{
	std::stringstream libSetupB;
	std::string precType = getPrecisionType();
	std::string precSuffix = getPrecisionFunctionSuffix();

	libSetupB <<
		"// C++ code automatically generated\n\n"
		"#include <ImfVersion.h>\n"
		"#include <ImathVec.h>\n"
		"#include <ImathMatrix.h>\n"
		"#include <ImathFun.h>\n"
		"#include <half.h>\n"
		"#include <float.h>\n"
		"#include <math.h>\n"
		"#include <limits.h>\n"
		"#include <iostream>\n"
		"#include <stdexcept>\n"
		"#include <limits>\n"
		"#include <vector>\n"
		"\n"
		"#ifdef OPENEXR_IMF_NAMESPACE\n"
		"# include <ImathNamespace.h>\n"
		"#else\n"
		"# define IMATH_NAMESPACE Imath\n"
		"#endif\n"
		"\n"
		"typedef " << precType << " ctl_number_t;\n"
		"typedef IMATH_NAMESPACE::Vec2<ctl_number_t> ctl_vec2f_t;\n"
		"typedef IMATH_NAMESPACE::Vec2<int> ctl_vec2i_t;\n"
		"typedef IMATH_NAMESPACE::Vec3<ctl_number_t> ctl_vec3f_t;\n"
		"typedef IMATH_NAMESPACE::Vec3<int> ctl_vec3i_t;\n"
		"typedef IMATH_NAMESPACE::Vec4<ctl_number_t> ctl_vec4f_t;\n"
		"typedef IMATH_NAMESPACE::Vec4<int> ctl_vec4i_t;\n"
		"typedef IMATH_NAMESPACE::Matrix33<ctl_number_t> ctl_mat33f_t;\n"
		"typedef IMATH_NAMESPACE::Matrix44<ctl_number_t> ctl_mat44f_t;\n"
		"\n"
		"struct Chromaticities { ctl_vec2f_t red; ctl_vec2f_t green; ctl_vec2f_t blue; ctl_vec2f_t white; };\n"
		"struct Box2i { ctl_vec2i_t min; ctl_vec2i_t max; };\n"
		"struct Box2f { ctl_vec2f_t min; ctl_vec2f_t max; };\n\n"
		"\n"
		"namespace _ctlcc_ {\n"
		"\n"
		"using Imath::clamp;\n"
		"\n";
	if ( myStdFuncsUsed.find( "assert" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline void assert( bool v ) { if (!v) throw std::logic_error( \"Assertion failure\" ); }\n\n";

	if ( myStdFuncsUsed.find( "print_bool" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline void print_bool( bool v ) { std::cout << (v ? \"true\" : \"false\"); }\n";
	if ( myStdFuncsUsed.find( "print_int" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline void print_int( int v ) { std::cout << v; }\n";
	
	if ( myStdFuncsUsed.find( "print_unsigned_int" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline void print_unsigned_int( unsigned int v ) { std::cout << v; }\n";
	if ( myStdFuncsUsed.find( "print_half" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline void print_half( half v ) { std::cout << v; }\n";
	
	if ( myStdFuncsUsed.find( "print_float" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline void print_float( ctl_number_t v ) { std::cout << v; }\n";
	
	if ( myStdFuncsUsed.find( "print_string" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline void print_string( const std::string &v ) { std::cout << v; }\n"
			"static inline void print_string( const char *v ) { std::cout << v; }\n";

	libSetupB <<
		"\n"
		"static inline bool isfinite_f( ctl_number_t v ) { return isfinite( v ); }\n"
		"static inline bool isnormal_f( ctl_number_t v ) { return isnormal( v ); }\n"
		"static inline bool isnan_f( ctl_number_t v ) { return isnan( v ); }\n"
		"static inline bool isinf_f( ctl_number_t v ) { return isinf( v ) != 0; }\n"
		"static inline bool isfinite_h( half v ) { return v.isFinite(); }\n"
		"static inline bool isnormal_h( half v ) { return v.isNormalized(); }\n"
		"static inline bool isnan_h( half v ) { return v.isNan(); }\n"
		"static inline bool isinf_h( half v ) { return v.isInfinity() != 0; }\n"
		"\n"
		"#define FLT_POS_INF std::numeric_limits<ctl_number_t>::infinity()\n"
		"#define FLT_NEG_INF (-std::numeric_limits<ctl_number_t>::infinity())\n"
		"#define FLT_NAN (-std::numeric_limits<ctl_number_t>::quiet_NaN())\n"
		"#define HALF_POS_INF half::posInf()\n"
		"#define HALF_NEG_INF half::negInf()\n"
		"#define HALF_NAN half::qNan()\n"
		"\n"
		"static inline ctl_number_t acos( ctl_number_t v ) { return acos" << precSuffix << "( v ); }\n"
		"static inline ctl_number_t asin( ctl_number_t v ) { return asin" << precSuffix << "( v ); }\n"
		"static inline ctl_number_t atan( ctl_number_t v ) { return atan" << precSuffix << "( v ); }\n"
		"static inline ctl_number_t atan2( ctl_number_t y, ctl_number_t x ) { return atan2" << precSuffix << "( y, x ); }\n"
		"static inline ctl_number_t cos( ctl_number_t v ) { return cos" << precSuffix << "( v ); }\n"
		"static inline ctl_number_t sin( ctl_number_t v ) { return sin" << precSuffix << "( v ); }\n"
		"static inline ctl_number_t tan( ctl_number_t v ) { return tan" << precSuffix << "( v ); }\n"
		"static inline ctl_number_t cosh( ctl_number_t v ) { return cosh" << precSuffix << "( v ); }\n"
		"static inline ctl_number_t sinh( ctl_number_t v ) { return sinh" << precSuffix << "( v ); }\n"
		"static inline ctl_number_t tanh( ctl_number_t v ) { return tanh" << precSuffix << "( v ); }\n"
		"static inline ctl_number_t exp( ctl_number_t v ) { return exp" << precSuffix << "( v ); }\n"
		"static inline ctl_number_t log( ctl_number_t v ) { return log" << precSuffix << "( v ); }\n"
		"static inline ctl_number_t log10( ctl_number_t v ) { return log10" << precSuffix << "( v ); }\n"
		"static inline ctl_number_t pow( ctl_number_t x, ctl_number_t y ) { return pow" << precSuffix << "( x, y ); }\n"
		"static inline ctl_number_t pow10( ctl_number_t y ) { return pow" << precSuffix << "( 10.0, y ); }\n"
		"static inline ctl_number_t sqrt( ctl_number_t v ) { return sqrt" << precSuffix << "( v ); }\n"
		"static inline ctl_number_t fabs( ctl_number_t v ) { return fabs" << precSuffix << "( v ); }\n"
		"static inline ctl_number_t floor( ctl_number_t v ) { return floor" << precSuffix << "( v ); }\n"
		"static inline ctl_number_t fmod( ctl_number_t x, ctl_number_t y ) { return fmod" << precSuffix << "( x, y ); }\n"
		"static inline ctl_number_t hypot( ctl_number_t x, ctl_number_t y ) { return hypot" << precSuffix << "( x, y ); }\n"
		"\n"
		"static inline half exp_h( ctl_number_t v ) { return half( exp( v ) ); }\n"
		"static inline ctl_number_t log_h( half v ) { return log( float( v ) ); }\n"
		"static inline ctl_number_t log10_h( half v ) { return log10( float( v ) ); }\n"
		"static inline half pow_h( half x, ctl_number_t y ) { return half( pow( float( x ), y ) ); }\n"
		"static inline half pow10_h( ctl_number_t v ) { return half( pow( 10.0, v ) ); }\n"
		"\n";
	
	if ( myStdFuncsUsed.find( "mult_f33_f33" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_mat33f_t mult_f33_f33( const ctl_mat33f_t &a, const ctl_mat33f_t &b ) { return a * b; }\n";
	if ( myStdFuncsUsed.find( "mult_f44_f44" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_mat44f_t mult_f44_f44( const ctl_mat44f_t &a, const ctl_mat44f_t &b ) { return a * b; }\n";

	if ( myStdFuncsUsed.find( "mult_f_f33" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_mat33f_t mult_f_f33( ctl_number_t a, const ctl_mat33f_t &b ) { return a * b; }\n";
	if ( myStdFuncsUsed.find( "mult_f_f44" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_mat44f_t mult_f_f44( ctl_number_t a, const ctl_mat44f_t &b ) { return a * b; }\n";
	if ( myStdFuncsUsed.find( "add_f33_f33" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_mat33f_t add_f33_f33( const ctl_mat33f_t &a, const ctl_mat33f_t &b ) { return a + b; }\n";
	if ( myStdFuncsUsed.find( "add_f44_f44" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_mat44f_t add_f44_f44( const ctl_mat44f_t &a, const ctl_mat44f_t &b ) { return a + b; }\n";
	if ( myStdFuncsUsed.find( "invert_f33" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_mat33f_t invert_f33( const ctl_mat33f_t &a ) { return a.inverse(); }\n";
	if ( myStdFuncsUsed.find( "invert_f44" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_mat44f_t invert_f44( const ctl_mat44f_t &a ) { return a.inverse(); }\n";
	if ( myStdFuncsUsed.find( "transpose_f33" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_mat33f_t transpose_f33( const ctl_mat33f_t &a ) { return a.transposed(); }\n";
	if ( myStdFuncsUsed.find( "transpose_f44" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_mat44f_t transpose_f44( const ctl_mat44f_t &a ) { return a.transposed(); }\n";
	if ( myStdFuncsUsed.find( "mult_f3_f33" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_vec3f_t mult_f3_f33( const ctl_vec3f_t &a, const ctl_mat33f_t &b ) { return a * b; }\n";
	if ( myStdFuncsUsed.find( "mult_f3_f44" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_vec3f_t mult_f3_f44( const ctl_vec3f_t &a, const ctl_mat44f_t &b ) { return a * b; }\n";
	if ( myStdFuncsUsed.find( "mult_f_f3" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_vec3f_t mult_f_f3( ctl_number_t a, const ctl_vec3f_t &b ) { return a * b; }\n";
	if ( myStdFuncsUsed.find( "add_f3_f3" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_vec3f_t add_f3_f3( const ctl_vec3f_t &a, const ctl_vec3f_t &b ) { return a + b; }\n";
	if ( myStdFuncsUsed.find( "sub_f3_f3" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_vec3f_t sub_f3_f3( const ctl_vec3f_t &a, const ctl_vec3f_t &b ) { return a - b; }\n";
	if ( myStdFuncsUsed.find( "cross_f3_f3" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_vec3f_t cross_f3_f3( const ctl_vec3f_t &a, const ctl_vec3f_t &b ) { return a.cross( b ); }\n";
	if ( myStdFuncsUsed.find( "dot_f3_f3" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_number_t dot_f3_f3( const ctl_vec3f_t &a, const ctl_vec3f_t &b ) { return a.dot( b ); }\n";
	if ( myStdFuncsUsed.find( "length_f3" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_number_t length_f3( const ctl_vec3f_t &a ) { return a.length(); }\n";

	if ( myStdFuncsUsed.find( "XYZtoLuv" ) != myStdFuncsUsed.end() ||
		 myStdFuncsUsed.find( "LuvtoXYZ" ) != myStdFuncsUsed.end() ||
		 myStdFuncsUsed.find( "XYZtoLab" ) != myStdFuncsUsed.end() ||
		 myStdFuncsUsed.find( "LabtoXYZ" ) != myStdFuncsUsed.end() )
		libSetupB << "\n"
			"namespace {\n"
			"static inline ctl_number_t __cspace_f( ctl_number_t x ) { if ( x > ctl_number_t(0.008856) ) return pow( x, ctl_number_t(1.0 / 3.0) ); return ctl_number_t(7.787) * x + ctl_number_t(16.0 / 116.0); }\n"
			"static inline ctl_number_t __cspace_fInverse( ctl_number_t t ) { if ( t > ctl_number_t(0.206892) ) return t * t * t; return ctl_number_t(1.0/7.787) * ( t - ctl_number_t(16.0/116.0) ); }\n"
			"static inline ctl_number_t __cspace_uprime( const ctl_vec3f_t &XYZ ) { return ( XYZ.x * ctl_number_t(4) ) / ( XYZ.x + ctl_number_t(15) * XYZ.y + ctl_number_t(3) * XYZ.z ); }\n"
			"static inline ctl_number_t __cspace_vprime( const ctl_vec3f_t &XYZ ) { return ( XYZ.y * ctl_number_t(9) ) / ( XYZ.x + ctl_number_t(15) * XYZ.y + ctl_number_t(3) * XYZ.z ); }\n"
			"} // empty namespace\n\n";
	if ( myStdFuncsUsed.find( "RGBtoXYZ" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_mat44f_t RGBtoXYZ( const Chromaticities &chroma, ctl_number_t Y )\n"
			"{\n"
			"    static const ctl_number_t one = ctl_number_t(1);\n"
			"    ctl_number_t X = chroma.white.x * Y / chroma.white.y;\n"
			"    ctl_number_t Z = (one - chroma.white.x - chroma.white.y) * Y / chroma.white.y;\n"
			"    ctl_number_t d = chroma.red.x * (chroma.blue.y - chroma.green.y) + chroma.blue.x * (chroma.green.y - chroma.red.y) + chroma.green.x * (chroma.red.y - chroma.blue.y);\n"
			"    ctl_number_t Sr = (X * (chroma.blue.y - chroma.green.y) - chroma.green.x * (Y * (chroma.blue.y - one) + chroma.blue.y * (X + Z)) + chroma.blue.x * (Y * (chroma.green.y - one) + chroma.green.y * (X + Z))) / d;\n"
			"    ctl_number_t Sg = (X * (chroma.red.y - chroma.blue.y) + chroma.red.x * (Y * (chroma.blue.y - one) + chroma.blue.y * (X + Z)) - chroma.blue.x * (Y * (chroma.red.y - one) + chroma.red.y * (X + Z))) / d;\n"
			"    ctl_number_t Sb = (X * (chroma.green.y - chroma.red.y) - chroma.red.x * (Y * (chroma.green.y - one) + chroma.green.y * (X + Z)) + chroma.green.x * (Y * (chroma.red.y - one) + chroma.red.y * (X + Z))) / d;\n"
			"    ctl_mat44f_t M;\n"
			"    M[0][0] = Sr * chroma.red.x;\n"
			"    M[0][1] = Sr * chroma.red.y;\n"
			"    M[0][2] = Sr * (1 - chroma.red.x - chroma.red.y);\n"
			"    M[1][0] = Sg * chroma.green.x;\n"
			"    M[1][1] = Sg * chroma.green.y;\n"
			"    M[1][2] = Sg * (1 - chroma.green.x - chroma.green.y);\n"
			"    M[2][0] = Sb * chroma.blue.x;\n"
			"    M[2][1] = Sb * chroma.blue.y;\n"
			"    M[2][2] = Sb * (1 - chroma.blue.x - chroma.blue.y);\n"
			"    return M;\n"
			"}\n";
	if ( myStdFuncsUsed.find( "XYZtoRGB" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_mat44f_t XYZtoRGB( const Chromaticities &chroma, ctl_number_t Y ) { return RGBtoXYZ( chroma, Y ).inverse(); }\n";
	if ( myStdFuncsUsed.find( "XYZtoLuv" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_vec3f_t XYZtoLuv( const ctl_vec3f_t &XYZ, const ctl_vec3f_t &XYZn )\n"
			"{\n"
			"    ctl_number_t Lstar = ctl_number_t(116) * __cspace_f( XYZ.y / XYZn.y ) - ctl_number_t(16);\n"
			"    ctl_number_t ustar = ctl_number_t(13) * Lstar * ( __cspace_uprime( XYZ ) - __cspace_uprime( XYZn ) );\n"
			"    ctl_number_t vstar = ctl_number_t(13) * Lstar * ( __cspace_vprime( XYZ ) - __cspace_vprime( XYZn ) );\n"
			"    return ctl_vec3f_t( Lstar, ustar, vstar );\n"
			"}\n";
	if ( myStdFuncsUsed.find( "LuvtoXYZ" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_vec3f_t LuvtoXYZ( const ctl_vec3f_t &Luv, const ctl_vec3f_t &XYZn )\n"
			"{\n"
			"    ctl_number_t Lstar = Luv.x;\n"
			"    ctl_number_t ustar = Luv.y;\n"
			"    ctl_number_t vstar = Luv.z;\n"
			"    ctl_number_t unprime = __cspace_uprime( XYZn );\n"
			"    ctl_number_t vnprime = __cspace_vprime( XYZn );\n"
			"    ctl_number_t fY = (Lstar + ctl_number_t(16)) / ctl_number_t(116);\n"
			"    ctl_number_t Y = XYZn.y * __cspace_fInverse( fY );\n"
			"    ctl_number_t d = ctl_number_t(4) * (ctl_number_t(13) * Lstar * vnprime + vstar);\n"
			"    ctl_number_t X = ctl_number_t(9) * (ctl_number_t(13) * Lstar * unprime + ustar) * Y / d;\n"
			"    ctl_number_t Z = -( ctl_number_t(3) * ustar + ctl_number_t(13) * Lstar * ( ctl_number_t(-12) + ctl_number_t(3) * unprime + ctl_number_t(20) * vnprime ) + ctl_number_t(20) * vstar ) * Y / d;\n"
			"    return ctl_vec3f_t( X, Y, Z );\n"
			"}\n";
	if ( myStdFuncsUsed.find( "XYZtoLab" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_vec3f_t XYZtoLab( const ctl_vec3f_t &XYZ, const ctl_vec3f_t &XYZn )\n"
			"{\n"
			"    ctl_number_t tmpY = __cspace_f( XYZ.y / XYZn.y );\n"
			"    ctl_number_t Lstar = ctl_number_t(116) * tmpY - ctl_number_t(16);\n"
			"    ctl_number_t astar = ctl_number_t(500) * ( __cspace_f( XYZ.x / XYZn.x ) -  tmpY );\n"
			"    ctl_number_t bstar = ctl_number_t(200) * ( tmpY - __cspace_f( XYZ.z / XYZn.z ) );\n"
			"    return ctl_vec3f_t( Lstar, astar, bstar );\n"
			"}\n";
	if ( myStdFuncsUsed.find( "LabtoXYZ" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline ctl_vec3f_t LabtoXYZ( const ctl_vec3f_t &Lab, const ctl_vec3f_t &XYZn )\n"
			"{\n"
			"    ctl_number_t Lstar = Lab.x;\n"
			"    ctl_number_t astar = Lab.y;\n"
			"    ctl_number_t bstar = Lab.z;\n"
			"    ctl_number_t fY = (Lstar + ctl_number_t(16)) / ctl_number_t(116);\n"
			"    ctl_number_t fX = astar / ctl_number_t(500) + fY;\n"
			"    ctl_number_t fZ = fY - bstar / ctl_number_t(200);\n"
			"    ctl_number_t X = XYZn.x * __cspace_fInverse( fX );\n"
			"    ctl_number_t Y = XYZn.y * __cspace_fInverse( fY );\n"
			"    ctl_number_t Z = XYZn.z * __cspace_fInverse( fZ );\n"
			"    return ctl_vec3f_t( X, Y, Z );\n"
			"}\n";
	if ( myStdFuncsUsed.find( "lookup1D" ) != myStdFuncsUsed.end() )
		libSetupB << "\n"
			"static inline ctl_number_t lookup1D( ctl_number_t table[], int size, ctl_number_t pMin, ctl_number_t pMax, ctl_number_t p )\n"
			"{\n"
			"    int iMax = size - 1;\n"
			"    ctl_number_t r = ( clamp( p, pMin, pMax ) - pMin ) / ( pMax - pMin ) * iMax;\n"
			"    int i = static_cast<int>( r );\n"
			"    ctl_number_t u = r - static_cast<ctl_number_t>( i );\n"
			"    ctl_number_t t0 = table[i];\n"
			"    ctl_number_t t1 = table[std::min( i + 1, iMax )];\n"
			"    return t0 + u * ( t1 - t0 );\n"
			"}\n"
			"template <typename T>\n"
			"static inline ctl_number_t lookup1D( const T &x, int size, ctl_number_t pMin, ctl_number_t pMax, ctl_number_t p )\n"
			"{\n"
			"    return lookup1D( &x[0], size, pMin, pMax, p );\n"
			"}\n";
	if ( myStdFuncsUsed.find( "lookupCubic1D" ) != myStdFuncsUsed.end() )
		libSetupB << "\n"
			"static inline ctl_number_t lookupCubic1D( ctl_number_t table[], int size, ctl_number_t pMin, ctl_number_t pMax, ctl_number_t p )\n"
			"{\n"
			"    if ( size < 3 ) return lookup1D( table, size, pMin, pMax, p );\n"
			"    int iMax = size - 1;\n"
			"    ctl_number_t r = ( clamp( p, pMin, pMax ) - pMin ) / ( pMax - pMin ) * iMax;\n"
			"    if ( r >= iMax ) return table[iMax];\n"
			"    int i = static_cast<int>( r );\n"
			"    const ctl_number_t kHalf = ctl_number_t(0.5);\n"
			"    const ctl_number_t kOne = ctl_number_t(1);\n"
			"    const ctl_number_t kTwo = ctl_number_t(2);\n"
			"    const ctl_number_t kThree = ctl_number_t(3);\n"
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
			"    ctl_number_t t = r - static_cast<ctl_number_t>( i );\n"
			"    ctl_number_t t2 = t * t;\n"
			"    ctl_number_t t3 = t2 * t;\n"
			"    return ( table[i] * (kTwo * t3 - kThree * t2 + kOne) +\n"
			"             m0 * ( t3 - kTwo * t2 + t ) +\n"
			"             table[i+1] * ( kThree * t2 - kTwo * t3 ) +\n"
			"             m1 * ( t3 - t2 ) );\n"
			"}\n"
			"\n"
			"template <typename T>\n"
			"static inline ctl_number_t lookupCubic1D( const T &x, int size, ctl_number_t pMin, ctl_number_t pMax, ctl_number_t p )\n"
			"{\n"
			"    return lookupCubic1D( &x[0], size, pMin, pMax, p );\n"
			"}\n";
	if ( myStdFuncsUsed.find( "lookup3D_f3" ) != myStdFuncsUsed.end() ||
		 myStdFuncsUsed.find( "lookup3D_f" ) != myStdFuncsUsed.end() ||
		 myStdFuncsUsed.find( "lookup3D_h" ) != myStdFuncsUsed.end() )
		libSetupB << "\n"
			"static inline ctl_vec3f_t lookup3D_f3( const ctl_number_t table[], const ctl_vec3i_t &size, const ctl_vec3f_t &pMin, const ctl_vec3f_t &pMax, const ctl_vec3f_t &p )\n"
			"{\n"
			"    int iMax = size.x - 1;\n"
			"    ctl_number_t r = ( clamp( p.x, pMin.x, pMax.x ) - pMin.x ) / ( pMax.x - pMin.x ) * iMax;\n"
			"    int jMax = size.y - 1;\n"
			"    ctl_number_t s = ( clamp( p.y, pMin.y, pMax.y ) - pMin.y ) / ( pMax.y - pMin.y ) * jMax;\n"
			"    int kMax = size.z - 1;\n"
			"    ctl_number_t t = ( clamp( p.z, pMin.z, pMax.z ) - pMin.z ) / ( pMax.z - pMin.z ) * iMax;\n"
			"    int i = static_cast<int>( r );\n"
			"    int i1 = std::min( i + 1, iMax );\n"
			"    ctl_number_t u = r - static_cast<ctl_number_t>( i );\n"
			"    ctl_number_t u1 = ctl_number_t(1) - u;\n"
			"    int j = static_cast<int>( s );\n"
			"    int j1 = std::min( j + 1, jMax );\n"
			"    ctl_number_t v = s - static_cast<ctl_number_t>( j );\n"
			"    ctl_number_t v1 = ctl_number_t(1) - v;\n"
			"    int k = static_cast<int>( t );\n"
			"    int k1 = std::min( k + 1, kMax );\n"
			"    ctl_number_t w = t - static_cast<ctl_number_t>( k );\n"
			"    ctl_number_t w1 = ctl_number_t(1) - w;\n"
			"    const ctl_number_t *aP = table + (( i * size.y + j ) * size.z + k)*3;\n"
			"    const ctl_number_t *bP = table + (( i1 * size.y + j ) * size.z + k)*3;\n"
			"    const ctl_number_t *cP = table + (( i * size.y + j1 ) * size.z + k)*3;\n"
			"    const ctl_number_t *dP = table + (( i1 * size.y + j1 ) * size.z + k)*3;\n"
			"    const ctl_number_t *eP = table + (( i * size.y + j ) * size.z + k1)*3;\n"
			"    const ctl_number_t *fP = table + (( i1 * size.y + j ) * size.z + k1)*3;\n"
			"    const ctl_number_t *gP = table + (( i * size.y + j1 ) * size.z + k1)*3;\n"
			"    const ctl_number_t *hP = table + (( i1 * size.y + j1 ) * size.z + k1)*3;\n"
			"    const ctl_vec3f_t a = ctl_vec3f_t( aP[0], aP[1], aP[2] );\n"
			"    const ctl_vec3f_t b = ctl_vec3f_t( bP[0], bP[1], bP[2] );\n"
			"    const ctl_vec3f_t c = ctl_vec3f_t( cP[0], cP[1], cP[2] );\n"
			"    const ctl_vec3f_t d = ctl_vec3f_t( dP[0], dP[1], dP[2] );\n"
			"    const ctl_vec3f_t e = ctl_vec3f_t( eP[0], eP[1], eP[2] );\n"
			"    const ctl_vec3f_t f = ctl_vec3f_t( fP[0], fP[1], fP[2] );\n"
			"    const ctl_vec3f_t g = ctl_vec3f_t( gP[0], gP[1], gP[2] );\n"
			"    const ctl_vec3f_t h = ctl_vec3f_t( hP[0], hP[1], hP[2] );\n"
			"    return ( w1 * ( v1 * ( u1 * a + u * b ) + v * ( u1 * c + u * d ) ) +\n"
			"             w * ( v1 * ( u1 * e + u * f ) + v * ( u1 * g + u * h ) ) );\n"
			"}\n";
	if ( myStdFuncsUsed.find( "lookup3D_f" ) != myStdFuncsUsed.end() )
		libSetupB << "\n"
			"static inline void lookup3D_f( const ctl_number_t table[], const ctl_vec3i_t &size, const ctl_vec3f_t &pMin, const ctl_vec3f_t &pMax, ctl_number_t p0, ctl_number_t p1, ctl_number_t p2, ctl_number_t &o0, ctl_number_t &o1, ctl_number_t &o2 )\n"
			"{\n"
			"    ctl_vec3f_t out = lookup3D_f3( table, size, pMin, pMax, ctl_vec3f_t( p0, p1, p2 ) );\n"
			"    o0 = out[0];\n"
			"    o1 = out[1];\n"
			"    o2 = out[2];\n"
			"}\n"
			"\n";
	if ( myStdFuncsUsed.find( "lookup3D_h" ) != myStdFuncsUsed.end() )
		libSetupB << "static inline void lookup3D_h( const ctl_number_t table[], const ctl_vec3i_t &size, const ctl_vec3f_t &pMin, const ctl_vec3f_t &pMax, const half &p0, const half &p1, const half &p2, half &o0, half &o1, half &o2 )\n"
			"{\n"
			"    ctl_vec3f_t out = lookup3D_f3( table, size, pMin, pMax, ctl_vec3f_t( p0, p1, p2 ) );\n"
			"    o0 = out[0];\n"
			"    o1 = out[1];\n"
			"    o2 = out[2];\n"
			"}\n";
	if ( myStdFuncsUsed.find( "interpolate1D" ) != myStdFuncsUsed.end() )
		libSetupB << "\n"
			"static inline ctl_number_t interpolate1D( const ctl_number_t table[][2], int size, ctl_number_t p )\n"
			"{\n"
			"    if ( size < 1 ) return ctl_number_t(0);\n"
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
			"    ctl_number_t x = table[i][1];\n"
			"    return x + t * ( table[i + 1][1] - x );\n"
			"}\n";
	if ( myStdFuncsUsed.find( "interpolateCubic1D" ) != myStdFuncsUsed.end() )
		libSetupB << "\n"
			"static inline ctl_number_t interpolateCubic1D( const ctl_number_t table[][2], int size, ctl_number_t p )\n"
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
			"}\n";

	if ( myStdFuncsUsed.find( "scatteredDataToGrid3D" ) != myStdFuncsUsed.end() )
		throw std::logic_error( "Sorry, the code generation for scatteredDataToGrid3D is not yet complete" );

	libSetupB <<
		"} // namespace _ctlcc_\n\n";

	return libSetupB.str();
}


////////////////////////////////////////


bool
CPPGenerator::usesFunctionInitializers( void ) const
{
	return ! myCPP11Mode;
}


////////////////////////////////////////


bool
CPPGenerator::supportsModuleDynamicInitialization( void ) const
{
	return true;
}


////////////////////////////////////////


bool
CPPGenerator::supportsNamespaces( void ) const
{
	return true;
}


////////////////////////////////////////


bool
CPPGenerator::supportsHalfType( void ) const
{
	return true;
}


////////////////////////////////////////


std::string
CPPGenerator::constructNamespaceTag( const std::string &modName )
{
	return modName + "::";
}


////////////////////////////////////////


const std::string &
CPPGenerator::getInlineKeyword( void ) const
{
	static std::string kInline = "inline";
	return kInline;
}


////////////////////////////////////////


const std::string &
CPPGenerator::getFunctionPrefix( void ) const
{
	static std::string kPrefix = "static";
	return kPrefix;
}


////////////////////////////////////////


const std::string &
CPPGenerator::getGlobalPrefix( void ) const
{
	static std::string kGlobal;
	return kGlobal;
}


////////////////////////////////////////


const std::string &
CPPGenerator::getCTLNamespaceTag( void ) const
{
	static std::string kTag = "_ctlcc_::";
	return kTag;
}


////////////////////////////////////////


const std::string &
CPPGenerator::getBoolTypeName( void ) const
{
	static std::string kType = "bool";
	return kType;
}


////////////////////////////////////////


const std::string &
CPPGenerator::getBoolLiteral( bool v ) const
{
	static std::string kBoolTrue = "true";
	static std::string kBoolFalse = "false";
	if ( v )
		return kBoolTrue;
	return kBoolFalse;
}


////////////////////////////////////////


const std::string &
CPPGenerator::getConstLiteral( void ) const
{
	static std::string kConst = "const";
	return kConst;
}


////////////////////////////////////////


void
CPPGenerator::startCast( const char *type )
{
	curStream() << "static_cast<" << type << ">( ";
}

} // namespace Ctl

