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
//	Types in the CODE implementation of the color transformation language
//
//-----------------------------------------------------------------------------

#include "CtlCodeType.h"
#include <CtlSyntaxTree.h>
#include <CtlSymbolTable.h>
#include "CtlCodeLContext.h"
#include "CtlCodeModule.h"
#include "CtlCodeAddr.h"
#include "CtlCodeLanguageGenerator.h"
#include <CtlAlign.h>
#include <CtlMessage.h>
#include <cassert>
#include <CtlErrors.h>

using namespace std;

namespace Ctl {
namespace {

bool
isAssignment( const SyntaxNodePtr &node)
{
	return node.cast<AssignmentNode>() ||
		node.cast<VariableNode>() ||
		node.cast<ReturnNode>();
}

} // namespace


CodeVoidType::CodeVoidType( void )
		: VoidType()
{
	// empty
}


size_t
CodeVoidType::objectSize( void ) const
{
	return 0;
}


size_t
CodeVoidType::alignedObjectSize( void ) const
{
	return 0;
}


size_t
CodeVoidType::objectAlignment( void ) const
{
	return 1;
}


void
CodeVoidType::generateCastFrom( const ExprNodePtr &expr,
								LContext &ctxt ) const
{
	expr->generateCode( ctxt );
}


void
CodeVoidType::generateCode( const SyntaxNodePtr &node,
							LContext &ctxt ) const
{
}


CodeBoolType::CodeBoolType( void )
		: BoolType()
{
	// empty
}


size_t
CodeBoolType::objectSize( void ) const
{
	return sizeof( bool );
}


size_t
CodeBoolType::alignedObjectSize( void ) const
{
	return sizeof( bool );
}


size_t
CodeBoolType::objectAlignment( void ) const
{
	return sizeof( bool );
}


void
CodeBoolType::generateCastFrom( const ExprNodePtr &expr,
								LContext &ctxt ) const
{
	if ( expr->type.cast<BoolType>() )
	{
		expr->generateCode( ctxt );
		return;
	}

	if ( expr->type.cast<IntType>() )
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<UIntType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<HalfType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<FloatType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	MESSAGE_LE( ctxt, ERR_TYPE, expr->lineNumber,
				"Cannot cast value of type " << expr->type->asString() <<
				" to type " << asString() << "." );
}


void
CodeBoolType::generateCode( const SyntaxNodePtr &node,
							LContext &ctxt ) const
{
	if ( isAssignment( node ) )
		return;

	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	if (UnaryOpNodePtr unOp = node.cast<UnaryOpNode>())
	{
		switch (unOp->op)
		{
			case TK_BITNOT:
			case TK_NOT:
				lctxt.generator().emitToken( unOp->op );
				break;

			default:
				MESSAGE_LE( ctxt, ERR_OP_TYPE, node->lineNumber,
							"Cannot apply " << tokenAsString (unOp->op) << " "
							"operator to value of type " << 
							unOp->operand->type->asString() << "." );
		}

		return;
	}

	if (BinaryOpNodePtr binOp = node.cast<BinaryOpNode>())
	{
		switch (binOp->op)
		{
			case TK_AND:
			case TK_OR:
			case TK_BITAND:
			case TK_BITOR:
			case TK_BITXOR:
			case TK_EQUAL:
			case TK_GREATER:
			case TK_GREATEREQUAL:
			case TK_LESS:
			case TK_LESSEQUAL:
			case TK_NOTEQUAL:
				lctxt.generator().emitToken( binOp->op );
				break;

			default:
				MESSAGE_LE( ctxt, ERR_OP_TYPE, node->lineNumber,
							"Invalid operand types "
							"for " << tokenAsString (binOp->op) << " operator "
							"(" << binOp->leftOperand->type->asString() << " " <<
							tokenAsString (binOp->op) << " " <<
							binOp->rightOperand->type->asString() << ")." );
		}

		return;
	}

	if (node.cast<CallNode>())
	{
		//
		// Push a placeholder for the return value for a call to
		// a function that returns an int.
		//
//		slcontext.addInst( new CodePushPlaceholderInst(alignedObjectSize(),
//													   node->lineNumber) );
		return;
	}
}


static AddrPtr
newStaticVariableGeneric(Module *module, size_t objectSize)
{
//    SimdModule *smodule = static_cast <SimdModule *> (module);
//    SimdReg* reg = new SimdReg (false, objectSize);
//    
//    smodule->addStaticData (reg);
    return new CodeDataAddr( 0 );
}

AddrPtr
CodeBoolType::newStaticVariable( Module *module ) const
{
	return newStaticVariableGeneric( module, objectSize() );
}

void
CodeBoolType::newAutomaticVariable( StatementNodePtr node, 
									LContext &ctxt ) const 
{
//	CodeLContext &slcontext = static_cast <CodeLContext &> (lcontext);
//	slcontext.addInst (new CodePushPlaceholderInst
//					   (objectSize(), node->lineNumber));
}


CodeIntType::CodeIntType( void )
		: IntType()
{
	// empty
}


size_t
CodeIntType::objectSize( void ) const
{
	return sizeof( int );
}


size_t
CodeIntType::alignedObjectSize( void ) const
{
	return sizeof( int );
}


size_t
CodeIntType::objectAlignment( void ) const
{
	return sizeof( int );
}


void
CodeIntType::generateCastFrom( const ExprNodePtr &expr,
							   LContext &ctxt ) const
{
	if (expr->type.cast<BoolType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<IntType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<UIntType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<HalfType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<FloatType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	MESSAGE_LE( ctxt, ERR_TYPE, expr->lineNumber,
				"Cannot cast value of type " << expr->type->asString() <<
				" to type " << asString() << "." );
}


void
CodeIntType::generateCode( const SyntaxNodePtr &node,
						   LContext &ctxt ) const
{
	if ( isAssignment( node ) )
		return;

	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	if (UnaryOpNodePtr unOp = node.cast<UnaryOpNode>())
	{
		switch (unOp->op)
		{
			case TK_BITNOT:
			case TK_MINUS:
				lctxt.generator().emitToken( unOp->op );
				break;

			default:
				MESSAGE_LE( ctxt, ERR_OP_TYPE, node->lineNumber,
							"Cannot apply " << tokenAsString (unOp->op) << " "
							"operator to value of type " << 
							unOp->operand->type->asString() << "." );
		}

		return;
	}

	if (BinaryOpNodePtr binOp = node.cast<BinaryOpNode>())
	{
		switch (binOp->op)
		{
			case TK_BITAND:
			case TK_BITOR:
			case TK_BITXOR:
			case TK_DIV:
			case TK_EQUAL:
			case TK_GREATER:
			case TK_GREATEREQUAL:
			case TK_LEFTSHIFT:
			case TK_LESS:
			case TK_LESSEQUAL:
			case TK_MINUS:
			case TK_MOD:
			case TK_NOTEQUAL:
			case TK_PLUS:
			case TK_RIGHTSHIFT:
			case TK_TIMES:
				lctxt.generator().emitToken( binOp->op );
				break;

			default:
				MESSAGE_LE( ctxt, ERR_OP_TYPE, node->lineNumber,
							"Invalid operand types "
							"for " << tokenAsString (binOp->op) << " operator "
							"(" << binOp->leftOperand->type->asString() << " " <<
							tokenAsString (binOp->op) << " " <<
							binOp->rightOperand->type->asString() << ")." );
		}

		return;
	}

	if (node.cast<CallNode>())
	{
		//
		// Push a placeholder for the return value for a call to
		// a function that returns an int.
		//
//		slcontext.addInst (new CodePushPlaceholderInst(alignedObjectSize(),
//													   node->lineNumber));
		return;
	}
}



AddrPtr
CodeIntType::newStaticVariable( Module *module ) const
{
	return newStaticVariableGeneric( module, objectSize() );
}

void
CodeIntType::newAutomaticVariable( StatementNodePtr node, 
								   LContext &ctxt ) const
{
//	CodeLContext &slcontext = static_cast <CodeLContext &> (lcontext);
//	slcontext.addInst (new CodePushPlaceholderInst
//					   (objectSize(), node->lineNumber));
}


CodeUIntType::CodeUIntType( void )
		: UIntType()
{
}


size_t
CodeUIntType::objectSize( void ) const
{
	return sizeof( unsigned int );
}


size_t
CodeUIntType::alignedObjectSize( void ) const
{
	return sizeof( unsigned int );
}


size_t
CodeUIntType::objectAlignment( void ) const
{
	return sizeof( unsigned int );
}


void
CodeUIntType::generateCastFrom( const ExprNodePtr &expr,
								LContext &ctxt ) const
{
	if (expr->type.cast<BoolType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<IntType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<UIntType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<HalfType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<FloatType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	MESSAGE_LE( ctxt, ERR_TYPE, expr->lineNumber,
				"Cannot cast value of type " << expr->type->asString() <<
				" to type " << asString() << "." );
}


void
CodeUIntType::generateCode( const SyntaxNodePtr &node,
							LContext &ctxt ) const
{
	if ( isAssignment( node ) )
		return;

	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	if (UnaryOpNodePtr unOp = node.cast<UnaryOpNode>())
	{
		switch (unOp->op)
		{
			case TK_BITNOT:
			case TK_MINUS:
				lctxt.generator().emitToken( unOp->op );
				break;

			default:
				MESSAGE_LE( ctxt, ERR_OP_TYPE, node->lineNumber,
							"Cannot apply " << tokenAsString (unOp->op) << " "
							"operator to value of type " << 
							unOp->operand->type->asString() << "." );
		}

		return;
	}

	if (BinaryOpNodePtr binOp = node.cast<BinaryOpNode>())
	{
		switch (binOp->op)
		{
			case TK_BITAND:
			case TK_BITOR:
			case TK_BITXOR:
			case TK_DIV:
			case TK_EQUAL:
			case TK_GREATER:
			case TK_GREATEREQUAL:
			case TK_LEFTSHIFT:
			case TK_LESS:
			case TK_LESSEQUAL:
			case TK_MINUS:
			case TK_MOD:
			case TK_NOTEQUAL:
			case TK_PLUS:
			case TK_RIGHTSHIFT:
			case TK_TIMES:
				lctxt.generator().emitToken( binOp->op );
				break;

			default:
				MESSAGE_LE( ctxt, ERR_OP_TYPE, node->lineNumber,
							"Invalid operand types "
							"for " << tokenAsString (binOp->op) << " operator "
							"(" << binOp->leftOperand->type->asString() << " " <<
							tokenAsString (binOp->op) << " " <<
							binOp->rightOperand->type->asString() << ")." );
		}

		return;
	}

	if (node.cast<CallNode>())
	{
		//
		// Push a placeholder for the return value for a call to
		// a function that returns an int.
		//
//		slcontext.addInst
//			(new CodePushPlaceholderInst (alignedObjectSize(),
//										  node->lineNumber));
		return;
	}
}


AddrPtr
CodeUIntType::newStaticVariable( Module *module ) const
{
	return newStaticVariableGeneric( module, objectSize() );
}


void
CodeUIntType::newAutomaticVariable( StatementNodePtr node, 
									LContext &ctxt ) const
{
//	CodeLContext &slcontext = static_cast <CodeLContext &> (lcontext);
//	slcontext.addInst (new CodePushPlaceholderInst
//					   (objectSize(), node->lineNumber));
}


CodeHalfType::CodeHalfType( void )
		: HalfType()
{
}


size_t
CodeHalfType::objectSize( void ) const
{
	return sizeof( half );
}


size_t
CodeHalfType::alignedObjectSize( void ) const
{
	return sizeof( half );
}


size_t
CodeHalfType::objectAlignment( void ) const
{
	return sizeof( half );
}


void
CodeHalfType::generateCastFrom( const ExprNodePtr &expr,
								LContext &ctxt ) const
{
	if (expr->type.cast<BoolType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<IntType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<UIntType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<HalfType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<FloatType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	MESSAGE_LE( ctxt, ERR_TYPE, expr->lineNumber,
				"Cannot cast value of type " << expr->type->asString() <<
				" to type " << asString() << "." );
}


void
CodeHalfType::generateCode( const SyntaxNodePtr &node,
							LContext &ctxt) const
{
	if ( isAssignment( node ) )
	{
//		slcontext.addInst
//			(new CodeAssignInst (alignedObjectSize(), node->lineNumber));
		return;
	}

	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	if (UnaryOpNodePtr unOp = node.cast<UnaryOpNode>())
	{
		switch (unOp->op)
		{
			case TK_MINUS:
				lctxt.generator().emitToken( unOp->op );
				break;

			default:
				MESSAGE_LE( ctxt, ERR_OP_TYPE, node->lineNumber,
							"Cannot apply " << tokenAsString (unOp->op) << " "
							"operator to value of type " << 
							unOp->operand->type->asString() << "." );
		}

		return;
	}

	if (BinaryOpNodePtr binOp = node.cast<BinaryOpNode>())
	{
		switch (binOp->op)
		{
			case TK_DIV:
			case TK_EQUAL:
			case TK_GREATER:
			case TK_GREATEREQUAL:
			case TK_LESS:
			case TK_LESSEQUAL:
			case TK_MINUS:
			case TK_NOTEQUAL:
			case TK_PLUS:
			case TK_TIMES:
				lctxt.generator().emitToken( binOp->op );
				break;

			default:
				MESSAGE_LE( ctxt, ERR_OP_TYPE, node->lineNumber,
							"Invalid operand types "
							"for " << tokenAsString (binOp->op) << " operator "
							"(" << binOp->leftOperand->type->asString() << " " <<
							tokenAsString (binOp->op) << " " <<
							binOp->rightOperand->type->asString() << ")." );
		}

		return;
	}

	if ( node.cast<CallNode>() )
	{
		//
		// Push a placeholder for the return value for a call to
		// a function that returns an int.
		//
//		slcontext.addInst
//			(new CodePushPlaceholderInst (alignedObjectSize(),
//										  node->lineNumber));
		return;
	}
}


AddrPtr
CodeHalfType::newStaticVariable( Module *module ) const
{
	return newStaticVariableGeneric( module, objectSize() );
}


void
CodeHalfType::newAutomaticVariable( StatementNodePtr node, 
									LContext &ctxt ) const
{
//	CodeLContext &slcontext = static_cast <CodeLContext &> (lcontext);
//	slcontext.addInst (new CodePushPlaceholderInst
//					   (objectSize(), node->lineNumber));
}


CodeFloatType::CodeFloatType( void )
		: FloatType()
{
}


size_t
CodeFloatType::objectSize( void ) const
{
	return sizeof( float );
}


size_t
CodeFloatType::alignedObjectSize( void ) const
{
	return sizeof( float );
}


size_t
CodeFloatType::objectAlignment( void ) const
{
	return sizeof( float );
}


void
CodeFloatType::generateCastFrom( const ExprNodePtr &expr,
								 LContext &ctxt ) const
{
	if (expr->type.cast<BoolType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<IntType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<UIntType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<HalfType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	if (expr->type.cast<FloatType>())
	{
		expr->generateCode( ctxt );
		return;
	}

	MESSAGE_LE( ctxt, ERR_TYPE, expr->lineNumber,
				"Cannot cast value of type " << expr->type->asString() <<
				" to type " << asString() << "." );
}


void
CodeFloatType::generateCode( const SyntaxNodePtr &node,
							 LContext &ctxt ) const
{
	if ( isAssignment( node ) )
		return;

	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	if (UnaryOpNodePtr unOp = node.cast<UnaryOpNode>())
	{
		switch (unOp->op)
		{
			case TK_MINUS:
				lctxt.generator().emitToken( unOp->op );
				break;

			default:
				MESSAGE_LE( ctxt, ERR_OP_TYPE, node->lineNumber,
							"Cannot apply " << tokenAsString (unOp->op) << " "
							"operator to value of type " << 
							unOp->operand->type->asString() << "." );
		}

		return;
	}

	if (BinaryOpNodePtr binOp = node.cast<BinaryOpNode>())
	{
		switch (binOp->op)
		{
			case TK_DIV:
			case TK_EQUAL:
			case TK_GREATER:
			case TK_GREATEREQUAL:
			case TK_LESS:
			case TK_LESSEQUAL:
			case TK_MINUS:
			case TK_NOTEQUAL:
			case TK_PLUS:
			case TK_TIMES:
				lctxt.generator().emitToken( binOp->op );
				break;

			default:
				MESSAGE_LE( ctxt, ERR_OP_TYPE, node->lineNumber,
							"Invalid operand types "
							"for " << tokenAsString (binOp->op) << " operator "
							"(" << binOp->leftOperand->type->asString() << " " <<
							tokenAsString (binOp->op) << " " <<
							binOp->rightOperand->type->asString() << ")." );
		}

		return;
	}

	if ( node.cast<CallNode>() )
	{
		//
		// Push a placeholder for the return value for a call to
		// a function that returns an int.
		//
//		slcontext.addInst (new CodePushPlaceholderInst (alignedObjectSize(),
//														node->lineNumber));
		return;
	}
}


AddrPtr
CodeFloatType::newStaticVariable( Module *module ) const
{
	return newStaticVariableGeneric( module, objectSize() );
}


void
CodeFloatType::newAutomaticVariable( StatementNodePtr node, 
									 LContext &ctxt ) const 
{
//	CodeLContext &slcontext = static_cast <CodeLContext &> (lcontext);
//	slcontext.addInst (new CodePushPlaceholderInst
//					   (objectSize(), node->lineNumber));
}


CodeStringType::CodeStringType( void )
		: StringType()
{
}


size_t
CodeStringType::objectSize( void ) const
{
	return sizeof( std::string * );
}


size_t
CodeStringType::alignedObjectSize( void ) const
{
	return sizeof( std::string * );
}


size_t
CodeStringType::objectAlignment( void ) const
{
	return sizeof( std::string * );
}


void
CodeStringType::generateCastFrom( const ExprNodePtr &expr,
								  LContext &ctxt ) const
{
	if ( expr->type.cast<StringType>() )
	{
		expr->generateCode( ctxt );
		return;
	}

	MESSAGE_LE( ctxt, ERR_TYPE, expr->lineNumber,
				"Cannot cast value of type " << expr->type->asString() <<
				" to type " << asString() << "." );
}


void
CodeStringType::generateCode( const SyntaxNodePtr &node,
							  LContext &ctxt ) const
{
	if ( isAssignment( node ) )
		return;

	if (UnaryOpNodePtr unOp = node.cast<UnaryOpNode>())
	{
		switch (unOp->op)
		{
			default:
				MESSAGE_LE( ctxt, ERR_OP_TYPE, node->lineNumber,
							"Cannot apply " << tokenAsString (unOp->op) << " "
							"operator to value of type " << 
							unOp->operand->type->asString() << "." );
		}

		return;
	}

	if (BinaryOpNodePtr binOp = node.cast<BinaryOpNode>())
	{
		switch (binOp->op)
		{
			default:
				MESSAGE_LE( ctxt, ERR_OP_TYPE, node->lineNumber,
							"Invalid operand types "
							"for " << tokenAsString (binOp->op) << " operator "
							"(" << binOp->leftOperand->type->asString() << " " <<
							tokenAsString (binOp->op) << " " <<
							binOp->rightOperand->type->asString() << ")." );
		}

		return;
	}

	if ( node.cast<CallNode>() )
	{
		//
		// Push a placeholder for the return value for a call to
		// a function that returns an int.
		//
//		slcontext.addInst (new CodePushPlaceholderInst (alignedObjectSize(),
//														node->lineNumber));
		return;
	}
}


AddrPtr
CodeStringType::newStaticVariable( Module *module ) const
{
	return newStaticVariableGeneric( module, alignedObjectSize() );
}


void
CodeStringType::newAutomaticVariable( StatementNodePtr node,
									  LContext &ctxt ) const 
{
//	CodeLContext &slcontext = static_cast <CodeLContext &> (lcontext);
//
//	slcontext.addInst (new CodePushPlaceholderInst
//					   (alignedObjectSize(), node->lineNumber));
}


CodeArrayType::CodeArrayType( const DataTypePtr &elementType, int size,
							  CodeLContext *lcontext /* = 0 */ )
		: ArrayType( elementType, size ),
		  _unknownSize(0),
		  _unknownESize(0)
{
//	if( lcontext)
//	{
//		// If size is not specified, this is a function parameter
//		// size will be stored as a parameter
//		if( size == 0 )
//		{
//			IntTypePtr intType = lcontext->newIntType( void );
//			_unknownSize = lcontext->parameterAddr (intType);
//		}
//
//		// if the element size is not known, create a local variable
//		// to store the element size computed later
//		CodeArrayTypePtr at = elementType.cast<CodeArrayType>();
//		if( at && (at->unknownElementSize() || at->unknownSize() ))
//		{
//			IntTypePtr intType = lcontext->newIntType( void );
//			_unknownESize = lcontext->autoVariableAddr (intType);
//		}
//	}
}


size_t
CodeArrayType::objectSize( void ) const
{
	return elementSize() * size();
}


size_t
CodeArrayType::alignedObjectSize( void ) const
{
	return elementSize() * size();
}


size_t
CodeArrayType::objectAlignment( void ) const
{
	return elementType()->objectAlignment();
}

CodeDataAddrPtr
CodeArrayType::unknownElementSize( void ) const
{
	return _unknownESize;
}

CodeDataAddrPtr
CodeArrayType::unknownSize( void ) const
{
	return _unknownSize;
}

void
CodeArrayType::generateCastFrom( const ExprNodePtr &expr,
								 LContext &ctxt ) const
{
	assert(isSameTypeAs(expr->type));
	expr->generateCode( ctxt );
	return;
}


void
CodeArrayType::generateCode( const SyntaxNodePtr &node,
							 LContext &ctxt ) const
{
//	CodeLContext &slcontext = static_cast <CodeLContext &> (lcontext);

	VariableNodePtr var = node.cast<VariableNode>();
	if( var && var->initialValue.cast<ValueNode>())
	{
		SizeVector sizes;
		SizeVector offsets;
		coreSizes(0, sizes, offsets);
//		slcontext.addInst (new CodeInitializeInst(sizes, 
//												  offsets,
//												  node->lineNumber));
		return;
	}
	else if ( isAssignment( node ) )  // return or assignment
	{
//		slcontext.addInst (new CodeAssignArrayInst
//						   (size(), elementSize(), node->lineNumber));
		return;
	}
	else if ( node.cast<ArrayIndexNode>() )
	{
		if(unknownSize() || unknownElementSize())
		{
//			slcontext.addInst (new CodeIndexVSArrayInst(elementSize(),
//														unknownElementSize(), 
//														size(),
//														unknownSize(),
//														node->lineNumber));
		}
		else
		{
//			slcontext.addInst (new CodeIndexArrayInst(elementSize(), 
//													  node->lineNumber,
//													  size()));
		}
		return;
	}
	else if ( node.cast<SizeNode>() )
	{
		assert(size() == 0);
//		slcontext.addInst (new CodePushRefInst (unknownSize(), 
//												node->lineNumber));
	}
	else if (node.cast<CallNode>())
	{
//		slcontext.addInst (new CodePushPlaceholderInst(objectSize(), 
//													   node->lineNumber));
		return;
	}
}


AddrPtr
CodeArrayType::newStaticVariable( Module *module ) const
{
	return newStaticVariableGeneric( module, objectSize() );
}


void
CodeArrayType::newAutomaticVariable (StatementNodePtr node, 
									 LContext &ctxt ) const
{
//	CodeLContext &slcontext = static_cast <CodeLContext &> (lcontext);
//	slcontext.addInst (new CodePushPlaceholderInst
//					   (objectSize(), node->lineNumber));
}


CodeStructType::CodeStructType( const string &name,
								const MemberVector &members )
		: StructType( name, members ),
		  _objectSize (0),
		  _alignedObjectSize (0),
		  _objectAlignment (1)
{
	for (size_t i = 0; i < this->members().size(); ++i)
	{
		Member &m = member( i );

		m.offset = align( _objectSize, m.type->objectAlignment() );
		_objectSize = m.offset + m.type->objectSize();

		_objectAlignment =
			leastCommonMultiple( _objectAlignment, m.type->objectAlignment() );
	}

	_alignedObjectSize = align( _objectSize, _objectAlignment );
}


size_t
CodeStructType::objectSize( void ) const
{
	return _objectSize;
}


size_t
CodeStructType::alignedObjectSize( void ) const
{
	return _alignedObjectSize;
}


size_t
CodeStructType::objectAlignment( void ) const
{
	return _objectAlignment;
}


void
CodeStructType::generateCastFrom( const ExprNodePtr &expr,
								  LContext &ctxt ) const
{
	expr->generateCode( ctxt );
	return;
}


void
CodeStructType::generateCode( const SyntaxNodePtr &node,
							  LContext &ctxt ) const
{
//	CodeLContext &slcontext = static_cast <CodeLContext &> (lcontext);

	VariableNodePtr var = node.cast<VariableNode>();
	if( var && var->initialValue.cast<ValueNode>())
	{
		SizeVector sizes;
		SizeVector offsets;
		coreSizes(0, sizes, offsets);
//		slcontext.addInst (new CodeInitializeInst(sizes, 
//												  offsets, 
//												  node->lineNumber));
		return;
	}

	if( MemberNodePtr mem = node.cast<MemberNode>() )
	{
//		slcontext.addInst (new CodeAccessMemberInst(mem->offset,
//													(node->lineNumber)));
		return;
	}

	if ( isAssignment( node ) )
	{
//		slcontext.addInst (new CodeAssignInst
//						   (alignedObjectSize(), node->lineNumber));
		return;
	}

	if ( node.cast<CallNode>() )
	{
		// Push a placeholder for the return value for a call to
		// a function that returns a struct
//		slcontext.addInst (new CodePushPlaceholderInst (alignedObjectSize(),
//														node->lineNumber));
		return;
	}
}


AddrPtr
CodeStructType::newStaticVariable( Module *module ) const
{
	return newStaticVariableGeneric( module, alignedObjectSize() );
}


void
CodeStructType::newAutomaticVariable (StatementNodePtr node, 
									  LContext &ctxt ) const
{
//	CodeLContext &slcontext = static_cast <CodeLContext &> (lcontext);
//	slcontext.addInst (new CodePushPlaceholderInst
//					   (alignedObjectSize(), node->lineNumber));
}


CodeFunctionType::CodeFunctionType( const DataTypePtr &returnType,
									bool returnVarying,
									const ParamVector &parameters )
		: FunctionType( returnType, returnVarying, parameters )
{
}


void
CodeFunctionType::generateCastFrom( const ExprNodePtr &expr,
									LContext &ctxt ) const
{
	expr->generateCode( ctxt );
}


void
CodeFunctionType::generateCode( const SyntaxNodePtr &node,
								LContext &ctxt ) const
{
}


} // namespace Ctl

