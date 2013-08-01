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

namespace Ctl
{

CPPGenerator::CPPGenerator( bool cpp11 )
		: LanguageGenerator(),
		  myCPP11Mode( cpp11 ),
		  myInElse( 0 ),
		  myInModuleInit( 0 ), myInFunction( 0 ),
		  myDoForwardDecl( 0 ),
		  myCurInitType( NONE ), myCurModule( NULL )
{
}


////////////////////////////////////////


CPPGenerator::~CPPGenerator( void )
{
}


////////////////////////////////////////


std::string
CPPGenerator::getHeaderCode( void )
{
	return myHeaderStream.str();
}


////////////////////////////////////////


std::string
CPPGenerator::getCode( void )
{
	return myCodeStream.str();
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
		"typedef IMATH_NAMESPACE::Vec3<ctl_number_t> ctl_vec3f_t;\n"
		"typedef IMATH_NAMESPACE::Vec4<ctl_number_t> ctl_vec4f_t;\n"
		"typedef IMATH_NAMESPACE::V2i ctl_vec2i_t;\n"
		"typedef IMATH_NAMESPACE::V3i ctl_vec3i_t;\n"
		"typedef IMATH_NAMESPACE::V3i ctl_vec4i_t;\n"
		"typedef IMATH_NAMESPACE::Matrix33<ctl_number_t> ctl_mat33f_t;\n"
		"typedef IMATH_NAMESPACE::Matrix44<ctl_number_t> ctl_mat44f_t;\n"
		"\n"
		"namespace _ctlcc_ {\n"
		"\n"
		"using Imath::clamp;\n"
		"\n"
		"struct Chromaticities { ctl_vec2f_t red; ctl_vec2f_t green; ctl_vec2f_t blue; ctl_vec2f_t white; };\n"
		"struct Box2i { ctl_vec2i_t min; ctl_vec2i_t max; };\n"
		"struct Box2f { ctl_vec2f_t min; ctl_vec2f_t max; };\n\n"
		"static inline void assert( bool v ) { if (!v) throw std::logic_error( \"Assertion failure\" ); }\n"
		"\n"
		"static inline void print_bool( bool v ) { std::cout << (v ? \"true\" : \"false\"); }\n"
		"static inline void print_int( int v ) { std::cout << v; }\n"
		"static inline void print_unsigned_int( unsigned int v ) { std::cout << v; }\n"
		"static inline void print_half( half v ) { std::cout << v; }\n"
		"static inline void print_float( ctl_number_t v ) { std::cout << v; }\n"
		"static inline void print_string( const std::string &v ) { std::cout << v; }\n"
		"static inline void print_string( const char *v ) { std::cout << v; }\n"
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
		"\n"
		"static inline ctl_mat33f_t mult_f33_f33( const ctl_mat33f_t &a, const ctl_mat33f_t &b ) { return a * b; }\n"
		"static inline ctl_mat44f_t mult_f44_f44( const ctl_mat44f_t &a, const ctl_mat44f_t &b ) { return a * b; }\n"
		"static inline ctl_mat33f_t mult_f_f33( ctl_number_t a, const ctl_mat33f_t &b ) { return a * b; }\n"
		"static inline ctl_mat44f_t mult_f_f44( ctl_number_t a, const ctl_mat44f_t &b ) { return a * b; }\n"
		"static inline ctl_mat33f_t add_f33_f33( const ctl_mat33f_t &a, const ctl_mat33f_t &b ) { return a + b; }\n"
		"static inline ctl_mat44f_t add_f44_f44( const ctl_mat44f_t &a, const ctl_mat44f_t &b ) { return a + b; }\n"
		"static inline ctl_mat33f_t invert_f33( const ctl_mat33f_t &a ) { return a.inverse(); }\n"
		"static inline ctl_mat44f_t invert_f44( const ctl_mat44f_t &a ) { return a.inverse(); }\n"
		"static inline ctl_mat33f_t transpose_f33( const ctl_mat33f_t &a ) { return a.transposed(); }\n"
		"static inline ctl_mat44f_t transpose_f44( const ctl_mat44f_t &a ) { return a.transposed(); }\n"
		"static inline ctl_vec3f_t mult_f3_f33( const ctl_vec3f_t &a, const ctl_mat33f_t &b ) { return a * b; }\n"
		"static inline ctl_vec3f_t mult_f3_f44( const ctl_vec3f_t &a, const ctl_mat44f_t &b ) { return a * b; }\n"
		"static inline ctl_vec3f_t mult_f_f3( ctl_number_t a, const ctl_vec3f_t &b ) { return a * b; }\n"
		"static inline ctl_vec3f_t add_f3_f3( const ctl_vec3f_t &a, const ctl_vec3f_t &b ) { return a + b; }\n"
		"static inline ctl_vec3f_t sub_f3_f3( const ctl_vec3f_t &a, const ctl_vec3f_t &b ) { return a - b; }\n"
		"static inline ctl_vec3f_t cross_f3_f3( const ctl_vec3f_t &a, const ctl_vec3f_t &b ) { return a.cross( b ); }\n"
		"static inline ctl_number_t dot_f3_f3( const ctl_vec3f_t &a, const ctl_vec3f_t &b ) { return a.dot( b ); }\n"
		"static inline ctl_number_t length_f3( const ctl_vec3f_t &a ) { return a.length(); }\n"
		"\n"
		"namespace {\n"
		"static inline ctl_number_t __cspace_f( ctl_number_t x ) { if ( x > ctl_number_t(0.008856) ) return pow( x, ctl_number_t(1.0 / 3.0) ); return ctl_number_t(7.787) * x + ctl_number_t(16.0 / 116.0); }\n"
		"static inline ctl_number_t __cspace_fInverse( ctl_number_t t ) { if ( t > ctl_number_t(0.206892) ) return t * t * t; return ctl_number_t(1.0/7.787) * ( t - ctl_number_t(16.0/116.0) ); }\n"
		"static inline ctl_number_t __cspace_uprime( const ctl_vec3f_t &XYZ ) { return ( XYZ.x * ctl_number_t(4) ) / ( XYZ.x + ctl_number_t(15) * XYZ.y + ctl_number_t(3) * XYZ.z ); }\n"
		"static inline ctl_number_t __cspace_vprime( const ctl_vec3f_t &XYZ ) { return ( XYZ.y * ctl_number_t(9) ) / ( XYZ.x + ctl_number_t(15) * XYZ.y + ctl_number_t(3) * XYZ.z ); }\n"
		"} // empty namespace\n\n"
		"static inline ctl_mat44f_t RGBtoXYZ( const Chromaticities &chroma, ctl_number_t Y )\n"
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
		"}\n"
		"static inline ctl_mat44f_t XYZtoRGB( const Chromaticities &chroma, ctl_number_t Y ) { return RGBtoXYZ( chroma, Y ).inverse(); }\n"
		"static inline ctl_vec3f_t XYZtoLuv( const ctl_vec3f_t &XYZ, const ctl_vec3f_t &XYZn )\n"
		"{\n"
		"    ctl_number_t Lstar = ctl_number_t(116) * __cspace_f( XYZ.y / XYZn.y ) - ctl_number_t(16);\n"
		"    ctl_number_t ustar = ctl_number_t(13) * Lstar * ( __cspace_uprime( XYZ ) - __cspace_uprime( XYZn ) );\n"
		"    ctl_number_t vstar = ctl_number_t(13) * Lstar * ( __cspace_vprime( XYZ ) - __cspace_vprime( XYZn ) );\n"
		"    return ctl_vec3f_t( Lstar, ustar, vstar );\n"
		"}\n"
		"static inline ctl_vec3f_t LuvtoXYZ( const ctl_vec3f_t &Luv, const ctl_vec3f_t &XYZn )\n"
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
		"}\n"
		"static inline ctl_vec3f_t XYZtoLab( const ctl_vec3f_t &XYZ, const ctl_vec3f_t &XYZn )\n"
		"{\n"
		"    ctl_number_t tmpY = __cspace_f( XYZ.y / XYZn.y );\n"
		"    ctl_number_t Lstar = ctl_number_t(116) * tmpY - ctl_number_t(16);\n"
		"    ctl_number_t astar = ctl_number_t(500) * ( __cspace_f( XYZ.x / XYZn.x ) -  tmpY );\n"
		"    ctl_number_t bstar = ctl_number_t(200) * ( tmpY - __cspace_f( XYZ.z / XYZn.z ) );\n"
		"    return ctl_vec3f_t( Lstar, astar, bstar );\n"
		"}\n"
		"static inline ctl_vec3f_t LabtoXYZ( const ctl_vec3f_t &Lab, const ctl_vec3f_t &XYZn )\n"
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
		"}\n"
		"\n"
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
		"\n"
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
		"static inline ctl_vec3f_t lookup3D( const ctl_vec3f_t table[], const ctl_vec3i_t &size, const ctl_vec3f_t &pMin, const ctl_vec3f_t &pMax, const ctl_vec3f_t &p )\n"
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
		"    const ctl_vec3f_t &a = table[( i * size.y + j ) * size.z + k];\n"
		"    const ctl_vec3f_t &b = table[( i1 * size.y + j ) * size.z + k];\n"
		"    const ctl_vec3f_t &c = table[( i * size.y + j1 ) * size.z + k];\n"
		"    const ctl_vec3f_t &d = table[( i1 * size.y + j1 ) * size.z + k];\n"
		"    const ctl_vec3f_t &e = table[( i * size.y + j ) * size.z + k1];\n"
		"    const ctl_vec3f_t &f = table[( i1 * size.y + j ) * size.z + k1];\n"
		"    const ctl_vec3f_t &g = table[( i * size.y + j1 ) * size.z + k1];\n"
		"    const ctl_vec3f_t &h = table[( i1 * size.y + j1 ) * size.z + k1];\n"
		"    return ( w1 * ( v1 * ( u1 * a + u * b ) + v * ( u1 * c + u * d ) ) +\n"
		"             w * ( v1 * ( u1 * e + u * f ) + v * ( u1 * g + u * h ) ) );\n"
		"}\n"
		"\n"
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
		"}\n"
		"\n"
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
		"}\n"
		"} // namespace _ctlcc_\n\n";

	return libSetupB.str();
}


////////////////////////////////////////


void
CPPGenerator::pushBlock( void )
{
	newlineAndIndent();
	curStream() << '{';
	pushIndent();
}


////////////////////////////////////////


void
CPPGenerator::popBlock( void )
{
	popIndent();
	newlineAndIndent();
	curStream() << '}';
}


////////////////////////////////////////


void
CPPGenerator::module( CodeLContext &ctxt, const CodeModuleNode &m )
{
	Module *oldModule = myCurModule;
	myCurModule = ctxt.module();
	std::string mName = cleanName( myCurModule->name() );

	extractLiteralConstants( m.constants, ctxt );
	pushStream( myCodeStream );

	newlineAndIndent();
	curStream() << "// Module " << ctxt.module()->name() << " ("
				<< ctxt.fileName() << ")\n";
	newlineAndIndent();

	curStream() << "namespace " << mName << " {";
	newlineAndIndent();

	++myInModuleInit;
	myCurModuleInit.push_back( std::vector<std::string>() );

	const std::vector< std::pair< std::string, MemberVector > > &theStructs =
		ctxt.structs();
	if ( ! theStructs.empty() )
	{
		for ( size_t i = 0, N = theStructs.size(); i != N; ++i )
		{
			const std::pair< std::string, MemberVector > &s = theStructs[i];
			std::string n = removeNSQuals( s.first );
			newlineAndIndent();
			curStream() << "struct " << n;
			pushBlock();
			for ( size_t m = 0, M = s.second.size(); m != M; ++m )
			{
				newlineAndIndent();
				const Member &mem = s.second[m];
				variable( ctxt, mem.name, mem.type, false, false, false );
				curStream() << ';';
			}
			popBlock();
			curStream() << ";\n";
		}
	}

	// Forward declare any functions used in initializing variables
	FunctionNodePtr function = m.functions;
	++myDoForwardDecl;
	while ( function )
	{
		function->generateCode( ctxt );
		function = function->next;
	}
	--myDoForwardDecl;
	newlineAndIndent();

	StatementNodePtr consts = m.constants;
	while ( consts )
	{
		consts->generateCode( ctxt );
		consts = consts->next;
	}

	if ( m.constants )
		newlineAndIndent();
	--myInModuleInit;

	if ( ! myCurModuleInit.back().empty() )
	{
		const std::vector<std::string> &initVals = myCurModuleInit.back();

		newlineAndIndent();
		curStream() << "struct __ctlcc_InitVals_" << mName;
		pushBlock();
		newlineAndIndent();
		curStream() << "__ctlcc_InitVals_" << mName << "( void )";
		pushBlock();
		for ( size_t i = 0, N = initVals.size(); i != N; ++i )
			curStream() << initVals[i] << '\n';
		newlineAndIndent();
		popBlock();
		popBlock();
		curStream() << ";";
		newlineAndIndent();
		curStream() << "static __ctlcc_InitVals_" << mName << " __ctlcc_GlobalInitializer_" << mName << ";\n\n";
	}

	function = m.functions;
	while ( function )
	{
		function->generateCode( ctxt );
		function = function->next;
	}

	newlineAndIndent();
	curStream() << "} // namespace " << mName;
	newlineAndIndent();

	myCurModuleInit.pop_back();
	popStream();

	myCurModule = oldModule;
}


////////////////////////////////////////


void
CPPGenerator::function( CodeLContext &ctxt, const CodeFunctionNode &f )
{
	CodeFunctionTypePtr functionType = f.info->functionType();
	const ParamVector &params = functionType->parameters();

	if ( functionType->returnVarying() )
		std::cout << "Check on what it means to return varying..." << std::endl;

	if ( myDoForwardDecl > 0 )
	{
		// if we aren't used in initializing constants, we can be
		// declared inline and don't need a forward decl
		if ( myFuncsUsedInInit.find( f.name ) == myFuncsUsedInInit.end() )
			return;
	}

	std::string funcName = removeNSQuals( f.name );
	bool isMain = funcName == ctxt.module()->name();
	if ( funcName == "main" )
	{
		funcName = myCurModule->name();
		isMain = true;
	}

	if ( isMain )
	{
		std::string nsName = myCurModule->name() + "::" + funcName;
		registerMainRoutine( funcName, nsName, f.info );

		// and put it in a header file in case someone cares
		// about that
		pushStream( myHeaderStream );
		newlineAndIndent();
		variable( ctxt, std::string(), functionType->returnType(),
				  false, false, false );

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
					  parm.access == RWA_WRITE || parm.access == RWA_READWRITE );

			SizeVector sizes;
			checkNeedsSizeArgument( parm.type, parm.name, sizes );
		}
		curStream() << " );";
		popStream();
	}
	else if ( myDoForwardDecl > 0 )
	{
		newlineAndIndent();
		curStream() << "static ";
		variable( ctxt, std::string(), functionType->returnType(),
				  false, false, false );

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
					  parm.access == RWA_WRITE || parm.access == RWA_READWRITE );

			SizeVector sizes;
			checkNeedsSizeArgument( parm.type, parm.name, sizes );
		}
		curStream() << " );";
		return;
	}

	newlineAndIndent();
	if ( ! isMain )
	{
		curStream() << "static ";

		if ( myFuncsUsedInInit.find( f.name ) == myFuncsUsedInInit.end() )
			curStream() << "inline ";
	}

	variable( ctxt, std::string(), functionType->returnType(),
			  false, false, false );
	newlineAndIndent();

	curStream() << funcName << "( ";
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
				  parm.access == RWA_WRITE || parm.access == RWA_READWRITE );

		SizeVector sizes;
		checkNeedsSizeArgument( parm.type, parm.name, sizes );
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
}


