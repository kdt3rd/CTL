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

//-----------------------------------------------------------------------------
//
//	The Standard Library of C++ functions that can be called from CTL
//	
//	- numeric limits
//	- usefult constants
//	- floating-point number classification
//
//-----------------------------------------------------------------------------

#include <CtlSimdStdLibLimits.h>
#include <CtlSimdStdLibTemplates.h>
#include <CtlSimdStdLibrary.h>
#include <CtlSimdStdTypes.h>
#include <CtlSimdCFunc.h>
#include <CtlSimdAddr.h>
#include <CtlSymbolTable.h>
#include <cmath>
#include <climits>
#include <limits>
#include <float.h>
#include <half.h>
#include <halfLimits.h>
#include <CtlNumber.h>

using namespace std;

namespace Ctl {
namespace {

// These rely on C99, if they are erroring for some platform
// we can bring back the old functions and make sure we force
// compilation to float only
inline bool isfinite_f( number f ) { return isfinite(f); }
inline bool isnormal_f( number f ) { return isnormal(f); }
inline bool isnan_f( number f ) { return isnan(f); }
inline bool isinf_f( number f ) { return isinf(f); }
inline bool isfinite_h (half h) { return h.isFinite(); }
inline bool isnormal_h (half h) { return h.isNormalized(); }
inline bool isnan_h (half h) { return h.isNan(); }
inline bool isinf_h (half h) { return h.isInfinity(); }


DEFINE_SIMD_FUNC_1_ARG (Isfinite_f, isfinite_f (a1), bool, number);
DEFINE_SIMD_FUNC_1_ARG (Isnormal_f, isnormal_f (a1), bool, number);
DEFINE_SIMD_FUNC_1_ARG (Isnan_f, isnan_f (a1), bool, number);
DEFINE_SIMD_FUNC_1_ARG (Isinf_f, isinf_f (a1), bool, number);
DEFINE_SIMD_FUNC_1_ARG (Isfinite_h, isfinite_h (a1), bool, half);
DEFINE_SIMD_FUNC_1_ARG (Isnormal_h, isnormal_h (a1), bool, half);
DEFINE_SIMD_FUNC_1_ARG (Isnan_h, isnan_h (a1), bool, half);
DEFINE_SIMD_FUNC_1_ARG (Isinf_h, isinf_h (a1), bool, half);


void
defineConst (SymbolTable &symtab,
	     const DataTypePtr &type,
	     const string &name,
	     SimdReg &reg)
{
    symtab.defineSymbol (name,
			 new SymbolInfo (0, // module
					 RWA_READ,
					 false,
					 type,
					 new SimdDataAddr (&reg)));
}


void
defineConstants (SymbolTable &symtab, SimdStdTypes &types)
{
    //
    // Statically allocated registers for pre-defined constants
    // are shared between all SIMD interpreter instances and
    // need to be initialized only once.
    //

    static SimdReg regM_E (false, sizeof (number));
    static SimdReg regM_PI (false, sizeof (number));
    static SimdReg regFLT_MAX (false, sizeof (number));
    static SimdReg regFLT_MIN (false, sizeof (number));
    static SimdReg regFLT_EPSILON (false, sizeof (number));
    static SimdReg regFLT_POS_INF (false, sizeof (number));
    static SimdReg regFLT_NEG_INF (false, sizeof (number));
    static SimdReg regFLT_NAN (false, sizeof (number));
    static SimdReg regHALF_MAX (false, sizeof (half));
    static SimdReg regHALF_MIN (false, sizeof (half));
    static SimdReg regHALF_EPSILON (false, sizeof (half));
    static SimdReg regHALF_POS_INF (false, sizeof (half));
    static SimdReg regHALF_NEG_INF (false, sizeof (half));
    static SimdReg regHALF_NAN (false, sizeof (half));
    static SimdReg regINT_MAX (false, sizeof (int));
    static SimdReg regINT_MIN (false, sizeof (int));
    static SimdReg regUINT_MAX (false, sizeof (unsigned int));

    static bool initialized = false;

    if (!initialized)
    {
#ifdef __USE_GNU
	*(number *)regM_E[0] = M_El;
	*(number *)regM_PI[0] = M_PIl;
#else
	*(number *)regM_E[0] = 2.7182818284590452354;
	*(number *)regM_PI[0] = 3.14159265358979323846;
#endif
	*(number *)regFLT_MAX[0] = std::numeric_limits<number>::max();
	*(number *)regFLT_MIN[0] = std::numeric_limits<number>::min();
	*(number *)regFLT_EPSILON[0] = std::numeric_limits<number>::epsilon();
	*(number *)regFLT_POS_INF[0] = std::numeric_limits<number>::infinity();
	*(number *)regFLT_NEG_INF[0] = -std::numeric_limits<number>::infinity();
	*(number *)regFLT_NAN[0] = std::numeric_limits<number>::quiet_NaN();
	*(half *)regHALF_MAX[0] = HALF_MAX;
	*(half *)regHALF_MIN[0] = HALF_MIN;
	*(half *)regHALF_EPSILON[0] = HALF_EPSILON;
	*(half *)regHALF_POS_INF[0] = half::posInf();
	*(half *)regHALF_NEG_INF[0] = half::negInf();
	*(half *)regHALF_NAN[0] = half::qNan();
	*(int *)regINT_MAX[0] = INT_MAX;
	*(int *)regINT_MIN[0] = INT_MIN;
	*(unsigned int *)regUINT_MAX[0] = UINT_MAX;

	initialized = true;
    }

    //
    // Insert pointers to the registers into the symbol table
    // of every interpreter instance.
    //

    defineConst (symtab, types.type_f(), "M_E", regM_E);
    defineConst (symtab, types.type_f(), "M_PI", regM_PI);
    defineConst (symtab, types.type_f(), "FLT_MAX", regFLT_MAX);
    defineConst (symtab, types.type_f(), "FLT_MIN", regFLT_MIN);
    defineConst (symtab, types.type_f(), "FLT_EPSILON", regFLT_EPSILON);
    defineConst (symtab, types.type_f(), "FLT_POS_INF", regFLT_POS_INF);
    defineConst (symtab, types.type_f(), "FLT_NEG_INF", regFLT_NEG_INF);
    defineConst (symtab, types.type_f(), "FLT_NAN", regFLT_NAN);
    defineConst (symtab, types.type_h(), "HALF_MAX", regHALF_MAX);
    defineConst (symtab, types.type_h(), "HALF_MIN", regHALF_MIN);
    defineConst (symtab, types.type_h(), "HALF_EPSILON", regHALF_EPSILON);
    defineConst (symtab, types.type_h(), "HALF_POS_INF", regHALF_POS_INF);
    defineConst (symtab, types.type_h(), "HALF_NEG_INF", regHALF_NEG_INF);
    defineConst (symtab, types.type_h(), "HALF_NAN", regHALF_NAN);
    defineConst (symtab, types.type_i(), "INT_MAX", regINT_MAX);
    defineConst (symtab, types.type_i(), "INT_MIN", regINT_MIN);
    defineConst (symtab, types.type_ui(), "UINT_MAX", regUINT_MAX);
}

} // namespace


void
declareSimdStdLibLimits (SymbolTable &symtab, SimdStdTypes &types)
{
    declareSimdCFunc (symtab, simdFunc1Arg <Isfinite_f>,
		      types.funcType_b_f(), "isfinite_f");

    declareSimdCFunc (symtab, simdFunc1Arg <Isnormal_f>,
		      types.funcType_b_f(), "isnormal_f");

    declareSimdCFunc (symtab, simdFunc1Arg <Isnan_f>,
		      types.funcType_b_f(), "isnan_f");

    declareSimdCFunc (symtab, simdFunc1Arg <Isinf_f>,
		      types.funcType_b_f(), "isinf_f");

    declareSimdCFunc (symtab, simdFunc1Arg <Isfinite_h>,
		      types.funcType_b_h(), "isfinite_h");

    declareSimdCFunc (symtab, simdFunc1Arg <Isnormal_h>,
		      types.funcType_b_h(), "isnormal_h");

    declareSimdCFunc (symtab, simdFunc1Arg <Isnan_h>,
		      types.funcType_b_h(), "isnan_h");

    declareSimdCFunc (symtab, simdFunc1Arg <Isinf_h>,
		      types.funcType_b_h(), "isinf_h");

    defineConstants (symtab, types);
}

} // namespace Ctl
