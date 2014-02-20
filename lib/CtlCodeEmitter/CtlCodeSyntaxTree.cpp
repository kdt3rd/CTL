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


//-----------------------------------------------------------------------------
//
//	The syntax tree for the CODE implementation of the
//	color transformation language.
//
//-----------------------------------------------------------------------------

#include "CtlCodeSyntaxTree.h"
#include "CtlCodeModule.h"
#include "CtlCodeLContext.h"
#include "CtlSymbolTable.h"
#include "CtlCodeType.h"
#include "CtlCodeLanguageGenerator.h"
#include <cassert>
#include <vector>

namespace Ctl
{


////////////////////////////////////////


CodeModuleNode::CodeModuleNode( int lineNumber,
								const StatementNodePtr &constants,
								const FunctionNodePtr &functions )
		: ModuleNode( lineNumber, constants, functions )
{
}

void
CodeModuleNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().module( lctxt, *this );
}


////////////////////////////////////////


CodeFunctionNode::CodeFunctionNode( int lineNumber,
									const std::string &name,
									const SymbolInfoPtr &info,
									const StatementNodePtr &body,
									const std::vector<DataTypePtr> &locals )
		: FunctionNode( lineNumber, name, info, body ),
		  _locals( locals )
{
}

void
CodeFunctionNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().function( lctxt, *this );
}


////////////////////////////////////////


CodeVariableNode::CodeVariableNode( int lineNumber,
									const std::string &name,
									const SymbolInfoPtr &info,
									const ExprNodePtr &initialValue,
									bool assignInitialValue )
		: VariableNode( lineNumber, name, info, initialValue, assignInitialValue )
{
}


void
CodeVariableNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().variable( lctxt, *this );
//	//
//	// Generate code for a variable declaration.
//	//
//	// If the variable has no initial value, then there is nothing to do.
//	//
//	// If the variable does have an initial value, then there are two cases:
//	//
//	//	   If the initial value is assigned to the variable,
//	//	   then generate code to
//	//		   - push a reference to the variable onto the stack
//	//		   - evaluate the expression that computes the initial value
//	//		   - cast the value to the type of the variable
//	//		   - assign initial value to the variable.
//	//
//	//	   Otherwise the variable is initialized by a side-effect of
//	//	   the initialization expression.  In this case we generate
//	//	   code to
//	//		   - evaluate the expression that computes the initial value
//	//		   - pop the expression's value off the stack (unless the
//	//			 expression's value is of type void).
//	// 
//
//	if (initialValue)
//	{
//		CodeLContext &slcontext = static_cast <CodeLContext &> (lcontext);
//	
//		CodeDataAddrPtr dataPtr = info->addr().cast<CodeDataAddr>();
//		CodeValueNodePtr valuePtr = initialValue.cast<CodeValueNode>();
//
//		if (assignInitialValue)
//		{
//			//
//			// Initial value is assigned to the variable.
//			//
//
//			if( valuePtr && valuePtr->type && dataPtr  && dataPtr->reg())
//			{
//				// The variable is static, and its value is a literal
//				// or a collection of literals (for structs and arrays).
//				// We can copy the initial value directly into the
//				// variable instead of generating code to assign the
//				// value.
//		
//				// get sizes & offsets of elements
//				SizeVector sizes;
//				SizeVector offsets;
//				DataTypePtr dataType = valuePtr->type;
//				dataType->coreSizes(0, sizes, offsets);
//
//				ExprNodeVector &elements = valuePtr->elements;
//				int numElements = elements.size();
//				assert((int)sizes.size() == numElements 
//					   && (int)offsets.size() == numElements);
//				assert(!dataPtr->reg()->isVarying());
//		
//				char* dest = (*dataPtr->reg())[0];
//		
//				int eIndex = 0;
//
//				try {
//					valuePtr->castAndCopyRec(lcontext, dataType, eIndex,
//											 dest, sizes, offsets);
//				} catch (Iex::TypeExc) {
//					// HACK: this should be avoided ahead of time
//					slcontext.addInst (new CodePushRefInst (info->addr(), lineNumber));
//					initialValue->generateCode (lcontext);
//					info->type()->generateCastFrom (initialValue, lcontext);
//					info->type()->generateCode (this, lcontext);
//				}
//			}
//			else
//			{
//				slcontext.addInst (new CodePushRefInst (info->addr(), lineNumber));
//				initialValue->generateCode (lcontext);
//				info->type()->generateCastFrom (initialValue, lcontext);
//				info->type()->generateCode (this, lcontext);
//			}
//		}
//		else
//		{
//			//
//			// Variable is initialized via side-effect.
//			//
//
//			initialValue->generateCode (lcontext);
//
//			const CodeCallNode *call = 
//				dynamic_cast <const CodeCallNode*>(initialValue.pointer());
//			
//			RcPtr<CodeVoidType> pv(new CodeVoidType());
//
//			if (call == 0 || !call->returnsType (pv))
//			{
//				slcontext.addInst (new CodePopInst (1, lineNumber));
//			}
//		}
//	}
}


