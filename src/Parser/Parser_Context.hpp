#pragma once

#include <memory>

#include "Visitor/Symbol_Manager.hpp"

#include "Lexer/Token.hpp"
#include "Lexer/TokenViewer.hpp"

#include "AST/AST_Base.hpp"
#include "AST/AST_Generic.hpp"
#include "AST/AST_Literal.hpp"
#include "AST/AST_Memory.hpp"
#include "AST/AST_Operation.hpp"
#include "AST/AST_Reference.hpp"
#include "AST/AST_Statement.hpp"
#include "AST/AST_Type.hpp"
#include "AST/AST_Declaration.hpp"
#include "AST/AST_Declaration_ECS.hpp"
#include "AST/AST_Declaration_Local.hpp"


struct ScriptInfo;
struct Symbol_Manager;

template <typename NodeType>
concept DerivedFromNode = 
	std::is_base_of_v<AST::Node, NodeType>
 	&& !std::is_base_of_v<AST::ADeclaration, NodeType>;

template <typename NodeType>
concept DerivedFromDecl = 
	std::is_base_of_v<AST::ADeclaration, NodeType> ||
	std::is_base_of_v<AST::ALocal, NodeType>;

namespace META {
	struct MetablockManager;
	struct MetaInstruct;
	struct MetaBlock;
}

struct Symbols_Manager;


namespace PAR {
	bool is_gen_args(TokenViewer& tokView);
	struct Parser_Base;
	struct Parser_Expression;
	struct Parser_Type;
	struct Parser_Literal;
	struct Parser_Declaration_Local;
	struct Parser_Reference;
	struct Parser_Operator;
	struct Parser_Memory;
	struct Parser_Declaration;
	struct Parser_Declaration_ECS;
	struct Parser_Statement;

	struct Parser_Context {
		explicit Parser_Context(std::shared_ptr<ScriptInfo> infoFile);
		~Parser_Context();

		std::shared_ptr<ScriptInfo> scr_info;
		Symbols_Manager *m_sym = nullptr;
		META::MetablockManager* m_meta = nullptr;
		TokenViewer tok_v;
		size_t node_count = 0;

		// debug purpose on error
		void												attempt_recovery();

		// match separator, or end instruction or and error
		// return true if end is encounter
		[[nodiscard]] bool									match_field_separator(TokTy separator = TokTy::COMMA, TokTy end = TokTy::CLOSE_BRACE);

		// return true if end is encounter
		[[nodiscard]] bool									match_field_any_separator(TokTy separator = TokTy::COMMA, std::initializer_list<TokTy> end = { TokTy::CLOSE_BRACE });


		// to create node, set some data, store in resolvers
		template <DerivedFromNode NodeType, typename... Args>
		inline std::unique_ptr<NodeType> Create_Node(Token token, Args&&... args)
		{
			static_assert(!std::is_abstract_v<NodeType>,
        		"Create_Node cannot instantiate abstract AST nodes");
				
			auto node = std::make_unique<NodeType>(std::forward<Args>(args)...);
			node->_token = token;
			node->_scope = m_sym->get_current_path();
			node_count++;
			return node;
		}
		template <DerivedFromDecl NodeType, typename... Args>
		inline std::shared_ptr<NodeType> Create_Decl(Token token, Args&&... args)
		{
			auto node = std::make_shared<NodeType>(std::forward<Args>(args)...);
			node->_token = token;
			node->_scope = m_sym->get_current_path();
			node_count++;
			return node;
		};

		// MetaBlockManager shortcut for ASTNode  
		[[nodiscard]] bool 											metablock_contains(const AST::Node &n, const std::string &s) const;
		[[nodiscard]] bool 											metablock_contains(const AST::Node &n, TokTy t) const;
		[[nodiscard]] std::string 									get_export_name(const AST::Node &n) const;
		[[nodiscard]] const META::MetaInstruct*						get_instruct(const AST::Node &n, const std::initializer_list<std::string>& pattern) const;
		[[nodiscard]] const META::MetaBlock*						get_metablock(const AST::Node &n, const std::initializer_list<std::string> &pattern);

		// sub parsers accessible to all
		Parser_Expression 			*p_expr;
		Parser_Type 				*p_type;
		Parser_Literal 				*p_lit;
		Parser_Declaration_Local	*p_loc;
		Parser_Reference 			*p_ref;
		Parser_Operator 			*p_op;
		Parser_Memory 				*p_mem;
		Parser_Declaration 			*p_decl;
		Parser_Declaration_ECS 		*p_ecs;
		Parser_Statement 			*p_state;
		Parser_Base 				*p_base;
	};
}