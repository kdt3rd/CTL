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

#include "CtlCodeCPPLanguage.h"
#include "CtlCodeLContext.h"
#include "CtlCodeType.h"
#include "CtlCodeAddr.h"
#include "CtlCodeModule.h"
#include <iomanip>
#include <limits>
#include <half.h>
#include <deque>
#include <ctype.h>
#include <stdexcept>
#include <cstdlib>
#include <algorithm>

namespace Ctl
{

CPPGenerator::CPPGenerator( bool cpp11 )
		: CCommonLanguage(),
		  myCPP11Mode( cpp11 )
{
}


////////////////////////////////////////


CPPGenerator::~CPPGenerator( void )
{
}


////////////////////////////////////////


bool
CPPGenerator::supportsPrecision( Precision p ) const
{
	return true;
}


////////////////////////////////////////


std::string
CPPGenerator::getDriver( void )
{
	std::stringstream driver;

	driver <<
		"// Automatically generated driver code\n\n"
		"#include <vector>\n"
		"#include <string>\n"
		"#include <stdexcept>\n"
		"#include <dpx.hh>\n";
	if ( myCPP11Mode )
	{
		
		driver <<
			"#include <thread>\n"
			"#include <mutex>\n"
			"#include <condition_variable>\n"
			"\n"
			"static std::condition_variable theWaitCond;\n"
			"static std::condition_variable theWorkCond;\n"
			"static size_t theFinishCount = 0;\n";
	}
	else
	{
		driver <<
			"#ifdef WIN32\n"
			"# error \"Need someone to implement thread routines\"\n"
			"#endif\n"
			"#include <errno.h>\n"
			"#include <pthread.h>\n"
			"#include <unistd.h>\n"
			"\n\n"
			"static pthread_cond_t theWaitCond;\n"
			"static pthread_cond_t theWorkCond;\n"
			"static size_t theFinishCount = 0;\n";
	}

	driver <<
		"\n\n"
		"typedef void(*dispatchFuncPtr)( int, int, ctl::dpx::fb<float> & );\n"
		"static std::vector<dispatchFuncPtr> theWorkFunc;\n"
		"static ctl::dpx::fb<float> *theWorkImage = NULL;\n"
		"\n";

	if ( myCPP11Mode )
	{
		driver <<
			"static std::mutex theMutex;\n"
			"static std::vector<std::thread> theThreads;\n"
			"\n"
			"void threadLoop( size_t i )\n"
			"{\n"
			"    while ( true )\n"
			"    {\n"
			"        dispatchFuncPtr curFunc = nullptr;\n"
			"        ctl::dpx::fb<float> *curImg = nullptr;\n"
			"        size_t nThreads = 1;\n"
			"        {\n"
			"            std::unique_lock<std::mutex> lk( theMutex );\n"
			"            nThreads = theThreads.size();\n"
			"            while ( ! theWorkFunc[i] )\n"
			"            {\n"
			"                if ( theThreads.empty() ) return;\n"
			"                theWorkCond.wait( lk );\n"
			"                if ( theThreads.empty() ) return;\n"
			"            }\n"
			"            curFunc = theWorkFunc[i];\n"
			"            curImg = theWorkImage;\n"
			"            theWorkFunc[i] = NULL;\n"
			"        }\n"
			"        if ( curFunc && curImg )\n"
			"        {\n"
			"            size_t nPer = ( curImg->height() + nThreads - 1 ) / nThreads;\n"
			"            int startY = nPer * i;\n"
			"            int endY = nPer * (i + 1);\n"
			"            if ( i == nThreads - 1 ) endY = curImg->height();\n"
			"            (*curFunc)( startY, endY, *curImg );\n"
			"            std::unique_lock<std::mutex> lk( theMutex );\n"
			"            ++theFinishCount;\n"
			"            theWaitCond.notify_one();\n"
			"        }\n"
			"    }\n"
			"}\n";
	}
	else
	{
		driver <<
			"static pthread_mutex_t theMutex = PTHREAD_MUTEX_INITIALIZER;\n"
			"static std::vector<pthread_t> theThreads;\n"
			"\n"
			"void *threadLoop( void *arg )\n"
			"{\n"
			"    size_t i = reinterpret_cast<size_t>( arg );\n"
			"    while ( true )\n"
			"    {\n"
			"        dispatchFuncPtr curFunc = NULL;\n"
			"        ctl::dpx::fb<float> *curImg = NULL;\n"
			"        pthread_mutex_lock( &theMutex );\n"
			"        size_t nThreads = theThreads.size();\n"
			"        while ( ! theWorkFunc[i] )\n"
			"        {\n"
			"            if ( theThreads.empty() ) { pthread_mutex_unlock( &theMutex ); return NULL; }\n"
			"            pthread_cond_wait( &theWorkCond, &theMutex );\n"
			"            if ( theThreads.empty() ) { pthread_mutex_unlock( &theMutex ); return NULL; }\n"
			"        }\n"
			"        curFunc = theWorkFunc[i];\n"
			"        theWorkFunc[i] = NULL;\n"
			"        curImg = theWorkImage;\n"
			"        pthread_mutex_unlock( &theMutex );\n"
			"        if ( curFunc && curImg )\n"
			"        {\n"
			"            size_t nPer = ( curImg->height() + nThreads - 1 ) / nThreads;\n"
			"            int startY = nPer * i;\n"
			"            int endY = nPer * (i + 1);\n"
			"            if ( i == nThreads - 1 ) endY = curImg->height();\n"
			"            (*curFunc)( startY, endY, *curImg );\n"
			"            pthread_mutex_lock( &theMutex );\n"
			"            ++theFinishCount;\n"
			"            pthread_cond_signal( &theWaitCond );\n"
			"            pthread_mutex_unlock( &theMutex );\n"
			"        }\n"
			"    }\n"
			"    return NULL;\n"
			"}\n";
	}
	
	driver <<
		"\n"
		"class Function\n"
		"{\n"
		"public:\n"
		"	Function( void );\n"
		"	virtual ~Function( void );\n"
		"	virtual const std::string &getName( void ) const = 0;\n"
		"	virtual const std::vector<std::string> &getArgumentFlags( void ) const = 0;\n"
		"	virtual const std::vector<std::string> &getFloatArgs( void ) const = 0;\n"
		"	virtual void setFlag( const std::string &flag ) const = 0;\n"
		"	virtual void setArgument( const std::string &arg, double val ) const = 0;\n"
		"	virtual void apply( ctl::dpx::fb<float> &pixels ) const = 0;\n"
		"};\n"
		"Function::Function( void ) {}\n"
		"Function::~Function( void ) {}\n";
	std::vector<std::string> funcClassPtrs;
	for ( MainRoutineMap::const_iterator i = myMainRoutines.begin(); i != myMainRoutines.end(); ++i )
	{
		const std::string &name = i->first;
		const std::string &nsName = i->second.first;
		const SymbolInfoPtr &fnInfo = i->second.second;
		FunctionTypePtr functionType = fnInfo->functionType();
		const ParamVector &params = functionType->parameters();
		std::vector<std::string> flags;
		std::vector<std::string> fargs;
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

				if ( parm.type->cDataType() != FloatTypeEnum )
					throw std::logic_error( "Unhandled varying type" );
				continue;
			}

			switch ( parm.type->cDataType() )
			{
				case FloatTypeEnum: fargs.push_back( parm.name ); break;
				case BoolTypeEnum: flags.push_back( parm.name ); break;
					break;
				default:
					throw std::logic_error( "Sorry, argument type to main function for argument '" + parm.name + "' not yet handled" );
			}
		}

