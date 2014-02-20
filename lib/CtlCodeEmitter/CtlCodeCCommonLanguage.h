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

#ifndef INCLUDED_CTL_CODE_C_COMMON_LANGUAGE_H
#define INCLUDED_CTL_CODE_C_COMMON_LANGUAGE_H

#include "CtlCodeLanguageGenerator.h"
#include "CtlCodeType.h"
#include <map>
#include <vector>
#include <set>


////////////////////////////////////////


///
/// @file CtlCodeCCommonLanguage.h
///
/// @author Kimball Thurston
///

namespace Ctl
{

///
/// @brief Class CCommonLanguage provides a class that
/// handles all the common stuff for C-like languages, with
/// routines to query specific keywords. Specific oddities for
/// a particular language can then be handled by overriding
/// the specific virtual functions without having to re-implement
/// everything.
///
class CCommonLanguage : public LanguageGenerator
{
public:
	CCommonLanguage( void );
	virtual ~CCommonLanguage( void );

	virtual void initStdLibrary( void );

	virtual std::string getHeaderCode( bool calledOnly );
	virtual std::string getCode( bool calledOnly );

	// Chunk of text that defines the standard CTL library
	// types and functions, as well as includes andy
	// language-specific includes
	virtual std::string stdLibraryAndSetup( void );

	virtual void pushBlock( void );
	virtual void popBlock( void );

	virtual void module( CodeLContext &ctxt, const CodeModuleNode &m );
	virtual void function( CodeLContext &ctxt, const CodeFunctionNode &f );
	virtual void variable( CodeLContext &ctxt, const CodeVariableNode &v );
	virtual void assignment( CodeLContext &ctxt, const CodeAssignmentNode &v );
	virtual void expr( CodeLContext &ctxt, const CodeExprStatementNode &v );
	virtual void cond( CodeLContext &ctxt, const CodeIfNode &v );
	virtual void retval( CodeLContext &ctxt, const CodeReturnNode &v );
	virtual void loop( CodeLContext &ctxt, const CodeWhileNode &v );
	virtual void binaryOp( CodeLContext &ctxt, const CodeBinaryOpNode &v );
	virtual void unaryOp( CodeLContext &ctxt, const CodeUnaryOpNode &v );
	virtual void index( CodeLContext &ctxt, const CodeArrayIndexNode &v );
	virtual void member( CodeLContext &ctxt, const CodeMemberNode &v );
	virtual void size( CodeLContext &ctxt, const CodeSizeNode &v );
	virtual void name( CodeLContext &ctxt, const CodeNameNode &v );
	virtual void boolLit( CodeLContext &ctxt, const CodeBoolLiteralNode &v );
	virtual void intLit( CodeLContext &ctxt, const CodeIntLiteralNode &v );
	virtual void uintLit( CodeLContext &ctxt, const CodeUIntLiteralNode &v );
	virtual void halfLit( CodeLContext &ctxt, const CodeHalfLiteralNode &v );
	virtual void floatLit( CodeLContext &ctxt, const CodeFloatLiteralNode &v );
	virtual void stringLit( CodeLContext &ctxt, const CodeStringLiteralNode &v );
	virtual void call( CodeLContext &ctxt, const CodeCallNode &v );
	virtual void value( CodeLContext &ctxt, const CodeValueNode &v );

	virtual void startToBool( void );
	virtual void startToInt( void );
	virtual void startToUint( void );
	virtual void startToHalf( void );
	virtual void startToFloat( void );
	virtual void endCoersion( void );

	virtual void emitToken( Token t );

protected:
	struct ModuleUsage
	{
		std::set<std::string> types;
		std::set<std::string> functions;
		std::set<std::string> variables;
	};
	typedef std::map<const Module *, ModuleUsage> ModuleUsageMap;

	struct TypeDefinition
	{
		inline TypeDefinition( void ) : emit( false ) {}
		inline TypeDefinition( const std::string &n, const std::string &d, const std::string &c = std::string() ) : name( n ), declare( d ), constfunc( c ), emit( false ) {}
		std::string name;
		std::string declare;
		std::string constfunc;
		ModuleUsageMap moduleUsage;
		bool emit;
	};

