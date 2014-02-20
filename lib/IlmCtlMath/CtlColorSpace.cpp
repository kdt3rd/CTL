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
//	Color space conversions
//
//	For an explanation of how the RGB to XYZ conversion matrix is derived,
//	see Roy Hall, "Illumination and Color in Computer Generated Imagery",
//	Springer-Verlag, 1989, chapter 3, "Perceptual Response"; and 
//	Charles A. Poynton, "A Technical Introduction to Digital Video",
//	John Wiley & Sons, 1996, chapter 7, "Color science for video".
//
//	The conversions between XYZ and L*u*v* and between XYZ and L*a*b*
//	are taken from chapter 1 of "Digital Color Imaging Handbook", by
//	Gaurav Sharma (ed.), CRC Press, 2003.
//
//-----------------------------------------------------------------------------

#include <CtlColorSpace.h>

using namespace Imath;

namespace Ctl {

M44
RGBtoXYZ (const Chromaticities &chroma, number Y)
{
    //
    // X and Z values of RGB value (1, 1, 1), or "white"
    //

    number X = chroma.white.x * Y / chroma.white.y;
    number Z = (1 - chroma.white.x - chroma.white.y) * Y / chroma.white.y;

    //
    // Scale factors for matrix rows
    //

    number d = chroma.red.x   * (chroma.blue.y  - chroma.green.y) +
		chroma.blue.x  * (chroma.green.y - chroma.red.y) +
		chroma.green.x * (chroma.red.y   - chroma.blue.y);

    number Sr = (X * (chroma.blue.y - chroma.green.y) -
				 chroma.green.x * (Y * (chroma.blue.y - 1) +
								   chroma.blue.y  * (X + Z)) +
				 chroma.blue.x  * (Y * (chroma.green.y - 1) +
								   chroma.green.y * (X + Z))) / d;

    number Sg = (X * (chroma.red.y - chroma.blue.y) +
				 chroma.red.x   * (Y * (chroma.blue.y - 1) +
								   chroma.blue.y  * (X + Z)) -
				 chroma.blue.x  * (Y * (chroma.red.y - 1) +
								   chroma.red.y   * (X + Z))) / d;

    number Sb = (X * (chroma.green.y - chroma.red.y) -
				 chroma.red.x   * (Y * (chroma.green.y - 1) +
								   chroma.green.y * (X + Z)) +
				 chroma.green.x * (Y * (chroma.red.y - 1) +
								   chroma.red.y   * (X + Z))) / d;

    //
    // Assemble the matrix
    //

    M44 M;

    M[0][0] = Sr * chroma.red.x;
    M[0][1] = Sr * chroma.red.y;
    M[0][2] = Sr * (1 - chroma.red.x - chroma.red.y);

    M[1][0] = Sg * chroma.green.x;
    M[1][1] = Sg * chroma.green.y;
    M[1][2] = Sg * (1 - chroma.green.x - chroma.green.y);

    M[2][0] = Sb * chroma.blue.x;
    M[2][1] = Sb * chroma.blue.y;
    M[2][2] = Sb * (1 - chroma.blue.x - chroma.blue.y);

    return M;
}


M44
XYZtoRGB (const Chromaticities &chroma, number Y)
{
    return RGBtoXYZ (chroma, Y).inverse();
}

namespace {

number
f (number x)
{
    if (x > number(0.008856))
	return std::pow (x, number(1. / 3.));

    return number(7.787) * x + number(16. / 116.);
}


number
fInverse (number t)
{
    if (t > number(0.206893))
	return t * t * t;

    return number(1. / 7.787) * (t - number(16. / 116.));
}


number
uprime (const Vec3 &XYZ)
{
    return (4 * XYZ.x) / (XYZ.x + 15 * XYZ.y + 3 * XYZ.z);
}


number
vprime (const Vec3 &XYZ)
{
    return (9 * XYZ.y) / (XYZ.x + 15 * XYZ.y + 3 * XYZ.z);
}


} // namespace


Vec3
XYZtoLuv (const Vec3 &XYZ, const Vec3 &XYZn)
{
    number Lstar = 116 * f (XYZ.y / XYZn.y) - 16;
    number ustar = 13 * Lstar * (uprime (XYZ) - uprime (XYZn));
    number vstar = 13 * Lstar * (vprime (XYZ) - vprime (XYZn));

    return Vec3 (Lstar, ustar, vstar);
}


Vec3
LuvtoXYZ (const Vec3 &Luv, const Vec3 &XYZn)
{
    number Lstar = Luv.x;
    number ustar = Luv.y;
    number vstar = Luv.z;
    number unprime = uprime (XYZn);
    number vnprime = vprime (XYZn);

    number fY = (Lstar + 16) / 116;
    number Y = XYZn.y * fInverse (fY);
    number d = 4 * (13 * Lstar * vnprime + vstar);
    number X = 9 * (13 * Lstar * unprime + ustar) * Y / d;
    number Z = -(3 * ustar + 13 * Lstar *
	      (-12 + 3 * unprime + 20 * vnprime) + 20 * vstar) * Y / d;

    return Vec3 (X, Y, Z);
}


Vec3
XYZtoLab (const Vec3 &XYZ, const Vec3 &XYZn)
{
    number Lstar = 116 * f (XYZ.y / XYZn.y) - 16;
    number astar = 500 * (f (XYZ.x / XYZn.x) - f (XYZ.y / XYZn.y));
    number bstar = 200 * (f (XYZ.y / XYZn.y) - f (XYZ.z / XYZn.z));

    return Vec3 (Lstar, astar, bstar);
}


Vec3
LabtoXYZ (const Vec3 &Lab, const Vec3 &XYZn)
{
    number Lstar = Lab.x;
    number astar = Lab.y;
    number bstar = Lab.z;

    number fY = (Lstar + 16) / 116;
    number fX = astar / 500 + fY;
    number fZ = fY - bstar / 200;

    number X = XYZn.x * fInverse (fX);
    number Y = XYZn.y * fInverse (fY);
    number Z = XYZn.z * fInverse (fZ);

    return Vec3 (X, Y, Z);
}

} // namespace Ctl
