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

//----------------------------------------------------------------------------
//
//	class RbfInterpolator -- performs scattered data
//
//----------------------------------------------------------------------------

#include <CtlRbfInterpolator.h>
#include <CtlPointTree.h>
#include <CtlSparseMatrix.h>
#include <CtlLinearSolver.h>

using namespace std;
using namespace Imath;

//#define DEBUG_RBF

namespace Ctl {


inline big_number 
RbfInterpolator::kernel (big_number val, big_number sigma) const
{
    assert(sigma > .0);

    if (val > 2.*sigma)
	return .0;
    else
    {
	big_number r = val/sigma;
	  
	if (r > 1.)
	{
	    r -= 2.;	    
	    return (-.25*r*r*r) / (M_PI*sigma);
	}
	else 
	{
	    big_number r2 = r*r;

	    return (1 - 1.5*r2 + .75*r*r2) / (M_PI*sigma);
	}
    }
}


inline big_number 
RbfInterpolator::kernelGrad (big_number val, big_number sigma) const
{
    assert(sigma > .0);

    if (val > 2.*sigma)
	return .0;
    else
    {
	big_number r = val/sigma;
	  
	if (val > sigma)
	{
	    r -= 2.;	    
	    return (-.75*r*r) / (M_PI*sigma);
	}
	else 
	{
	    return (-3*r + 2.25*r*r) / (M_PI*sigma);
	}
    }
}


RbfInterpolator::RbfInterpolator (size_t n, const Vec3 p[/*n*/][2]):
    _samplePts (n),
    _numSamples (n),
    _lambdas (3*_numSamples),
    _sigmas (_numSamples),
    _affine (12),
    _pointTree (0)
{
    for ( size_t s = 0; s < _numSamples; s++)
	_samplePts[s] = p[s][0];

    std::vector<big_number> b(3*_numSamples);
    std::vector<big_number> bX(_numSamples);
    std::vector<big_number> bY(_numSamples);
    std::vector<big_number> bZ(_numSamples);
    std::vector<big_number> valSparse;
    std::vector<size_t> colInd;
    std::vector<size_t> rowPts;
    
    // Solve for affine term    
    rowPts.push_back(0);	    

    size_t endRow = 0;
    for ( size_t s = 0; s < _numSamples; s++)
    {
	valSparse.push_back (p[s][0][0]);
	colInd.push_back (0);
	valSparse.push_back (p[s][0][1]);
	colInd.push_back (1);
	valSparse.push_back (p[s][0][2]);
	colInd.push_back (2);
	valSparse.push_back (1.);
	colInd.push_back (3);
	
	endRow += 4;	
	rowPts.push_back(endRow);	    
	b[3*s+0] = p[s][1][0];

	valSparse.push_back (p[s][0][0]);
	colInd.push_back (4);
	valSparse.push_back (p[s][0][1]);
	colInd.push_back (5);
	valSparse.push_back (p[s][0][2]);
	colInd.push_back (6);
	valSparse.push_back (1.);
	colInd.push_back (7);
	
	endRow += 4;	
	rowPts.push_back(endRow);	    
	b[3*s+1] = p[s][1][1];

	valSparse.push_back (p[s][0][0]);
	colInd.push_back (8);
	valSparse.push_back (p[s][0][1]);
	colInd.push_back (9);
	valSparse.push_back (p[s][0][2]);
	colInd.push_back (10);
	valSparse.push_back (1.);
	colInd.push_back (11);
	
	endRow += 4;	
	rowPts.push_back(endRow);	 
	b[3*s+2] = p[s][1][2];	
    }
    CRSOperator<big_number> OMAAff(valSparse, colInd, rowPts, 12);
    
    LSSCG<big_number, CRSOperator<big_number> > lssAffine(OMAAff);

    fill(_affine.begin(), _affine.end(), 0.0);
    
    lssAffine.solver.maxNumIterations = 12;
    lssAffine.solver.tolerance = 1.e-7;
        
    lssAffine(b.begin(), b.end(), _affine.begin(), _affine.end());

    // Fit residual with Rbf
	
    // Use k-d tree for fast spatial queries
    _pointTree = new PointTree (&_samplePts[0], _numSamples);

    // Compute spread for each kernel
    size_t numNeighbor = 4;

    _maxSigma = .0;    
    for ( size_t i = 0; i < _numSamples; i++) 
    {	
	std::vector <size_t> indices;

	_pointTree->nearestPoints (_samplePts[i], numNeighbor, indices);
	
	big_number sum = .0;
	for (size_t n = 0; n < indices.size(); n++)
	{
	    size_t nidx = indices[n];
	    big_number delta[3];
	    
	    delta[0] = _samplePts[i][0] - _samplePts[nidx][0];
	    delta[1] = _samplePts[i][1] - _samplePts[nidx][1];
	    delta[2] = _samplePts[i][2] - _samplePts[nidx][2];

	    sum += delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2];
	}
	
	_sigmas[i] = .5 * std::sqrt(sum);

	if ( _sigmas[i] > _maxSigma)
	    _maxSigma = _sigmas[i];
    }      

#ifdef DEBUG_RBF
    std::cout << "Sigmas" << std::endl;
    for ( size_t i = 0; i < _numSamples; i++) 
    {	
	std::cout << _sigmas[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "MaxSigma = " << _maxSigma << std::endl;
#endif

    // Solve for the weights (lambdas) using a sparse solver
    valSparse.clear();
    colInd.clear();
    rowPts.clear();
    
    rowPts.push_back(0);	    

    endRow = 0;
    for ( size_t s = 0; s < _numSamples; s++)
    {
	std::vector <size_t> indices;
	
	Vec3 center = _samplePts[s];
	_pointTree->intersect(center, 2.*_maxSigma, indices);
		
	size_t nonZero = 0;
	for (size_t n = 0; n < indices.size(); n++)
	{
	    size_t nidx = indices[n];
	    big_number dist = (center - _samplePts[nidx]).length();
	    big_number weight = kernel (dist, _sigmas[nidx]);
	    
	    if (weight > .0) 
	    {
		valSparse.push_back (weight);
		colInd.push_back (nidx);
		nonZero++;
	    }
	}

	// fit data minus estimated affine function
	bX[s] = p[s][1][0] - (_affine[0]*p[s][0][0] + _affine[1]*p[s][0][1] + _affine[2]*p[s][0][2] + _affine[3]);
	bY[s] = p[s][1][1] - (_affine[4]*p[s][0][0] + _affine[5]*p[s][0][1] + _affine[6]*p[s][0][2] + _affine[7]);
	bZ[s] = p[s][1][2] - (_affine[8]*p[s][0][0] + _affine[9]*p[s][0][1] + _affine[10]*p[s][0][2] + _affine[11]);
	
	endRow += nonZero;
	
	if (endRow > 0)
	    rowPts.push_back(endRow);	    
    }

    CRSOperator<big_number> OMA(valSparse, colInd, rowPts, _numSamples);
    
    LSSCG<big_number, CRSOperator<big_number> > lss(OMA);

    std::vector<big_number> solX(_numSamples, 0.0);
    std::vector<big_number> solY(_numSamples, 0.0);
    std::vector<big_number> solZ(_numSamples, 0.0);
    
    lss.solver.maxNumIterations = 30*_numSamples;
    lss.solver.tolerance = 1.e-7;
    
#ifndef DEBUG_RBF
    lss(bX.begin(), bX.end(), solX.begin(), solX.end());
    lss(bY.begin(), bY.end(), solY.begin(), solY.end());
    lss(bZ.begin(), bZ.end(), solZ.begin(), solZ.end());        
#else
    std::vector<big_number> s(_numSamples);
    big_number tolX = lss(bX.begin(), bX.end(), solX.begin(), solX.end());
    big_number tolY = lss(bY.begin(), bY.end(), solY.begin(), solY.end());
    big_number tolZ = lss(bZ.begin(), bZ.end(), solZ.begin(), solZ.end());    
    
    std::cout << "bX\n";
    copy(bX.begin(), bX.end(), ostream_iterator<big_number>(std::cout, " "));
    cout << "\nsX\n";
    OMA.apply(solX.begin(), solX.end(), s.begin(), s.end());    
    copy(s.begin(), s.end(), ostream_iterator<big_number>(cout, " "));

    std::cout << "\n\nbY\n";
    copy(bY.begin(), bY.end(), ostream_iterator<big_number>(std::cout, " "));
    cout << "\nsY\n";
    OMA.apply(solY.begin(), solY.end(), s.begin(), s.end());    
    copy(s.begin(), s.end(), ostream_iterator<big_number>(cout, " "));

    std::cout << "\n\nbZ\n";
    copy(bZ.begin(), bZ.end(), ostream_iterator<big_number>(std::cout, " "));
    cout << "\nsZ\n";
    OMA.apply(solZ.begin(), solZ.end(), s.begin(), s.end());    
    copy(s.begin(), s.end(), ostream_iterator<big_number>(cout, " "));


    std::cout << "\n\nTolerance " << tolX << " " << tolY << " " << tolZ <<
    std::endl;
#endif

    for (size_t s = 0; s < _numSamples; s++) 
    {
	_lambdas[3*s+0] = solX[s];
	_lambdas[3*s+1] = solY[s];
	_lambdas[3*s+2] = solZ[s];
    }
}


RbfInterpolator::~RbfInterpolator ()
{
    delete _pointTree;
}


Vec3
RbfInterpolator::value (const Vec3 &x) const
{
    std::vector <size_t> indices;
    _pointTree->intersect(x, 2.*_maxSigma, indices);
    
    big_number sumX = .0;
    big_number sumY = .0;
    big_number sumZ = .0;

    for (size_t n = 0; n < indices.size(); n++)
    {
	size_t nidx = indices[n];
	
	big_number weight = kernel((_samplePts[nidx] - x).length(), 
			       _sigmas[nidx]);
	
	sumX += weight*_lambdas[3*nidx];
	sumY += weight*_lambdas[3*nidx+1];
	sumZ += weight*_lambdas[3*nidx+2];
    }    

    sumX += _affine[0]*x[0] + _affine[1]*x[1] + _affine[2]*x[2] + _affine[3];
    sumY += _affine[4]*x[0] + _affine[5]*x[1] + _affine[6]*x[2] + _affine[7];
    sumZ += _affine[8]*x[0] + _affine[9]*x[1] + _affine[10]*x[2] + _affine[11];

    return Vec3(sumX, sumY, sumZ);
}


Vec3
RbfInterpolator::gradient (const Vec3 &x) const
{
    std::vector <size_t> indices;
    _pointTree->intersect(x, 2.*_maxSigma, indices);
    
    big_number sumX = .0;
    big_number sumY = .0;
    big_number sumZ = .0;
    
    for (size_t n = 0; n < indices.size(); n++)
    {
	size_t nidx = indices[n];
	
	big_number weight = kernelGrad((_samplePts[nidx] - x).length(), 
				   _sigmas[nidx]);
	
	sumX += weight*_lambdas[3*nidx];
	sumY += weight*_lambdas[3*nidx+1];
	sumZ += weight*_lambdas[3*nidx+2];
    }    
    
    return Vec3(sumX, sumY, sumZ);
}

} // namespace Ctl
