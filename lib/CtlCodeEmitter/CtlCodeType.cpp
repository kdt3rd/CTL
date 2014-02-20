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
	return node.is_subclass<AssignmentNode>() ||
		node.is_subclass<VariableNode>() ||
		node.is_subclass<ReturnNode>();
}


static AddrPtr
newStaticVariableGeneric(Module *module, size_t objectSize)
{
    return new CodeDataAddr( 0 );
}

template <typename Native, typename O1, typename O2, typename O3, typename O4>
void
generateCast( const Native *me, const ExprNodePtr &expr, LContext &ctxt, void (LanguageGenerator::* StartConversionFunc)( void ) )
{
	if ( expr->type.is_subclass<Native>() )
	{
		expr->generateCode( ctxt );
		return;
	}

	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	LanguageGenerator &gen = lctxt.generator();

	(gen.*StartConversionFunc)();

	if (expr->type.is_subclass<O1>())
		expr->generateCode( ctxt );
	else if (expr->type.is_subclass<O2>())
		expr->generateCode( ctxt );
	else if (expr->type.is_subclass<O3>())
		expr->generateCode( ctxt );
	else if (expr->type.is_subclass<O4>())
		expr->generateCode( ctxt );
	else
	{
		MESSAGE_LE( ctxt, ERR_TYPE, expr->lineNumber,
					"Cannot cast value of type " << expr->type->asString() <<
					" to type " << me->asString() << "." );
	}
	gen.endCoersion();
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
	generateCast<BoolType, IntType, UIntType, HalfType, FloatType>(
		this, expr, ctxt, &LanguageGenerator::startToBool );
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
	generateCast<IntType, BoolType, UIntType, HalfType, FloatType>(
		this, expr, ctxt, &LanguageGenerator::startToInt );
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
	generateCast<UIntType, BoolType, IntType, HalfType, FloatType>(
		this, expr, ctxt, &LanguageGenerator::startToUint );
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
	generateCast<HalfType, BoolType, IntType, UIntType, FloatType>(
		this, expr, ctxt, &LanguageGenerator::startToHalf );
}


void
CodeHalfType::generateCode( const SyntaxNodePtr &node,
							LContext &ctxt) const
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
}


CodeFloatType::CodeFloatType( void )
		: FloatType()
{
}


size_t
CodeFloatType::objectSize( void ) const
{
	return sizeof( number );
}


size_t
CodeFloatType::alignedObjectSize( void ) const
{
	return sizeof( number );
}


size_t
CodeFloatType::objectAlignment( void ) const
{
	return sizeof( number );
}


void
CodeFloatType::generateCastFrom( const ExprNodePtr &expr,
								 LContext &ctxt ) const
{
	generateCast<FloatType, BoolType, IntType, UIntType, HalfType>(
		this, expr, ctxt, &LanguageGenerator::startToFloat );
}


void
CodeFloatType::generateCode( const SyntaxNodePtr &node,
							 LContext &ctxt ) const
{
	if ( isAssignment( node ) )
		return;

	CodeLContext &lctxt = static_cast<CodeLContext &>(ctxt);
	UnaryOpNode *unOp = dynamic_cast<UnaryOpNode *>( node.pointer() );
	if ( unOp )
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

	BinaryOpNode *binOp = dynamic_cast<BinaryOpNode *>( node.pointer() );
	if ( binOp )
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
	if ( expr->type.is_subclass<StringType>() )
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
}


CodeArrayType::CodeArrayType( const DataTypePtr &elementType, int size,
							  CodeLContext *lcontext /* = 0 */ )
		: ArrayType( elementType, size ),
		  _unknownSize(0),
		  _unknownESize(0)
{
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
	expr->generateCode( ctxt );
	return;
}


void
CodeArrayType::generateCode( const SyntaxNodePtr &node,
							 LContext &ctxt ) const
{
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

