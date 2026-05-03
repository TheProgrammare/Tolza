#pragma once


namespace ast
{

// identifiers
struct ID;
struct ID_Qualified;
struct ID_Typed;

struct Path_Regex;

struct Root;

struct Import;

// declarations
// globals
struct Global_Variable;
struct Global_Function;
struct Global_Module;
struct Global_Extern;
struct Global_Export;
struct Global_Reexport;
struct Global_Enum;
struct Global_Flag;
struct Global_Union;
struct Global_Alias_Type;
struct Global_Alias_Module;
struct Global_Generic;

// declarations
// locals
struct Local_CodeBlock;
struct Local_Lambda;
struct Local_Lambda_Capture;
struct Local_Parameter;
struct Local_Gen_Param_Elem;
struct Local_Gen_Params;
struct Local_Pattern_Element;
struct Local_Pattern_Enum;
struct Local_Pattern_Tuple;
struct Local_Pattern_Entity;
struct Local_Pattern_Sys_Comp;
struct Local_Pattern_Comp;
struct Local_Binding;
struct Local_Tuple_Destructuring;
struct Local_Variable;
struct Local_Capability;

// compositional oriented paradigm
struct COP_Component;
struct COP_Role;
struct COP_Entity;
struct COP_Entity_New;
struct COP_Entity_Del;
struct COP_Entity_Cast;
struct COP_Entity_Op;
struct COP_Entity_Access_Op;
struct COP_Entity_Transfert;
struct COP_System;
struct COP_Component_Field;
struct COP_System_Case;

// generics
struct Generic_Is_Type;
struct Generic_Can_Cast;
struct Generic_Have_Op;
struct Generic_Have_Role;
struct Generic_Use_Component;
struct Generic_Compatible_System;

// literals
struct Literal_Boolean;
struct Literal_Integral;
struct Literal_Fixed_Point;
struct Literal_Floating_Point;
struct Literal_Cune;
struct Literal_Rune;
struct Literal_Text_Pure;
struct Literal_Text_Interpolation;
struct Literal_Textual_Format;
struct Literal_Format_Specifier;
struct Literal_Table;
struct Literal_Table_Population;
struct Literal_Map;
struct Literal_Tuple;
struct Literal_Range;
struct Literal_Iterator;
struct Literal_Enum;
struct Literal_Structured_Data;
struct Literal_Entity;

// expressions
struct Expression_If_Ternary;
struct Expression_Member_Access;
struct Expression_Self;
struct Expression_Other;
struct Expression_Call;
struct Expression_Call_Argument;
struct Expression_Call_System;
struct Expression_Call_Pipe;
struct Expression_Table_Access;
struct Expression_Ptr_Val;
struct Expression_Mut_Of;
struct Expression_Ref_Of;
struct Expression_Move_Of;
struct Expression_Copy_Of;
struct Expression_Addr_Of;
struct Expression_Size_Of;
struct Expression_GetBits;
struct Expression_New_Ptr;
struct Expression_Get_Type;

// statements
struct Statement_If;
struct Statement_For;
struct Statement_Loop;
struct Statement_While;
struct Statement_GoTo;
struct Statement_GoTo_Label;
struct Statement_Return;
struct Statement_Break;
struct Statement_Continue;
struct Statement_Match;
struct Statement_Match_Case;

// operations
struct Operation_Cast_As;
struct Operation_Is;
struct Operation_In;
struct Operation_Assignment;
struct Operation_Binary;
struct Operation_Unary;
struct Operation_Interval;

// memory
struct Memory_Del;
struct Memory_Align;
struct Memory_Drop;

} // namespace ast