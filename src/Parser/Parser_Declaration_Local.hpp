#pragma	once

#include <memory>
#include <vector>
#include <functional>

#include "AST/AST_Forward.hpp"
#include "AST/AST_CodeBlock_Instruction.hpp"
#include "AST/AST_Evaluator.hpp"

namespace PAR {
	struct Parser_Context;

	struct Parser_Declaration_Local {
		Parser_Declaration_Local(Parser_Context& ctx) : ctx(ctx) {}

		[[nodiscard]] std::shared_ptr<AST::ALocal>										parse_local(bool silent_error = false);

        [[nodiscrad]] AST::Declaration::Local::Pattern_Element                          pattern_mapping(ECapability capa);

        [[nodiscard]] AST::Evaluator										            parse_evaluator(std::shared_ptr<AST::AReference> comparison_ref);
        [[nodiscard]] std::unique_ptr<AST::Declaration::Local::Pattern>                 parse_pattern(std::shared_ptr<AST::AReference> comparison_ref);

        [[nodiscard]] std::shared_ptr<AST::Declaration::Local::Variable>				variable();
		[[nodiscard]] std::shared_ptr<AST::Declaration::Local::Variable_Unpack>		    variable_unpack();
		[[nodiscard]] std::shared_ptr<AST::Declaration::Local::Lambda>					lambda();
        [[nodiscard]] std::shared_ptr<AST::Declaration::Local::Capability>              capability();
        [[nodiscard]] std::unique_ptr<AST::Declaration::Local::CodeBlock> 				code_block(bool is_silent_error, std::function<AST::CodeBlock_instruction()> in_function);
        [[nodiscard]] std::unique_ptr<AST::Declaration::Local::CodeBlock> 				code_block_instruction();
        [[nodiscard]] std::unique_ptr<AST::Declaration::Local::Lambda_Capture>          lambda_capture();
        [[nodiscard]] std::vector<std::shared_ptr<AST::Declaration::Local::Parameter>>	parameters();
        [[nodiscard]] std::shared_ptr<AST::Statement::GoTo_Label>                       goto_label_statement();


        [[nodiscard]] std::unique_ptr<AST::Declaration::Local::Pattern_Component> 		component_pattern(ECapability capa, AST::ID &comp_id, std::shared_ptr<AST::AReference> comparison_ref);
        [[nodiscard]] std::unique_ptr<AST::Declaration::Local::Pattern_Entity> 			entity_pattern(ECapability capa, AST::ID &entity_id, std::shared_ptr<AST::AReference> comparison_ref);
        [[nodiscard]] std::unique_ptr<AST::Declaration::Local::Pattern_Tuple> 			tuple_pattern(ECapability capa, std::shared_ptr<AST::AReference> comparison_ref);
        [[nodiscard]] std::unique_ptr<AST::Declaration::Local::Pattern_Enum>            enum_pattern(ECapability capa, AST::ID &enum_id, std::shared_ptr<AST::AReference> comparison_ref);


        PAR::Parser_Context& ctx;
	};
}