		if ( maxCin != maxCout )
			throw std::logic_error( "Unhandled different number of image channels in and out" );

		std::string cName = name + "_Function";
		std::string varName = "the_" + cName;

		for ( size_t i = 0; i != flags.size(); ++i )
			driver << "static bool " << cName << "_" << flags[i] << "_flag = false;\n";
		for ( size_t i = 0; i != fargs.size(); ++i )
			driver << "static ctl_number_t " << cName << "_" << fargs[i] << "_number = 0;\n";

		driver <<
			"static void dispatch_" << name << "( int startY, int endY, ctl::dpx::fb<float> &pixels )\n"
			"{\n"
			"    float *img = pixels.ptr();\n"
			"    int w = pixels.width();\n"
			"    int c = pixels.depth();\n"
			"    int linesize = w * c;\n"
			"    img += startY * linesize;\n";
		if ( ! inargs.empty() )
		{
			driver << "    ctl_number_t ";
			for ( size_t i = 0; i != inargs.size(); ++i )
			{
				if ( i > 0 ) driver << ", ";
				driver << inargs[i];
			}
			driver << ";\n";
		}
		if ( ! inoutargs.empty() )
		{
			driver << "    ctl_number_t ";
			for ( size_t i = 0; i != inoutargs.size(); ++i )
			{
				if ( i > 0 ) driver << ", ";
				driver << inoutargs[i];
			}
			driver << ";\n";
		}
		if ( ! outargs.empty() )
		{
			driver << "    ctl_number_t ";
			for ( size_t i = 0; i != outargs.size(); ++i )
			{
				if ( i > 0 ) driver << ", ";
				driver << outargs[i];
			}
			driver << ";\n";
		}
			