////////////////////////////////////////


CodeAssignmentNode::CodeAssignmentNode( int lineNumber,
										const ExprNodePtr &lhs,
										const ExprNodePtr &rhs )
		: AssignmentNode( lineNumber, lhs, rhs )
{
}


void
CodeAssignmentNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().assignment( lctxt, *this );
}


////////////////////////////////////////


CodeExprStatementNode::CodeExprStatementNode( int lineNumber,
											  const ExprNodePtr &expr)
		: ExprStatementNode( lineNumber, expr )
{
}


void
CodeExprStatementNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().expr( lctxt, *this );
}


////////////////////////////////////////


CodeIfNode::CodeIfNode( int lineNumber,
						const ExprNodePtr &condition,
						const StatementNodePtr &truePath,
						const StatementNodePtr &falsePath )
		: IfNode( lineNumber, condition, truePath, falsePath )
{
}

void
CodeIfNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().cond( lctxt, *this );
}


////////////////////////////////////////


CodeReturnNode::CodeReturnNode( int lineNumber,
								const SymbolInfoPtr &info,
								const ExprNodePtr &returnedValue )
		: ReturnNode( lineNumber, info, returnedValue )
{
}

void
CodeReturnNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().retval( lctxt, *this );
}


////////////////////////////////////////


CodeWhileNode::CodeWhileNode( int lineNumber,
							  const ExprNodePtr &condition,
							  const StatementNodePtr &loopBody )
		: WhileNode( lineNumber, condition, loopBody )
{
}

void
CodeWhileNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().loop( lctxt, *this );
}


////////////////////////////////////////


CodeBinaryOpNode::CodeBinaryOpNode( int lineNumber,
									Token op,
									const ExprNodePtr &leftOperand,
									const ExprNodePtr &rightOperand )
		: BinaryOpNode( lineNumber, op, leftOperand, rightOperand )
{
}

void
CodeBinaryOpNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().binaryOp( lctxt, *this );
}


////////////////////////////////////////


CodeUnaryOpNode::CodeUnaryOpNode( int lineNumber,
								  Token op,
								  const ExprNodePtr &operand )
		: UnaryOpNode( lineNumber, op, operand )
{
}

void
CodeUnaryOpNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().unaryOp( lctxt, *this );
}


////////////////////////////////////////


CodeArrayIndexNode::CodeArrayIndexNode( int lineNumber,
										const ExprNodePtr &array,
										const ExprNodePtr &index )
		: ArrayIndexNode( lineNumber, array, index )
{
}

void
CodeArrayIndexNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().index( lctxt, *this );
}


////////////////////////////////////////


CodeMemberNode::CodeMemberNode( int lineNumber,
								const ExprNodePtr &obj,
								const std::string &member )
		: MemberNode (lineNumber, obj, member)
{
}

void
CodeMemberNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().member( lctxt, *this );
}


////////////////////////////////////////


CodeSizeNode::CodeSizeNode( int lineNumber,
							const ExprNodePtr &obj )
		: SizeNode( lineNumber, obj )
{
}

void
CodeSizeNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().size( lctxt, *this );
}


////////////////////////////////////////


CodeNameNode::CodeNameNode( int lineNumber,
							const std::string &name,
							const SymbolInfoPtr &info )
		: NameNode( lineNumber, name, info )
{
}

void
CodeNameNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().name( lctxt, *this );
}


////////////////////////////////////////


CodeBoolLiteralNode::CodeBoolLiteralNode( int lineNumber,
										  const LContext &lcontext,
										  bool value,
										  const std::string &raw )
		: BoolLiteralNode( lineNumber, lcontext, value, raw )
{
}

void
CodeBoolLiteralNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().boolLit( lctxt, *this );
}

char*
CodeBoolLiteralNode::valuePtr( void )
{
	return (char*)(&value);
}


