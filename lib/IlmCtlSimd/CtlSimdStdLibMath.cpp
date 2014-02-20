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
//	- math functions (cos(), exp(), sqrt(), etc.)
//	- vectors and matrices
//
//-----------------------------------------------------------------------------

#include <CtlSimdStdLibMath.h>
#include <CtlSimdStdLibTemplates.h>
#include <CtlSimdStdLibrary.h>
#include <CtlSimdStdTypes.h>
#include <CtlSimdCFunc.h>
#include <CtlSimdHalfExpLog.h>
#include <CtlNumber.h>
#include <ImathMatrix.h>
#include <cmath>

using namespace std;

namespace Ctl {
namespace {

DEFINE_SIMD_FUNC_1_ARG (Acos, acos (a1), number, number);
DEFINE_SIMD_FUNC_1_ARG (Asin, asin (a1), number, number);
DEFINE_SIMD_FUNC_1_ARG (Atan, atan (a1), number, number);
DEFINE_SIMD_FUNC_2_ARG (Atan2, atan2 (a1, a2), number, number, number);
DEFINE_SIMD_FUNC_1_ARG (Cos, cos (a1), number, number);
DEFINE_SIMD_FUNC_1_ARG (Sin, sin (a1), number, number);
DEFINE_SIMD_FUNC_1_ARG (Tan, tan (a1), number, number);
DEFINE_SIMD_FUNC_1_ARG (Cosh, cosh (a1), number, number);
DEFINE_SIMD_FUNC_1_ARG (Sinh, sinh (a1), number, number);
DEFINE_SIMD_FUNC_1_ARG (Tanh, tanh (a1), number, number);
DEFINE_SIMD_FUNC_1_ARG (Exp, exp (a1), number, number);
DEFINE_SIMD_FUNC_1_ARG (ExpH, exp_h (a1), half, number);
DEFINE_SIMD_FUNC_1_ARG (Log, log (a1), number, number);
DEFINE_SIMD_FUNC_1_ARG (LogH, log_h (a1), number, half);
DEFINE_SIMD_FUNC_1_ARG (Log10, log10 (a1), number, number);
DEFINE_SIMD_FUNC_1_ARG (Log10H, log10_h (a1), number, half);
DEFINE_SIMD_FUNC_2_ARG (Pow, pow (a1, a2), number, number, number);
DEFINE_SIMD_FUNC_2_ARG (PowH, pow_h (a1, a2), half, half, number);
DEFINE_SIMD_FUNC_1_ARG (Pow10, pow (10.0f, a1), number, number);
DEFINE_SIMD_FUNC_1_ARG (Pow10H, pow10_h (a1), half, number);
DEFINE_SIMD_FUNC_1_ARG (Sqrt, sqrt (a1), number, number);
DEFINE_SIMD_FUNC_1_ARG (Fabs, fabs (a1), number, number);
DEFINE_SIMD_FUNC_1_ARG (Floor, floor (a1), number, number);
DEFINE_SIMD_FUNC_2_ARG (Fmod, fmod (a1, a2), number, number, number);
DEFINE_SIMD_FUNC_2_ARG (Hypot, hypot (a1, a2), number, number, number);
DEFINE_SIMD_FUNC_2_ARG (Mult_f33_f33, a1 * a2, M33, M33, M33);
DEFINE_SIMD_FUNC_2_ARG (Mult_f44_f44, a1 * a2, M44, M44, M44);
DEFINE_SIMD_FUNC_2_ARG (Mult_f_f33, a1 * a2, M33, number, M33);
DEFINE_SIMD_FUNC_2_ARG (Mult_f_f44, a1 * a2, M44, number, M44);
DEFINE_SIMD_FUNC_2_ARG (Add_f33_f33, a1 + a2, M33, M33, M33);
DEFINE_SIMD_FUNC_2_ARG (Add_f44_f44, a1 + a2, M44, M44, M44);
DEFINE_SIMD_FUNC_1_ARG (Invert_f33, a1.inverse(), M33, M33);
DEFINE_SIMD_FUNC_1_ARG (Invert_f44, a1.inverse(), M44, M44);
DEFINE_SIMD_FUNC_1_ARG (Transpose_f33, a1.transposed(), M33, M33);
DEFINE_SIMD_FUNC_1_ARG (Transpose_f44, a1.transposed(), M44, M44);
DEFINE_SIMD_FUNC_2_ARG (Mult_f3_f33, a1 * a2, Vec3, Vec3, M33);
DEFINE_SIMD_FUNC_2_ARG (Mult_f3_f44, a1 * a2, Vec3, Vec3, M44);
DEFINE_SIMD_FUNC_2_ARG (Mult_f_f3, a1 * a2, Vec3, number, Vec3);
DEFINE_SIMD_FUNC_2_ARG (Add_f3_f3, a1 + a2, Vec3, Vec3, Vec3);
DEFINE_SIMD_FUNC_2_ARG (Sub_f3_f3, a1 - a2, Vec3, Vec3, Vec3);
DEFINE_SIMD_FUNC_2_ARG (Cross_f3_f3, a1.cross(a2), Vec3, Vec3, Vec3);
DEFINE_SIMD_FUNC_2_ARG (Dot_f3_f3, a1.dot(a2), number, Vec3, Vec3);
DEFINE_SIMD_FUNC_1_ARG (Length_f3, a1.length(), number, Vec3);

} // namespace


void
declareSimdStdLibMath (SymbolTable &symtab, SimdStdTypes &types)
{
    declareSimdCFunc (symtab, simdFunc1Arg <Acos>,
		      types.funcType_f_f(), "acos");

    declareSimdCFunc (symtab, simdFunc1Arg <Asin>,
		      types.funcType_f_f(), "asin");

    declareSimdCFunc (symtab, simdFunc1Arg <Atan>,
		      types.funcType_f_f(), "atan");

    declareSimdCFunc (symtab, simdFunc2Arg <Atan2>,
		      types.funcType_f_f_f(), "atan2");

    declareSimdCFunc (symtab, simdFunc1Arg <Cos>,
		      types.funcType_f_f(), "cos");

    declareSimdCFunc (symtab, simdFunc1Arg <Sin>,
		      types.funcType_f_f(), "sin");

    declareSimdCFunc (symtab, simdFunc1Arg <Tan>,
		      types.funcType_f_f(), "tan");

    declareSimdCFunc (symtab, simdFunc1Arg <Cosh>,
		      types.funcType_f_f(), "cosh");

    declareSimdCFunc (symtab, simdFunc1Arg <Sinh>,
		      types.funcType_f_f(), "sinh");

    declareSimdCFunc (symtab, simdFunc1Arg <Tanh>,
		      types.funcType_f_f(), "tanh");

    declareSimdCFunc (symtab, simdFunc1Arg <Exp>,
		      types.funcType_f_f(), "exp");

    declareSimdCFunc (symtab, simdFunc1Arg <ExpH>,
		      types.funcType_h_f(), "exp_h");

    declareSimdCFunc (symtab, simdFunc1Arg <Log>,
		      types.funcType_f_f(), "log");

    declareSimdCFunc (symtab, simdFunc1Arg <LogH>,
		      types.funcType_f_h(), "log_h");

    declareSimdCFunc (symtab, simdFunc1Arg <Log10>,
		      types.funcType_f_f(), "log10");

    declareSimdCFunc (symtab, simdFunc1Arg <Log10H>,
		      types.funcType_f_h(), "log10_h");

    declareSimdCFunc (symtab, simdFunc2Arg <Pow>,
		      types.funcType_f_f_f(), "pow");

    declareSimdCFunc (symtab, simdFunc2Arg <PowH>,
		      types.funcType_h_h_f(), "pow_h");

    declareSimdCFunc (symtab, simdFunc1Arg <Pow10>,
		      types.funcType_f_f(), "pow10");

    declareSimdCFunc (symtab, simdFunc1Arg <Pow10H>,
		      types.funcType_h_f(), "pow10_h");

    declareSimdCFunc (symtab, simdFunc1Arg <Sqrt>,
		      types.funcType_f_f(), "sqrt");

    declareSimdCFunc (symtab, simdFunc1Arg <Fabs>,
		      types.funcType_f_f(), "fabs");

    declareSimdCFunc (symtab, simdFunc1Arg <Floor>,
		      types.funcType_f_f(), "floor");

    declareSimdCFunc (symtab, simdFunc2Arg <Fmod>,
		      types.funcType_f_f_f(), "fmod");

    declareSimdCFunc (symtab, simdFunc2Arg <Hypot>,
		      types.funcType_f_f_f(), "hypot");

    declareSimdCFunc (symtab, simdFunc2Arg <Mult_f33_f33>,
		      types.funcType_f33_f33_f33(), "mult_f33_f33");

    declareSimdCFunc (symtab, simdFunc2Arg <Mult_f44_f44>,
		      types.funcType_f44_f44_f44(), "mult_f44_f44");
		  
    declareSimdCFunc (symtab, simdFunc2Arg <Mult_f_f33>,
		      types.funcType_f33_f_f33(), "mult_f_f33");
		  
    declareSimdCFunc (symtab, simdFunc2Arg <Mult_f_f44>,
		      types.funcType_f44_f_f44(), "mult_f_f44");

    declareSimdCFunc (symtab, simdFunc2Arg <Add_f33_f33>,
		      types.funcType_f33_f33_f33(), "add_f33_f33");

    declareSimdCFunc (symtab, simdFunc2Arg <Add_f44_f44>,
		      types.funcType_f44_f44_f44(), "add_f44_f44");

    declareSimdCFunc (symtab, simdFunc1Arg <Invert_f33>,
		      types.funcType_f33_f33(), "invert_f33");

    declareSimdCFunc (symtab, simdFunc1Arg <Invert_f44>,
		      types.funcType_f44_f44(), "invert_f44");

    declareSimdCFunc (symtab, simdFunc1Arg <Transpose_f33>,
		      types.funcType_f33_f33(), "transpose_f33");

    declareSimdCFunc (symtab, simdFunc1Arg <Transpose_f44>,
		      types.funcType_f44_f44(), "transpose_f44");

    declareSimdCFunc (symtab, simdFunc2Arg <Mult_f3_f33>,
		      types.funcType_f3_f3_f33(), "mult_f3_f33");

    declareSimdCFunc (symtab, simdFunc2Arg <Mult_f3_f44>,
		      types.funcType_f3_f3_f44(), "mult_f3_f44");

    declareSimdCFunc (symtab, simdFunc2Arg <Mult_f_f3>,
		      types.funcType_f3_f_f3(), "mult_f_f3");

    declareSimdCFunc (symtab, simdFunc2Arg <Add_f3_f3>,
		      types.funcType_f3_f3_f3(), "add_f3_f3");

    declareSimdCFunc (symtab, simdFunc2Arg <Sub_f3_f3>,
		      types.funcType_f3_f3_f3(), "sub_f3_f3");

    declareSimdCFunc (symtab, simdFunc2Arg <Cross_f3_f3>,
		      types.funcType_f3_f3_f3(), "cross_f3_f3");

    declareSimdCFunc (symtab, simdFunc2Arg <Dot_f3_f3>,
		      types.funcType_f_f3_f3(), "dot_f3_f3");

    declareSimdCFunc (symtab, simdFunc1Arg <Length_f3>,
		      types.funcType_f_f3(), "length_f3");
}

} // namespace Ctl
