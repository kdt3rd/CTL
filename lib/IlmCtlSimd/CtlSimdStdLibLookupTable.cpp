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
//	- interpolated 1D and 3D table lookups
//
//-----------------------------------------------------------------------------

#include <CtlSimdStdLibLookupTable.h>
#include <CtlSimdStdLibTemplates.h>
#include <CtlSimdStdLibrary.h>
#include <CtlSimdStdTypes.h>
#include <CtlSimdCFunc.h>
#include <CtlLookupTable.h>
#include <CtlNumber.h>
#include <half.h>
#include <cmath>
#include <cassert>

using namespace std;

namespace Ctl {
namespace {

typedef number (*Lookup1DFunc) (const number[], int, number, number, number);


void
simdDoLookup1D
    (const SimdBoolMask &mask,
     SimdXContext &xcontext,
     Lookup1DFunc func)
{
    //
    // float func (float table[], float pMin, float pMax, float p)
    //

    const SimdReg &size  = xcontext.stack().regFpRelative (-1);
    const SimdReg &table = xcontext.stack().regFpRelative (-2);
    const SimdReg &pMin  = xcontext.stack().regFpRelative (-3);
    const SimdReg &pMax  = xcontext.stack().regFpRelative (-4);
    const SimdReg &p     = xcontext.stack().regFpRelative (-5);
    SimdReg &returnValue = xcontext.stack().regFpRelative (-6);

    assert (!size.isVarying());
    int s = *(int *)(size[0]);

    if (table.isVarying() ||
	pMin.isVarying() ||
	pMax.isVarying() ||
	p.isVarying())
    {
	returnValue.setVarying (true);

	if (!mask.isVarying() &&
	    !table.isVarying() &&
	    !pMin.isVarying() &&
	    !pMax.isVarying())
	{
	    //
	    // Fast path -- only p is varying, everything else is uniform.
	    //

	    number *table0 = (number *)(table[0]);
	    number pMin0 = *(number *)(pMin[0]);
	    number pMax0 = *(number *)(pMax[0]);

	    for (int i = xcontext.regSize(); --i >= 0;)
	    {
		*(number *)(returnValue[i]) = func (table0,
						   s,
						   pMin0,
						   pMax0,
						   *(number *)(p[i]));
	    }
	}
	else
	{
	    for (int i = xcontext.regSize(); --i >= 0;)
	    {
		if (mask[i])
		{
		    *(number *)(returnValue[i]) = func ((number *)(table[i]), 
						       s,
						       *(number *)(pMin[i]),
						       *(number *)(pMax[i]),
						       *(number *)(p[i]));
		}
	    }
	}
    }
    else
    {
	returnValue.setVarying (false);

	*(number *)(returnValue[0]) = func ((number *)(table[0]), 
					   s,
					   *(number *)(pMin[0]),
					   *(number *)(pMax[0]),
					   *(number *)(p[0]));
    }
}


void
simdLookup1D (const SimdBoolMask &mask, SimdXContext &xcontext)
{
    //
    // float lookup1D (float table[], float pMin, float pMax, float p)
    //

    simdDoLookup1D (mask, xcontext, lookup1D);
}


void
simdLookupCubic1D (const SimdBoolMask &mask, SimdXContext &xcontext)
{
    //
    // float lookupCubic1D (float table[], float pMin, float pMax, float p)
    //

    simdDoLookup1D (mask, xcontext, lookupCubic1D);
}


void
simdLookup3D_f3 (const SimdBoolMask &mask, SimdXContext &xcontext)
{
    //
    // float[3] lookup3D_f3 (float table[][][][3],
    //			     float pMin[3], float pMax[3],
    //			     float p[3])
    //

    const SimdReg &size2  = xcontext.stack().regFpRelative (-1);
    const SimdReg &size1  = xcontext.stack().regFpRelative (-2);
    const SimdReg &size0  = xcontext.stack().regFpRelative (-3);
    const SimdReg &table  = xcontext.stack().regFpRelative (-4);
    const SimdReg &pMin   = xcontext.stack().regFpRelative (-5);
    const SimdReg &pMax   = xcontext.stack().regFpRelative (-6);
    const SimdReg &p      = xcontext.stack().regFpRelative (-7);
    SimdReg &returnValue  = xcontext.stack().regFpRelative (-8);

    assert (!size0.isVarying() && !size1.isVarying() && !size2.isVarying());

    Vec3i s (*(int *)(size0[0]),
	   *(int *)(size1[0]),
	   *(int *)(size2[0]));

    if (table.isVarying() ||
	pMin.isVarying() ||
	pMax.isVarying() ||
	p.isVarying())
    {
	returnValue.setVarying (true);

	for (int i = xcontext.regSize(); --i >= 0;)
	{
	    if (mask[i])
	    {
		*(Vec3 *)(returnValue[i]) = lookup3D ((Vec3 *)(table[i]), 
						     s,
						     *(Vec3 *)(pMin[i]),
						     *(Vec3 *)(pMax[i]),
						     *(Vec3 *)(p[i]));
	    }
	}
    }
    else
    {
	returnValue.setVarying (false);

	*(Vec3 *)(returnValue[0]) = lookup3D ((Vec3 *)(table[0]), 
					     s,
					     *(Vec3 *)(pMin[0]),
					     *(Vec3 *)(pMax[0]),
					     *(Vec3 *)(p[0]));
    }
}


void
simdLookup3D_f (const SimdBoolMask &mask, SimdXContext &xcontext)
{
    //
    // void lookup3D_f (float table[][][][3],
    //		        float pMin[3], float pMax[3],
    //		        float p0, float p1, float p2,
    //		        float q0, float q1, float q2)
    //

    const SimdReg &size2  = xcontext.stack().regFpRelative (-1);
    const SimdReg &size1  = xcontext.stack().regFpRelative (-2);
    const SimdReg &size0  = xcontext.stack().regFpRelative (-3);
    const SimdReg &table  = xcontext.stack().regFpRelative (-4);
    const SimdReg &pMin   = xcontext.stack().regFpRelative (-5);
    const SimdReg &pMax   = xcontext.stack().regFpRelative (-6);
    const SimdReg &p0     = xcontext.stack().regFpRelative (-7);
    const SimdReg &p1     = xcontext.stack().regFpRelative (-8);
    const SimdReg &p2     = xcontext.stack().regFpRelative (-9);
    SimdReg &q0           = xcontext.stack().regFpRelative (-10);
    SimdReg &q1           = xcontext.stack().regFpRelative (-11);
    SimdReg &q2           = xcontext.stack().regFpRelative (-12);

    assert (!size0.isVarying() && !size1.isVarying() && !size2.isVarying());

    Vec3i s (*(int *)(size0[0]),
	   *(int *)(size1[0]),
	   *(int *)(size2[0]));

    if (table.isVarying() ||
	pMin.isVarying() ||
	pMax.isVarying() ||
	p0.isVarying() ||
	p1.isVarying() ||
	p2.isVarying())
    {
	q0.setVarying (true);
	q1.setVarying (true);
	q2.setVarying (true);

	for (int i = xcontext.regSize(); --i >= 0;)
	{
	    if (mask[i])
	    {
		Vec3 p (*(number *)p0[i], *(number *)p1[i], *(number *)p2[i]);

		Vec3 q = lookup3D ((Vec3 *)(table[i]), 
				  s,
				  *(Vec3 *)(pMin[i]),
				  *(Vec3 *)(pMax[i]),
				  p);

		*(number *)q0[i] = q[0];
		*(number *)q1[i] = q[1];
		*(number *)q2[i] = q[2];
	    }
	}
    }
    else
    {
	q0.setVarying (false);
	q1.setVarying (false);
	q2.setVarying (false);

	Vec3 p (*(number *)p0[0], *(number *)p1[0], *(number *)p2[0]);

	Vec3 q = lookup3D ((Vec3 *)(table[0]), 
			  s,
			  *(Vec3 *)(pMin[0]),
			  *(Vec3 *)(pMax[0]),
			  p);

	*(number *)q0[0] = q[0];
	*(number *)q1[0] = q[1];
	*(number *)q2[0] = q[2];
    }
}


void
simdLookup3D_h (const SimdBoolMask &mask, SimdXContext &xcontext)
{
    //
    // void lookup3D_h (float table[][][][3],
    //		        float pMin[3], float pMax[3],
    //		        half p0, half p1, half p2,
    //		        half q0, half q1, half q2)
    //

    const SimdReg &size2  = xcontext.stack().regFpRelative (-1);
    const SimdReg &size1  = xcontext.stack().regFpRelative (-2);
    const SimdReg &size0  = xcontext.stack().regFpRelative (-3);
    const SimdReg &table  = xcontext.stack().regFpRelative (-4);
    const SimdReg &pMin   = xcontext.stack().regFpRelative (-5);
    const SimdReg &pMax   = xcontext.stack().regFpRelative (-6);
    const SimdReg &p0     = xcontext.stack().regFpRelative (-7);
    const SimdReg &p1     = xcontext.stack().regFpRelative (-8);
    const SimdReg &p2     = xcontext.stack().regFpRelative (-9);
    SimdReg &q0           = xcontext.stack().regFpRelative (-10);
    SimdReg &q1           = xcontext.stack().regFpRelative (-11);
    SimdReg &q2           = xcontext.stack().regFpRelative (-12);

    assert (!size0.isVarying() && !size1.isVarying() && !size2.isVarying());

    Vec3i s (*(int *)(size0[0]),
	   *(int *)(size1[0]),
	   *(int *)(size2[0]));

    if (table.isVarying() ||
	pMin.isVarying() ||
	pMax.isVarying() ||
	p0.isVarying() ||
	p1.isVarying() ||
	p2.isVarying())
    {
	q0.setVarying (true);
	q1.setVarying (true);
	q2.setVarying (true);

	for (int i = xcontext.regSize(); --i >= 0;)
	{
	    if (mask[i])
	    {
		Vec3 p (*(half *)p0[i], *(half *)p1[i], *(half *)p2[i]);

		Vec3 q = lookup3D ((Vec3 *)(table[i]), 
				  s,
				  *(Vec3 *)(pMin[i]),
				  *(Vec3 *)(pMax[i]),
				  p);

		*(half *)q0[i] = q[0];
		*(half *)q1[i] = q[1];
		*(half *)q2[i] = q[2];
	    }
	}
    }
    else
    {
	q0.setVarying (false);
	q1.setVarying (false);
	q2.setVarying (false);

	Vec3 p (*(half *)p0[0], *(half *)p1[0], *(half *)p2[0]);

	Vec3 q = lookup3D ((Vec3 *)(table[0]), 
			  s,
			  *(Vec3 *)(pMin[0]),
			  *(Vec3 *)(pMax[0]),
			  p);

	*(half *)q0[0] = q[0];
	*(half *)q1[0] = q[1];
	*(half *)q2[0] = q[2];
    }
}


typedef number (*Interpolate1DFunc) (const number[][2], int, number);


void
simdDoInterpolate1D
    (const SimdBoolMask &mask,
     SimdXContext &xcontext,
     Interpolate1DFunc func)
{
    //
    // float func (float table[][2], float p)
    //

    const SimdReg &size  = xcontext.stack().regFpRelative (-1);
    const SimdReg &table = xcontext.stack().regFpRelative (-2);
    const SimdReg &p     = xcontext.stack().regFpRelative (-3);
    SimdReg &returnValue = xcontext.stack().regFpRelative (-4);

    assert (!size.isVarying());
    int s = *(int *)(size[0]);

    if (table.isVarying() ||
	p.isVarying())
    {
	returnValue.setVarying (true);

	if (!mask.isVarying() &&
	    !table.isVarying())
	{
	    //
	    // Fast path -- only p is varying, everything else is uniform.
	    //

	    number (*table0)[2] = (number (*)[2])(table[0]);

	    for (int i = xcontext.regSize(); --i >= 0;)
	    {
		*(number *)(returnValue[i]) = func (table0,
						   s,
						   *(number *)(p[i]));
	    }
	}
	else
	{
	    for (int i = xcontext.regSize(); --i >= 0;)
	    {
		if (mask[i])
		{
		    *(number *)(returnValue[i]) = func ((number (*)[2])(table[i]),
						       s,
						       *(number *)(p[i]));
		}
	    }
	}
    }
    else
    {
	returnValue.setVarying (false);

	*(number *)(returnValue[0]) = func ((number (*)[2])(table[0]), 
					   s,
					   *(number *)(p[0]));
    }
}


void
simdInterpolate1D (const SimdBoolMask &mask, SimdXContext &xcontext)
{
    //
    // float interpolate1D (float table[][2], float p)
    //

    simdDoInterpolate1D (mask, xcontext, interpolate1D);
}


void
simdInterpolateCubic1D (const SimdBoolMask &mask, SimdXContext &xcontext)
{
    //
    // float interpolateCubic1D (float table[][2], float p)
    //

    simdDoInterpolate1D (mask, xcontext, interpolateCubic1D);
}

} // namespace


void
declareSimdStdLibLookupTable (SymbolTable &symtab, SimdStdTypes &types)
{
    declareSimdCFunc (symtab, simdLookup1D,
		      types.funcType_f_f0_f_f_f(), "lookup1D");

    declareSimdCFunc (symtab, simdLookupCubic1D,
		      types.funcType_f_f0_f_f_f(), "lookupCubic1D");

    declareSimdCFunc (symtab, simdLookup3D_f3,
		      types.funcType_f3_f0003_f3_f3_f3(), "lookup3D_f3");

    declareSimdCFunc (symtab, simdLookup3D_f,
		      types.funcType_v_f0003_f3_f3_fff_offf(), "lookup3D_f");

    declareSimdCFunc (symtab, simdLookup3D_h,
		      types.funcType_v_f0003_f3_f3_hhh_ohhh(), "lookup3D_h");

    declareSimdCFunc (symtab, simdInterpolate1D,
		      types.funcType_f_f02_f(), "interpolate1D");

    declareSimdCFunc (symtab, simdInterpolateCubic1D,
		      types.funcType_f_f02_f(), "interpolateCubic1D");
}

} // namespace Ctl