		driver <<
			"    switch ( c )\n"
			"    {\n"
			"        case 0: return;\n";
		for ( int curC = 1; curC <= maxCin; ++curC )
		{
			driver << "        case " << curC << ":\n";
			if ( curC == maxCin )
				driver << "        default:\n";

			driver <<
				"            for ( int y = startY; y < endY; ++y )\n"
				"                for ( int x = 0; x < w; ++x, img += c )\n"
				"                {\n";
			for ( int i = 0; i < curC; ++i )
				driver << "                    " << chanMapIn[i] << " = img[" << i << "];\n";
			for ( int i = curC; i < maxCin; ++i )
				driver << "                    " << chanMapIn[i] << " = 0.0;\n";
			driver << "                    " << nsName << "( ";
			for ( size_t i = 0, N = params.size(); i != N; ++i )
			{
				if ( i > 0 ) driver << ", ";

				const Param &parm = params[i];
				if ( parm.varying )
				{
					driver << parm.name;
					continue;
				}
				switch ( parm.type->cDataType() )
				{
					case FloatTypeEnum: driver << cName << "_" << parm.name << "_number"; break;
					case BoolTypeEnum: driver << cName << "_" << parm.name << "_flag"; break;
					default:
						throw std::logic_error( "Sorry, argument type to main function for argument '" + parm.name + "' not yet handled" );
				}
			}
			driver << " );\n";
			for ( int i = 0; i < curC; ++i )
				driver << "                    img[" << i << "] = " << chanMapOut[i] << ";\n";
			driver <<
				"                }\n"
				"            break;\n";
		}
		driver <<
			"    }\n"
			"}\n";
		driver <<
			"class " << cName << " : public Function\n"
			"{\n"
			"public:\n"
			"    " << cName << "( void ) : myName( \"" << name << "\" )\n"
			"    {\n";
		for ( size_t i = 0; i != flags.size(); ++i )
			driver << "        myFlags.push_back( \"" << flags[i] << "\" );\n";
		for ( size_t i = 0; i != fargs.size(); ++i )
			driver << "        myFloatArgs.push_back( \"" << fargs[i] << "\" );\n";
		driver <<
			"    }\n"
			"    virtual ~" << cName << "( void ) {}\n"
			"    virtual const std::string &getName( void ) const { return myName; }\n"
			"    virtual const std::vector<std::string> &getArgumentFlags( void ) const { return myFlags; }\n"
			"    virtual const std::vector<std::string> &getFloatArgs( void ) const { return myFloatArgs; }\n"
			"    virtual void setFlag( const std::string &flag ) const;\n"
			"    virtual void setArgument( const std::string &arg, double val ) const;\n"
			"    virtual void apply( ctl::dpx::fb<float> &pixels ) const;\n"
			"private:\n"
			"    std::string myName;\n"
			"    std::vector<std::string> myFlags;\n"
			"    std::vector<std::string> myFloatArgs;\n"
			"};\n";
		driver << "void\n" << cName <<
			"::setFlag( const std::string &flag ) const\n"
			"{\n";
		for ( size_t i = 0; i != flags.size(); ++i )
			driver << "    if ( flag == \"" << flags[i] << "\" ) " << cName << "_" << flags[i] << "_flag = true;\n";
		driver <<
			"}\n";
		driver << "void\n" << cName <<
			"::setArgument( const std::string &arg, double v ) const\n"
			"{\n";
		for ( size_t i = 0; i != fargs.size(); ++i )
			driver << "    if ( arg == \"" << fargs[i] << "\" ) " << cName << "_" << flags[i] << "_number = v;\n";
		driver <<
			"}\n";