////////////////////////////////////////


void
CPPGenerator::variable( CodeLContext &ctxt, const CodeVariableNode &v )
{
	if ( myInModuleInit > 0 )
	{
		std::map<std::string, std::string>::const_iterator i = myGlobalLiterals.find( v.name );

		// We'll just put the literal in directly
		if ( i != myGlobalLiterals.end() )
			return;

		bool doConst = ! v.info->isWritable();
		bool overrideInit = false;
		if ( ! myCPP11Mode )
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

		std::stringstream varDeclB;
		pushStream( varDeclB );
		InitType initT = variable( ctxt, v.name, v.info->type(),
								   doConst, false, false );
		popStream();
		std::string varDecl = varDeclB.str();
		bool doEmit = true;
		if ( overrideInit )
			initT = FUNC;

		if ( v.name.find( '$' ) != std::string::npos )
		{
			varDecl = v.name;

			std::string initVal;
			if ( v.initialValue )
			{
				if ( initT == FUNC )
				{
					if ( ! v.initialValue.cast<NameNode>() )
						throw std::logic_error( "NYI: complex default value handling for function type" );
				}
				std::stringstream initB;
				pushStream( initB );
				myCurInitType = initT;
				v.initialValue->generateCode( ctxt );
				myCurInitType = NONE;
				popStream();
				initVal = initB.str();
				if ( initT == CTOR )
				{
					std::stringstream typeB;
					pushStream( typeB );
					variable( ctxt, std::string(), v.info->type(),
							  false, false, false );
					popStream();
					initVal = typeB.str() + "( " + initVal + " )";
				}
			}
			myDefaultMappings[varDecl] = initVal;
			doEmit = false;
		}

		if ( doEmit )
		{
			myGlobalInitType[v.name] = initT;
			myGlobalVariables.insert( v.name );
			newlineAndIndent();
			curStream() << varDecl;
			doInit( initT, ctxt, v.initialValue, v.name );
		}
	}
	else
	{
		newlineAndIndent();
		InitType initT = variable( ctxt, v.name, v.info->type(),
								   ! v.info->isWritable(), false, false );

		doInit( initT, ctxt, v.initialValue, v.name );
	}
}


