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


#ifndef INCLUDED_CTL_CODE_TYPE_H
#define INCLUDED_CTL_CODE_TYPE_H

//-----------------------------------------------------------------------------
//
//	Types in the CODE implementation of the color transformation language
//
//-----------------------------------------------------------------------------

#include <CtlType.h>
#include <CtlRcPtr.h>


namespace Ctl {

class CodeDataAddr;
typedef RcPtr<CodeDataAddr> CodeDataAddrPtr;

class CodeArrayType;
typedef RcPtr<CodeArrayType> CodeArrayTypePtr;

class CodeFunctionType;
typedef RcPtr<CodeFunctionType> CodeFunctionTypePtr;

class CodeLContext;

class CodeVoidType : public VoidType
{
public:
    CodeVoidType( void );

    virtual size_t objectSize( void ) const;
    virtual size_t alignedObjectSize( void ) const;
    virtual size_t objectAlignment( void ) const;

    virtual void generateCastFrom( const ExprNodePtr &expr,
								   LContext &lcontext ) const;

    virtual void generateCode( const SyntaxNodePtr &node,
							   LContext &lcontext ) const;
};


class CodeBoolType : public BoolType
{
public:
    CodeBoolType( void );

    virtual size_t objectSize( void ) const;
    virtual size_t alignedObjectSize( void ) const;
    virtual size_t objectAlignment( void ) const;

    virtual void generateCastFrom( const ExprNodePtr &expr,
								   LContext &lcontext ) const;

    virtual void generateCode( const SyntaxNodePtr &node,
							   LContext &lcontext ) const;

    virtual AddrPtr newStaticVariable( Module *module ) const;

    virtual void newAutomaticVariable( StatementNodePtr node, 
									   LContext &lcontext ) const;
};


class CodeIntType : public IntType
{
public:
    CodeIntType( void );

    virtual size_t objectSize( void ) const;
    virtual size_t alignedObjectSize( void ) const;
    virtual size_t objectAlignment( void ) const;

    virtual void generateCastFrom( const ExprNodePtr &expr,
								   LContext &lcontext ) const;

    virtual void generateCode( const SyntaxNodePtr &node,
							   LContext &lcontext ) const;

    virtual AddrPtr newStaticVariable( Module *module ) const;

    virtual void newAutomaticVariable( StatementNodePtr node, 
									   LContext &lcontext ) const;
};


class CodeUIntType : public UIntType
{
public:
    CodeUIntType( void );

    virtual size_t objectSize( void ) const;
    virtual size_t alignedObjectSize( void ) const;
    virtual size_t objectAlignment( void ) const;

    virtual void generateCastFrom( const ExprNodePtr &expr,
								   LContext &lcontext ) const;

    virtual void generateCode( const SyntaxNodePtr &node,
							   LContext &lcontext ) const;

    virtual AddrPtr	newStaticVariable( Module *module ) const;

    virtual void newAutomaticVariable( StatementNodePtr node, 
									   LContext &lcontext ) const;
};


class CodeHalfType : public HalfType
{
public:
    CodeHalfType( void );

    virtual size_t objectSize( void ) const;
    virtual size_t alignedObjectSize( void ) const;
    virtual size_t objectAlignment( void ) const;

    virtual void generateCastFrom( const ExprNodePtr &expr,
								   LContext &lcontext ) const;

    virtual void generateCode( const SyntaxNodePtr &node,
							   LContext &lcontext ) const;

    virtual AddrPtr newStaticVariable( Module *module ) const;

    virtual void newAutomaticVariable( StatementNodePtr node, 
									   LContext &lcontext ) const;
};


class CodeFloatType : public FloatType
{
public:
    CodeFloatType( void );

    virtual size_t objectSize( void ) const;
    virtual size_t alignedObjectSize( void ) const;
    virtual size_t objectAlignment( void ) const;

    virtual void generateCastFrom( const ExprNodePtr &expr,
								   LContext &lcontext ) const;

    virtual void generateCode( const SyntaxNodePtr &node,
							   LContext &lcontext ) const;

    virtual AddrPtr newStaticVariable( Module *module ) const;

    virtual void newAutomaticVariable( StatementNodePtr node, 
									   LContext &lcontext ) const;
};


class CodeStringType : public StringType
{
public:
    CodeStringType( void );

    virtual size_t objectSize( void ) const;
    virtual size_t alignedObjectSize( void ) const;
    virtual size_t objectAlignment( void ) const;

    virtual void generateCastFrom( const ExprNodePtr &expr,
								   LContext &lcontext ) const;

    virtual void generateCode( const SyntaxNodePtr &node,
							   LContext &lcontext ) const;

    virtual AddrPtr newStaticVariable( Module *module ) const;

    virtual void newAutomaticVariable( StatementNodePtr node, 
									   LContext &lcontext ) const;
};


class CodeArrayType : public ArrayType
{
public:
    CodeArrayType( const DataTypePtr &elementType, int size,
				   CodeLContext *lcontext = 0 );

    virtual size_t objectSize( void ) const;
    virtual size_t alignedObjectSize( void ) const;
    virtual size_t objectAlignment( void ) const;
    virtual CodeDataAddrPtr unknownElementSize( void ) const;
    virtual CodeDataAddrPtr unknownSize( void ) const;

    virtual void generateCastFrom( const ExprNodePtr &expr,
								   LContext &lcontext ) const;

    virtual void generateCode( const SyntaxNodePtr &node,
							   LContext &lcontext ) const;

    virtual AddrPtr newStaticVariable( Module *module ) const;

    virtual void newAutomaticVariable( StatementNodePtr node, 
									   LContext &lcontext ) const;

private:
    //  Addresses storing the location of unkown size and element size
    //  used in functions with unspecified array sizes
    CodeDataAddrPtr     _unknownSize;
    CodeDataAddrPtr     _unknownESize;
};


class CodeStructType : public StructType
{
public:
    CodeStructType( const std::string &name,
					const MemberVector &members );

    virtual size_t objectSize( void ) const;
    virtual size_t alignedObjectSize( void ) const;
    virtual size_t objectAlignment( void ) const;

    virtual void generateCastFrom( const ExprNodePtr &expr,
								   LContext &lcontext ) const;

    virtual void generateCode( const SyntaxNodePtr &node,
							   LContext &lcontext ) const;

    virtual AddrPtr newStaticVariable( Module *module ) const;

    virtual void newAutomaticVariable( StatementNodePtr node, 
									   LContext &lcontext ) const;
private:
    size_t		_objectSize;
    size_t		_alignedObjectSize;
    size_t		_objectAlignment;
};


class CodeFunctionType : public FunctionType
{
public:
    CodeFunctionType( const DataTypePtr &returnType,
					  bool returnVarying,
					  const ParamVector &parameters );

    virtual void generateCastFrom( const ExprNodePtr &expr,
								   LContext &lcontext ) const;

    virtual void generateCode( const SyntaxNodePtr &node,
							   LContext &lcontext ) const;
};


} // namespace Ctl

#endif