		driver << "void\n" << cName <<
			"::apply( ctl::dpx::fb<float> &pixels ) const\n"
			"{\n";
		if ( myCPP11Mode )
		{
			driver <<
				"    std::unique_lock<std::mutex> lk( theMutex );\n"
				"    if ( theThreads.empty() )\n"
				"    {\n"
				"        dispatch_" << name << "( 0, pixels.height(), pixels );\n"
				"        return;\n"
				"    }\n"
				"    for ( size_t i = 0; i != theThreads.size(); ++i )\n"
				"        theWorkFunc[i] = &dispatch_" << name << ";\n"
				"    theWorkImage = &pixels;\n"
				"    theFinishCount = 0;\n"
				"    theWorkCond.notify_all();\n"
				"    size_t N = theThreads.size();\n"
				"    while ( theFinishCount != N ) theWaitCond.wait( lk );\n"
				"    theWorkImage = NULL;\n";
		}
		else
		{
			driver <<
				"    pthread_mutex_lock( &theMutex );\n"
				"    if ( theThreads.empty() )\n"
				"    {\n"
				"        pthread_mutex_unlock( &theMutex );\n"
				"        dispatch_" << name << "( 0, pixels.height(), pixels );\n"
				"        return;\n"
				"    }\n"
				"    size_t N = theThreads.size();\n"
				"    for ( size_t i = 0; i != N; ++i )\n"
				"        theWorkFunc[i] = &dispatch_" << name << ";\n"
				"    theWorkImage = &pixels;\n"
				"    theFinishCount = 0;\n"
				"    pthread_cond_broadcast( &theWorkCond );\n"
				"    while ( theFinishCount != N ) pthread_cond_wait( &theWaitCond, &theMutex );\n"
				"    theWorkImage = NULL;\n"
				"    theFinishCount = 0;\n"
				"    pthread_mutex_unlock( &theMutex );\n";
		}
		driver << "}\n";
		