////////////////////////////////////////


void
CPPGenerator::assignment( CodeLContext &ctxt, const CodeAssignmentNode &v )
{
	newlineAndIndent();
	v.lhs->generateCode( ctxt );
	curStream() << " = ";
	v.lhs->type->generateCastFrom( v.rhs, ctxt );
	curStream() << ';';
}


////////////////////////////////////////


void
CPPGenerator::expr( CodeLContext &ctxt, const CodeExprStatementNode &v )
{
	newlineAndIndent();
	v.expr->generateCode( ctxt );
	curStream() << ';';
}


////////////////////////////////////////


void CPPGenerator::cond( CodeLContext &ctxt, const CodeIfNode &v )
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
CPPGenerator::retval( CodeLContext &ctxt, const CodeReturnNode &v )
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
CPPGenerator::loop( CodeLContext &ctxt, const CodeWhileNode &v )
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
CPPGenerator::binaryOp( CodeLContext &ctxt, const CodeBinaryOpNode &v )
{
	// operator precedence in CTL is same as C++, but we will have
	// lost any parens during the parse stage, so we should
	// introduce them all the time just in case
	curStream() << '(';
	v.operandType->generateCastFrom( v.leftOperand, ctxt );
	curStream() << ' ';
	v.operandType->generateCode( const_cast<CodeBinaryOpNode *>( &v ), ctxt );
	curStream() << ' ';
	v.operandType->generateCastFrom( v.rightOperand, ctxt );
	curStream() << ')';
}


