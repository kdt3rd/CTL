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

#ifndef INCLUDED_CTL_LOOKUP_TABLE_H
#define INCLUDED_CTL_LOOKUP_TABLE_H

//-----------------------------------------------------------------------------
//
//	1D and 3D table lookups with linear, trilinear and
//	cubic interpolation.
//
//	lookup1D(t,s,pMin,pMax,p)
//
//		Lookup table, t, with s entries of type float. 
//		The table defines a piecewise linear function, f, with
//		s-1 segments of equal length, such that f(pMin) == t[0]
//		and f(pMax) == t(s-1).
//
//		lookup1D(t,s,pMin,pMax,p) returns f(clamp(p,pMin,pMax)).
//
//	lookup3D(t,s,pMin,pMax,p)
//
//		Lookup table t, which contains s.x by s.y by s.z
//		entries, defines a piecewise trilinear function, f, with
//		(s.x - 1) by (s.y - 1) by (s.z - 1) segments, such that
//		f(pMin) == t[0][0][0] and f(pMax) == t[s.x-1][s.y-1][s.z-1].
//
//		lookup1D(t,s,pMin,pMax,p) returns f(pClamp), where
//
//		pClamp == V3f (clamp (p.x, pMin.x, pMax.x),
//		               clamp (p.y, pMin.y, pMax.y),
//		               clamp (p.z, pMin.z, pMax.z))
//
//		Since C++ doesn't directly support 3D arrays whose size
//		is not known at compile time, lookup table t is passed
//		to lookup3D as a 1D array, with table entry t[i][j][k]
//		at location t[(i * s.y + j) * s.z + k];
//
//	interpolate1D(t,s,p)
//
//		Lookup table with linear interpolation between entries
//              of type float[2]: the table, t, defines a piecewise
//		linear function, f, with s-1 segments, such that f(x)
//		interpolates between t[i][1] and t[i+1][1] for
//		t[i][0]< x < t[i+1][0].
//
//		interpolate1D(t,s,p) returns f(clamp(p,t[0][0],t[s-1][0])).
//
//	interpolateCubic1D(t,s,p)
//
//		Like interpolate1D(t,s,p), except with piecewise cubic
//		rather than linear interpolation of the table entries.
//
//-----------------------------------------------------------------------------

#include <CtlNumber.h>

namespace Ctl {

number	 	lookup1D (const number table[],
			  int size,
			  number pMin,
			  number pMax,
			  number p);


number	 	lookupCubic1D (const number table[],
			       int size,
			       number pMin,
			       number pMax,
			       number p);

Vec3	lookup3D (const Vec3 table[],
			  const Vec3i &size,
			  const Vec3 &pMin,
			  const Vec3 &pMax,
			  const Vec3 &p);

number		interpolate1D (const number table[][2],
			       int size,
			       number p);

number		interpolateCubic1D (const number table[][2],
				    int size,
				    number p);

} // namespace Ctl

#endif
