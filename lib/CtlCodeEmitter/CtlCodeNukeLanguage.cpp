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

#include "CtlCodeNukeLanguage.h"
#include <stdexcept>

namespace Ctl
{


////////////////////////////////////////


NukeGenerator::NukeGenerator( void )
		: CPPGenerator( false )
{
}


////////////////////////////////////////


NukeGenerator::~NukeGenerator( void )
{
}


////////////////////////////////////////


std::string
NukeGenerator::getDriver( void )
{
	std::stringstream driver;

	driver <<
		"// Automatically generated Nuke driver code\n\n"
		"#include \"DDImage/PixelIop.h\"\n"
		"#include \"DDImage/Row.h\"\n"
		"#include \"DDImage/Knobs.h\"\n"
		"#include \"DDImage/NukeWrapper.h\"\n"
		"#include \"DDImage/DDMath.h\"\n"
		"#include \"DDImage/RGB.h\"\n"
		"\nusing namespace DD::Image;\n\n"
		"\n";

	pushStream( driver );

	for ( MainRoutineMap::const_iterator i = myMainRoutines.begin(); i != myMainRoutines.end(); ++i )
	{
		const std::string &name = i->first;
		const std::string &nsName = i->second.first;
		const SymbolInfoPtr &fnInfo = i->second.second;
		FunctionTypePtr functionType = fnInfo->functionType();
		const ParamVector &params = functionType->parameters();
		std::vector<std::string> fargs;
		std::vector<std::string> hargs;
		std::vector<std::string> inargs;
		std::vector<std::string> inoutargs;
		std::vector<std::string> outargs;
		std::map<int, std::string> chanMapIn;
		std::map<int, std::string> chanMapOut;
		int maxCin = 0;
		int maxCout = 0;
		for ( size_t i = 0, N = params.size(); i != N; ++i )
		{
			const Param &parm = params[i];
			if ( parm.varying )
			{
				if ( parm.access == RWA_READ )
				{
					inargs.push_back( parm.name );
					chanMapIn[maxCin++] = parm.name;
				}
				if ( parm.access == RWA_WRITE )
				{
					outargs.push_back( parm.name );
					chanMapOut[maxCout++] = parm.name;
				}
				if ( parm.access == RWA_READWRITE )
				{
					inoutargs.push_back( parm.name );
					chanMapIn[maxCin++] = parm.name;
					chanMapOut[maxCout++] = parm.name;
				}

				if ( parm.type->cDataType() != FloatTypeEnum &&
					 parm.type->cDataType() != HalfTypeEnum )
					throw std::logic_error( "Unhandled varying type" );
				continue;
			}

			switch ( parm.type->cDataType() )
			{
				case FloatTypeEnum: fargs.push_back( parm.name ); break;
				case HalfTypeEnum: hargs.push_back( parm.name ); break;
					break;
				default:
					throw std::logic_error( "Sorry, argument type to main function for argument '" + parm.name + "' not yet handled" );
			}
		}

		if ( maxCin != maxCout )
			throw std::logic_error( "Unhandled different number of image channels in and out" );

		if ( maxCin < 3 || maxCin > 4 )
			throw std::logic_error( "Unhandled number of channels in and out (only support 3 or 4" );

		std::string cName = name + "_Iop";

		curStream() << "class " << cName << " : public PixelIop\n"
			"{\n";

		pushIndent();
		for ( size_t i = 0; i != fargs.size(); ++i )
		{
			newlineAndIndent();
			curStream() << "double m_parm_" << fargs[i] << ';';
		}
		for ( size_t i = 0; i != hargs.size(); ++i )
		{
			newlineAndIndent();
			curStream() << "double m_parm_" << hargs[i] << ';';
		}
		popIndent();
		newlineAndIndent();
		curStream() << "public:";
		pushIndent();
		newlineAndIndent();
		curStream() << cName << "( Node *node ) : PixelIop( node )";
		pushBlock();
		for ( size_t i = 0; i != fargs.size(); ++i )
		{
			newlineAndIndent();
			curStream() << "m_parm_" << fargs[i] << " = 0.0;";
		}
		for ( size_t i = 0; i != hargs.size(); ++i )
		{
			newlineAndIndent();
			curStream() << "m_parm_" << hargs[i] << " = 0.0;";
		}
		popBlock();
		newlineAndIndent();

		curStream() << "void in_channels( int input_number, ChannelSet &channels ) const";
		pushBlock();
		newlineAndIndent();
		curStream() << "ChannelSet done;";
		newlineAndIndent();
		curStream() << "foreach (z, channels)";
		pushBlock();
		newlineAndIndent();
		curStream() << "if ( colourIndex(z) < " << maxCin << " && ! ( done & z ) )";
		pushBlock();
		newlineAndIndent();
		curStream() << "done.addBrothers( z, " << maxCin << " );";
		popBlock();
		popBlock();
		newlineAndIndent();
		curStream() << "channels += done;";
		popBlock();

		newlineAndIndent();
		curStream() << "void knobs( Knob_Callback f )";
		pushBlock();
		for ( size_t i = 0; i != fargs.size(); ++i )
		{
			newlineAndIndent();
			curStream() << "Double_knob( f, &m_parm_" << fargs[i] << ", IRange( 0, 1 ), \"" << fargs[i] << "\" );";
		}
		for ( size_t i = 0; i != hargs.size(); ++i )
		{
			newlineAndIndent();
			curStream() << "Double_knob( f, &m_parm_" << fargs[i] << ", IRange( 0, 1 ), \"" << fargs[i] << "\" );";
		}
		newlineAndIndent();
		popBlock();
		
		newlineAndIndent();
		curStream() << "void _validate( bool for_real )";
		pushBlock();
		newlineAndIndent();
		curStream() << "set_out_channels( Mask_All );";
		newlineAndIndent();
		curStream() << "PixelIop::_validate( for_real );";
		popBlock();
		
		newlineAndIndent();
		curStream() << "void pixel_engine( const Row &in, int y, int x, int r, ChannelMask channels, Row &out )";
		pushBlock();

		for ( size_t i = 0; i != fargs.size(); ++i )
		{
			newlineAndIndent();
			curStream() << "float " << fargs[i] << " = static_cast<float>( m_parm_" << fargs[i] << " );";
		}
		for ( size_t i = 0; i != hargs.size(); ++i )
		{
			newlineAndIndent();
			curStream() << "half " << fargs[i] << " = static_cast<float>( m_parm_" << fargs[i] << " );";
		}
		newlineAndIndent(); curStream() << "ChannelSet done;";

		newlineAndIndent(); curStream() << "foreach ( z, channels )";
		pushBlock();
		newlineAndIndent();
		curStream() << "if ( done & z ) continue;\n";

		newlineAndIndent();
		curStream() << "if ( colourIndex( z ) >= " << maxCout << " )";
		pushBlock();
		newlineAndIndent();
		curStream() << "out.copy( in, z, x, r );";
		newlineAndIndent();
		curStream() << "continue;";
		popBlock(); // if colourIndex(z)

		for ( int i = 0; i < maxCin; ++i )
		{
			newlineAndIndent();
			curStream() << "Channel chan" << i << " = brother( z, " << i << " );";
			newlineAndIndent();
			curStream() << "done += chan" << i << ';';
			newlineAndIndent();
			curStream() << "const float *input" << i << " = in[chan" << i << "] + x;";
			newlineAndIndent();
			curStream() << "float *output" << i << " = out.writable(chan" << i << ") + x;";
			if ( i == 0 )
			{
				newlineAndIndent();
				curStream() << "const float *END = input0 + (r - x);";
			}
		}

		for ( size_t i = 0; i != inargs.size(); ++i )
		{
			newlineAndIndent();
			curStream() << "ctl_number_t " << inargs[i] << ";";
		}
		for ( size_t i = 0; i != inoutargs.size(); ++i )
		{
			newlineAndIndent();
			curStream() << "ctl_number_t " << inoutargs[i] << ";";
		}
		for ( size_t i = 0; i != outargs.size(); ++i )
		{
			newlineAndIndent();
			curStream() << "ctl_number_t " << outargs[i] << ";";
		}
		for ( size_t i = 0; i != fargs.size(); ++i )
		{
			newlineAndIndent();
			curStream() << "ctl_number_t " << fargs[i] << " = static_cast<ctl_number_t>( m_parm_" << fargs[i] << " );";
		}

		newlineAndIndent();
		curStream() << "while ( input0 < END )";
		pushBlock();
		for ( int i = 0; i < maxCin; ++i )
		{
			newlineAndIndent();
			curStream() << chanMapIn[i] << " = *input" << i << "++;";
		}
		newlineAndIndent();
		curStream() << nsName << "( ";
		for ( size_t i = 0, N = params.size(); i != N; ++i )
		{
			if ( i > 0 ) curStream() << ", ";

			const Param &parm = params[i];
			if ( parm.varying )
			{
				curStream() << parm.name;
				continue;
			}

			switch ( parm.type->cDataType() )
			{
				case FloatTypeEnum: curStream() << parm.name; break;
				case HalfTypeEnum: curStream() << parm.name; break;
				default:
					throw std::logic_error( "Sorry, argument type to main function for argument '" + parm.name + "' not yet handled" );
			}
		}
		curStream() << " );";
		for ( int i = 0; i < maxCout; ++i )
		{
			newlineAndIndent();
			curStream() << "*output" << i << "++ = " << chanMapOut[i] << ";";
		}

		popBlock(); // while input < END

		popBlock(); // foreach

		popBlock(); // pixel_engine

		curStream() << "\n";
		newlineAndIndent();
		curStream() << "static const Iop::Description d;";
		newlineAndIndent();
		curStream() << "const char *Class() const { return d.name; }";
		newlineAndIndent();
		curStream() << "const char *node_help() const { return \"Nuke Node generated from CTL code for function " << name << "\"; }";
		curStream() << "\n}; // class " << cName << "\n\n"
			"static Iop *build_" << name << "( Node *node )\n"
			"{\n"
			"    return (new NukeWrapper( new " << cName <<
			"( node ) ) )->channels( " << (maxCin == 3 ? "Mask_RGB" : "Mask_RGBA") << " );\n"
			"}\n\n"
			"const Iop::Description " << cName << "::d( \"" << name << "\", \"Color/" << name << "\", build_" << name << " );\n\n";
	}

	popStream();

	return driver.str();
}

}