////////////////////////////////////////


void
CPPGenerator::unaryOp( CodeLContext &ctxt, const CodeUnaryOpNode &v )
{
	v.type->generateCode( const_cast<CodeUnaryOpNode *>( &v ), ctxt );
	v.type->generateCastFrom( v.operand, ctxt );
}


////////////////////////////////////////


void
CPPGenerator::index( CodeLContext &ctxt, const CodeArrayIndexNode &v )
{
	v.array->generateCode( ctxt );
	IntTypePtr intType = ctxt.newIntType();
	curStream() << '[';
	intType->generateCastFrom( v.index, ctxt );
	curStream() << ']';
}


////////////////////////////////////////


void
CPPGenerator::member( CodeLContext &ctxt, const CodeMemberNode &v )
{
	v.obj->generateCode( ctxt );
	curStream() << '.' << v.member;
}


////////////////////////////////////////


void
CPPGenerator::size( CodeLContext &ctxt, const CodeSizeNode &v )
{
	std::cout << "Need to check if it's a function parameter and retrieve <name>_size, or just extract the size value and inject it" << std::endl;
	throw std::logic_error( "Function not yet implemented" );
}


////////////////////////////////////////


void
CPPGenerator::name( CodeLContext &ctxt, const CodeNameNode &v )
{
	std::map<std::string, std::string>::const_iterator i = myGlobalLiterals.find( v.name );
	if ( i != myGlobalLiterals.end() )
		curStream() << i->second;
	else if ( v.info->isFunction() )
	{
		const Module *m = v.info->module();
		if ( m == myCurModule )
			curStream() << removeNSQuals( v.name );
		else if ( m )
			curStream() << cleanName( m->name() ) << "::" << removeNSQuals( v.name );
		else
			curStream() << "_ctlcc_::" << removeNSQuals( v.name );
	}
	else
	{
		if ( myGlobalVariables.find( v.name ) != myGlobalVariables.end() )
		{
			const Module *m = v.info->module();
			if ( m == myCurModule )
				curStream() << removeNSQuals( v.name );
			else if ( m )
				curStream() << cleanName( m->name() ) << "::" << removeNSQuals( v.name );
			else
			{
				// these are currently all #defines, so no namespace
				curStream() << removeNSQuals( v.name );
			}
		}
		else
			curStream() << removeNSQuals( v.name );
	}
}