		driver << "static " << cName << ' ' << varName << ";\n";
		funcClassPtrs.push_back( varName );
	}
	driver <<
		"\n"
		"class Driver\n"
		"{\n"
		"public:\n"
		"	Driver( void );\n"
		"	~Driver( void );\n"
		"	bool usesThreads( void ) const;\n"
		"	void init( int threads );\n"
		"	void shutdown( void );\n"
		"	const std::vector<Function *> &getFunctions( void );\n"
		"};\n"
		"Driver::Driver( void ) {}\n"
		"Driver::~Driver( void ) { shutdown(); }\n"
		"bool Driver::usesThreads( void ) const { return true; }\n"
		"const std::vector<Function *> &Driver::getFunctions( void )\n"
		"{\n"
		"    static std::vector<Function *> theFuncs;\n"
		"    if ( theFuncs.empty() )\n"
		"    {\n";
	for ( size_t i = 0; i != funcClassPtrs.size(); ++i )
		driver << "        theFuncs.push_back( &" << funcClassPtrs[i] << " );\n";
	driver <<
		"    }\n"
		"    return theFuncs;\n"
		"}\n";

	if ( myCPP11Mode )
	{
		driver <<
			"void Driver::init( int nThreads )\n"
			"{\n"
			"    if ( nThreads < 0 )\n"
			"        nThreads = std::thread::hardware_concurrency();\n"
			"    if ( nThreads > 1 )\n"
			"        theThreads.resize( nThreads );\n"
			"    std::unique_lock<std::mutex> lk( theMutex );\n"
			"    if ( ! theThreads.empty() )\n"
			"    {\n"
			"        theWorkFunc.resize( nThreads );\n"
			"        for ( size_t i = 0, N = theThreads.size(); i != N; ++i )\n"
			"            theThreads[i] = std::thread( &threadLoop, i );\n"
			"    }\n"
			"}\n"
			"void Driver::shutdown( void )\n"
			"{\n"
			"    std::unique_lock<std::mutex> lk( theMutex );\n"
			"    std::vector<std::thread> threads;\n"
			"    std::swap( threads, theThreads );\n"
			"    size_t N = threads.size();\n"
			"    lk.unlock();\n"
			"    theWorkCond.notify_all();\n"
			"    for ( size_t i = 0; i != N; ++i )\n"
			"        threads[i].join();\n"
			"    theWorkFunc.clear();\n"
			"}\n";
	}
	else
	{
		driver <<
			"void Driver::init( int nThreads )\n"
			"{\n"
			"    pthread_mutex_lock( &theMutex );\n"
			"    pthread_cond_init( &theWorkCond, NULL );\n"
			"    pthread_cond_init( &theWaitCond, NULL );\n"
			"    if ( nThreads < 0 )\n"
			"        nThreads = sysconf( _SC_NPROCESSORS_ONLN );\n"
			"    if ( nThreads > 1 )\n"
			"        theThreads.resize( nThreads );\n"
			"    if ( ! theThreads.empty() )\n"
			"    {\n"
			"        theWorkFunc.resize( nThreads );\n"
			"        for ( size_t i = 0, N = theThreads.size(); i != N; ++i )\n"
			"            pthread_create( &theThreads[i], NULL, &threadLoop, reinterpret_cast<void *>( i ) );\n"
			"    }\n"
			"    pthread_mutex_unlock( &theMutex );\n"
			"}\n"
			"void Driver::shutdown( void )\n"
			"{\n"
			"    pthread_mutex_lock( &theMutex );\n"
			"    if ( theThreads.empty() )\n"
			"    {\n"
			"        pthread_mutex_unlock( &theMutex );\n"
			"        return;\n"
			"    }\n"
			"    std::vector<pthread_t> threads;\n"
			"    std::swap( threads, theThreads );\n"
			"    size_t N = threads.size();\n"
			"    void *threadRet;\n"
			"    pthread_mutex_unlock( &theMutex );\n"
			"    for ( size_t i = 0; i != N; ++i )\n"
			"    {\n"
			"        pthread_cond_broadcast( &theWorkCond );\n"
			"        pthread_join( threads[i], &threadRet );\n"
			"    }\n"
			"    pthread_mutex_lock( &theMutex );\n"
			"    pthread_cond_destroy( &theWorkCond );\n"
			"    pthread_cond_destroy( &theWaitCond );\n"
			"    theWorkFunc.clear();\n"
			"    pthread_mutex_unlock( &theMutex );\n"
			"}\n";
	}

	driver << "Driver theDriver;\n";

	return driver.str();
}


////////////////////////////////////////


void
CPPGenerator::addStandardIncludes( void )
{
	CCommonLanguage::addStandardIncludes();
	curStream() <<
		"\n#include <iostream>"
		"\n#include <stdexcept>";
}


////////////////////////////////////////


