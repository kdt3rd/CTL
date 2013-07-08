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
#include <ImathNamespace.h>
#include <half.h>
#include <deque>
#include <ctype.h>
#include <stdexcept>

#define STRINGIZE(X) #X
#define NAMESPACE(N, T) STRINGIZE(N) "::" T

namespace Ctl
{

CPPGenerator::CPPGenerator( bool cpp11 )
		: LanguageGenerator(),
		  myCPP11Mode( cpp11 ),
		  myInElse( 0 ),
		  myInModuleInit( 0 ), myInFunction( 0 ),
		  myCurInitType( NONE ), myCurModule( NULL )
{
}


////////////////////////////////////////


CPPGenerator::~CPPGenerator( void )
{
}


////////////////////////////////////////


std::string
CPPGenerator::getHeaderCode( void )
{
	return myHeaderStream.str();
}


////////////////////////////////////////


std::string
CPPGenerator::getCode( void )
{
	return myCodeStream.str();
}


////////////////////////////////////////


std::string
CPPGenerator::stdLibraryAndSetup( void )
{
	return std::string(
		"// C++ code automatically generated\n\n"
		"#include <ImathVec.h>\n"
		"#include <ImathMatrix.h>\n"
		"#include <half.h>\n"
		"#include <float.h>\n"
		"#include <math.h>\n"
		"#include <limits.h>\n"
		"#include <iostream>\n"
		"#include <stdexcept>\n"
		"#include <limits>\n"
		"#include <vector>\n"
		"#include <CtlColorSpace.h>\n"
		"#include <CtlLookupTable.h>\n"
		"\n"
		"using namespace Ctl;\n"
		"\n"
		"static inline void assert( bool v ) { if (!v) throw std::logic_error( \"Assertion failure\" ); }\n"
		"\n"
		"static inline void print_bool( bool v ) { std::cout << (v ? \"true\" : \"false\"); }\n"
		"static inline void print_int( int v ) { std::cout << v; }\n"
		"static inline void print_unsigned_int( unsigned int v ) { std::cout << v; }\n"
		"static inline void print_half( half v ) { std::cout << v; }\n"
		"static inline void print_float( float v ) { std::cout << v; }\n"
		"static inline void print_string( const std::string &v ) { std::cout << v; }\n"
		"static inline void print_string( const char *v ) { std::cout << v; }\n"
		"\n"
		"static inline bool isfinite_f( float v ) { return isfinite( v ); }\n"
		"static inline bool isnormal_f( float v ) { return isnormal( v ); }\n"
		"static inline bool isnan_f( float v ) { return isnan( v ); }\n"
		"static inline bool isinf_f( float v ) { return isinf( v ) != 0; }\n"
		"static inline bool isfinite_h( half v ) { return v.isFinite(); }\n"
		"static inline bool isnormal_h( half v ) { return v.isNormalized(); }\n"
		"static inline bool isnan_h( half v ) { return v.isNan(); }\n"
		"static inline bool isinf_h( half v ) { return v.isInfinity() != 0; }\n"
		"\n"
		"#define FLT_POS_INF std::numeric_limits<float>::infinity()\n"
		"#define FLT_NEG_INF (-std::numeric_limits<float>::infinity())\n"
		"#define FLT_NAN (-std::numeric_limits<float>::quiet_NaN())\n"
		"#define HALF_POS_INF half::posInf()\n"
		"#define HALF_NEG_INF half::negInf()\n"
		"#define HALF_NAN half::qNan()\n"
		"\n"
		"static inline half exp_h( float v ) { return half( exp( v ) ); }\n"
		"static inline float log_h( half v ) { return log( float( v ) ); }\n"
		"static inline float log10_h( half v ) { return log10( float( v ) ); }\n"
		"static inline half pow_h( half x, float y ) { return half( pow( float(x), y ) ); }\n"
		"static inline float pow10( float v ) { return pow( 10.0, v ); }\n"
		"static inline half pow10_h( float v ) { return half( pow( 10.0, v ) ); }\n"
		"\n"
		"static inline Imath::M33f mult_f33_f33( const Imath::M33f &a, const Imath::M33f &b ) { return a * b; }\n"
		"static inline Imath::M44f mult_f44_f44( const Imath::M44f &a, const Imath::M44f &b ) { return a * b; }\n"
		"static inline Imath::M33f mult_f_f33( float a, const Imath::M33f &b ) { return a * b; }\n"
		"static inline Imath::M44f mult_f_f44( float a, const Imath::M44f &b ) { return a * b; }\n"
		"static inline Imath::M33f add_f33_f33( const Imath::M33f &a, const Imath::M33f &b ) { return a + b; }\n"
		"static inline Imath::M44f add_f44_f44( const Imath::M44f &a, const Imath::M44f &b ) { return a + b; }\n"
		"static inline Imath::M33f invert_f33( const Imath::M33f &a ) { return a.inverse(); }\n"
		"static inline Imath::M44f invert_f44( const Imath::M44f &a ) { return a.inverse(); }\n"
		"static inline Imath::M33f transpose_f33( const Imath::M33f &a ) { return a.transposed(); }\n"
		"static inline Imath::M44f transpose_f44( const Imath::M44f &a ) { return a.transposed(); }\n"
		"static inline Imath::V3f mult_f3_f33( const Imath::V3f &a, const Imath::M33f &b ) { return a * b; }\n"
		"static inline Imath::V3f mult_f3_f44( const Imath::V3f &a, const Imath::M44f &b ) { return a * b; }\n"
		"static inline Imath::V3f mult_f_f3( float a, const Imath::V3f &b ) { return a * b; }\n"
		"static inline Imath::V3f add_f3_f3( const Imath::V3f &a, const Imath::V3f &b ) { return a + b; }\n"
		"static inline Imath::V3f sub_f3_f3( const Imath::V3f &a, const Imath::V3f &b ) { return a - b; }\n"
		"static inline Imath::V3f cross_f3_f3( const Imath::V3f &a, const Imath::V3f &b ) { return a.cross( b ); }\n"
		"static inline float dot_f3_f3( const Imath::V3f &a, const Imath::V3f &b ) { return a.dot( b ); }\n"
		"static inline float length_f3( const Imath::V3f &a ) { return a.length(); }\n"
					   );
}


////////////////////////////////////////


void
CPPGenerator::pushBlock( void )
{
	newlineAndIndent();
	curStream() << '{';
	pushIndent();
}


////////////////////////////////////////


void
CPPGenerator::popBlock( void )
{
	popIndent();
	newlineAndIndent();
	curStream() << '}';
}


////////////////////////////////////////


void
CPPGenerator::module( CodeLContext &ctxt, const CodeModuleNode &m )
{
	Module *oldModule = myCurModule;
	myCurModule = ctxt.module();

	pushStream( myCodeStream );

	newlineAndIndent();
	curStream() << "// Module " << ctxt.module()->name() << " ("
				<< ctxt.fileName() << ")\n";
	newlineAndIndent();
	if ( m.constants )
		curStream() << "////////// CONSTANTS //////////\n\n";
	++myInModuleInit;
	myCurModuleInit.push_back( std::vector<std::string>() );
	StatementNodePtr consts = m.constants;
	while ( consts )
	{
		consts->generateCode( ctxt );
		consts = consts->next;
	}
	if ( m.constants )
		curStream() << "\n\n";
	--myInModuleInit;

	std::string mName = cleanName( ctxt.module()->name() );
	if ( ! myCurModuleInit.back().empty() )
	{
		curStream() << "////////// INIT_CODE //////////";
		newlineAndIndent();
		curStream() << "static void init_" << mName << "( void );";
		newlineAndIndent();
		curStream() << "struct InitVals_" << mName;
		pushBlock();
		newlineAndIndent();
		curStream() << "InitVals_" << mName << "() { init_" << mName << "(); }";
		popBlock();
		curStream() << ";";
		newlineAndIndent();
		curStream() << "static InitVals_" << mName << " theDoInit_" << mName << ";\n\n";
	}

	if ( m.functions )
	{
		newlineAndIndent();
		curStream() << "////////// FUNCTIONS //////////\n\n";
	}
	FunctionNodePtr function = m.functions;
	while ( function )
	{
		function->generateCode( ctxt );
		function = function->next;
	}

	if ( ! myCurModuleInit.back().empty() )
	{
		const std::vector<std::string> &initVals = myCurModuleInit.back();
		newlineAndIndent();
		curStream() << "void init_" << mName << "( void )";
		pushBlock();
		newlineAndIndent();
		curStream() << "static bool doneInit = false;";
		newlineAndIndent();
		curStream() << "if ( doneInit ) return;\n";
		for ( size_t i = 0, N = initVals.size(); i != N; ++i )
			curStream() << initVals[i] << '\n';
		newlineAndIndent();
		curStream() << "doneInit = true;";
		popBlock();
		newlineAndIndent();
	}
	else
	{
		// make sure we end the module on a new line
		newlineAndIndent();
	}

	myCurModuleInit.pop_back();
	popStream();

	myCurModule = oldModule;
}


////////////////////////////////////////


void
CPPGenerator::function( CodeLContext &ctxt, const CodeFunctionNode &f )
{
	CodeFunctionTypePtr functionType = f.info->functionType();
	const ParamVector &params = functionType->parameters();

	if ( functionType->returnVarying() )
		std::cout << "Check on what it means to return varying..." << std::endl;

	std::string funcName = removeNSQuals( f.name );
	bool isMain = funcName == ctxt.module()->name();
	if ( funcName == "main" )
	{
		funcName = ctxt.module()->name();
		isMain = true;
	}

	if ( isMain )
	{
		pushStream( myHeaderStream );
		newlineAndIndent();
		variable( ctxt, std::string(), functionType->returnType(),
				  false, false, false );

		curStream() << funcName << "( ";
		bool notfirst = false;
		for ( size_t i = 0, N = params.size(); i != N; ++i )
		{
			const Param &parm = params[i];
			if ( notfirst )
				curStream() << ", ";
			else
				notfirst = true;

			variable( ctxt, parm.name, parm.type,
					  parm.access == RWA_READ, true,
					  parm.access == RWA_WRITE || parm.access == RWA_READWRITE );

			if ( parm.defaultValue )
			{
				curStream() << " = ";
				NameNodePtr n = parm.defaultValue.cast<NameNode>();
				std::string defVal;
				if ( n )
				{
					defVal = n->name;
				}
				else
				{
					std::stringstream nameLookupB;
					pushStream( nameLookupB );
					parm.defaultValue->generateCode( ctxt );
					popStream();
					defVal = nameLookupB.str();
				}
				std::map<std::string, std::string>::const_iterator found = myDefaultMappings.find( defVal );
				if ( found != myDefaultMappings.end() )
					curStream() << found->second;
				else
					curStream() << defVal;
			}
		}
		curStream() << " );";
		popStream();
	}

	newlineAndIndent();
	if ( ! isMain )
		curStream() << "static inline ";
	variable( ctxt, std::string(), functionType->returnType(),
			  false, false, false );
	newlineAndIndent();

	curStream() << funcName << "( ";
	bool notfirst = false;
	for ( size_t i = 0, N = params.size(); i != N; ++i )
	{
		const Param &parm = params[i];
		if ( notfirst )
			curStream() << ", ";
		else
			notfirst = true;

		variable( ctxt, parm.name, parm.type,
				  parm.access == RWA_READ, true,
				  parm.access == RWA_WRITE || parm.access == RWA_READWRITE );

		if ( ! isMain && parm.defaultValue )
		{
			curStream() << " = ";
			NameNodePtr n = parm.defaultValue.cast<NameNode>();
			std::string defVal;
			if ( n )
			{
				defVal = n->name;
			}
			else
			{
				std::stringstream nameLookupB;
				pushStream( nameLookupB );
				parm.defaultValue->generateCode( ctxt );
				popStream();
				defVal = nameLookupB.str();
			}
			std::map<std::string, std::string>::const_iterator found = myDefaultMappings.find( defVal );
			if ( found != myDefaultMappings.end() )
				curStream() << found->second;
			else
				curStream() << defVal;
		}
	}
	curStream() << " )";
	pushBlock();
	++myInFunction;
	StatementNodePtr bodyNode = f.body;
	while ( bodyNode )
	{
		bodyNode->generateCode( ctxt );
		bodyNode = bodyNode->next;
	}
	--myInFunction;
	popBlock();
	curStream() << "\n\n";
}


////////////////////////////////////////


void
CPPGenerator::variable( CodeLContext &ctxt, const CodeVariableNode &v )
{
	if ( myInModuleInit > 0 )
	{
		std::stringstream varDeclB;
		pushStream( varDeclB );
		bool doConst = ! v.info->isWritable();

		// introspect a bit and if a value is
		// initialized by a function call, override
		// the default init and just do it in the init
		// function since the functions may not yet be
		// declared since the interpreter coalesces everything into
		// modules constants at the beginning
		bool overrideInit = checkNeedInitInModuleInit( v.initialValue );
		if ( overrideInit )
			doConst = false;
		InitType initT = variable( ctxt, v.name, v.info->type(),
								   doConst, false, false );
		if ( overrideInit )
			initT = FUNC;

		popStream();
		std::string varDecl = varDeclB.str();
		bool doEmit = true;

		if ( v.name.find( '$' ) != std::string::npos )
		{
			varDecl = v.name;
			
			std::string initVal;
			if ( v.initialValue )
			{
				if ( initT == FUNC )
				{
					if ( ! v.initialValue.cast<NameNode>() )
						throw std::logic_error( "NYI: complex default value handling for function type" );
				}
				std::stringstream initB;
				pushStream( initB );
				myCurInitType = initT;
				v.initialValue->generateCode( ctxt );
				myCurInitType = NONE;
				popStream();
				initVal = initB.str();
				if ( initT == CTOR )
				{
					std::stringstream typeB;
					pushStream( typeB );
					variable( ctxt, std::string(), v.info->type(),
							  false, false, false );
					popStream();
					initVal = typeB.str() + "( " + initVal + " )";
				}
			}
			myDefaultMappings[varDecl] = initVal;
			doEmit = false;
		}

		if ( doEmit )
		{
			newlineAndIndent();
			curStream() << varDecl;
			doInit( initT, ctxt, v.initialValue, v.name );
		}
	}
	else
	{
		newlineAndIndent();
		InitType initT = variable( ctxt, v.name, v.info->type(),
								   ! v.info->isWritable(), false, false );

		doInit( initT, ctxt, v.initialValue, v.name );
	}
}


////////////////////////////////////////


void
CPPGenerator::assignment( CodeLContext &ctxt, const CodeAssignmentNode &v )
{
	newlineAndIndent();
	v.lhs->generateCode( ctxt );
	curStream() << " = ";
	v.lhs->type->generateCastFrom( v.rhs, ctxt );
	curStream() << ';';
}


////////////////////////////////////////


void
CPPGenerator::expr( CodeLContext &ctxt, const CodeExprStatementNode &v )
{
	v.expr->generateCode( ctxt );
}


////////////////////////////////////////


void CPPGenerator::cond( CodeLContext &ctxt, const CodeIfNode &v )
{
	if ( myInElse > 0 )
		curStream() << ' ';
	else
		newlineAndIndent();
	curStream() << "if ( ";
	BoolTypePtr boolType = ctxt.newBoolType();
	boolType->generateCastFrom( v.condition, ctxt );
	curStream() << " )";
	pushBlock();
	StatementNodePtr tPath = v.truePath;
	StatementNodePtr fPath = v.falsePath;
	while ( tPath )
	{
		tPath->generateCode( ctxt );
		tPath = tPath->next;
	}
	popBlock();
	if ( fPath )
	{
		newlineAndIndent();
		curStream() << "else";
		if ( fPath.cast<CodeIfNode>() && ! fPath->next )
		{
			++myInElse;
			fPath->generateCode( ctxt );
			--myInElse;
		}
		else
		{
			pushBlock();
			while ( fPath )
			{
				fPath->generateCode( ctxt );
				fPath = fPath->next;
			}
			popBlock();
		}
	}
}


////////////////////////////////////////


void
CPPGenerator::retval( CodeLContext &ctxt, const CodeReturnNode &v )
{
	newlineAndIndent();
	curStream() << "return";
	if ( v.returnedValue )
	{
		curStream() << ' ';
		v.info->type()->generateCastFrom( v.returnedValue, ctxt );
	}
	curStream() << ';';
}


////////////////////////////////////////


void
CPPGenerator::loop( CodeLContext &ctxt, const CodeWhileNode &v )
{
	newlineAndIndent();
	curStream() << "while ( ";
	BoolTypePtr boolType = ctxt.newBoolType();
	boolType->generateCastFrom( v.condition, ctxt );
	curStream() << " )";
	pushBlock();
	StatementNodePtr body = v.loopBody;
	while ( body )
	{
		body->generateCode( ctxt );
		body = body->next;
	}
	popBlock();
}


////////////////////////////////////////


void
CPPGenerator::binaryOp( CodeLContext &ctxt, const CodeBinaryOpNode &v )
{
	curStream() << '(';
	v.operandType->generateCastFrom( v.leftOperand, ctxt );
	curStream() << ' ';
	v.operandType->generateCode( const_cast<CodeBinaryOpNode *>( &v ), ctxt );
	curStream() << ' ';
	v.operandType->generateCastFrom( v.rightOperand, ctxt );
	curStream() << ')';
}


////////////////////////////////////////


void
CPPGenerator::unaryOp( CodeLContext &ctxt, const CodeUnaryOpNode &v )
{
	v.type->generateCode( const_cast<CodeUnaryOpNode *>( &v ), ctxt );
	curStream() << '(';
	v.type->generateCastFrom( v.operand, ctxt );
	curStream() << ')';
}


////////////////////////////////////////


void
CPPGenerator::index( CodeLContext &ctxt, const CodeArrayIndexNode &v )
{
	v.array->generateCode( ctxt );
	IntTypePtr intType = ctxt.newIntType();
	curStream() << '[';
	intType->generateCastFrom( v.index, ctxt );
	curStream() << ']';
}


////////////////////////////////////////


void
CPPGenerator::member( CodeLContext &ctxt, const CodeMemberNode &v )
{
	v.obj->generateCode( ctxt );
	curStream() << '.' << v.member;
}


////////////////////////////////////////


void
CPPGenerator::size( CodeLContext &ctxt, const CodeSizeNode &v )
{
	std::cout << "what is the size operator supposed to return? Should just be v.size()" << std::endl;
}


////////////////////////////////////////


void
CPPGenerator::name( CodeLContext &ctxt, const CodeNameNode &v )
{
	curStream() << removeNSQuals( v.name );
}


////////////////////////////////////////


void
CPPGenerator::boolLit( CodeLContext &ctxt, const CodeBoolLiteralNode &v )
{
	if ( v.value )
		curStream() << "true";
	else
		curStream() << "false";
}


////////////////////////////////////////


void
CPPGenerator::intLit( CodeLContext &ctxt, const CodeIntLiteralNode &v )
{
	curStream() << v.value;
}


////////////////////////////////////////


void
CPPGenerator::uintLit( CodeLContext &ctxt, const CodeUIntLiteralNode &v )
{
	curStream() << v.value;
}


////////////////////////////////////////


void
CPPGenerator::halfLit( CodeLContext &ctxt, const CodeHalfLiteralNode &v )
{
	curStream() << "half( " << std::setprecision( std::numeric_limits<half>::digits10 ) << static_cast<float>( v.value ) << " )";
}


////////////////////////////////////////


void
CPPGenerator::floatLit( CodeLContext &ctxt, const CodeFloatLiteralNode &v )
{
	curStream() << std::setprecision( std::numeric_limits<float>::digits10 ) << static_cast<float>( v.value );
}


////////////////////////////////////////


void
CPPGenerator::stringLit( CodeLContext &ctxt, const CodeStringLiteralNode &v )
{
	curStream() << '"' << escapeLiteral( v.value ) << '"';
}


////////////////////////////////////////


static bool
isStdPrintFunction( const NameNodePtr &n )
{
	return ( n->name == "::print_bool" ||
			 n->name == "::print_int" ||
			 n->name == "::print_unsigned_int" ||
			 n->name == "::print_half" ||
			 n->name == "::print_float" ||
			 n->name == "::print_string" );
}

////////////////////////////////////////


void
CPPGenerator::call( CodeLContext &ctxt, const CodeCallNode &v )
{
	bool isPrint = isStdPrintFunction( v.function );

	if ( isPrint )
		newlineAndIndent();

	v.function->generateCode( ctxt );
	if ( v.arguments.empty() )
		curStream() << "()";
	else
	{
		SymbolInfoPtr info = v.function->info;
		FunctionTypePtr functionType = info->functionType();
		const ParamVector &parameters = functionType->parameters();
		curStream() << "( ";
		for ( size_t i = 0, N = v.arguments.size(); i != N; ++i )
		{
			if ( i > 0 )
				curStream() << ", ";

			parameters[i].type->generateCastFrom( v.arguments[i], ctxt );
		}
		curStream() << " )";
	}
	if ( isPrint )
		curStream() << ';';
}


////////////////////////////////////////


void
CPPGenerator::value( CodeLContext &ctxt, const CodeValueNode &v )
{
	size_t idx = 0;
	valueRecurse( ctxt, v.elements, v.type, idx );
}


////////////////////////////////////////


void
CPPGenerator::startToBool( void )
{
	curStream() << "static_cast<bool>( ";
}


////////////////////////////////////////


void
CPPGenerator::startToInt( void )
{
	curStream() << "static_cast<int>( ";
}


////////////////////////////////////////


void
CPPGenerator::startToUint( void )
{
	curStream() << "static_cast<unsigned int>( ";
}


////////////////////////////////////////


void
CPPGenerator::startToHalf( void )
{
	curStream() << "half( ";
}


////////////////////////////////////////


void
CPPGenerator::startToFloat( void )
{
	curStream() << "static_cast<float>( ";
}


////////////////////////////////////////


void
CPPGenerator::endCoersion( void )
{
	curStream() << " )";
}


////////////////////////////////////////


void
CPPGenerator::emitToken( Token t )
{
	switch ( t )
	{
		case TK_AND: curStream() << "&&"; break;
		case TK_OR: curStream() << "||"; break;

		case TK_BITAND: curStream() << "&"; break;
		case TK_BITNOT: curStream() << "~"; break;
		case TK_BITOR: curStream() << "|"; break;
		case TK_BITXOR: curStream() << "^"; break;

		case TK_LEFTSHIFT: curStream() << "<<"; break;
		case TK_RIGHTSHIFT: curStream() << "<<"; break;
			
		case TK_DIV: curStream() << "/"; break;
		case TK_MINUS: curStream() << "-"; break;
		case TK_MOD: curStream() << "%"; break;
		case TK_PLUS: curStream() << "+"; break;
		case TK_TIMES: curStream() << "*"; break;

		case TK_EQUAL: curStream() << "=="; break;
		case TK_GREATER: curStream() << ">"; break;
		case TK_GREATEREQUAL: curStream() << ">="; break;
		case TK_LESS: curStream() << "<"; break;
		case TK_LESSEQUAL: curStream() << "<="; break;
		case TK_NOT: curStream() << "!"; break;

		default:
			break;
	}
}


////////////////////////////////////////


void
CPPGenerator::valueRecurse( CodeLContext &ctxt, const ExprNodeVector &elements,
							const DataTypePtr &t, size_t &index )
{
	if ( StructTypePtr structType = t.cast<StructType>() )
	{
		if ( myCurInitType == ASSIGN )
			curStream() << '{';
		pushIndent();
		if ( myCurInitType == FUNC )
		{
			for( MemberVectorConstIterator it = structType->members().begin();
				 it != structType->members().end();
				 ++it )
			{
				newlineAndIndent();

				std::stringstream tmpB;
				pushStream( tmpB );
				valueRecurse( ctxt, elements, it->type, index );
				popStream();
				std::string name = "$$$$." + it->name;
				std::string initV = tmpB.str();
				replaceInit( initV, name );
				curStream() << initV;
			}
		}
		else
		{
			for( MemberVectorConstIterator it = structType->members().begin();
				 it != structType->members().end();
				 ++it )
			{
				if ( it != structType->members().begin() )
					curStream() << ",";

				newlineAndIndent();
				valueRecurse( ctxt, elements, it->type, index );
			}
		}
		popIndent();
		if ( myCurInitType == ASSIGN )
			curStream() << " }";
	}
	else if ( ArrayTypePtr arrayType = t.cast<ArrayType>() )
	{
		if ( myCurInitType == ASSIGN )
		{
			newlineAndIndent();
			curStream() << '{';
		}
		size_t N = arrayType->size();
		bool lineEveryItem = ( N > 4 );

		pushIndent();
		if ( myCurInitType == FUNC )
		{
			std::string builtinType;
			std::string postDecl;
			if ( ! findBuiltinType( builtinType, postDecl, arrayType, ctxt ) )
			{
				newlineAndIndent();
				curStream() << "$$$$.resize( " << arrayType->size() << " );";
			}

			for (int i = 0; i < arrayType->size(); ++i)
			{
				newlineAndIndent();
				std::stringstream tmpB;
				pushStream( tmpB );
				valueRecurse( ctxt, elements, arrayType->elementType(), index );
				popStream();
				std::stringstream nameB;
				nameB << "$$$$[" << i << "]";
				std::string name = nameB.str();
				std::string initV = tmpB.str();
				replaceInit( initV, name );
				curStream() << initV;
			}
		}
		else
		{
			for (int i = 0; i < arrayType->size(); ++i)
			{
				if ( i > 0 )
					curStream() << ',';
				if ( lineEveryItem )
					newlineAndIndent();
				else
					curStream() << ' ';

				valueRecurse( ctxt, elements, arrayType->elementType(), index );
			}
		}
		popIndent();
		
		if ( myCurInitType == ASSIGN )
		{
			newlineAndIndent();
			curStream() << "}";
		}
	}
	else
	{
		if ( myCurInitType == FUNC )
		{
			curStream() << "$$$$ = ";
			t->generateCastFrom( elements[index], ctxt );
			curStream() << ';';
		}
		else
			t->generateCastFrom( elements[index], ctxt );
		++index;
	}
}


////////////////////////////////////////


CPPGenerator::InitType
CPPGenerator::variable( CodeLContext &ctxt,
						const std::string &name, const DataTypePtr &t,
						bool isConst, bool isInput, bool isWritable )
{
	InitType retval = ASSIGN;

	std::string postDecl;
	switch ( t->cDataType() )
	{
		case VoidTypeEnum:
			curStream() << "void";
			break;
		case BoolTypeEnum:
			if ( isConst )
				curStream() << "const bool";
			else
				curStream() << "bool";
			break;
		case IntTypeEnum:
			if ( isConst )
				curStream() << "const int";
			else
				curStream() << "int";
			break;
		case UIntTypeEnum:
			if ( isConst )
				curStream() << "const unsigned int";
			else
				curStream() << "unsigned int";
			break;
		case HalfTypeEnum:
			if ( isConst )
				curStream() << "const half";
			else
				curStream() << "half";
			break;
		case FloatTypeEnum:
			if ( isConst )
				curStream() << "const float";
			else
				curStream() << "float";
			break;
		case StringTypeEnum:
			if ( isConst )
				curStream() << "const std:string";
			else
				curStream() << "std::string";
			retval = CTOR;
			break;
		case StructTypeEnum:
		{
			StructTypePtr structType = t.cast<StructType>();
			if ( myCPP11Mode &&  isConst )
				curStream() << "const ";
			curStream() << removeNSQuals( structType->name() );
			isWritable = isInput || isWritable;
			if ( myCPP11Mode )
				retval = ASSIGN;
			else
				retval = FUNC;
			break;
		}
		case ArrayTypeEnum:
		{
			ArrayTypePtr arrayType = t.cast<ArrayType>();

			std::string typeName;

			if ( myCPP11Mode )
				retval = ASSIGN;
			else
				retval = FUNC;

			if ( ! findBuiltinType( typeName, postDecl, arrayType, ctxt ) )
			{
				std::stringstream x;

				x << "std::vector< ";
				pushStream( x );
				variable( ctxt, std::string(), arrayType->elementType(),
						  false, false, false );
				popStream();
				x << " >";
				typeName = x.str();
			}
			else
			{
				if ( ! postDecl.empty() )
					retval = ASSIGN;
				else
					retval = CTOR;
				if ( isConst )
					curStream() << "const ";
			}
			if ( retval == ASSIGN && isConst )
				curStream() << "const ";
			curStream() << typeName;
			isWritable = ( isInput || isWritable ) && postDecl.empty();
			break;
		}
	}

	if ( isWritable )
		curStream() << " &";

	if ( ! name.empty() )
	{
		if ( ! isWritable )
			curStream() << ' ';
		curStream() << removeNSQuals( name );
	}
	curStream() << postDecl;

	return retval;
}


////////////////////////////////////////


void
CPPGenerator::doInit( InitType initT, CodeLContext &ctxt,
					  const ExprNodePtr &initV, const std::string &varName )
{
	myCurInitType = initT;
	if ( initV )
	{
		switch ( initT )
		{
			case CTOR:
				curStream() << "( ";
				initV->generateCode( ctxt );
				curStream() << " );";
				break;
			case FUNC:
			{
				curStream() << ';';
				std::stringstream varAssignB;
				pushStream( varAssignB );
				if ( ! initV.cast<ValueNode>() )
				{
					pushIndent();
					addIndent();
					curStream() << "$$$$ = ";
					initV->generateCode( ctxt );
					curStream() << ';';
					popIndent();
				}
				else
					initV->generateCode( ctxt );
				popStream();
				std::string initVal = varAssignB.str();
				replaceInit( initVal, varName );

				if ( myInFunction )
				{
					pushBlock();
					curStream() << initVal;
					popBlock();
				}
				else if ( varName.find_first_of( '$' ) == std::string::npos )
					myCurModuleInit.back().push_back( initVal );
				break;
			}
			case ASSIGN:
				curStream() << " = ";
				initV->generateCode( ctxt );
				curStream() << ";";
				break;
			case NONE:
				throw std::logic_error( "Invalid initialization type" );
				break;
		}
	}
	else
		curStream() << ';';
	myCurInitType = NONE;
}


////////////////////////////////////////


void
CPPGenerator::replaceInit( std::string &initS, const std::string &name )
{
	std::string::size_type replB = initS.find( "$$$$" );
	while ( replB != std::string::npos )
	{
		initS.replace( replB, 4, name );
		replB = initS.find( "$$$$", replB + name.size() );
	}
}


////////////////////////////////////////


bool
CPPGenerator::findBuiltinType( std::string &typeName,
							   std::string &postDecl,
							   const ArrayTypePtr &arrayType,
							   CodeLContext &ctxt )
{
	FloatTypePtr fltType = ctxt.newFloatType();

	SizeVector sizes;
	arrayType->sizes( sizes );

	if ( arrayType->elementType()->isSameTypeAs( fltType ) )
	{
		if ( sizes.size() == 1 )
		{
			switch ( sizes[0] )
			{
				case 2:
					typeName = NAMESPACE(IMATH_NAMESPACE, "V2f");
					break;
				case 3:
					typeName = NAMESPACE(IMATH_NAMESPACE, "V3f");
					break;
				case 4:
					typeName = NAMESPACE(IMATH_NAMESPACE, "V4f");
					break;
				default:
				{
					// just do a C array of the low level type
					std::cout << "sizes[0]: " << sizes[0] << std::endl;
					typeName = "float";
					std::stringstream pB;
					pB << '[' << sizes[0] << ']';
					postDecl = pB.str();
					break;
				}
			}
		}
	}
	else if ( sizes.size() == 2 && sizes[0] == sizes[1] )
	{
		if ( arrayType->elementType()->isSameTypeAs(
				 ctxt.newArrayType( fltType, 3 ) ) )
		{
			typeName = NAMESPACE(IMATH_NAMESPACE, "M33f");
		}
		else if ( arrayType->elementType()->isSameTypeAs(
					  ctxt.newArrayType( fltType, 4 ) ) )
		{
			typeName = NAMESPACE(IMATH_NAMESPACE, "M44f");
		}
	}
	else if ( arrayType->elementType()->isSameTypeAs( ctxt.newIntType() ) )
	{
		if ( sizes.size() == 1 )
		{
			switch ( sizes[0] )
			{
				case 2:
					typeName = NAMESPACE(IMATH_NAMESPACE, "V2i");
					break;
				case 3:
					typeName = NAMESPACE(IMATH_NAMESPACE, "V3i");
					break;
				case 4:
					typeName = NAMESPACE(IMATH_NAMESPACE, "V4i");
					break;
				default:
				{
					// just do a C array of the low level type
					std::cout << "sizes[0]: " << sizes[0] << std::endl;
					typeName = "int";
					std::stringstream pB;
					pB << '[' << sizes[0] << ']';
					postDecl = pB.str();
					break;
				}
			}
		}
	}

	return ! typeName.empty();
}


////////////////////////////////////////


bool
CPPGenerator::checkNeedInitInModuleInit( const ExprNodePtr &initV )
{
	if ( ! initV )
		return false;

	CallNodePtr c = initV.cast<CallNode>();
	if ( c )
	{
		SymbolInfoPtr info = c->function->info;
		if ( info->module() == myCurModule )
			return true;

		if ( ! c->arguments.empty() )
		{
			for ( size_t i = 0, N = c->arguments.size(); i != N; ++i )
			{
				bool needed = checkNeedInitInModuleInit( c->arguments[i] );
				if ( needed )
					return true;
			}
		}
	}

	return false;
}


////////////////////////////////////////


std::string
CPPGenerator::cleanName( const std::string &x )
{
	std::string retval = x;
	for ( size_t i = 0, N = retval.size(); i != N; ++i )
	{
		char &curC = retval[i];
		if ( i == 0 )
		{
			if ( curC != '_' && ! isalpha( curC ) )
				curC = '_';
		}
		else if ( curC != '_' && ! isalnum( curC ) )
			curC = '_';
	}
	return retval;
}


////////////////////////////////////////


std::string
CPPGenerator::escapeLiteral( const std::string &s )
{
	std::string retval;
	retval.reserve( s.size() );
	for ( size_t i = 0, N = s.size(); i != N; ++i )
	{
		switch ( s[i] )
		{
			case '\n':
				retval.push_back( '\\' );
				retval.push_back( 'n' );
				break;
			case '\r':
				retval.push_back( '\\' );
				retval.push_back( 'r' );
				break;
			case '\t':
				retval.push_back( '\\' );
				retval.push_back( 't' );
				break;
			case '"':
				retval.push_back( '\\' );
				retval.push_back( '"' );
				break;
			default:
				retval.push_back( s[i] );
				break;
		}

	}
	return retval;
}


////////////////////////////////////////


std::string
CPPGenerator::removeNSQuals( const std::string &x )
{
	std::string retval = x;
	std::string::size_type firstP = retval.find( "::" );
	if ( firstP == 0 )
		retval.erase( 0, firstP + 2 );
	else if ( firstP != std::string::npos )
	{
		do
		{
			retval.replace( firstP, 2, "__" );
			firstP = retval.find( "::" );
		} while ( firstP != std::string::npos );
	}
	return cleanName( retval );
}

} // namespace Ctl