////////////////////////////////////////


void
CPPGenerator::boolLit( CodeLContext &ctxt, const CodeBoolLiteralNode &v )
{
	if ( v.value )
		curStream() << "true";
	else
		curStream() << "false";
}


////////////////////////////////////////


void
CPPGenerator::intLit( CodeLContext &ctxt, const CodeIntLiteralNode &v )
{
	curStream() << v.value;
}


////////////////////////////////////////


void
CPPGenerator::uintLit( CodeLContext &ctxt, const CodeUIntLiteralNode &v )
{
	curStream() << v.value;
}


////////////////////////////////////////


void
CPPGenerator::halfLit( CodeLContext &ctxt, const CodeHalfLiteralNode &v )
{
	curStream() << "half( " << std::setprecision( std::numeric_limits<half>::digits ) << static_cast<float>( v.value ) << " )";
}


////////////////////////////////////////


void
CPPGenerator::floatLit( CodeLContext &ctxt, const CodeFloatLiteralNode &v )
{
	curStream() << std::setprecision( std::numeric_limits<float>::digits ) << static_cast<float>( v.value );
}


////////////////////////////////////////


void
CPPGenerator::stringLit( CodeLContext &ctxt, const CodeStringLiteralNode &v )
{
	curStream() << '"' << escapeLiteral( v.value ) << '"';
}


////////////////////////////////////////


static bool
isStdPrintFunction( const NameNodePtr &n )
{
	return ( n->name == "::print_bool" ||
			 n->name == "::print_int" ||
			 n->name == "::print_unsigned_int" ||
			 n->name == "::print_half" ||
			 n->name == "::print_float" ||
			 n->name == "::print_string" );
}

////////////////////////////////////////