void
CPPGenerator::defineStandardTypes( std::vector<TypeDefinition> &types, const std::string &funcPref, const std::string & precSuffix )
{
	types.push_back( TypeDefinition( "ctl_vec2f_t",
"struct ctl_vec2f_t\n"
"{\n"
"    inline ctl_vec2f_t( void ) : x( 0 ), y( 0 ) {}\n"
"    inline ctl_vec2f_t( ctl_number_t a, ctl_number_t b ) : x( a ), y( b ) {}\n"
"    inline ctl_number_t &operator[]( int i ) { return vals[i]; }\n"
"    inline ctl_number_t operator[]( int i ) const { return vals[i]; }\n"
"    union\n"
"    {\n"
"        ctl_number_t vals[2];\n"
"        struct\n"
"        {\n"
"            ctl_number_t x;\n"
"            ctl_number_t y;\n"
"        };\n"
"    };\n"
"};" ) );

	types.push_back( TypeDefinition( "ctl_vec2i_t",
"struct ctl_vec2i_t\n"
"{\n"
"    inline ctl_vec2i_t( void ) : x( 0 ), y( 0 ) {}\n"
"    inline ctl_vec2i_t( int a, int b ) : x( a ), y( b ) {}\n"
"    inline int &operator[]( int i ) { return vals[i]; }\n"
"    inline int operator[]( int i ) const { return vals[i]; }\n"
"    union\n"
"    {\n"
"        int vals[2];\n"
"        struct\n"
"        {\n"
"            int x;\n"
"            int y;\n"
"        };\n"
"    };\n"
"};", funcPref + "ctl_vec2i_t make_vec2i( int x, int y ) { return ctl_vec2i_t( x, y ); }" ) );

	types.push_back( TypeDefinition( "ctl_vec3f_t",
"struct ctl_vec3f_t\n"
"{\n"
"    inline ctl_vec3f_t( void ) : x( 0 ), y( 0 ), z( 0 ) {}\n"
"    inline ctl_vec3f_t( ctl_number_t a, ctl_number_t b, ctl_number_t c ) : x( a ), y( b ), z( c ) {}\n"
"    inline ctl_number_t &operator[]( int i ) { return vals[i]; }\n"
"    inline ctl_number_t operator[]( int i ) const { return vals[i]; }\n"
"    union\n"
"    {\n"
"        ctl_number_t vals[3];\n"
"        struct\n"
"        {\n"
"            ctl_number_t x;\n"
"            ctl_number_t y;\n"
"            ctl_number_t z;\n"
"        };\n"
"    };\n"
"};", funcPref + "ctl_vec3f_t make_vec3f( ctl_number_t x, ctl_number_t y, ctl_number_t z ) { return ctl_vec3f_t( x, y, z ); }" ) );

	types.push_back( TypeDefinition( "ctl_vec3i_t",
"struct ctl_vec3i_t\n"
"{\n"
"    inline ctl_vec3i_t( void ) : x( 0 ), y( 0 ), z( 0 ) {}\n"
"    inline ctl_vec3i_t( int a, int b, int c ) : x( a ), y( b ), z( c ) {}\n"
"    inline int &operator[]( int i ) { return vals[i]; }\n"
"    inline int operator[]( int i ) const { return vals[i]; }\n"
"    union\n"
"    {\n"
"        int vals[3];\n"
"        struct\n"
"        {\n"
"            int x;\n"
"            int y;\n"
"            int z;\n"
"        };\n"
"    };\n"
"};", funcPref + "ctl_vec3i_t make_vec3i( int x, int y, int z ) { return ctl_vec3i_t( x, y, z ); }" ) );

	types.push_back( TypeDefinition( "ctl_vec4f_t",
"struct ctl_vec4f_t\n"
"{\n"
"    inline ctl_vec4f_t( void ) : x( 0 ), y( 0 ), z( 0 ), w( 0 ) {}\n"
"    inline ctl_vec4f_t( ctl_number_t a, ctl_number_t b, ctl_number_t c, ctl_number_t d ) : x( a ), y( b ), z( c ), w( d ) {}\n"
"    inline ctl_number_t &operator[]( int i ) { return vals[i]; }\n"
"    inline ctl_number_t operator[]( int i ) const { return vals[i]; }\n"
"    union\n"
"    {\n"
"        ctl_number_t vals[4];\n"
"        struct\n"
"        {\n"
"            ctl_number_t x;\n"
"            ctl_number_t y;\n"
"            ctl_number_t z;\n"
"            ctl_number_t w;\n"
"        };\n"
"    };\n"
"};" ) );

	types.push_back( TypeDefinition( "ctl_vec4i_t",
"struct ctl_vec4i_t\n"
"{\n"
"    inline ctl_vec4i_t( void ) : x( 0 ), y( 0 ), z( 0 ), w( 0 ) {}\n"
"    inline ctl_vec4i_t( int a, int b, int c, int d ) : x( a ), y( b ), z( c ), w( d ) {}\n"
"    inline int &operator[]( int i ) { return vals[i]; }\n"
"    inline int operator[]( int i ) const { return vals[i]; }\n"
"    union\n"
"    {\n"
"        int vals[4];\n"
"        struct\n"
"        {\n"
"            int x;\n"
"            int y;\n"
"            int z;\n"
"            int w;\n"
"        };\n"
"    };\n"
"};" ) );

	types.push_back( TypeDefinition( "ctl_mat33f_t",
"struct ctl_mat33f_t\n"
"{\n"
"    inline ctl_mat33f_t( void ) { identity(); }\n"
"    inline ctl_mat33f_t( ctl_number_t m00, ctl_number_t m01, ctl_number_t m02, ctl_number_t m10, ctl_number_t m11, ctl_number_t m12, ctl_number_t m20, ctl_number_t m21, ctl_number_t m22 ) { vals[0][0] = m00; vals[0][1] = m01; vals[0][2] = m02; vals[1][0] = m10; vals[1][1] = m11; vals[1][2] = m12; vals[2][0] = m20; vals[2][1] = m21; vals[2][2] = m22; }\n"
"    inline void identity( void ) { vals[0][0] = vals[1][1] = vals[2][2] = ctl_number_t(1); vals[0][1] = vals[0][2] = vals[1][0] = vals[1][1] = vals[1][2] = vals[2][0] = vals[2][1] = vals[2][2] = ctl_number_t(0); }\n"
"    inline ctl_number_t *operator[]( int i ) { return vals[i]; }\n"
"    inline const ctl_number_t *operator[]( int i ) const { return vals[i]; }\n"
"    ctl_number_t vals[3][3];\n"
"};" ) );

	types.push_back( TypeDefinition( "ctl_mat44f_t",
"struct ctl_mat44f_t\n"
"{\n"
"    inline ctl_mat44f_t( void ) { identity(); }\n"
"    inline ctl_mat44f_t( ctl_number_t m00, ctl_number_t m01, ctl_number_t m02, ctl_number_t m03, ctl_number_t m10, ctl_number_t m11, ctl_number_t m12, ctl_number_t m13, ctl_number_t m20, ctl_number_t m21, ctl_number_t m22, ctl_number_t m23, ctl_number_t m30, ctl_number_t m31, ctl_number_t m32, ctl_number_t m33 ) { vals[0][0] = m00; vals[0][1] = m01; vals[0][2] = m02; vals[0][3] = m03; vals[1][0] = m10; vals[1][1] = m11; vals[1][2] = m12; vals[1][3] = m13; vals[2][0] = m20; vals[2][1] = m21; vals[2][2] = m22; vals[2][3] = m23; vals[3][0] = m30; vals[3][1] = m31; vals[3][2] = m32; vals[3][3] = m33; }\n"
"    inline void identity( void )\n"
"    {\n"
"        for ( int i = 0; i < 4; ++i )\n"
"            for ( int j = 0; j < 4; ++j )\n"
"                vals[i][j] = (i == j) ? ctl_number_t(1) : ctl_number_t(0);\n"
"    }\n"
"    inline ctl_number_t *operator[]( int i ) { return vals[i]; }\n"
"    inline const ctl_number_t *operator[]( int i ) const { return vals[i]; }\n"
"    ctl_number_t vals[4][4];\n"
"};" ) );

	types.push_back( TypeDefinition( "Chromaticities", "struct Chromaticities { ctl_vec2f_t red; ctl_vec2f_t green; ctl_vec2f_t blue; ctl_vec2f_t white; };" ) );
	types.back().moduleUsage[NULL].types.insert( "ctl_vec2f_t" );
	types.push_back( TypeDefinition( "Box2i", "struct Box2i { ctl_vec2i_t min; ctl_vec2i_t max; };" ) );
	types.back().moduleUsage[NULL].types.insert( "ctl_vec2i_t" );
	types.push_back( TypeDefinition( "Box2f", "struct Box2f { ctl_vec2f_t min; ctl_vec2f_t max; };" ) );
	types.back().moduleUsage[NULL].types.insert( "ctl_vec2f_t" );
}