////////////////////////////////////////


CodeIntLiteralNode::CodeIntLiteralNode( int lineNumber,
										const LContext &lcontext,
										int value,
										const std::string &raw )
		: IntLiteralNode( lineNumber, lcontext, value, raw )
{
}

void
CodeIntLiteralNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().intLit( lctxt, *this );
}

char*
CodeIntLiteralNode::valuePtr( void )
{
	return (char*)(&value);
}


////////////////////////////////////////


CodeUIntLiteralNode::CodeUIntLiteralNode( int lineNumber,
										  const LContext &lcontext,
										  unsigned int value,
										  const std::string &raw )
		: UIntLiteralNode( lineNumber, lcontext, value, raw )
{
}

void
CodeUIntLiteralNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().uintLit( lctxt, *this );
}

char*
CodeUIntLiteralNode::valuePtr( void )
{
	return (char*)(&value);
}


////////////////////////////////////////


CodeHalfLiteralNode::CodeHalfLiteralNode( int lineNumber,
										  const LContext &lcontext,
										  half value,
										  const std::string &raw )
		: HalfLiteralNode( lineNumber, lcontext, value, raw )
{
}

void
CodeHalfLiteralNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().halfLit( lctxt, *this );
}

char*
CodeHalfLiteralNode::valuePtr( void )
{
	return (char*)(&value);
}


////////////////////////////////////////


CodeFloatLiteralNode::CodeFloatLiteralNode( int lineNumber,
											const LContext &lcontext,
											number value,
											const std::string &raw )
		: FloatLiteralNode( lineNumber, lcontext, value, raw )
{
}

void
CodeFloatLiteralNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().floatLit( lctxt, *this );
}

char*
CodeFloatLiteralNode::valuePtr()
{
	return (char*)(&value);
}


////////////////////////////////////////


CodeStringLiteralNode::CodeStringLiteralNode( int lineNumber,
											  const LContext &lcontext,
											  const std::string &value )
		: StringLiteralNode( lineNumber, lcontext, value )
{
}


void
CodeStringLiteralNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().stringLit( lctxt, *this );
}

char*
CodeStringLiteralNode::valuePtr( void )
{
	return 0;
}


////////////////////////////////////////


CodeCallNode::CodeCallNode( int lineNumber,
							const NameNodePtr &function,
							const ExprNodeVector &arguments )
		: CallNode( lineNumber, function, arguments )
{
}

bool
CodeCallNode::returnsType( const TypePtr &t ) const
{
	SymbolInfoPtr info = function->info;

	if ( !info )
		return false;

	FunctionTypePtr functionType = function->info->functionType();
	DataTypePtr returnType = functionType->returnType();

	return returnType->isSameTypeAs( t );
}

void
CodeCallNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().call( lctxt, *this );
}


////////////////////////////////////////


CodeValueNode::CodeValueNode( int lineNumber,
							  const ExprNodeVector &elements )
		: ValueNode( lineNumber, elements )
{
}


void
CodeValueNode::generateCode( LContext &ctxt )
{
	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	lctxt.generator().value( lctxt, *this );
}


void
CodeValueNode::castAndCopyRec( LContext &lcontext,
							   const DataTypePtr &dataType,
							   int &eIndex,
							   char *dest,
							   const SizeVector &sizes,
							   const SizeVector &offsets )
{
//	// For each element in the array, 
//	//	  cast to the correct type
//	//	  and copy into dest
//	// Then assign multiple to the array
//	//
//	if( StructTypePtr structType = dataType.cast<StructType>())
//	{
//		for(MemberVectorConstIterator it = structType->members().begin();
//			it != structType->members().end();
//			it++)
//		{
//			castAndCopyRec(lcontext, it->type, eIndex, dest, sizes, offsets);
//		}
//	}
//	else if( ArrayTypePtr arrayType = dataType.cast<ArrayType>())
//	{
//		for (int i = 0; i < arrayType->size(); ++i)
//		{
//			castAndCopyRec(lcontext, arrayType->elementType(), eIndex, 
//						   dest, sizes, offsets);
//		}
//	}
//	else
//	{
//		assert(eIndex < (int)elements.size());
//		LiteralNodePtr literal = elements[eIndex];
//		literal = dataType->castValue(lcontext, literal);
//		memcpy(dest+offsets[eIndex], literal->valuePtr(), sizes[eIndex]);
//		eIndex++;
//	}
}


} // namespace Ctl