void
CPPGenerator::call( CodeLContext &ctxt, const CodeCallNode &v )
{
	bool isPrint = isStdPrintFunction( v.function );

	if ( isPrint )
		newlineAndIndent();

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
				throw std::logic_error( "Too many arguments in function call" );

			if ( i > 0 )
				curStream() << ", ";

			parameters[i].type->generateCastFrom( v.arguments[i], ctxt );
			SizeVector sizes;
			if ( checkNeedsSizeArgument( parameters[i].type, std::string(), sizes ) )
				extractSizeAndAdd( v.arguments[i], sizes, ctxt );
		}
		for ( size_t N = parameters.size(); i < N; ++i )
		{
			const Param &parm = parameters[i];

			if ( i > 0 )
				curStream() << ", ";

			if ( ! parm.defaultValue )
				throw std::logic_error( "Missing argument in function call (no default value)" );

			NameNodePtr n = parm.defaultValue.cast<NameNode>();
			std::string defVal;
			std::string namesp;
			if ( n )
			{
				const Module *m = n->info->module();
				if ( m && m != myCurModule )
					namesp = cleanName( m->name() ) + "::";

				defVal = n->name;
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
				curStream() << namesp << found->second;
			else
				curStream() << defVal;

			SizeVector sizes;
			if ( checkNeedsSizeArgument( parm.type, std::string(), sizes ) )
			{
				curStream() << ", ";
				extractSizeAndAdd( parm.defaultValue, sizes, ctxt );
			}
		}
		curStream() << " )";
	}
	if ( isPrint )
		curStream() << ';';
}


////////////////////////////////////////


void
CPPGenerator::value( CodeLContext &ctxt, const CodeValueNode &v )
{
	size_t idx = 0;
	valueRecurse( ctxt, v.elements, v.type, idx, "$$$$" );
}


////////////////////////////////////////


void
CPPGenerator::startToBool( void )
{
	curStream() << "static_cast<bool>( ";
}


////////////////////////////////////////


void
CPPGenerator::startToInt( void )
{
	curStream() << "static_cast<int>( ";
}


////////////////////////////////////////


void
CPPGenerator::startToUint( void )
{
	curStream() << "static_cast<unsigned int>( ";
}


////////////////////////////////////////


void
CPPGenerator::startToHalf( void )
{
	curStream() << "half( ";
}


////////////////////////////////////////


void
CPPGenerator::startToFloat( void )
{
	curStream() << "static_cast<ctl_number_t>( ";
}


////////////////////////////////////////


void
CPPGenerator::endCoersion( void )
{
	curStream() << " )";
}


////////////////////////////////////////


