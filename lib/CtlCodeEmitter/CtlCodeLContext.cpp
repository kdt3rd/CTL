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
//	 * Redistributions of source code must retain the above copyright
//	   notice, this list of conditions and the Disclaimer of Warranty.
// 
//	 * Redistributions in binary form must reproduce the above copyright
//	   notice, this list of conditions and the Disclaimer of Warranty
//	   in the documentation and/or other materials provided with the
//	   distribution.
// 
//	 * Nothing in this license shall be deemed to grant any rights to
//	   trademarks, copyrights, patents, trade secrets or any other
//	   intellectual property of A.M.P.A.S. or any contributors, except
//	   as expressly stated herein, and neither the name of A.M.P.A.S.
//	   nor of any other contributors to this software, may be used to
//	   endorse or promote products derived from this software without
//	   specific prior written permission of A.M.P.A.S. or contributor,
//	   as appropriate.
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

//-----------------------------------------------------------------------------
//
//	class CodeLContext
//
//-----------------------------------------------------------------------------

#include "CtlCodeLContext.h"
#include "CtlCodeSyntaxTree.h"
#include "CtlCodeType.h"
#include "CtlCodeModule.h"
#include "CtlCodeAddr.h"
#include "CtlSymbolTable.h"

#include "CtlCodeLanguageGenerator.h"

#include <cassert>
#include <stdexcept>

#if 1
# include <iostream>
# define debug(x) (std::cout << x << std::endl)
#else
# define debug(x)
#endif

