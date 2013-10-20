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

#ifndef INCLUDE_FILE_IO_H
#define INCLUDE_FILE_IO_H

#include <stdint.h>
#include <vector>
#include <string>

#include <dpx.hh>

struct compression_t
{
	inline compression_t( const char *n, int s ) : name( n ), exrCompressionScheme( s ) {}
	std::string name;
	int exrCompressionScheme;
};

const std::vector<compression_t> &getAvailableCompressionSchemes( void );
compression_t findCompressionByName( const std::string &name );

struct format_t
{
	inline format_t( void )
			: bps( 0 ), src_bps( 0 ), squish( false ), descriptor( 0 )
	{}
	inline format_t( const char *_ext, uint8_t _bps )
			: ext( _ext ), bps( _bps ),
			  src_bps( 0 ), squish( false ), descriptor( 0 )
	{}

	std::string ext;
	uint8_t bps;
	uint8_t src_bps;
	bool squish; // remove alpha channel.
	uint8_t descriptor; // DPX enumeration, plus the following values:
	                    // 157 - XYZ
	                    // 158 - XYZA
	                    // 159 - YA
	                    // 160 - RA
	                    // 161 - GA
	                    // 162 - BA
};

struct file_format_t
{
	inline file_format_t( const char *n, const format_t &fmt ) : name( n ), output_fmt( fmt ) {}

	std::string name;
	format_t output_fmt;
};

const std::vector<file_format_t> &getAllowedFormats( void );

bool
readImage( const std::string &filename, float scale,
		   ctl::dpx::fb<float> &pixels,
		   format_t &format );

bool
writeImage( const std::string &filename, float scale,
			const ctl::dpx::fb<float> &pixels,
			const format_t &format,
			const compression_t &compression );

#endif
