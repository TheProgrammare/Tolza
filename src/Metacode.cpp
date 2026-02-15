#include "Metacode.hpp"

#include "Compilation.hpp"
#include "ScriptInfo.hpp"

const size_t META::MetaBlock_Expand::k_placeholder_flag = k_metacode_flag - 1;
const size_t META::MetaBlock_Expand::k_expand_if_flag = k_metacode_flag - 2;


bool META::is_equivalent_ReusableBlock_Param(const MetaBlock_Reuse_Param &a, const MetaBlock_Reuse_Param &b) {
    const bool same_pass_mode = a.pass_mode == b.pass_mode;
    const bool same_type = a.type.get() == b.type.get();
    const bool same_variadic = a.is_variadic == b.is_variadic;
    
    return same_pass_mode && same_type && same_variadic;
}

std::vector<Token> META::MetaBlock::generate_tokens(ScriptInfo &scr_info) const
{
    std::vector<Token> result;
    if (tokens_to_generate.empty()) return {};

    // arbitrary optimization
    result.reserve(scr_info.tokens.size() / 4);

    size_t children_generated_count = 0;
    for (auto pos : tokens_to_generate) {
        // if children must be generated before
        if (pos == k_metacode_flag) {
            MetaBlock *child = _childrens[children_generated_count++].get();
            std::vector<Token> child_toks = child->generate_tokens(scr_info);
            
            if (!child_toks.empty()) {
                result.insert(
                    result.end(),
                    child_toks.begin(),
                    child_toks.end()
                );
            }
            
            continue;
        }
        else if (pos >= scr_info.tokens.size()) {
			throw std::runtime_error("Current token source index to generate is out of source tokens bounds !");
		}
        else {
			Token tok = scr_info.tokens[pos]; 
			result.push_back(tok);
		}
    }

    return result;
}

std::vector<Token> META::MetaBlock_If::generate_tokens(ScriptInfo &scr_info) const
{
    if (!eval(COMPILATION_ARGS) && alternative.get()) return alternative->generate_tokens(scr_info);
    
    return MetaBlock::generate_tokens(scr_info);
}

std::vector<Token> META::MetaBlock_Expand::generate_tokens(ScriptInfo &scr_info) const
{
    std::vector<Token> model;
    model.reserve(tokens_to_generate.size());

    size_t children_generated_count = 0;
    size_t expand_if_count = 0;
    for (size_t i = 0; i < tokens_to_generate.size(); i++) {
        const size_t pos = tokens_to_generate[i];

        if (pos == k_metacode_flag) {
            MetaBlock *child = _childrens[children_generated_count++].get();
            std::vector<Token> child_toks = child->generate_tokens(scr_info);
            
            if (!child_toks.empty()) {
                model.insert(
                    model.end(),
                    child_toks.begin(),
                    child_toks.end()
                );
            }
        }
        else if (pos == k_expand_if_flag) {
            // temporary special token for expansion condition evaluation
            Token expand_tok("", TokTy::S_METACODE_EXPAND_IF, Span(k_expand_if_flag, 0, 0, 0));
            model.push_back(expand_tok);
        }
        else if (pos == k_placeholder_flag) {
            // convention: next token is the placeholder identifier !
            // jump + 1 in the loop
            size_t next_pos = tokens_to_generate[++i];
            Token next_tok = scr_info.tokens[next_pos];

            // temporary special token for placeholder
            // value = placeholder name
            // type = special metacode placeholder
            // position = placeholder_flag 
            model.push_back(next_tok);
        }
        else if (pos >= scr_info.tokens.size()) {
			throw std::runtime_error("Current token source index to generate is out of source tokens bounds !");
		}
        else {
            Token tok = scr_info.tokens[pos];
            model.push_back(tok);
        }
    }

    return generate_model_expansion(scr_info, model);
}

std::vector<Token> META::MetaBlock_Expand::generate_model_expansion(ScriptInfo &scr_info, std::vector<Token> &model) const
{
    auto combos = generate_all_combinations();

    std::vector<Token> result;
    result.reserve(model.size() * combos.size());

    // generate code model for each combination (combo)
    for (const auto &combo : combos) {
        size_t expand_if_count = 0;

        for (const auto &tok : model) {
            // expand if encounted
            if (tok.type == TokTy::S_METACODE_EXPAND_IF) {
                Expand_If *exp_if = expand_conditions[expand_if_count++].get();

                if (exp_if->eval(combo)) {
                    std::vector<Token> exp_if_model = exp_if->generate_tokens(scr_info);

                    std::vector<Token> exp_if_model_placeholded;
                    exp_if_model_placeholded.reserve(exp_if_model.size());
                    for (auto &exp_if_tok : exp_if_model) {
                        if (exp_if_tok.type == ETokenType::S_METACODE_PLACEHOLDER) {
                            // find placeholder symbol by identifier
                            if (auto it = combo.find(exp_if_tok.val); it != combo.end()) {
                                auto placeholder_tok = it->second;
                                // replace placeholder by the symbol combination case
                                exp_if_tok.type = placeholder_tok.type;
                                exp_if_tok.val = placeholder_tok.val;
                                exp_if_tok.span.size = placeholder_tok.val.size();
                            }
                            else {
                                throw std::runtime_error("Unexpected placeholder name (" + tok.val + ") the naming prevention seems to have failed");
                            }
                        }
                    }

                    result.insert(
                        result.end(),
                        exp_if_model.begin(),
                        exp_if_model.end()  
                    );
                }
            }
            // placeholder encounted
            else if (tok.type == TokTy::S_METACODE_PLACEHOLDER) {
                // find placeholder symbol by identifier
                if (auto it = combo.find(tok.val); it != combo.end()) {
                    auto placeholder_tok = it->second;
                    // replace placeholder by the symbol combination case
                    result.push_back(placeholder_tok);
                }
                else {
                    throw std::runtime_error("Unexpected placeholder name (" + tok.val + ") the naming prevention seems to have failed");
                }
            }
            // classic token
            else {
                result.push_back(tok);
            }
        }
    }
    return result;
}

void META::MetaBlock_Expand::add_expand_condition(std::unique_ptr<Expand_If> exp_cond)
{
    expand_conditions.push_back(std::move(exp_cond));
    tokens_to_generate.push_back(k_expand_if_flag);
}

void META::MetaBlock_Expand::generate_combinations(
    std::map<std::string, Token> &current, 
    std::map<std::string, std::vector<Token>>::const_iterator it, 
    std::vector<std::map<std::string, Token>> &result) const
{
    if (it == placeholders.end()) {
        // All placeholders are assigned : save the combination
        result.push_back(current);
        return;
    }

    const std::string& key = it->first;
    const std::vector<Token>& tokens = it->second;

    for (const Token& token : tokens) {
        current[key] = token;
        generate_combinations(current, std::next(it), result);
    }
}

std::vector<std::map<std::string, Token>> META::MetaBlock_Expand::generate_all_combinations() const
{
    std::vector<std::map<std::string, Token>> result;
    std::map<std::string, Token> current;
    generate_combinations(current, placeholders.begin(), result);
    return result;
}

bool META::Expand_If::eval(const std::map<std::string, Token> &ctx) const
{
    std::multimap<std::string, std::string> final_ctx;

    for (auto [placeholder, tok] : ctx) {
        final_ctx.insert({ placeholder, tok.val });
    }

    if (condition) 
        return condition->eval(final_ctx);
    else return true;
}