namespace Ctl
{


CodeLContext::CodeLContext( LanguageGenerator &gen,
							std::istream &file,
							Module *module,
							SymbolTable &symtab )
		: LContext( file, module, symtab ),
		  _nextParameterAddr( -1 ),
		  myLanguageGen( gen )
{
}


void
CodeLContext::newStackFrame( void )
{
	_nextParameterAddr = -1;
	_locals.clear();
}


AddrPtr
CodeLContext::parameterAddr( const DataTypePtr &parameterType )
{
	return new CodeDataAddr( _nextParameterAddr-- );
}


AddrPtr
CodeLContext::returnValueAddr( const DataTypePtr &returnType )
{
	return new CodeDataAddr( _nextParameterAddr-- );
}


AddrPtr
CodeLContext::autoVariableAddr( const DataTypePtr &variableType )
{
	_locals.push_back(variableType);
	return new CodeDataAddr( _locals.size() - 1 );
}


std::vector<DataTypePtr> &
CodeLContext::locals( void )
{
	return _locals;
}


ModuleNodePtr
CodeLContext::newModuleNode( int lineNumber,
							 const StatementNodePtr &constants,
							 const FunctionNodePtr &functions ) const
{
	return new CodeModuleNode( lineNumber, constants, functions );
}


FunctionNodePtr
CodeLContext::newFunctionNode( int lineNumber,
							   const std::string &name,
							   const SymbolInfoPtr &info,
							   const StatementNodePtr &body ) const
{
	return new CodeFunctionNode( lineNumber, name, info, body, _locals );
}


VariableNodePtr
CodeLContext::newVariableNode( int lineNumber,
							   const std::string &name,
							   const SymbolInfoPtr &info,
							   const ExprNodePtr &initialValue,
							   bool assignInitialValue) const
{
	return new CodeVariableNode( lineNumber, name, info,
								 initialValue, assignInitialValue );
}


AssignmentNodePtr
CodeLContext::newAssignmentNode( int lineNumber,
								 const ExprNodePtr &lhs,
								 const ExprNodePtr &rhs ) const
{
	return new CodeAssignmentNode( lineNumber, lhs, rhs );
}


ExprStatementNodePtr
CodeLContext::newExprStatementNode( int lineNumber,
									const ExprNodePtr &expr ) const
{
	return new CodeExprStatementNode( lineNumber, expr );
}


IfNodePtr
CodeLContext::newIfNode( int lineNumber,
						 const ExprNodePtr &condition,
						 const StatementNodePtr &truePath,
						 const StatementNodePtr &falsePath ) const
{
	return new CodeIfNode( lineNumber, condition, truePath, falsePath );
}


ReturnNodePtr
CodeLContext::newReturnNode( int lineNumber,
							 const SymbolInfoPtr &info,
							 const ExprNodePtr &returnedValue ) const
{
	return new CodeReturnNode( lineNumber, info, returnedValue );
}


WhileNodePtr
CodeLContext::newWhileNode( int lineNumber,
							const ExprNodePtr &condition,
							const StatementNodePtr &loopBody ) const
{
	return new CodeWhileNode( lineNumber, condition, loopBody );
}


BinaryOpNodePtr
CodeLContext::newBinaryOpNode( int lineNumber,
							   Token op,
							   const ExprNodePtr &leftOperand,
							   const ExprNodePtr &rightOperand) const
{
	return new CodeBinaryOpNode( lineNumber, op, leftOperand, rightOperand );
}


UnaryOpNodePtr
CodeLContext::newUnaryOpNode( int lineNumber,
							  Token op,
							  const ExprNodePtr &operand) const
{
	return new CodeUnaryOpNode( lineNumber, op, operand );
}


ArrayIndexNodePtr
CodeLContext::newArrayIndexNode( int lineNumber,
								 const ExprNodePtr &array,
								 const ExprNodePtr &index) const
{
	return new CodeArrayIndexNode( lineNumber, array, index );
}

MemberNodePtr
CodeLContext::newMemberNode( int lineNumber,
							 const ExprNodePtr &obj,
							 const std::string &member) const
{
	return new CodeMemberNode( lineNumber, obj, member );
}


SizeNodePtr
CodeLContext::newSizeNode( int lineNumber,
						   const ExprNodePtr &obj ) const
{
	return new CodeSizeNode( lineNumber, obj );
}

NameNodePtr
CodeLContext::newNameNode( int lineNumber,
						   const std::string &name,
						   const SymbolInfoPtr &info ) const
{
	return new CodeNameNode( lineNumber, name, info );
}


BoolLiteralNodePtr
CodeLContext::newBoolLiteralNode( int lineNumber, bool value, const std::string &raw ) const
{
	return new CodeBoolLiteralNode( lineNumber, *this, value, raw );
}


IntLiteralNodePtr
CodeLContext::newIntLiteralNode( int lineNumber, int value, const std::string &raw ) const
{
	return new CodeIntLiteralNode( lineNumber, *this, value, raw );
}


UIntLiteralNodePtr
CodeLContext::newUIntLiteralNode( int lineNumber, unsigned value, const std::string &raw ) const
{
	return new CodeUIntLiteralNode( lineNumber, *this, value, raw );
}


HalfLiteralNodePtr
CodeLContext::newHalfLiteralNode( int lineNumber, half value, const std::string &raw ) const
{
	return new CodeHalfLiteralNode( lineNumber, *this, value, raw );
}


FloatLiteralNodePtr
CodeLContext::newFloatLiteralNode( int lineNumber, number value, const std::string &raw ) const
{
	return new CodeFloatLiteralNode( lineNumber, *this, value, raw );
}


StringLiteralNodePtr
CodeLContext::newStringLiteralNode( int lineNumber, const std::string &value ) const
{
	return new CodeStringLiteralNode( lineNumber, *this, value );
}


CallNodePtr
CodeLContext::newCallNode( int lineNumber,
						   const NameNodePtr &function,
						   const ExprNodeVector &arguments) const
{
	return new CodeCallNode( lineNumber, function, arguments );
}


ValueNodePtr
CodeLContext::newValueNode( int lineNumber,
							const ExprNodeVector &elements ) const
{
	return new CodeValueNode( lineNumber, elements );
}


VoidTypePtr
CodeLContext::newVoidType( void ) const
{
	static VoidTypePtr voidType = new CodeVoidType;
	return voidType;
}


BoolTypePtr
CodeLContext::newBoolType( void ) const
{
	static BoolTypePtr boolType = new CodeBoolType;
	return boolType;
}


IntTypePtr
CodeLContext::newIntType( void ) const
{
	static IntTypePtr intType = new CodeIntType;
	return intType;
}


UIntTypePtr
CodeLContext::newUIntType( void ) const
{
	static UIntTypePtr uintType = new CodeUIntType;
	return uintType;
}


HalfTypePtr
CodeLContext::newHalfType( void ) const
{
	static HalfTypePtr halfType = new CodeHalfType;
	return halfType;
}


FloatTypePtr
CodeLContext::newFloatType( void ) const
{
	static FloatTypePtr floatType = new CodeFloatType;
	return floatType;
}


StringTypePtr
CodeLContext::newStringType( void ) const
{
	static StringTypePtr stringType = new CodeStringType;
	return stringType;
}


ArrayTypePtr
CodeLContext::newArrayType( const DataTypePtr &baseType, 
							int size,
							ArrayTypeUsage usage /*= NON_PARAMETER */ )
{
	if( usage == PARAMETER)
		return new CodeArrayType( baseType, size, this );

	return new CodeArrayType( baseType, size );
}


StructTypePtr	
CodeLContext::newStructType( const std::string &name,
							 const MemberVector &members ) const
{
	myStructDefines.push_back( std::make_pair( name, members ) );
	return new CodeStructType(name, members);
}

FunctionTypePtr
CodeLContext::newFunctionType( const DataTypePtr &returnType,
							   bool returnVarying,
							   const ParamVector &parameters ) const
{
	return new CodeFunctionType( returnType, returnVarying, parameters );
}


} // namespace Ctl
