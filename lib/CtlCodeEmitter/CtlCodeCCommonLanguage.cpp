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


#include "CtlCodeCCommonLanguage.h"
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
#include <CtlErrors.h>
#include <CtlMessage.h>

////////////////////////////////////////


namespace Ctl
{


////////////////////////////////////////


CCommonLanguage::CCommonLanguage( void )
		: LanguageGenerator(),
		  myCurModule( NULL ),
		  myInElse( 0 ),
		  myInModuleInit( 0 ), myInFunction( 0 ),
		  myDoForwardDecl( 0 ),
		  myCurInitType( NONE )
{
}


////////////////////////////////////////


CCommonLanguage::~CCommonLanguage( void )
{
}


////////////////////////////////////////


std::string
CCommonLanguage::getHeaderCode( void )
{
	return myHeaderStream.str();
}


////////////////////////////////////////


std::string
CCommonLanguage::getCode( void )
{
	return myCodeStream.str();
}


////////////////////////////////////////


void
CCommonLanguage::pushBlock( void )
{
	newlineAndIndent();
	curStream() << '{';
	pushIndent();
}


////////////////////////////////////////


void
CCommonLanguage::popBlock( void )
{
	popIndent();
	newlineAndIndent();
	curStream() << '}';
}


////////////////////////////////////////


void
CCommonLanguage::module( CodeLContext &ctxt, const CodeModuleNode &m )
{
	std::string oldModuleName = myCurModuleName;
	std::string oldModPrefix = myModPrefix;
	Module *oldModule = myCurModule;

	myCurModule = ctxt.module();
	myCurModuleName = cleanName( myCurModule->name() );
	myNamespaceTags[myCurModule] = constructNamespaceTag( myCurModuleName );

	extractLiteralConstants( m.constants, ctxt );
	pushStream( myCodeStream );

	newlineAndIndent();
	curStream() << "// Module " << ctxt.module()->name() << " ("
				<< ctxt.fileName() << ")\n";
	newlineAndIndent();

	// really only for c++, but you never know...
	if ( supportsNamespaces() )
	{
		curStream() << "namespace " << myCurModuleName << " {";
		newlineAndIndent();
	}
	else
	{
		myModPrefix = myNamespaceTags[myCurModule];
	}

	++myInModuleInit;
	myCurModuleInit.push_back( std::vector<std::string>() );

	const std::vector< std::pair< std::string, MemberVector > > &theStructs =
		ctxt.structs();

	if ( ! theStructs.empty() )
	{
		for ( size_t i = 0, N = theStructs.size(); i != N; ++i )
		{
			const std::pair< std::string, MemberVector > &s = theStructs[i];
			std::string n = removeNSQuals( s.first );
			newlineAndIndent();
			// To support plain-old C, we are going to use the
			// typedef foo_XX {} XX; mechanism
			curStream() << "typedef struct ctl_typedef_" << myModPrefix << n;
			pushBlock();
			for ( size_t m = 0, M = s.second.size(); m != M; ++m )
			{
				newlineAndIndent();
				const Member &mem = s.second[m];
				variable( ctxt, mem.name, mem.type, false, false, false );
				curStream() << ';';
			}
			popBlock();
			curStream() << ' ' << myModPrefix << n << ";\n";
		}
	}

	// Forward declare any functions used in initializing variables
	FunctionNodePtr function = m.functions;
	++myDoForwardDecl;
	while ( function )
	{
		function->generateCode( ctxt );
		function = function->next;
	}
	--myDoForwardDecl;
	newlineAndIndent();

	StatementNodePtr consts = m.constants;
	while ( consts )
	{
		consts->generateCode( ctxt );
		consts = consts->next;
	}

	if ( m.constants )
		newlineAndIndent();
	--myInModuleInit;

	std::cout << "switch the module init struct to a check for supportsDynamicInitialization" << std::endl;
	if ( ! myCurModuleInit.back().empty() )
	{
		const std::vector<std::string> &initVals = myCurModuleInit.back();

		newlineAndIndent();
		curStream() << "struct __ctlcc_InitVals_" << myCurModuleName;
		pushBlock();
		newlineAndIndent();
		curStream() << "__ctlcc_InitVals_" << myCurModuleName << "( void )";
		pushBlock();
		for ( size_t i = 0, N = initVals.size(); i != N; ++i )
			curStream() << initVals[i] << '\n';
		newlineAndIndent();
		popBlock();
		popBlock();
		curStream() << ";";
		newlineAndIndent();
		curStream() << "static __ctlcc_InitVals_" << myCurModuleName << " __ctlcc_GlobalInitializer_" << myCurModuleName << ";\n\n";
	}

	function = m.functions;
	while ( function )
	{
		function->generateCode( ctxt );
		function = function->next;
	}

	newlineAndIndent();

	if ( supportsNamespaces() )
	{
		curStream() << "} // namespace " << myCurModuleName;
		newlineAndIndent();
	}

	myCurModuleInit.pop_back();
	popStream();

	myCurModule = oldModule;
	myCurModuleName = oldModuleName;
	myModPrefix = oldModPrefix;
}


////////////////////////////////////////


void
CCommonLanguage::function( CodeLContext &ctxt, const CodeFunctionNode &f )
{
	CodeFunctionTypePtr functionType = f.info->functionType();
	const ParamVector &params = functionType->parameters();

	if ( functionType->returnVarying() )
	{
		MESSAGE_LE( ctxt, ERR_RETURN_CONV, f.lineNumber,
					"Code Generator does not yet implement return of a varying type from a function" );
		return;
	}

	if ( myDoForwardDecl > 0 )
	{
		// if we aren't used in initializing constants, we can be
		// declared inline and don't need a forward decl
		if ( myFuncsUsedInInit.find( f.name ) == myFuncsUsedInInit.end() )
			return;
	}

	std::string funcName = removeNSQuals( f.name );
	bool isMain = funcName == myCurModuleName;
	if ( funcName == "main" )
	{
		funcName = myCurModuleName;
		isMain = true;
	}

	if ( isMain )
	{
		std::string nsName = myCurModuleName + "::" + funcName;
		registerMainRoutine( funcName, nsName, f.info );

		// and put it in a header file in case someone cares
		// about that
		pushStream( myHeaderStream );
		newlineAndIndent();
		if ( supportsNamespaces() )
		{
			curStream() << "namespace " << myCurModuleName << " {";
			newlineAndIndent();
		}

		variable( ctxt, std::string(), functionType->returnType(),
				  false, false, false );

		curStream() << ' ' << myModPrefix << funcName << "( ";
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

			checkNeedsSizeArgument( parm.type, parm.name );
		}
		curStream() << " );";

		if ( supportsNamespaces() )
		{
			newlineAndIndent();
			curStream() << "} // namespace " << myCurModuleName;
		}

		popStream();
	}
	else if ( myDoForwardDecl > 0 )
	{
		newlineAndIndent();
		curStream() << "static ";
		variable( ctxt, std::string(), functionType->returnType(),
				  false, false, false );

		curStream() << ' ' << funcName << "( ";
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

			checkNeedsSizeArgument( parm.type, parm.name );
		}
		curStream() << " );";
		return;
	}

	newlineAndIndent();
	if ( ! isMain )
	{
		curStream() << getFunctionPrefix() << ' ';

		if ( myFuncsUsedInInit.find( f.name ) == myFuncsUsedInInit.end() )
			curStream() << getInlineKeyword() << ' ';
	}

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

		checkNeedsSizeArgument( parm.type, parm.name );
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
CCommonLanguage::variable( CodeLContext &ctxt, const CodeVariableNode &v )
{
	if ( myInModuleInit > 0 )
	{
		std::map<std::string, std::string>::const_iterator i = myGlobalLiterals.find( v.name );

		// We'll just put the literal in directly
		if ( i != myGlobalLiterals.end() )
			return;

		bool doConst = ! v.info->isWritable();
		bool overrideInit = false;
		if ( usesFunctionInitializers() )
		{
			// in C++11 we can use initializer lists or constructors
			// for everything. In old c++, some things have to be
			// initialized in a function. if we use one of those, we
			// can't initialize ourselves that quickly...
			if ( usesUninitLocalGlobals( v.initialValue ) )
			{
				doConst = false;
				overrideInit = true;
			}
		}

		std::stringstream varDeclB;
		pushStream( varDeclB );
		InitType initT = variable( ctxt, v.name, v.info->type(),
								   doConst, false, false );
		popStream();
		std::string varDecl = varDeclB.str();
		bool doEmit = true;
		if ( overrideInit )
			initT = FUNC;

		if ( v.name.find( '$' ) != std::string::npos )
		{
			varDecl = v.name;

			std::string initVal;
			if ( v.initialValue )
			{
				if ( initT == FUNC )
				{
					if ( ! v.initialValue.cast<NameNode>() )
					{
						MESSAGE_LE( ctxt, ERR_TYPE, v.lineNumber,
									"Code Generator does not yet implement complex default value handling for functions" );
						throw std::logic_error( "NYI: complex default value handling for function type" );
					}
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
			myGlobalInitType[v.name] = initT;
			myGlobalVariables.insert( v.name );
			newlineAndIndent();
			const std::string &pref = getGlobalPrefix();
			if ( ! pref.empty() )
				curStream() << pref << ' ';
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
CCommonLanguage::assignment( CodeLContext &ctxt, const CodeAssignmentNode &v )
{
	newlineAndIndent();
	v.lhs->generateCode( ctxt );
	curStream() << " = ";
	v.lhs->type->generateCastFrom( v.rhs, ctxt );
	curStream() << ';';
}


////////////////////////////////////////


void
CCommonLanguage::expr( CodeLContext &ctxt, const CodeExprStatementNode &v )
{
	newlineAndIndent();
	v.expr->generateCode( ctxt );
	curStream() << ';';
}


////////////////////////////////////////


void CCommonLanguage::cond( CodeLContext &ctxt, const CodeIfNode &v )
{
	if ( myInElse > 0 )
		curStream() << ' ';
	else
		newlineAndIndent();
	curStream() << "if ( ";
	BoolTypePtr boolType = ctxt.newBoolType();
	boolType->generateCastFrom( v.condition, ctxt );
	curStream() << " )";

	StatementNodePtr tPath = v.truePath;
	StatementNodePtr fPath = v.falsePath;

	if ( tPath && ! tPath->next )
	{
		pushIndent();
		tPath->generateCode( ctxt );
		popIndent();
	}
	else
	{
		pushBlock();
		while ( tPath )
		{
			tPath->generateCode( ctxt );
			tPath = tPath->next;
		}
		popBlock();
	}

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
		else if ( ! fPath->next )
		{
			pushIndent();
			fPath->generateCode( ctxt );
			popIndent();
		}
		else
		{
			int oldV = myInElse;
			myInElse = 0;
			pushBlock();
			while ( fPath )
			{
				fPath->generateCode( ctxt );
				fPath = fPath->next;
			}
			popBlock();
			myInElse = oldV;
		}
	}
}


////////////////////////////////////////


void
CCommonLanguage::retval( CodeLContext &ctxt, const CodeReturnNode &v )
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
CCommonLanguage::loop( CodeLContext &ctxt, const CodeWhileNode &v )
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
CCommonLanguage::binaryOp( CodeLContext &ctxt, const CodeBinaryOpNode &v )
{
	// operator precedence in CTL is same as C++, but we will have
	// lost any parens during the parse stage, so we should
	// introduce them all the time just in case
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
CCommonLanguage::unaryOp( CodeLContext &ctxt, const CodeUnaryOpNode &v )
{
	v.type->generateCode( const_cast<CodeUnaryOpNode *>( &v ), ctxt );
	v.type->generateCastFrom( v.operand, ctxt );
}


////////////////////////////////////////


void
CCommonLanguage::index( CodeLContext &ctxt, const CodeArrayIndexNode &v )
{
	IntTypePtr intType = ctxt.newIntType();

	ExprNodePtr root;
	ArrayIndexNode *parentArray = dynamic_cast<ArrayIndexNode *>( v.array.pointer() );
	std::vector<std::string> parSubs;
	while ( parentArray )
	{
		std::stringstream tmpbuf;
		pushStream( tmpbuf );
		intType->generateCastFrom( parentArray->index, ctxt );
		popStream();
		ArrayIndexNode *nextPar = dynamic_cast<ArrayIndexNode *>( parentArray->array.pointer() );
		parSubs.push_back( tmpbuf.str() );

		if ( ! nextPar )
			root = parentArray->array;
		parentArray = nextPar;
	}
	if ( ! root )
		root = v.array;

	const ArrayInfo &arrInfo = collapseArray( dynamic_cast<ArrayType *>( root->type.pointer() ) );

	root->generateCode( ctxt );
	curStream() << '[';
	bool needAdd = false;
	if ( ! parSubs.empty() )
	{
		std::reverse( parSubs.begin(), parSubs.end() );
		const std::vector<int> &arr_sizes = arrInfo.first;
		if ( parSubs.size() != ( arr_sizes.size() - 1 ) )
		{
			MESSAGE_LE( ctxt, ERR_ARR_LEN, v.lineNumber,
						"Code Generator does not know how to interpret set of array subscripts" );
			throw std::logic_error( "Invalid set of array subscripts" );
		}

		for ( size_t pI = 0; pI != parSubs.size(); ++pI )
		{
			if ( arr_sizes[pI] < 0 )
			{
				curStream() << parSubs[pI] << "][";
				continue;
			}

			size_t nUnder = 1;
			for ( size_t x = pI + 1; x < arr_sizes.size(); ++x )
			{
				if ( arr_sizes[x] < 0 )
					break;

				nUnder *= arr_sizes[x];
			}
			
			if ( needAdd )
				curStream() << " + ";

			curStream() << "(" << parSubs[pI] << ") * " << nUnder;
			if ( arr_sizes[pI + 1] < 0 )
			{
				curStream() << "][";
				needAdd = false;
			}
			else
				needAdd = true;
		}
	}
	if ( needAdd )
		curStream() << " + ";
	intType->generateCastFrom( v.index, ctxt );
	curStream() << ']';
}


////////////////////////////////////////


void
CCommonLanguage::member( CodeLContext &ctxt, const CodeMemberNode &v )
{
	v.obj->generateCode( ctxt );
	curStream() << '.' << v.member;
}


////////////////////////////////////////


void
CCommonLanguage::size( CodeLContext &ctxt, const CodeSizeNode &v )
{
	std::cout << "size function for array - need to check if the array is a function parameter and retrieve <name>_size, or just extract the size value and inject it" << std::endl;
	MESSAGE_LE( ctxt, ERR_SIZE_SYNTAX, v.lineNumber,
				"Code Generator does not yet implement the size operator" );
	throw std::logic_error( "Function not yet implemented" );
}


////////////////////////////////////////


void
CCommonLanguage::name( CodeLContext &ctxt, const CodeNameNode &v )
{
	std::map<std::string, std::string>::const_iterator i = myGlobalLiterals.find( v.name );

	if ( i != myGlobalLiterals.end() )
	{
		curStream() << i->second;
		return;
	}
	
	std::string outname = removeNSQuals( v.name );

	if ( v.info->isFunction() )
	{
		const Module *m = v.info->module();
		if ( m == myCurModule )
			curStream() << myModPrefix << outname;
		else if ( m )
			curStream() << myNamespaceTags[m] << outname;
		else
			curStream() << getCTLNamespaceTag() << outname;

		return;
	}

	if ( myGlobalVariables.find( v.name ) != myGlobalVariables.end() )
	{
		const Module *m = v.info->module();
		if ( m == myCurModule )
			curStream() << myModPrefix << outname;
		else if ( m )
			curStream() << myNamespaceTags[m] << outname;
		else
		{
			// these are currently all #defines, so no namespace
			curStream() << outname;
		}

		return;
	}

	curStream() << outname;
}


////////////////////////////////////////


void
CCommonLanguage::boolLit( CodeLContext &ctxt, const CodeBoolLiteralNode &v )
{
	curStream() << getBoolLiteral( v.value );
}


////////////////////////////////////////


void
CCommonLanguage::intLit( CodeLContext &ctxt, const CodeIntLiteralNode &v )
{
	curStream() << v.value;
}


////////////////////////////////////////


void
CCommonLanguage::uintLit( CodeLContext &ctxt, const CodeUIntLiteralNode &v )
{
	curStream() << v.value;
}


////////////////////////////////////////


void
CCommonLanguage::halfLit( CodeLContext &ctxt, const CodeHalfLiteralNode &v )
{
	if ( ! supportsHalfType() )
	{
		MESSAGE_LE( ctxt, ERR_TYPE, v.lineNumber,
					"Language does not support the half data type" );
		throw std::logic_error( "Language does not support half" );
	}
	curStream() << "half( " << std::setprecision( std::numeric_limits<half>::digits ) << static_cast<float>( v.value ) << " )";
}


////////////////////////////////////////


void
CCommonLanguage::floatLit( CodeLContext &ctxt, const CodeFloatLiteralNode &v )
{
	curStream() << std::setprecision( std::numeric_limits<float>::digits ) << static_cast<float>( v.value );
}


////////////////////////////////////////


void
CCommonLanguage::stringLit( CodeLContext &ctxt, const CodeStringLiteralNode &v )
{
	curStream() << '"' << escapeLiteral( v.value ) << '"';
}


////////////////////////////////////////


static bool
isStdPrintFunction( const std::string &n )
{
	return ( n == "::print_bool" ||
			 n == "::print_int" ||
			 n == "::print_unsigned_int" ||
			 n == "::print_half" ||
			 n == "::print_float" ||
			 n == "::print_string" );
}

////////////////////////////////////////


void
CCommonLanguage::call( CodeLContext &ctxt, const CodeCallNode &v )
{
	std::string baseFuncName = removeNSQuals( v.function->name );
	bool isPrint = isStdPrintFunction( baseFuncName );

	if ( isPrint )
		newlineAndIndent();

	myStdFuncsUsed.insert( baseFuncName );
	v.function->generateCode( ctxt );
	if ( v.arguments.empty() )
		curStream() << "()";
	else
	{
		SymbolInfoPtr info = v.function->info;
		FunctionTypePtr functionType = info->functionType();
		const ParamVector &parameters = functionType->parameters();
		curStream() << "( ";
		size_t i = 0;
		for ( size_t N = v.arguments.size(); i != N; ++i )
		{
			if ( i >= parameters.size() )
			{
				MESSAGE_LE( ctxt, ERR_FUNC_ARG_NUM, v.lineNumber,
							"Too many arguments in function call" );
				throw std::logic_error( "Too many arguments in function call" );
			}

			if ( i > 0 )
				curStream() << ", ";

			parameters[i].type->generateCastFrom( v.arguments[i], ctxt );
			if ( checkNeedsSizeArgument( parameters[i].type, std::string() ) )
				extractSizeAndAdd( v.arguments[i], parameters[i].type, ctxt );
		}
		for ( size_t N = parameters.size(); i < N; ++i )
		{
			const Param &parm = parameters[i];

			if ( i > 0 )
				curStream() << ", ";

			if ( ! parm.defaultValue )
			{
				MESSAGE_LE( ctxt, ERR_UNKNOWN_TYPE, v.lineNumber,
							"Missing argument in function call (no default value)" );
				throw std::logic_error( "Missing argument in function call (no default value)" );
			}

			NameNodePtr n = parm.defaultValue.cast<NameNode>();
			std::string defVal;
			std::string namesp;
			if ( n )
			{
				const Module *m = n->info->module();
				if ( m )
				{
					if ( m != myCurModule )
						namesp = myNamespaceTags[m];
					else
						namesp = myModPrefix;
				}

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
				curStream() << namesp << found->second;
			else
				curStream() << defVal;

			if ( checkNeedsSizeArgument( parm.type, std::string() ) )
			{
				curStream() << ", ";
				extractSizeAndAdd( parm.defaultValue, parm.type, ctxt );
			}
		}
		curStream() << " )";
	}
	if ( isPrint )
		curStream() << ';';
}


////////////////////////////////////////


void
CCommonLanguage::value( CodeLContext &ctxt, const CodeValueNode &v )
{
	size_t idx = 0;
	valueRecurse( ctxt, v.elements, v.type, idx, "$$$$" );
}


////////////////////////////////////////


void
CCommonLanguage::startToBool( void )
{
	startCast( "bool" );
}


////////////////////////////////////////


void
CCommonLanguage::startToInt( void )
{
	startCast( "int" );
}


////////////////////////////////////////


void
CCommonLanguage::startToUint( void )
{
	startCast( "unsigned int" );
}


////////////////////////////////////////


void
CCommonLanguage::startToHalf( void )
{
	if ( ! supportsHalfType() )
		throw std::logic_error( "Language does not support the half type" );
	startCast( "half" );
}


////////////////////////////////////////


void
CCommonLanguage::startToFloat( void )
{
	startCast( "ctl_number_t" );
}


////////////////////////////////////////


void
CCommonLanguage::endCoersion( void )
{
	curStream() << " )";
}


////////////////////////////////////////


void
CCommonLanguage::emitToken( Token t )
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
CCommonLanguage::valueRecurse( CodeLContext &ctxt, const ExprNodeVector &elements,
							const DataTypePtr &t, size_t &index,
							const std::string &varName,
							bool isSubItem )
{
	ArrayType *arrayType = dynamic_cast<ArrayType *>( t.pointer() );
	if ( arrayType )
	{
		const ArrayInfo &arrInfo = collapseArray( arrayType );
		const std::vector<int> &arr_sizes = arrInfo.first;
		const DataTypePtr &coreType = arrayType->coreType();

		size_t nSubScripts = arr_sizes.size();
		size_t nItems = 1;
		size_t nPerCollapseChunk = 1;
		for ( size_t i = 0; i < nSubScripts; ++i )
		{
			if ( arr_sizes[i] == 0 )
				throw std::logic_error( "Attempt to initialize unknown array size" );

			if ( arr_sizes[i] > 0 )
				nItems *= abs( arr_sizes[i] );
			else
				nPerCollapseChunk *= abs( arr_sizes[i] );
		}

		bool doCtor = myCurInitType != CTOR;
		if ( arrInfo.second == "int" || arrInfo.second == "ctl_number_t" )
			doCtor = false;

		if ( myCurInitType == FUNC )
		{
			pushIndent();
			for ( size_t i = 0; i < nItems; ++i )
			{
				newlineAndIndent();
				if ( nItems > 1 )
					curStream() << varName << '[' << i << "] = ";
				else
					curStream() << varName << " = ";

				if ( doCtor )
					curStream() << arrInfo.second << "( ";

				for ( size_t x = 0; x < nPerCollapseChunk; ++x, ++index )
				{
					if ( x > 0 )
						curStream() << ", ";
					coreType->generateCastFrom( elements[index], ctxt );
				}
				if ( doCtor )
					curStream() << " );";
			}
			popIndent();
		}
		else
		{
			bool doBrace = false;
			if ( myCurInitType == ASSIGN )
			{
				newlineAndIndent();
				curStream() << "{ ";
				if ( nItems > 1 && nPerCollapseChunk > 1 && ! doCtor )
					doBrace = true;
			}
			pushIndent();

			bool lineEveryItem = nItems > 4;
			for ( size_t i = 0; i < nItems; ++i )
			{
				if ( doBrace )
				{
					if ( i > 0 )
						curStream() << ",\n";

					newlineAndIndent();
					curStream() << "{ ";
				}
				else if ( i > 0 )
				{
					if ( nPerCollapseChunk == 1 && ! lineEveryItem && i > 0 )
						curStream() << ", ";
					else
						curStream() << ',';
				}
				if ( doCtor )
				{
					if ( lineEveryItem )
						newlineAndIndent();

					curStream() << arrInfo.second << "( ";
					for ( size_t x = 0; x < nPerCollapseChunk; ++x, ++index )
					{
						if ( x > 0 )
							curStream() << ", ";
					
						coreType->generateCastFrom( elements[index], ctxt );
					}
					curStream() << " )";
				}
				else
				{
					for ( size_t x = 0; x < nPerCollapseChunk; ++x, ++index )
					{
						if ( x > 0 )
							curStream() << ',';

						if ( lineEveryItem )
							newlineAndIndent();
						else if ( x > 0 )
							curStream() << ' ';

						coreType->generateCastFrom( elements[index], ctxt );
					}
				}

				if ( doBrace )
					curStream() << " }";
			}
			popIndent();

			if ( myCurInitType == ASSIGN )
			{
				if ( lineEveryItem )
				{
					newlineAndIndent();
					curStream() << "}";
				}
				else
					curStream() << " }";
			}
		}

		return;
	}

	StructType *structType = dynamic_cast<StructType *>( t.pointer() );
	if ( structType )
	{
		if ( myCurInitType == ASSIGN )
			curStream() << '{';

		if ( myCurInitType == FUNC )
		{
			for ( MemberVectorConstIterator it = structType->members().begin();
				 it != structType->members().end();
				 ++it )
			{
				std::string name = varName + "." + it->name;
				valueRecurse( ctxt, elements, it->type, index, name, true );
			}
		}
		else
		{
			pushIndent();
			for ( MemberVectorConstIterator it = structType->members().begin();
				 it != structType->members().end();
				 ++it )
			{
				if ( it != structType->members().begin() )
					curStream() << ", ";

				valueRecurse( ctxt, elements, it->type, index, varName, true );
			}
			popIndent();
		}
		if ( myCurInitType == ASSIGN )
		{
			newlineAndIndent();
			curStream() << "}";
		}
		return;
	}

	if ( myCurInitType == FUNC )
	{
		newlineAndIndent();
		curStream() << varName << " = ";
		t->generateCastFrom( elements[index], ctxt );
		curStream() << ';';
	}
	else
		t->generateCastFrom( elements[index], ctxt );
	++index;
}


////////////////////////////////////////


CCommonLanguage::InitType
CCommonLanguage::variable( CodeLContext &ctxt,
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
				curStream() << getConstLiteral() << ' ';
			curStream() << "bool";
			break;
		case IntTypeEnum:
			if ( isConst )
				curStream() << getConstLiteral() << ' ';
			curStream() << "int";
			break;
		case UIntTypeEnum:
			if ( isConst )
				curStream() << getConstLiteral() << ' ';
			curStream() << "unsigned int";
			break;
		case HalfTypeEnum:
			if ( ! supportsHalfType() )
				throw std::logic_error( "Language does not support the half type" );
			if ( isConst )
				curStream() << getConstLiteral() << ' ';
			curStream() << "half";
			break;
		case FloatTypeEnum:
			if ( isConst )
				curStream() << getConstLiteral() << ' ';
			curStream() << "ctl_number_t";
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
			if ( usesFunctionInitializers() )
				retval = FUNC;
			else
				retval = ASSIGN;

			if ( retval == ASSIGN && isConst )
				curStream() << getConstLiteral() << ' ';
			curStream() << removeNSQuals( structType->name() );
			isWritable = isInput || isWritable;
			break;
		}
		case ArrayTypeEnum:
		{
			const ArrayInfo &arrInfo = collapseArray( dynamic_cast<ArrayType *>( t.pointer() ) );

			retval = ASSIGN;

			std::stringstream postBuf;
			bool needOpen = true;
			int mulCnt = 0;
			for ( size_t i = 0, N = arrInfo.first.size(); i != N; ++i )
			{
				int sz = arrInfo.first[i];
				// built-in vec / matrix set size to negative in collapse
				if ( sz < 0 )
					break;

				if ( needOpen )
				{
					postBuf << '[';
					needOpen = false;
				}
				if ( sz == 0 )
				{
					postBuf << ']';
					needOpen = true;
					mulCnt = 0;
					continue;
				}
				if ( mulCnt > 0 )
					postBuf << " * ";

				postBuf << sz;
				++mulCnt;
			}
			if ( ! needOpen )
				postBuf << ']';
			postDecl = postBuf.str();

			if ( postDecl.empty() && ( arrInfo.second != "int" && arrInfo.second != "ctl_number_t" ) )
				retval = CTOR;

			if ( isConst )
				curStream() << getConstLiteral() << ' ';

			curStream() << arrInfo.second;
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
CCommonLanguage::doInit( InitType initT, CodeLContext &ctxt,
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
				if ( ! initV.is_subclass<ValueNode>() )
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
CCommonLanguage::replaceInit( std::string &initS, const std::string &name )
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
CCommonLanguage::checkNeedInitInModuleInit( const ExprNodePtr &initV, bool deep )
{
	if ( ! initV )
		return false;

	if ( isAllLiterals( initV ) )
		return false;

	ValueNode *val = dynamic_cast<ValueNode *>( initV.pointer() );
	if ( val )
	{
		bool retval = false;
		// make sure we check every value for all possible functions
		for ( size_t i = 0, N = val->elements.size(); i != N; ++i )
		{
			bool a = checkNeedInitInModuleInit( val->elements[i] );
			if ( a && ! retval )
			{
				if ( ! deep )
					return true;
				retval = true;
			}
		}
		return true;
	}

	CallNode *c = dynamic_cast<CallNode *>( initV.pointer() );
	if ( c )
	{
		bool retval = false;
		SymbolInfoPtr info = c->function->info;
		if ( info->module() == myCurModule )
		{
			myFuncsUsedInInit.insert( c->function->name );
			if ( deep )
				retval = true;
			else
				return true;
		}

		if ( ! c->arguments.empty() )
		{
			// make sure we check every value for all possible functions
			for ( size_t i = 0, N = c->arguments.size(); i != N; ++i )
			{
				bool needed = checkNeedInitInModuleInit( c->arguments[i] );
				if ( needed && ! retval )
				{
					if ( ! deep )
						return true;
					retval = true;
				}
			}
		}
		return retval;
	}

	BinaryOpNodePtr bOp = initV.cast<BinaryOpNode>();
	if ( bOp )
	{
		// make sure both run to pick up
		// any functions used...
		bool l = checkNeedInitInModuleInit( bOp->leftOperand );
		bool r = checkNeedInitInModuleInit( bOp->rightOperand );
		return l || r;
	}
	UnaryOpNodePtr uOp = initV.cast<UnaryOpNode>();
	if ( uOp )
	{
		return checkNeedInitInModuleInit( uOp->operand );
	}

	return false;
}


////////////////////////////////////////


bool
CCommonLanguage::isAllLiterals( const ExprNodePtr &v )
{
	if ( ! v )
		return false;

	if ( v.is_subclass<LiteralNode>() )
		return true;

	if ( v.is_subclass<NameNode>() )
		return false;

	ValueNode *val = dynamic_cast<ValueNode *>( v.pointer() );
	if ( val )
	{
		for ( size_t i = 0, N = val->elements.size(); i != N; ++i )
		{
			if ( ! isAllLiterals( val->elements[i] ) )
				return false;
		}
		return true;
	}

	BinaryOpNodePtr bOp = v.cast<BinaryOpNode>();
	if ( bOp )
	{
		if ( ! isAllLiterals( bOp->leftOperand ) ||
			 ! isAllLiterals( bOp->rightOperand ) )
			return false;

		return true;
	}
	UnaryOpNodePtr uOp = v.cast<UnaryOpNode>();
	if ( uOp )
	{
		return isAllLiterals( uOp->operand );
	}

	return false;
}


////////////////////////////////////////


bool
CCommonLanguage::usesUninitLocalGlobals( const ExprNodePtr &v )
{
	if ( ! v )
		return false;

	if ( v.is_subclass<LiteralNode>() )
		return false;

	ValueNode *val = dynamic_cast<ValueNode *>( v.pointer() );
	if ( val )
	{
		for ( size_t i = 0, N = val->elements.size(); i != N; ++i )
		{
			if ( usesUninitLocalGlobals( val->elements[i] ) )
				return true;
		}
		return false;
	}

	NameNode *namePtr = dynamic_cast<NameNode *>( v.pointer() );
	if ( namePtr )
	{
		if ( namePtr->info->module() == myCurModule )
		{
			std::map<std::string, InitType>::const_iterator i = myGlobalInitType.find( namePtr->name );
			if ( i != myGlobalInitType.end() )
			{
				if ( i->second == FUNC )
					return true;
			}
		}

		return false;
	}

	CallNodePtr callPtr = v.cast<CallNode>();
	if ( callPtr )
	{
		for ( size_t i = 0, N = callPtr->arguments.size(); i != N; ++i )
		{
			if ( usesUninitLocalGlobals( callPtr->arguments[i] ) )
				return true;
		}

		return false;
	}

	BinaryOpNodePtr bOp = v.cast<BinaryOpNode>();
	if ( bOp )
	{
		if ( usesUninitLocalGlobals( bOp->leftOperand ) ||
			 usesUninitLocalGlobals( bOp->rightOperand ) )
			return true;

		return false;
	}
	UnaryOpNodePtr uOp = v.cast<UnaryOpNode>();
	if ( uOp )
	{
		return usesUninitLocalGlobals( uOp->operand );
	}

	return false;
}


////////////////////////////////////////


void
CCommonLanguage::extractLiteralConstants( const StatementNodePtr &consts,
										  CodeLContext &ctxt )
{
	StatementNodePtr curConst = consts;
	while ( curConst )
	{
		VariableNodePtr var = curConst.cast<VariableNode>();
		if ( isAllLiterals( var->initialValue ) )
		{
			if ( ! var->initialValue.cast<ValueNode>() )
			{
				std::stringstream x;
				pushStream( x );
				var->initialValue->generateCode( ctxt );
				popStream();
				myGlobalLiterals[var->name] = x.str();
			}
		}
		else if ( var->initialValue )
		{
			// don't care about the result, but need to get the function names
			// into the func used table
			checkNeedInitInModuleInit( var->initialValue, true );
		}
		curConst = curConst->next;
	}
}


////////////////////////////////////////


bool
CCommonLanguage::checkNeedsSizeArgument( const DataTypePtr &p, const std::string &name )
{
	int cnt = 0;
	if ( p->cDataType() == ArrayTypeEnum )
	{
		const ArrayInfo &arrInfo = collapseArray( dynamic_cast<ArrayType *>( p.pointer() ) );
		const std::vector<int> &sizes = arrInfo.first;

		for ( size_t i = 0, N = sizes.size(); i != N; ++i )
		{
			if ( sizes[i] == 0 )
				++cnt;
		}
	}

	if ( name.empty() )
		return cnt > 0;

	switch ( cnt )
	{
		case 0:
			break;
		case 1:
			curStream() << ", int " << name << "_size";
			break;
		case 2:
			curStream() << ", const ctl_vec2i_t &" << name << "_size";
			break;
		case 3:
			curStream() << ", const ctl_vec3i_t &" << name << "_size";
			break;
		default:
			throw std::logic_error( "Unimplemented size of variable array passing" );
			break;
	}
	
	return cnt > 0;
}


////////////////////////////////////////


void
CCommonLanguage::extractSizeAndAdd( const ExprNodePtr &p, const DataTypePtr &funcParm, CodeLContext &ctxt )
{
	if ( p->type->cDataType() == ArrayTypeEnum )
	{
		const ArrayInfo &arrInfo = collapseArray( dynamic_cast<ArrayType *>( p->type.pointer() ) );
		const std::vector<int> &arg_sizes = arrInfo.first;
		const ArrayInfo &funcInfo = collapseArray( dynamic_cast<ArrayType *>( funcParm.pointer() ) );
		const std::vector<int> &func_sizes = funcInfo.first;

		if ( arg_sizes.size() == func_sizes.size() )
		{
			int cnt = 0;
			SizeVector outSizes;
			for ( size_t i = 0, N = func_sizes.size(); i != N; ++i )
			{
				if ( func_sizes[i] == 0 )
				{
					++cnt;
					if ( arg_sizes[i] == 0 )
						throw std::logic_error( "Unknown argument size" );

					// take the absolute value since it could be a 'built-in' type
					// but we emit templates for lookup1D, et al. to translate
					// those...
					outSizes.push_back( abs( arg_sizes[i] ) );
				}
			}
			switch ( cnt )
			{
				case 0: throw std::logic_error( "Unhandled missing array size" );
				case 1: curStream() << ", static_cast<int>( "; break;
				case 2: curStream() << ", ctl_vec2i_t( "; break;
				case 3: curStream() << ", ctl_vec3i_t( "; break;
				default: throw std::logic_error( "Unimplemented size of variable array passing" );
			}
			for ( size_t i = 0, N = outSizes.size(); i != N; ++i )
			{
				if ( i > 0 )
					curStream() << ", ";
				curStream() << outSizes[i];
			}
			curStream() << " )";
		}
		else
		{
			throw std::logic_error( "Unhandled differing array sizes" );
		}
	}
	else
	{
		throw std::logic_error( "Unhandled array type coersion" );
	}
}


////////////////////////////////////////


const CCommonLanguage::ArrayInfo &
CCommonLanguage::collapseArray( const ArrayType *arrPtr )
{
	if ( ! arrPtr )
		throw std::logic_error( "Invalid array pointer passed" );

	ArrayInfoContainer::const_iterator ati = myArrayTypes.find( arrPtr );
	if ( ati == myArrayTypes.end() )
	{
		SizeVector usizes;
		arrPtr->sizes( usizes );
		std::vector<int> asize( usizes.size(), 0 );
		for ( size_t i = 0, N = usizes.size(); i != N; ++i )
			asize[i] = static_cast<int>( usizes[i] );

		std::string coreType;
		if ( asize.empty() )
			throw std::logic_error( "Empty array size list" );

		DataTypePtr coreTypePtr = arrPtr->coreType();
		if ( coreTypePtr.is_subclass<FloatType>() )
			coreType = "ctl_number_t";
		else if ( coreTypePtr.is_subclass<IntType>() )
			coreType = "int";
		else
			throw std::logic_error( "Currently unhandled core data type for array" );

		if ( asize.size() == 2 && asize[0] == asize[1] &&
			 ( asize[0] == 3 || asize[0] == 4 ) &&
			 coreType != "int" )
		{
			if ( asize[0] == 3 )
				coreType = "ctl_mat33f_t";
			else
				coreType = "ctl_mat44f_t";
			asize[0] = -asize[0];
			asize[1] = -asize[1];
		}
		else if ( asize.size() <= 2 )
		{
			// huh, if we do large tables with c++ objects, i.e. 3D luts,
			// both g++ and clang++ are pretty catatonic. gcc way more so
			// than clang++ but 3+ minutes for clang to compile and I never
			// could wait for g++ to finish a -O3 build...
			// so let's just do giant number table in those scenarios
			switch ( asize.back() )
			{
				case 2:
					if ( coreType == "int" )
						coreType = "ctl_vec2i_t";
					else
						coreType = "ctl_vec2f_t";
					asize.back() *= -1;
					break;
				case 3:
					if ( coreType == "int" )
						coreType = "ctl_vec3i_t";
					else
						coreType = "ctl_vec3f_t";
					asize.back() *= -1;
					break;
				case 4:
					if ( coreType == "int" )
						coreType = "ctl_vec4i_t";
					else
						coreType = "ctl_vec4f_t";
					asize.back() *= -1;
					break;
				default:
					break;
			}
		}

		ati = myArrayTypes.insert( std::make_pair( arrPtr, std::make_pair( asize, coreType ) ) ).first;
	}
	return ati->second;
}


////////////////////////////////////////


} // CtlCodeEmitter



