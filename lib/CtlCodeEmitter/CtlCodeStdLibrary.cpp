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
//	 * Redistributions of source code must retain the above copyright
//	   notice, this list of conditions and the Disclaimer of Warranty.
// 
//	 * Redistributions in binary form must reproduce the above copyright
//	   notice, this list of conditions and the Disclaimer of Warranty
//	   in the documentation and/or other materials provided with the
//	   distribution.
// 
//	 * Nothing in this license shall be deemed to grant any rights to
//	   trademarks, copyrights, patents, trade secrets or any other
//	   intellectual property of A.M.P.A.S. or any contributors, except
//	   as expressly stated herein, and neither the name of A.M.P.A.S.
//	   nor of any other contributors to this software, may be used to
//	   endorse or promote products derived from this software without
//	   specific prior written permission of A.M.P.A.S. or contributor,
//	   as appropriate.
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
//	The Standard Library of C++ functions that can be called from CTL
//
//-----------------------------------------------------------------------------

#include <CtlCodeStdLibrary.h>
#include <CtlCodeStdTypes.h>
#include <CtlCodeLContext.h>
#include <CtlSymbolTable.h>
#include <iomanip>

namespace
{

using namespace Ctl;

void
defineSymbol( SymbolTable &symtab, const FunctionTypePtr &type, const std::string &name )
{
	symtab.defineSymbol( name, new SymbolInfo( 0, RWA_NONE, false, type ) );
}

void
defineConst( SymbolTable &symtab, const DataTypePtr &type,
			 const std::string &name )
{
	symtab.defineSymbol( name, new SymbolInfo( 0, RWA_READ, false, type ) );
}

void
declareAssert( SymbolTable &symtab, CodeStdTypes &types )
{
	defineSymbol( symtab, types.funcType_v_b(), "assert" );
}

void
declarePrint( SymbolTable &symtab, CodeStdTypes &types )
{
	defineSymbol( symtab, types.funcType_v_b(), "print_bool" );
	defineSymbol( symtab, types.funcType_v_i(), "print_int" );
	defineSymbol( symtab, types.funcType_v_ui(), "print_unsigned_int" );
	defineSymbol( symtab, types.funcType_v_h(), "print_half" );
	defineSymbol( symtab, types.funcType_v_f(), "print_float" );
	defineSymbol( symtab, types.funcType_v_s(), "print_string" );
}

void
declareLimits( SymbolTable &symtab, CodeStdTypes &types )
{
	defineSymbol( symtab, types.funcType_b_f(), "isfinite_f" );
	defineSymbol( symtab, types.funcType_b_f(), "isnormal_f" );
	defineSymbol( symtab, types.funcType_b_f(), "isnan_f" );
	defineSymbol( symtab, types.funcType_b_f(), "isinf_f" );
	defineSymbol( symtab, types.funcType_b_h(), "isfinite_h" );
	defineSymbol( symtab, types.funcType_b_h(), "isnormal_h" );
	defineSymbol( symtab, types.funcType_b_h(), "isnan_h" );
	defineSymbol( symtab, types.funcType_b_h(), "isinf_h" );
	defineConst( symtab, types.type_f(), "M_E" );
	defineConst( symtab, types.type_f(), "M_PI" );
	defineConst( symtab, types.type_f(), "FLT_MAX" );
	defineConst( symtab, types.type_f(), "FLT_MIN" );
	defineConst( symtab, types.type_f(), "FLT_EPSILON" );
	defineConst( symtab, types.type_f(), "FLT_POS_INF" );
	defineConst( symtab, types.type_f(), "FLT_NEG_INF" );
	defineConst( symtab, types.type_f(), "FLT_NAN" );
	defineConst( symtab, types.type_h(), "HALF_MAX" );
	defineConst( symtab, types.type_h(), "HALF_MIN" );
	defineConst( symtab, types.type_h(), "HALF_EPSILON" );
	defineConst( symtab, types.type_h(), "HALF_POS_INF" );
	defineConst( symtab, types.type_h(), "HALF_NEG_INF" );
	defineConst( symtab, types.type_h(), "HALF_NAN" );
	defineConst( symtab, types.type_i(), "INT_MAX" );
	defineConst( symtab, types.type_i(), "INT_MIN" );
	defineConst( symtab, types.type_ui(), "UINT_MAX" );
}

void
declareMath( SymbolTable &symtab, CodeStdTypes &types )
{
	defineSymbol( symtab, types.funcType_f_f(), "acos" );
	defineSymbol( symtab, types.funcType_f_f(), "asin" );
	defineSymbol( symtab, types.funcType_f_f(), "atan" );
	defineSymbol( symtab, types.funcType_f_f_f(), "atan2" );
	defineSymbol( symtab, types.funcType_f_f(), "cos" );
	defineSymbol( symtab, types.funcType_f_f(), "sin" );
	defineSymbol( symtab, types.funcType_f_f(), "tan" );
	defineSymbol( symtab, types.funcType_f_f(), "cosh" );
	defineSymbol( symtab, types.funcType_f_f(), "sinh" );
	defineSymbol( symtab, types.funcType_f_f(), "tanh" );
	defineSymbol( symtab, types.funcType_f_f(), "exp" );
	defineSymbol( symtab, types.funcType_h_f(), "exp_h" );
	defineSymbol( symtab, types.funcType_f_f(), "log" );
	defineSymbol( symtab, types.funcType_f_h(), "log_h" );
	defineSymbol( symtab, types.funcType_f_f(), "log10" );
	defineSymbol( symtab, types.funcType_f_h(), "log10_h" );
	defineSymbol( symtab, types.funcType_f_f_f(), "pow" );
	defineSymbol( symtab, types.funcType_h_h_f(), "pow_h" );
	defineSymbol( symtab, types.funcType_f_f(), "pow10" );
	defineSymbol( symtab, types.funcType_h_f(), "pow10_h" );
	defineSymbol( symtab, types.funcType_f_f(), "sqrt" );
	defineSymbol( symtab, types.funcType_f_f(), "fabs" );
	defineSymbol( symtab, types.funcType_f_f(), "floor" );
	defineSymbol( symtab, types.funcType_f_f_f(), "fmod" );
	defineSymbol( symtab, types.funcType_f_f_f(), "hypot" );
	defineSymbol( symtab, types.funcType_f33_f33_f33(), "mult_f33_f33" );
	defineSymbol( symtab, types.funcType_f44_f44_f44(), "mult_f44_f44" );
	defineSymbol( symtab, types.funcType_f33_f_f33(), "mult_f_f33" );
	defineSymbol( symtab, types.funcType_f44_f_f44(), "mult_f_f44" );
	defineSymbol( symtab, types.funcType_f33_f33_f33(), "add_f33_f33" );
	defineSymbol( symtab, types.funcType_f44_f44_f44(), "add_f44_f44" );
	defineSymbol( symtab, types.funcType_f33_f33(), "invert_f33" );
	defineSymbol( symtab, types.funcType_f44_f44(), "invert_f44" );
	defineSymbol( symtab, types.funcType_f33_f33(), "transpose_f33" );
	defineSymbol( symtab, types.funcType_f44_f44(), "transpose_f44" );
	defineSymbol( symtab, types.funcType_f3_f3_f33(), "mult_f3_f33" );
	defineSymbol( symtab, types.funcType_f3_f3_f44(), "mult_f3_f44" );
	defineSymbol( symtab, types.funcType_f3_f_f3(), "mult_f_f3" );
	defineSymbol( symtab, types.funcType_f3_f3_f3(), "add_f3_f3" );
	defineSymbol( symtab, types.funcType_f3_f3_f3(), "sub_f3_f3" );
	defineSymbol( symtab, types.funcType_f3_f3_f3(), "cross_f3_f3" );
	defineSymbol( symtab, types.funcType_f_f3_f3(), "dot_f3_f3" );
	defineSymbol( symtab, types.funcType_f_f3(), "length_f3" );
}

void
declareColorSpace( SymbolTable &symtab, CodeStdTypes &types )
{
	defineSymbol( symtab, types.funcType_f44_chr_f(), "RGBtoXYZ" );
	defineSymbol( symtab, types.funcType_f44_chr_f(), "XYZtoRGB" );
	defineSymbol( symtab, types.funcType_f3_f3_f3(), "LuvtoXYZ" );
	defineSymbol( symtab, types.funcType_f3_f3_f3(), "XYZtoLuv" );
	defineSymbol( symtab, types.funcType_f3_f3_f3(), "LabtoXYZ" );
	defineSymbol( symtab, types.funcType_f3_f3_f3(), "XYZtoLab" );
}

void
declareLookupTable( SymbolTable &symtab, CodeStdTypes &types )
{
	defineSymbol( symtab, types.funcType_f_f0_f_f_f(), "lookup1D" );
	defineSymbol( symtab, types.funcType_f_f0_f_f_f(), "lookupCubic1D" );
	defineSymbol( symtab, types.funcType_f3_f0003_f3_f3_f3(), "lookup3D_f3" );
	defineSymbol( symtab, types.funcType_v_f0003_f3_f3_fff_offf(), "lookup3D_f" );
	defineSymbol( symtab, types.funcType_v_f0003_f3_f3_hhh_ohhh(), "lookup3D_h" );
	defineSymbol( symtab, types.funcType_f_f02_f(), "interpolate1D" );
	defineSymbol( symtab, types.funcType_f_f02_f(), "interpolateCubic1D" );
}

void
declareInterpolator( SymbolTable &symtab, CodeStdTypes &types )
{
	defineSymbol( symtab, types.funcType_v_f023_f3_f3_of0003(),
				  "scatteredDataToGrid3D" );
}

} // empty namespace

namespace Ctl
{

void
declareCodeStdLibrary( LContext &lcontext )
{
	CodeStdTypes types( lcontext );

	types.type_box2i();
	types.type_box2f();

	declareAssert( lcontext.symtab(), types );
	declarePrint( lcontext.symtab(), types );
	declareLimits( lcontext.symtab(), types );
	declareMath( lcontext.symtab(), types );
	declareColorSpace( lcontext.symtab(), types );
	declareLookupTable( lcontext.symtab(), types );
	declareInterpolator( lcontext.symtab(), types );
}

} // namespace Ctl