////////////////////////////////////////


void
CPPGenerator::getStandardPrintBodies( FuncDeclList &d, const std::string &funcPref, const std::string & precSuffix )
{
	d.push_back( FunctionDefinition( "assert", funcPref + "void assert( bool v ) { if (!v) throw std::logic_error( \"Assertion failure\" ); }" ) );
	d.push_back( FunctionDefinition( "print_bool", funcPref + "void print_bool( bool v ) { std::cout << (v ? \"true\" : \"false\"); }" ) );
	d.push_back( FunctionDefinition( "print_int", funcPref + "void print_int( int v ) { std::cout << v; }" ) );
	d.push_back( FunctionDefinition( "print_unsigned_int", funcPref + "void print_unsigned_int( unsigned int v ) { std::cout << v; }" ) );
	d.push_back( FunctionDefinition( "print_half", funcPref + "void print_half( const half &v ) { std::cout << v; }" ) );
	d.push_back( FunctionDefinition( "print_float", funcPref + "void print_float( ctl_number_t v ) { std::cout << v; }" ) );
	d.push_back( FunctionDefinition( "print_string", funcPref + "void print_string( const char *v ) { std::cout << v; }\n" + funcPref + "void print_string( const std::string &v ) { std::cout << v; }" ) );

}


////////////////////////////////////////


