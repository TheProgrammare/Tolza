
#pragma once

#include "VisitorDefault.hpp"


struct VisitorCodegen : public VisitorDefault {
	// keep parent constructor
	using VisitorDefault::VisitorDefault;
	// all commented visit are not concerned by the codegen
	// but the default visitor will visit and can execute inferior node in CodegenVisitor
	/*
	// === Operation ===
	void visit(AST::Op_Index& n) override;
	void visit(AST::Op_Assignment& n) override;
	void visit(AST::Op_Binary& n) override;
	void visit(AST::Op_Unary& n) override;
	void visit(AST::Cond_Interval& n) override;
	void visit(AST::Ternary_If& n) override;
	void visit(AST::Cast_As& n) override;

	// === Instance ===
	void visit(AST::Inst_Entity& n) override;
	void visit(AST::Ref_Call& n) override;
	void visit(AST::Ref_PipeCall& n) override;
	void visit(AST::Inst_Return& n) override;

	// === Memory manipulation ===
	void visit(AST::Mem_MoveOf& n) override;
	void visit(AST::Mem_Del& n) override;
	void visit(AST::Mem_New& n) override;
	void visit(AST::Mem_Dist& n) override;
	void visit(AST::Mem_Ref& n) override;
	void visit(AST::Mem_Deref& n) override;

	// === Path & Identifier handling ===
	void visit(AST::Path& n) override;
	void visit(AST::Identifier& n) override;

	// === Literals ===			
	void visit(AST::Lit_Bool& n) override;
	void visit(AST::Lit_ASCII& n) override;
	void visit(AST::Lit_Decimal& n) override;
	void visit(AST::Lit_Float& n) override;
	void visit(AST::Lit_FStr& n) override;
	void visit(AST::Lit_Int& n) override;
	void visit(AST::Lit_str& n) override;
	void visit(AST::Lit_Tuple& n) override;
	void visit(AST::Lit_Array& n) override;
	void visit(AST::Lit_Map& n) override;
	void visit(AST::Lit_Range& n) override;

	// === Control Flow ===
	void visit(AST::State_If& n) override;
	void visit(AST::State_While& n) override;
	void visit(AST::State_For& n) override;
	void visit(AST::State_Match& n) override;
	void visit(AST::State_MatchCase& n) override;
	void visit(AST::State_GoTo& n) override;
	void visit(AST::State_GoToLabel& n) override;

	// === Function & Lambda Definitions ===
	void visit(AST::Def_Fn& n) override;
	void visit(AST::Def_Param& n) override;
	void visit(AST::Call_Arg& n) override;
	void visit(AST::Def_Lam& n) override;
	void visit(AST::Def_Capture& n) override;
	void visit(AST::Def_CaptureElem& n) override;

	// === Entity Definitions ===
	void visit(AST::Decl_Entity& n) override;
	void visit(AST::Entity_Op& n) override;
	void visit(AST::Entity_OpIndex& n) override;
	void visit(AST::Entity_Cast& n) override;
	void visit(AST::Decl_Self& n) override;
	void visit(AST::Decl_Other& n) override;

	// === system Definitions ===
	void visit(AST::Def_System& n) override;
	void visit(AST::Sys_Where& n) override;
	void visit(AST::Ref_Syscall& n) override;

	// === Component Definitions ===
	void visit(AST::Decl_Comp& n) override;

	// === Types ===
	void visit(AST::Def_TypeAlias& n) override;

	// === Type definition ===
	void visit(AST::Ty_Primitive& n) override;
	void visit(AST::Ty_Ptr& n) override;
	void visit(AST::Ty_Table& n) override;
	void visit(AST::Ty_Identifier& n) override;
	void visit(AST::Ty_Tuple& n) override;
	void visit(AST::Ty_Fn& n) override;
	void visit(AST::Decl_Enum& n) override;
	void visit(AST::Enum_Elem& n) override;
	void visit(AST::Decl_Flag& n) override;

	// === Variable ===
	void visit(AST::Def_GlobalVar& n) override;
	void visit(AST::Def_LocalVar& n) override;
	void visit(AST::Def_UnpackVar& n) override;
	void visit(AST::Def_CondVar& n) override;

	// === Namespace ===
	void visit(AST::Def_Namespace& n) override;

	// === Advanced operation ===
	void visit(AST::Inst_In& n) override;
	void visit(AST::Inst_Is& n) override;

	// === Generic Usage (Use of Generics, Traits, etc.) ===
	void visit(AST::Generic& n) override;
	void visit(AST::Have_Op& n) override;
	void visit(AST::Can_Cast& n) override;
	void visit(AST::GenRole& n) override;
	void visit(AST::GenSys& n) override;
	void visit(AST::GenComp& n) override;
	void visit(AST::GenIs& n) override;

	// === Metacode ===
	void visit(AST::Def_MetacodeBlock& n) override;
	void visit(AST::Inst_Metacode& n) override;

	// === Raw Parsing ===
	void visit(AST::Raw_Tok& n) override;
	void visit(AST::Node& n) override;
	void visit(AST::Root& n) override;
	*/
};