void
CPPGenerator::emitToken( Token t )
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
CPPGenerator::valueRecurse( CodeLContext &ctxt, const ExprNodeVector &elements,
							const DataTypePtr &t, size_t &index,
							const std::string &varName,
							bool isSubItem )
{
	ArrayType *arrayType = dynamic_cast<ArrayType *>( t.pointer() );
	if ( arrayType )
	{
		if ( myCurInitType == ASSIGN )
		{
			newlineAndIndent();
			curStream() << '{';
		}


		size_t N = arrayType->size();
		bool lineEveryItem = ( N > 4 );

		pushIndent();
		if ( myCurInitType == FUNC )
		{
			std::string typeName, postDecl;
//			if ( ! canBeBuiltinType( arrayType ) )
			bool doCtor = false;
			if ( ! findBuiltinType( typeName, postDecl, arrayType, ctxt ) )
			{
				newlineAndIndent();
				curStream() << varName << ".resize( " << arrayType->size() << " );";
			}
			else
				doCtor = true;

			if ( doCtor && isSubItem && postDecl.empty() )
			{
				myCurInitType = CTOR;
				// we're a sub-type, we can treat this as a CTOR scenario
				curStream() << varName << " = " << typeName << "( ";
				for (int i = 0; i < arrayType->size(); ++i)
				{
					if ( i > 0 )
						curStream() << ',';
					if ( lineEveryItem )
						newlineAndIndent();
					else if ( i > 0 )
						curStream() << ' ';

					valueRecurse( ctxt, elements, arrayType->elementType(), index, varName, true );
				}
				curStream() << " );";
				myCurInitType = FUNC;
			}
			else
			{
				std::stringstream nameB;
				nameB << varName << "[" << arrayType->size() << "]";
				std::string varNameSubscript = nameB.str();
				std::string::size_type startPos = varNameSubscript.find_last_of( '[' ) + 1;
				std::string::size_type endPos = varNameSubscript.find_last_of( ']' );
				for (int i = 0; i < arrayType->size(); ++i)
				{
					int curOut = i;
					bool done = false;
					for ( std::string::size_type x = endPos - 1; x >= startPos; --x )
					{
						if ( done )
							varNameSubscript[x] = ' ';
						else
						{
							varNameSubscript[x] = '0' + (curOut % 10);
							curOut /= 10;
							if ( curOut == 0 )
								done = true;
						}
					}

					newlineAndIndent();
					valueRecurse( ctxt, elements, arrayType->elementType(), index, varNameSubscript, true );
				}
			}
		}
		else
		{
			for (int i = 0; i < arrayType->size(); ++i)
			{
				if ( i > 0 )
					curStream() << ',';
				if ( lineEveryItem )
					newlineAndIndent();
				else if ( i > 0 )
					curStream() << ' ';

				valueRecurse( ctxt, elements, arrayType->elementType(), index, varName, true );
			}
		}
		popIndent();

		if ( myCurInitType == ASSIGN )
		{
			if ( lineEveryItem )
			{
				newlineAndIndent();
				curStream() << "}";
			}
			else
				curStream() << " }";
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

				newlineAndIndent();

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
		curStream() << varName << " = ";
		t->generateCastFrom( elements[index], ctxt );
		curStream() << ';';
	}
	else
		t->generateCastFrom( elements[index], ctxt );
	++index;
}


////////////////////////////////////////


CPPGenerator::InitType
CPPGenerator::variable( CodeLContext &ctxt,
						const std::string &name, const DataTypePtr &t,
						bool isConst, bool isInput, bool isWritable )
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
				curStream() << "const ";
			curStream() << "bool";
			break;
		case IntTypeEnum:
			if ( isConst )
				curStream() << "const ";
			curStream() << "int";
			break;
		case UIntTypeEnum:
			if ( isConst )
				curStream() << "const ";
			curStream() << "unsigned int";
			break;
		case HalfTypeEnum:
			if ( isConst )
				curStream() << "const ";
			curStream() << "half";
			break;
		case FloatTypeEnum:
			if ( isConst )
				curStream() << "const ";
			curStream() << "ctl_number_t";
			break;
		case StringTypeEnum:
			if ( isConst )
				curStream() << "const std:string";
			else
				curStream() << "std::string";
			retval = CTOR;
			break;
		case StructTypeEnum:
		{
			StructTypePtr structType = t.cast<StructType>();
			if ( myCPP11Mode &&  isConst )
				curStream() << "const ";
			curStream() << removeNSQuals( structType->name() );
			isWritable = isInput || isWritable;
			if ( myCPP11Mode )
				retval = ASSIGN;
			else
				retval = FUNC;
			break;
		}
		case ArrayTypeEnum:
		{
			ArrayTypePtr arrayType = t.cast<ArrayType>();

			std::string typeName;

			if ( myCPP11Mode )
				retval = ASSIGN;
			else
				retval = FUNC;

			if ( ! findBuiltinType( typeName, postDecl, arrayType, ctxt ) )
			{
				std::stringstream x;

				x << "std::vector< ";
				pushStream( x );
				variable( ctxt, std::string(), arrayType->elementType(),
						  false, false, false );
				popStream();
				x << " >";
				typeName = x.str();

				if ( retval == ASSIGN && isConst )
					curStream() << "const ";
			}
			else
			{
				if ( ! postDecl.empty() )
					retval = ASSIGN;
				else
					retval = CTOR;

				if ( isConst )
					curStream() << "const ";
			}
			curStream() << typeName;
			isWritable = ( isInput || isWritable ) && postDecl.empty();
			break;
		}
	}

	if ( isWritable )
		curStream() << " &";

	if ( ! name.empty() )
	{
		if ( ! isWritable )
			curStream() << ' ';
		curStream() << removeNSQuals( name );
	}
	curStream() << postDecl;

	return retval;
}


////////////////////////////////////////


void
CPPGenerator::doInit( InitType initT, CodeLContext &ctxt,
					  const ExprNodePtr &initV, const std::string &varName )
{
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
				if ( ! dynamic_cast<ValueNode *>( initV.pointer() ) )
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
					myCurModuleInit.back().push_back( initVal );
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
		curStream() << ';';
	myCurInitType = NONE;
}


////////////////////////////////////////


