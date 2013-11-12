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

#ifndef INCLUDED_CTL_CODE_LANGUAGE_GENERATOR_H
#define INCLUDED_CTL_CODE_LANGUAGE_GENERATOR_H

#include <sstream>
#include <stack>
#include <map>

#include <CtlSymbolTable.h>
#include "CtlCodeSyntaxTree.h"

namespace Ctl
{

class CodeLContext;

class LanguageGenerator
{
public:
	typedef std::map<std::string, std::pair<std::string, SymbolInfoPtr> > MainRoutineMap;

	enum Precision
	{
		FLOAT,
		DOUBLE,
		LONG_DOUBLE
	};

	LanguageGenerator( void );
	virtual ~LanguageGenerator( void );

	// Any additional setup necessary during std library init
	virtual void initStdLibrary( void );

	// Needs to be called before the init stuff is called
	// or any modules are parsed
	virtual bool supportsPrecision( Precision p ) const = 0;
	void setPrecision( Precision p );
	inline Precision getPrecision( void ) const { return myPrecision; }

	inline const MainRoutineMap &getMainRoutines( void ) const { return myMainRoutines; }

	// any necessary setup, i.e. ifdef __cplusplus extern "C" stuff
	virtual std::string getHeaderPrefix( void ) const;
	// Should just be the setup code (i.e. forward decls
	// for functions in C)
	virtual std::string getHeaderCode( bool calledOnly ) = 0;
	// any necessary cleanup i.e. ifdef __cplusplus stuff
	virtual std::string getHeaderSuffix( void ) const;

	// Chunk of text that sets up all the CTL library
	// and any language specific includes
	virtual std::string stdLibraryAndSetup( void ) = 0;

	// Returns the body of code
	virtual std::string getCode( bool calledOnly ) = 0;

	virtual std::string getDriver( void );

	virtual void pushBlock( void ) = 0;
	virtual void popBlock( void ) = 0;

	virtual void module( CodeLContext &ctxt, const CodeModuleNode &m ) = 0;
	virtual void function( CodeLContext &ctxt, const CodeFunctionNode &f ) = 0;
	virtual void variable( CodeLContext &ctxt, const CodeVariableNode &v ) = 0;
	virtual void assignment( CodeLContext &ctxt, const CodeAssignmentNode &v ) = 0;
	virtual void expr( CodeLContext &ctxt, const CodeExprStatementNode &v ) = 0;
	virtual void cond( CodeLContext &ctxt, const CodeIfNode &v ) = 0;
	virtual void retval( CodeLContext &ctxt, const CodeReturnNode &v ) = 0;
	virtual void loop( CodeLContext &ctxt, const CodeWhileNode &v ) = 0;
	virtual void binaryOp( CodeLContext &ctxt, const CodeBinaryOpNode &v ) = 0;
	virtual void unaryOp( CodeLContext &ctxt, const CodeUnaryOpNode &v ) = 0;
	virtual void index( CodeLContext &ctxt, const CodeArrayIndexNode &v ) = 0;
	virtual void member( CodeLContext &ctxt, const CodeMemberNode &v ) = 0;
	virtual void size( CodeLContext &ctxt, const CodeSizeNode &v ) = 0;
	virtual void name( CodeLContext &ctxt, const CodeNameNode &v ) = 0;
	virtual void boolLit( CodeLContext &ctxt, const CodeBoolLiteralNode &v ) = 0;
	virtual void intLit( CodeLContext &ctxt, const CodeIntLiteralNode &v ) = 0;
	virtual void uintLit( CodeLContext &ctxt, const CodeUIntLiteralNode &v ) = 0;
	virtual void halfLit( CodeLContext &ctxt, const CodeHalfLiteralNode &v ) = 0;
	virtual void floatLit( CodeLContext &ctxt, const CodeFloatLiteralNode &v ) = 0;
	virtual void stringLit( CodeLContext &ctxt, const CodeStringLiteralNode &v ) = 0;
	virtual void call( CodeLContext &ctxt, const CodeCallNode &v ) = 0;
	virtual void value( CodeLContext &ctxt, const CodeValueNode &v ) = 0;

	// type coersion
	virtual void startToBool( void ) = 0;
	virtual void startToInt( void ) = 0;
	virtual void startToUint( void ) = 0;
	virtual void startToHalf( void ) = 0;
	virtual void startToFloat( void ) = 0;
	virtual void endCoersion( void ) = 0;

	virtual void emitToken( Token t ) = 0;

protected:

	void setIndentText( const std::string &i );

	void addIndent( void );
	void newlineAndIndent( void );
	void pushIndent( void );
	void popIndent( void );

	inline std::ostream &curStream( void ) { return *(myStreamStack.top()); }

	void pushStream( std::ostream &os );
	void popStream( void );

	// might be useful in sub classing
	std::string cleanName( const std::string &x );
	std::string escapeLiteral( const std::string &s );
	std::string removeNSQuals( const std::string &x );

	virtual std::string getPrecisionFunctionSuffix( void ) const;
	virtual std::string getPrecisionType( void ) const;
	virtual std::string getPrecisionTypeSuffix( void ) const;

	void registerMainRoutine( const std::string &name, const std::string &nsName,
							  const SymbolInfoPtr &fnInfo );

	MainRoutineMap myMainRoutines;

private:
	Precision myPrecision;
	std::string myIndent;
	int myCurIndent;
	std::stack<std::ostream *> myStreamStack;
};

} // namespace Ctl

#endif // INCLUDED_CTL_CODE_LANGUAGE_GENERATOR_H