	struct GlobalVariableDefinition
	{
		inline GlobalVariableDefinition( void ) : emit( false ), module( NULL ) {}
		const Module *module;
		std::string name;
		std::string declare;
		std::string delayedInit;
		ModuleUsageMap moduleUsage;
		bool emit;
	};

	struct FunctionDefinition
	{
		inline FunctionDefinition( void ) : emit( false ), emitForward( false ) {}
		inline FunctionDefinition( const std::string &n, const std::string &b ) : name( n ), body( b ), emit( false ), emitForward( false ) {}
		std::string name;
		std::string header;
		std::string forwardDecl;
		std::string body;
		ModuleUsageMap moduleUsage;
		bool emit;
		bool emitForward;
	};
	typedef std::vector<FunctionDefinition> FuncDeclList;

	struct ModuleDefinition
	{
		inline ModuleDefinition( void ) : module( NULL ), typeCode( NULL ), fwdCode( NULL ), varCode( NULL ), funCode( NULL ) {}
		inline ~ModuleDefinition( void ) { delete typeCode; delete fwdCode; delete varCode; delete funCode; }
		const Module *module;
		std::string name;
		std::string call_prefix;
		std::string prefix;
		std::string suffix;
		std::vector<TypeDefinition> types;
		std::vector<GlobalVariableDefinition> variables;
		FuncDeclList functions;
		std::string headerCode;
		std::stringstream *typeCode;
		std::stringstream *fwdCode;
		std::stringstream *varCode;
		std::stringstream *funCode;
		std::string initCode;
	};

	virtual void outputInitCode( const std::string &modName, const std::string &initCode );
	virtual void recurseUsage( const std::map<const Module *, size_t> &indexmap, ModuleUsageMap &uMap, const Module *curModule = NULL );
	virtual void traverseAndEmit( const std::map<const Module *, size_t> &indexmap, TypeDefinition &t );
	virtual void traverseAndEmit( const std::map<const Module *, size_t> &indexmap, GlobalVariableDefinition &glob );
	virtual void traverseAndEmit( const std::map<const Module *, size_t> &indexmap, FunctionDefinition &func, bool fwdOnly = false );

	virtual void addStandardIncludes( void );
	virtual void initStandardLibraryBodies( ModuleDefinition &m );
	virtual void defineStandardTypes( std::vector<TypeDefinition> &types, const std::string &funcPref, const std::string &precSuffix );
	virtual void getStandardMathBodies( FuncDeclList &d, const std::string &funcPref, const std::string &precSuffix );
	virtual void getStandardHalfBodies( FuncDeclList &d, const std::string &funcPref, const std::string &precSuffix );
	virtual void getStandardPrintBodies( FuncDeclList &d, const std::string &funcPref, const std::string &precSuffix );
	virtual void getStandardColorBodies( FuncDeclList &d, const std::string &funcPref, const std::string &precSuffix );
	virtual void getStandardInterpBodies( FuncDeclList &d, const std::string &funcPref, const std::string &precSuffix );

	/// Should return true if we might have to initialize stuff
	/// in a function (pretty much everything except c++11)
	virtual bool usesFunctionInitializers( void ) const = 0;
	/// Whether this language requires a function to initialize
	/// module global variables, or if it can do it automatically
	virtual bool supportsModuleDynamicInitialization( void ) const = 0;
	virtual bool supportsNamespaces( void ) const = 0;
	virtual bool supportsHalfType( void ) const = 0;
	virtual bool supportsReferences( void ) const = 0;
	virtual bool supportsStructOperators( void ) const = 0;
	virtual bool needsStructTypedefs( void ) const = 0;

