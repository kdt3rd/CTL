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

#ifndef INCLUDED_CTL_CODE_CPP_LANGUAGE_H
#define INCLUDED_CTL_CODE_CPP_LANGUAGE_H

#include "CtlCodeLanguageGenerator.h"
#include "CtlCodeType.h"
#include <map>
#include <vector>

namespace Ctl
{

class CPPGenerator : public LanguageGenerator
{
public:
	CPPGenerator( bool cpp11 = false );
	virtual ~CPPGenerator( void );

	virtual std::string getHeaderCode( void );
	virtual std::string getCode( void );

	// Chunk of text that sets up all the CTL library
	// and any language specific includes
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
	enum InitType
	{
		CTOR,
		FUNC,
		ASSIGN,
		NONE
	};

	void valueRecurse( CodeLContext &ctxt, const ExprNodeVector &elements,
					   const DataTypePtr &t, size_t &index );
	InitType variable( CodeLContext &ctxt,
					   const std::string &name, const DataTypePtr &t,
					   bool isConst, bool isInput, bool isWritable );
	void doInit( InitType initT, CodeLContext &ctxt,
				 const ExprNodePtr &initV, const std::string &varName );
	void replaceInit( std::string &initS, const std::string &name );
	bool findBuiltinType( std::string &name,
						  std::string &postDecl,
						  const ArrayTypePtr &array,
						  CodeLContext &ctxt );
	bool checkNeedInitInModuleInit( const ExprNodePtr &initV );

	// might be useful in sub classing
	std::string cleanName( const std::string &x );
	std::string escapeLiteral( const std::string &s );
	std::string removeNSQuals( const std::string &x );

	bool myCPP11Mode;
	std::stringstream myCodeStream;
	std::stringstream myHeaderStream;
	int myInElse;
	int myInModuleInit;
	int myInFunction;
	InitType myCurInitType;
	std::map<std::string, std::string> myDefaultMappings;
	std::vector< std::vector<std::string> > myCurModuleInit;
	Module *myCurModule;
};

}

#endif // INCLUDED_CTL_CODE_CPP_LANGUAGE_H