void
CPPGenerator::replaceInit( std::string &initS, const std::string &name )
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
CPPGenerator::findBuiltinType( std::string &typeName,
							   std::string &postDecl,
							   const ArrayTypePtr &arrayType,
							   CodeLContext &ctxt )
{
	int sx = arrayType->size();

	if ( dynamic_cast<FloatType *>( arrayType->elementType().pointer() ) )
	{
		ArrayType *x = dynamic_cast<ArrayType *>( arrayType->elementType().pointer() );
		// not x makes sure we are of size 1 (performance critical code, avoid
		// creation of RcPtr type w/ lock)
		if ( ! x && sx > 0 )
		{
			switch ( sx )
			{
				case 2: typeName = "ctl_vec2f_t"; break;
				case 3: typeName = "ctl_vec3f_t"; break;
				case 4: typeName = "ctl_vec4f_t"; break;
				default:
				{
					// just do a C array of the low level type
					typeName = "ctl_number_t";
					std::stringstream pB;
					pB << '[' << sx << ']';
					postDecl = pB.str();
					break;
				}
			}
		}
		return ! typeName.empty();
	}

	ArrayType *subArray = dynamic_cast<ArrayType *>( arrayType->elementType().pointer() );
	if ( subArray && dynamic_cast<FloatType *>( subArray->elementType().pointer() ) &&
		 sx == subArray->size() )
	{
		if ( sx == 3 )
			typeName = "ctl_mat33f_t";
		else if ( sx == 4 )
			typeName = "ctl_mat44f_t";

		return ! typeName.empty();
	}

	if ( dynamic_cast<IntType *>( arrayType->elementType().pointer() ) )
	{
		ArrayType *x = dynamic_cast<ArrayType *>( arrayType->elementType().pointer() );
		int sx = arrayType->size();
		// not x makes sure we are of size 1 (performance critical code, avoid
		// creation of RcPtr type w/ lock)
		if ( ! x && sx > 0 )
		{
			switch ( sx )
			{
				case 2:
					typeName = "ctl_vec2i_t";
					break;
				case 3:
					typeName = "ctl_vec3i_t";
					break;
				case 4:
					typeName = "ctl_vec4i_t";
					break;
				default:
				{
					// just do a C array of the low level type
					typeName = "int";
					std::stringstream pB;
					pB << '[' << sx << ']';
					postDecl = pB.str();
					break;
				}
			}
		}
		return ! typeName.empty();
	}

	return ! typeName.empty();
}


////////////////////////////////////////


bool
CPPGenerator::canBeBuiltinType( const ArrayType *arrayType )
{
	if ( dynamic_cast<FloatType *>( arrayType->elementType().pointer() ) ||
		 dynamic_cast<IntType *>( arrayType->elementType().pointer() ) )
	{
		ArrayType *x = dynamic_cast<ArrayType *>( arrayType->elementType().pointer() );
		// not x makes sure we are of size 1 (performance critical code, avoid
		// creation of RcPtr type w/ lock)
		if ( ! x && arrayType->size() > 0 )
			return true;
		return false;
	}

	ArrayType *subArray = dynamic_cast<ArrayType *>( arrayType->elementType().pointer() );
	if ( subArray && dynamic_cast<FloatType *>( subArray->elementType().pointer() ) &&
		 arrayType->size() == subArray->size() )
	{
		if ( arrayType->size() == 3 || arrayType->size() == 4 )
			return true;
	}

	return false;
}


////////////////////////////////////////


bool
CPPGenerator::checkNeedInitInModuleInit( const ExprNodePtr &initV, bool deep )
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
CPPGenerator::isAllLiterals( const ExprNodePtr &v )
{
	if ( ! v )
		return false;

	if ( dynamic_cast<LiteralNode *>( v.pointer() ) )
		return true;

	if ( dynamic_cast<NameNode *>( v.pointer() ) )
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
CPPGenerator::usesUninitLocalGlobals( const ExprNodePtr &v )
{
	if ( ! v )
		return false;

	if ( dynamic_cast<LiteralNode *>( v.pointer() ) )
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
CPPGenerator::extractLiteralConstants( const StatementNodePtr &consts,
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
CPPGenerator::checkNeedsSizeArgument( const DataTypePtr &p, const std::string &name, SizeVector &sizes )
{
	sizes.clear();
	int cnt = 0;
	if ( p->cDataType() == ArrayTypeEnum )
	{
		const ArrayType *arrT = dynamic_cast<const ArrayType *>( p.pointer() );
		arrT->sizes( sizes );
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
			curStream() << ", const ctl_vec2i_t &" << name << "_size";
			break;
		case 3:
			curStream() << ", const ctl_vec3i_t &" << name << "_size";
			break;
		default:
			throw std::logic_error( "Unimplemented size of variable array passing" );
			break;
	}
	
	return cnt > 0;
}


////////////////////////////////////////


void
CPPGenerator::extractSizeAndAdd( const ExprNodePtr &p, SizeVector &func_sizes, CodeLContext &ctxt )
{
	if ( p->type->cDataType() == ArrayTypeEnum )
	{
		const ArrayType *arrT = dynamic_cast<const ArrayType *>( p->type.pointer() );
		SizeVector arg_sizes;
		arrT->sizes( arg_sizes );
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
					outSizes.push_back( arg_sizes[i] );
				}
			}
			switch ( cnt )
			{
				case 0: throw std::logic_error( "Unhandled missing array size" );
				case 1: curStream() << ", static_cast<int>( "; break;
				case 2: curStream() << ", ctl_vec2i_t( "; break;
				case 3: curStream() << ", ctl_vec3i_t( "; break;
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

} // namespace Ctl