	virtual std::string constructNamespaceTag( const std::string &modName ) = 0;
	virtual const std::string &getInlineKeyword( void ) const = 0;
	/// @brief function prefix prior to declaring a non-main function
	virtual const std::string &getFunctionPrefix( void ) const = 0;
	/// @brief prefix prior to declaring a 'global' variable (might be in namespace)
	virtual const std::string &getGlobalPrefix( void ) const = 0;
	/// @brief return the type name for a boolean type
	virtual const std::string &getBoolTypeName( void ) const = 0;
	/// @brief return the literal string a boolean value
	virtual const std::string &getBoolLiteral( bool v ) const = 0;
	/// @brief return the literal string to tag a value as const
	virtual const std::string &getConstLiteral( void ) const = 0;
	virtual void startCast( const char *type ) = 0;

	virtual const std::string &beginComment( void ) const = 0;
	virtual const std::string &beginCommentLine( void ) const = 0;
	virtual const std::string &endComment( void ) const = 0;

//	virtual void emitStdFunction( const std::string &name ) = 0;

	enum InitType
	{
		CTOR,
		FUNC,
		ASSIGN,
		NONE
	};

	virtual void valueRecurse( CodeLContext &ctxt, const ExprNodeVector &elements,
							   const DataTypePtr &t, size_t &index,
							   const std::string &varName,
							   bool isSubItem = false );
	virtual InitType variable( CodeLContext &ctxt,
							   const std::string &name, const DataTypePtr &t,
							   bool isConst, bool isInput, bool isWritable,
							   bool isGlobal );
	virtual std::string doInit( InitType initT, CodeLContext &ctxt,
								const DataTypePtr &t,
								const ExprNodePtr &initV,
								const std::string &varName );
	virtual void replaceInit( std::string &initS, const std::string &name );

	virtual bool checkNeedInitInModuleInit( const ExprNodePtr &initV,
											bool deep = false );
	virtual bool isAllLiterals( const ExprNodePtr &v );
	virtual bool usesUninitLocalGlobals( const ExprNodePtr &v );

	// returns true if there are any non-trivial constants
	virtual void extractLiteralConstants( const StatementNodePtr &consts,
										  CodeLContext &ctxt );

	virtual bool checkNeedsSizeArgument( const DataTypePtr &p,
										 const std::string &base_name );
	virtual void extractSizeAndAdd( const ExprNodePtr &p,
									const DataTypePtr &funcParm,
									CodeLContext &ctxt );

	struct ArrayInfo
	{
		std::vector<int> sizes;
		std::string type;
		std::string maker;
		bool isCore;
	};

	// NB: The RcPtr type doesn't work for an array container, guess
	// it doesn't implement the less than operator in a std::container
	// kind of way, so we use the bare pointer...
	typedef std::map<const ArrayType *, std::pair<ArrayTypePtr, ArrayInfo> > ArrayInfoContainer;
	const ArrayInfo &collapseArray( const ArrayTypePtr &arrPtr );

	Module *myCurModule;
	std::string myCurModuleName;
	std::string myModPrefix;
	std::map<const Module *, std::string> myNamespaceTags;

	int myInElse;
	int myInFunction;
	int myInModuleInit;
	int myDoForwardDecl;
	int myExprLevel;
	InitType myCurInitType;
	std::map<std::string, InitType> myGlobalInitType;
	std::map<std::string, std::string> myGlobalLiterals;
	std::map<std::string, std::string> myDefaultMappings;
	std::set<std::string> myCurOutputVars;

	ArrayInfoContainer myArrayTypes;
	// functions that can't be inline...
	std::set<std::string> myFuncsUsedInInit;
	std::set<std::string> myGlobalVariables;
	std::stringstream myForwardDecl;
	std::set<std::string> myStdFunctionsCalled;
	std::map<std::string, std::string> myStdMathFuncs;
	std::map<std::string, std::string> myStdNames;

	ModuleDefinition *myCurModuleDefinition;
	ModuleUsageMap *myCurModuleUsage;
	std::vector<ModuleDefinition> myModules;
	std::stringstream *myCurInitDestination;
};

} // namespace CtlCodeEmitter

#endif // INCLUDED_CTL_CODE_C_COMMON_LANGUAGE_H


