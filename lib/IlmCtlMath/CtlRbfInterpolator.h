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


#ifndef INCLUDED_RBF_INTERPOLATOR_H
#define INCLUDED_RBF_INTERPOLATOR_H

//----------------------------------------------------------------------------
//
//	class RbfInterpolator -- performs scattered data
//	interpolation on a set of 3D points.
//
//	Author: Fred Pighin
//
//----------------------------------------------------------------------------

#include <assert.h>
#include <ImathVec.h>
#include <ImathBox.h>
#include <vector>
#include <CtlNumber.h>

namespace Ctl {

class PointTree;

class RbfInterpolator
{
  public:

    //------------------------------------------------------------
    // Constructor:
    //
    // We have n pairs, p[0], p[1], ... p[n-1], of 3D points.
    // Each pair represents a sample of an unknown function, f.
    // For 0 <= i < n, f(p[i][0]) == f(p[i][1]).
    // The newly constructed RbfInterpolator object approximates
    // f by interpolating the given samples with a C2-continuous
    // function, g, where g is the sum of a set of n appropriately
    // shaped radial basis functions.
    //------------------------------------------------------------

     RbfInterpolator (size_t n, const Vec3 p[/*n*/][2]);
    ~RbfInterpolator ();

    //---------------------------------------------------------------
    // Evaluation:
    //
    // value(x) returns g(x).  If x == p[i][0] for some i with
    // 0 <= i < n, then value(x) is approximately equal to p[i][1].
    // (Floating-point ounding errors will typically prevent value(x)
    // from being exactly equal to p[i][1].)
    //
    // gradient(x) returns the gradient of g(x).
    //---------------------------------------------------------------

    Vec3  value (const Vec3 &x) const;
    Vec3  gradient (const Vec3 &x) const;


  private:    
        
    big_number	kernel (big_number val,  big_number sigma) const;
    big_number	kernelGrad (big_number val, big_number sigma) const;

    std::vector <Vec3>	_samplePts;
    size_t			_numSamples;
    std::vector <big_number>	_lambdas;
    std::vector <big_number>	_sigmas;
    std::vector <big_number>	_affine;
    big_number			_maxSigma;
    PointTree *			_pointTree;
};

} // namespace Ctl

#endif