bool
CPPGenerator::usesFunctionInitializers( void ) const
{
	return ! myCPP11Mode;
}


////////////////////////////////////////


bool
CPPGenerator::supportsModuleDynamicInitialization( void ) const
{
	return true;
}


////////////////////////////////////////


bool
CPPGenerator::supportsNamespaces( void ) const
{
	return true;
}


////////////////////////////////////////


bool
CPPGenerator::supportsHalfType( void ) const
{
	return true;
}


////////////////////////////////////////


bool
CPPGenerator::supportsReferences( void ) const
{
	return true;
}


////////////////////////////////////////


bool
CPPGenerator::supportsStructOperators( void ) const
{
	return true;
}


////////////////////////////////////////


bool
CPPGenerator::needsStructTypedefs( void ) const
{
	return false;
}


////////////////////////////////////////


std::string
CPPGenerator::constructNamespaceTag( const std::string &modName )
{
	return modName + "::";
}


////////////////////////////////////////


const std::string &
CPPGenerator::getInlineKeyword( void ) const
{
	static std::string kInline = "inline";
	return kInline;
}


////////////////////////////////////////


const std::string &
CPPGenerator::getFunctionPrefix( void ) const
{
	static std::string kPrefix = "static";
	return kPrefix;
}


////////////////////////////////////////


const std::string &
CPPGenerator::getGlobalPrefix( void ) const
{
	static std::string kGlobal;
	return kGlobal;
}


////////////////////////////////////////


const std::string &
CPPGenerator::getBoolTypeName( void ) const
{
	static std::string kType = "bool";
	return kType;
}


////////////////////////////////////////


const std::string &
CPPGenerator::getBoolLiteral( bool v ) const
{
	static std::string kBoolTrue = "true";
	static std::string kBoolFalse = "false";
	if ( v )
		return kBoolTrue;
	return kBoolFalse;
}


////////////////////////////////////////


const std::string &
CPPGenerator::getConstLiteral( void ) const
{
	static std::string kConst = "const";
	return kConst;
}


////////////////////////////////////////


void
CPPGenerator::startCast( const char *type )
{
	curStream() << "static_cast<" << type << ">( ";
}


////////////////////////////////////////


static std::string kComment = "//";

const std::string &
CPPGenerator::beginComment( void ) const
{
	return kComment;
}


////////////////////////////////////////


const std::string &
CPPGenerator::beginCommentLine( void ) const
{
	return kComment;
}


////////////////////////////////////////


const std::string &
CPPGenerator::endComment( void ) const
{
	static std::string blank;
	return blank;
}

} // namespace Ctl

