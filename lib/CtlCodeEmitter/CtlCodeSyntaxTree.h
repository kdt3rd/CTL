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


#ifndef INCLUDED_CTL_CODE_SYNTAX_TREE_H
#define INCLUDED_CTL_CODE_SYNTAX_TREE_H

//-----------------------------------------------------------------------------
//
//	The syntax tree for the CODE implementation of the
//	color transformation language.
//
//-----------------------------------------------------------------------------

#include <CtlSyntaxTree.h>

namespace Ctl
{

class CodeLContext;

class DataType;
typedef RcPtr<DataType> DataTypePtr;

class CodeArrayType;
typedef RcPtr<CodeArrayType> CodeArrayTypePtr;

class CodeValueNode;
typedef RcPtr<CodeValueNode> CodeValueNodePtr;

struct CodeModuleNode : public ModuleNode
{
    CodeModuleNode (int lineNumber,
		    const StatementNodePtr &constants,
		    const FunctionNodePtr &functions);

    virtual void	generateCode (LContext &lcontext);
};


struct CodeFunctionNode : public FunctionNode
{
    CodeFunctionNode( int lineNumber,
					  const std::string &name,
					  const SymbolInfoPtr &info,
					  const StatementNodePtr &body,
					  const std::vector<DataTypePtr> &locals );

    virtual void generateCode( LContext &lcontext );
    std::vector<DataTypePtr> _locals;
};


struct CodeVariableNode : public VariableNode
{
    CodeVariableNode( int lineNumber,
					  const std::string &name,
					  const SymbolInfoPtr &info,
					  const ExprNodePtr &initialValue,
					  bool assignInitialValue );

    virtual void generateCode( LContext &lcontext );
};


struct CodeAssignmentNode : public AssignmentNode
{
    CodeAssignmentNode( int lineNumber,
						const ExprNodePtr &lhs,
						const ExprNodePtr &rhs );

    virtual void generateCode( LContext &lcontext );
};


struct CodeExprStatementNode : public ExprStatementNode
{
    CodeExprStatementNode( int lineNumber, const ExprNodePtr &expr );

    virtual void generateCode( LContext &lcontext );
};


struct CodeIfNode : public IfNode
{
    CodeIfNode( int lineNumber,
				const ExprNodePtr &condition,
				const StatementNodePtr &truePath,
				const StatementNodePtr &falsePath);

    virtual void generateCode( LContext &lcontext );
};


struct CodeReturnNode : public ReturnNode
{
    CodeReturnNode( int lineNumber,
					const SymbolInfoPtr &info,
					const ExprNodePtr &returnedValue );

    virtual void generateCode( LContext &lcontext );
};


struct CodeWhileNode : public WhileNode
{
    CodeWhileNode( int lineNumber,
				   const ExprNodePtr &condition,
				   const StatementNodePtr &loopBody );

    virtual void generateCode( LContext &lcontext );
};


struct CodeBinaryOpNode : public BinaryOpNode
{
    CodeBinaryOpNode( int lineNumber,
					  Token op,
					  const ExprNodePtr &leftOperand,
					  const ExprNodePtr &rightOperand );

    virtual void generateCode( LContext &lcontext );
};


struct CodeUnaryOpNode : public UnaryOpNode
{
    CodeUnaryOpNode( int lineNumber,
					 Token op,
					 const ExprNodePtr &operand );

    virtual void generateCode( LContext &lcontext );
};


struct CodeArrayIndexNode : public ArrayIndexNode
{
    CodeArrayIndexNode( int lineNumber,
						const ExprNodePtr &array,
						const ExprNodePtr &index );

    virtual void generateCode( LContext &lcontext );
};


struct CodeMemberNode : public MemberNode
{
    CodeMemberNode( int lineNumber,
					const ExprNodePtr &obj,
					const std::string &member );

    virtual void generateCode( LContext &lcontext );
};

struct CodeSizeNode : public SizeNode
{
    CodeSizeNode( int lineNumber,
				  const ExprNodePtr &obj);

    virtual void generateCode( LContext &lcontext );
};

struct CodeNameNode : public NameNode
{
    CodeNameNode( int lineNumber,
				  const std::string &name,
				  const SymbolInfoPtr &info );

    virtual void generateCode( LContext &lcontext );
};


struct CodeBoolLiteralNode : public BoolLiteralNode
{
    CodeBoolLiteralNode( int lineNumber,
						 const LContext &lcontext,
						 bool value,
						 const std::string &raw );

    virtual void generateCode (LContext &lcontext);
    virtual char *valuePtr( void );
};


struct CodeIntLiteralNode : public IntLiteralNode
{
    CodeIntLiteralNode( int lineNumber,
						const LContext &lcontext,
						int value,
						const std::string &raw );

    virtual void generateCode( LContext &lcontext );
    virtual char *valuePtr( void );
};


struct CodeUIntLiteralNode : public UIntLiteralNode
{
    CodeUIntLiteralNode( int lineNumber,
						 const LContext &lcontext,
						 unsigned int value,
						 const std::string &raw );

    virtual void generateCode( LContext &lcontext );
    virtual char *valuePtr();
};


struct CodeHalfLiteralNode : public HalfLiteralNode
{
    CodeHalfLiteralNode( int lineNumber,
						 const LContext &lcontext,
						 half value,
						 const std::string &raw );

    virtual void generateCode( LContext &lcontext );
    virtual char *valuePtr( void );
};


struct CodeFloatLiteralNode : public FloatLiteralNode
{
    CodeFloatLiteralNode( int lineNumber,
						  const LContext &lcontext,
						  number value,
						  const std::string &raw );

    virtual void generateCode( LContext &lcontext );
    virtual char *valuePtr( void );
};


struct CodeStringLiteralNode : public StringLiteralNode
{
    CodeStringLiteralNode( int lineNumber,
						   const LContext &lcontext,
						   const std::string &value );

    virtual void generateCode( LContext &lcontext );
    virtual char *valuePtr( void );
};


struct CodeCallNode : public CallNode
{
    CodeCallNode( int lineNumber,
				  const NameNodePtr &function,
				  const ExprNodeVector &arguments );

    bool returnsType( const TypePtr &t ) const;
    virtual void generateCode( LContext &lcontext );
};


struct CodeValueNode : public ValueNode
{
    CodeValueNode( int lineNumber, const ExprNodeVector &elements );

    virtual void generateCode( LContext &lcontext );

    virtual void castAndCopyRec( LContext &lcontext,
								 const DataTypePtr &dataType,
								 int &eIndex,
								 char *dest,
								 const SizeVector &sizes,
								 const SizeVector &offsets );
};


} // namespace Ctl

#endif
