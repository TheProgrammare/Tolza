
#include "Lexer.hpp"

#include <iomanip>

#include "Globals.hpp"
#include "Token.hpp"
#include "ErrorOutput.hpp"


void Lexer::tokenize(const std::set<char>& exit_char) {
    auto stop_guard = [&]() -> bool {
        while ((std::iscntrl(ch) || std::isspace(ch)) && exit_char.find(ch) == exit_char.end()) { if (!eat()) return false; }
        if (ch == EOF) return true;
        if (exit_char.find(ch) != exit_char.end()) return true;
        return false;
    };

    while (eat()) {
        // check if must exit
        if (stop_guard()) return;

        // no char before keyword : clear all char control and spaces
        while ((std::iscntrl(ch) || std::isspace(ch)) && eat()) { }

        // it's a literal string
        if (ch == '"') {
            tokenize_textual(); 
            continue;
        }
        // it's a comment
        else if (ch == '/' && (stream.peek() == '/' || stream.peek() == '*')) {
            tokenize_comment(); 
            continue;
        }
        // it's a metacode instruction
        else if (ch == '#') {
            // handle multiple metacode in one line # static # const
            for (;;) {
                tokenize_metacode();
                if (ch != '#') break; 
            }
            
            continue;
        }
        // it's a placeholder: [[Identifier]]
        else if (ch == '[' && stream.peek() == '[') {
            eat(); // consume [
            eat(); // consume [
            tokenize_identifier();

            if (stream.peek() == ']') {
                eat(); // consume ]
                if (stream.peek() == ']') {
                    eat(); // consume ]
                    addToken(TokTy::S_METACODE_PLACEHOLDER);
                    continue;
                }
            }
            
            add_error("LEX1050", "Expected end of placeholder end ']]' after placeholder start '[['", "define placeholders in code like: `[[_U]]`");
        }
        // can be a numeric value or a range token (.. or ..=) or a variadic (...)
        else if (std::isdigit(ch) || ch == '.') {
            tokenize_numeric(); 
            continue;
        }
        // can be a identifier or keyword
        else if (std::isalpha(ch) || ch == '_') {
            tokenize_identifier();

            // try to avoid tokenize_keyword who is expensive :(
            // no prefix possible: can be a keyword or identifier
            if (std::iscntrl(stream.peek()) || std::isspace(stream.peek())) {
                // it's a keyword
                if (TokTy ty = Str_to_ETokenType(buffer); ty != TokTy::UNKNOWN) {
                    addToken(ty);
                }
                // it's a identifier
                else addToken(TokTy::IDENTIFIER);
            }
            // can be a keyword
            else if (is_valid_prefix(stream.peek(), buffer)) {
                tokenize_keyword();
            }
            // it's a keyword
            else if (TokTy ty = Str_to_ETokenType(buffer); ty != TokTy::UNKNOWN) {
                addToken(ty);
            }
            // it's a identifier
            else {
                addToken(TokTy::IDENTIFIER);
            }
            continue;
        }
        // it's can be a keyword or identifier or an error 
        else {
            tokenize_keyword();
        }
    }
    buffer.clear();
    addToken(TokTy::S_END_OF_FILE);
}


void Lexer::process_escape() {
    // read next chracter after backslash
    if (!stream.get(ch)) {
        add_error("LEX1060", "Unexpected end of input after escape sequence.", "");
        return;
    }

    switch (ch) {
        case 'n':  buffer += '\n'; break;
        case 't':  buffer += '\t'; break;
        case 'r':  buffer += '\r'; break;
        case '\\': buffer += '\\'; break;
        case '\'': buffer += '\''; break;
        case '"':  buffer += '"';  break;
        case '0':  buffer += '\0'; break;
        case 'a':  buffer += '\a'; break;
        case 'b':  buffer += '\b'; break;
        case 'f':  buffer += '\f'; break;
        case 'v':  buffer += '\v'; break;

        // hex sequence \xHH
        case 'x': {
            std::string hex;
            for (int i = 0; i < 2; ++i) { // On lit 1 ou 2 chiffres hexadécimaux
                if (!stream.get(ch) || !isxdigit(ch)) {
                    if (hex.empty()) {
                        add_error("LEX1061", "Invalid hex escape sequence", "define hex escape like: `\\xHH`");
                        return;
                    } else {
                        // return partial char if only one digit is read
                        break;
                    }
                }
                hex.push_back(ch);
            }
            try {
                char value = static_cast<char>(std::stoul(hex, nullptr, 16));
                buffer += value;
            } catch (...) {
                add_error("LEX1062", "Invalid hex escape value", "define hex escape like: `\\xHH`");
            }
            break;
        }

        // Unicode sequence \uXXXX or \UXXXXXXXX
        case 'u':
        case 'U': {
            int numDigits = (ch == 'u') ? 4 : 8;
            std::string hex;
            for (int i = 0; i < numDigits; ++i) {
                if (!stream.get(ch) || !isxdigit(ch)) {
                    add_error("LEX1063", "Invalid Unicode escape sequence", "define unicode escape like: `\\uXXXX`");
                    return;
                }
                hex.push_back(ch);
            }
            try {
                char32_t codepoint = std::stoul(hex, nullptr, 16);
                // basic convertion UTF-32 -> UTF-8
                if (codepoint <= 0x7F)
                    buffer += static_cast<char>(codepoint);
                else if (codepoint <= 0x7FF) {
                    buffer += static_cast<char>(0xC0 | (codepoint >> 6));
                    buffer += static_cast<char>(0x80 | (codepoint & 0x3F));
                } else if (codepoint <= 0xFFFF) {
                    buffer += static_cast<char>(0xE0 | (codepoint >> 12));
                    buffer += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                    buffer += static_cast<char>(0x80 | (codepoint & 0x3F));
                } else if (codepoint <= 0x10FFFF) {
                    buffer += static_cast<char>(0xF0 | (codepoint >> 18));
                    buffer += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
                    buffer += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                    buffer += static_cast<char>(0x80 | (codepoint & 0x3F));
                } else {
                    add_error("LEX1064", "Unicode codepoint out of range (" + std::to_string(codepoint) + ")", "");
                }
            } catch (...) {
                add_error("LEX1065", "Invalid Unicode escape value", "");
            }
            break;
        }

        default:
            // Caractère d’échappement inconnu : on le garde littéralement ou on signale une erreur
            // escape char unknown : keep literally or ring the error
            add_error("LEX1066", "Unknown escape sequence: \\" + std::to_string(ch), "");
            buffer += ch;
            break;
    }
}


void Lexer::tokenize_textual() {
    eat(); // consume "
    while (eat()) {
        if (ch == '{') {
            if (!buffer.empty()) addToken(TokTy::L_TEXTUAL);
            addToken(TokTy::S_TEXTUAL_EXPR_START);
            
            tokenize({ '}', ':' });

            if (ch == ':') {
                buffer += ch;
                addToken(TokTy::COLON);
                eat(); // consume :
                while (tokenize_spec()) { eat(); }
            }

            if (ch == '}') {
                addToken(TokTy::S_TEXTUAL_EXPR_END);
                eat(); // consume }
                if (ch == '"') {
                    buffer.clear();
                    return;
                }
            }
        }

        if (ch == '"') {
            addToken(TokTy::L_TEXTUAL);
            return;
        }
        else if (ch == '\\') {
            process_escape();
        }

        buffer += ch;
    }
}


bool Lexer::tokenize_spec() {
    // Unsigned integrals
    if (isdigit(ch)) {
        buffer += ch;
        while (isdigit(stream.peek())) {
            eat(); // consume current
            buffer += ch; // save peeked digit
        }
        
        addToken(TokTy::L_U);
        return true;
    }
    // Letters
    else if (isalpha(ch)) {
        buffer = ch;
        addToken(TokTy::L_ASCII);
        return true;
    }

    // Symbols and operators;
    buffer = ch;
    switch (ch) {
        case '.':   addToken(TokTy::DOT); return true;
        case ',':   addToken(TokTy::COMMA); return true;
        case '_':   addToken(TokTy::UNDERSCORE); return true;
        case '\'':  addToken(TokTy::TICK); return true;
        case '+':   addToken(TokTy::OP_PLUS); return true;
        case '-':   addToken(TokTy::OP_MINUS); return true;
        case '<':   addToken(TokTy::OPEN_BRACKETS); return true;
        case '>':   addToken(TokTy::CLOSE_BRACKETS); return true;
        case '^':   addToken(TokTy::OP_CIRCUMFLEX); return true;
        case '~':   addToken(TokTy::TILDE); return true;
        case '=':   addToken(TokTy::ASSIGN); return true;
        case '%':   addToken(TokTy::OP_MODULO); return true;
        case ' ':   addToken(TokTy::SPACE); return true;
        case '}': return false;
        default:
            errors.push_back(Error_Text(stream.get_line(), stream.get_column(), 1, 
                "", "LEX1005", "Unexpected format specifier character", 
                "define format specifier like:"
                "\n  - right-aligned: `{val:>10}`\n  - 2 decimals `{val:.2f}`\n  - hexadecimal `{val:#x}`", file_path)
                .print_error());
    }
    return false;
}

void Lexer::tokenize_comment() {
    if (ch == '/' && stream.peek() == '/') {
        while (stream.peek() != EOF && stream.peek() != '\n') eat();
        return;
    }
    // Ignore block comments /* */
    else if (ch == '/' && stream.peek() == '*') {
        eat(); // consume '*'
        while (eat()) {
            if (ch == '*' && stream.peek() == '/') { eat(); /* consume '/' */ return; }
        }
    }
}

void Lexer::tokenize_metacode() {
    addToken(TokTy::METACODE);
    tokenize({ '\n', '#' });
    addToken(TokTy::S_METACODE_END);
}

void Lexer::tokenize_numeric() {
    bool isBin = false, isOct = false, isHex = false;
    bool isFloat = false, isDecimal = false, isuDecimal = false;

    auto check_range_case = [&]() -> bool {
        if (ch == '.' && stream.peek() == '.') {
            eat();
            // case range included
            if (stream.peek() == '=') {
                eat(); buffer = "..="; addToken(TokTy::RANGE_INCLUSIVE);
            }
            // case variadic
            else if (stream.peek() == '.') {
                eat(); buffer = "..."; addToken(TokTy::VARIADIC);
            }
            // case range excluded
            else {
                buffer = ".."; addToken(TokTy::RANGE);
            }
            return true;
        }
        return false;
    };

    if (ch == '0' && stream.peek() != EOF) {
        char nt = stream.peek();
             if (nt == 'b' || nt == 'B') { isBin = true; eat(); buffer += ch; }
        else if (nt == 'o' || nt == 'O') { isOct = true; eat(); buffer += ch; }
        else if (nt == 'x' || nt == 'X') { isHex = true; eat(); buffer += ch; }
    }

    if (!isBin && !isOct && !isHex) {
        if (std::isdigit(ch)) buffer += ch;
        else if (check_range_case()) return;
        // start by dot . : can be float or member access
        else if (ch == '.') {
            // member access : a.b

            if (!tokens.empty() && tokens.back().ty == TokTy::IDENTIFIER && std::isalpha(stream.peek())) {
                buffer = ".";
                addToken(TokTy::DOT);
                return;
            }
            // floating value : 8. or 10.f or 3.14
            else {
                buffer = '.'; isFloat = true;
                if (stream.peek() == 'f' || stream.peek() == 'F') eat(); // consume .
            }
        }
    }

    while (eat()) {
        // Only allow 0 1 ' _
        if (isBin) {
                 if (ch == '0' || ch == '1') buffer += ch;
            else if (ch == '\'' || ch == '_') continue;
            else { stream.putback(ch); break; }
        }
        // Only allow 0 1 2 3 4 5 6 7 ' _
        else if (isOct) {
                 if (ch >= '0' && ch <= '7') buffer += ch;
            else if (ch == '\'' || ch == '_') continue;
            else { stream.putback(ch); break; }
        }
        // Only allow 0 1 2 3 4 5 6 7 8 9 A B C D E F ' _
        else if (isHex) {
                 if (std::isdigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) buffer += ch;
            else if (ch == '\'' || ch == '_') continue;
            else { stream.putback(ch); break; }
        }
        // numeric 
        else {
            // classic numeric
                 if (std::isdigit(ch)) { buffer += ch; }
            // prevent range creation : save buffer vals
            else if (ch == '.' && stream.peek() == '.') {
                if (!buffer.empty()) addToken(TokTy::L_I); // create literal integral
                check_range_case(); // create range
                return; // must stop after range creation
            }
            else if (ch == '.') {
                buffer += ch; isDecimal = true; 
            }
            // floating numeric 0.0f
            else if (ch == 'f') { isFloat = true; break; }
            // floating numeric (scientific notation) 10000e+10 100e-15
            else if (ch == 'e' || ch == 'E') {
                buffer += ch; isFloat = true;
                // exponent sign
                if (stream.peek() == '+' || stream.peek() == '-') { eat(); buffer += ch; }
            }
            // decimal numeric
            else if (ch == 'd') { isDecimal = true; break; }
            // udecimal numeric
            else if (ch == 'u' && stream.peek() == 'd') { isuDecimal = true; break; }
            // end
            else { stream.putback(ch); break; }
        }
    }
    
    if (isBin)      return addToken(TokTy::L_BIN);
    if (isOct)      return addToken(TokTy::L_OCT);
    if (isHex)      return addToken(TokTy::L_HEX);
    if (isFloat)    return addToken(TokTy::L_F);
    if (isDecimal)  return addToken(TokTy::L_DECIMAL);
    if (isuDecimal) return addToken(TokTy::L_UDECIMAL);
    
    return addToken(TokTy::L_I);
}


Lexer::EPrefixFound Lexer::get_prefix_keyword(TokTy _ty, const std::string& _key, const std::string& _search) {
    if (_key == _search) return EPrefixFound::All;
    if (_search.size() < _key.size() && _key.rfind(_search, 0) == 0)
        return EPrefixFound::Prefix;
    return EPrefixFound::None;
}

bool Lexer::is_valid_prefix(char prefix, const std::string& _current) {
    for (auto& [val, ty] : kSortedKeywords()) {
        if (val.size() <= _current.size()) continue;
        if (val.rfind(_current, 0) != 0) continue;
        if (val[_current.size()] == prefix) return true; 
    }
    return false;
};

void Lexer::tokenize_keyword() {
    // if no start by id : buffer must have the current character
    if (buffer.empty()) buffer = ch;

    bool keep_searching = true;
    while (keep_searching) {
        keep_searching = false;

        for (auto& [val, ty] : kSortedKeywords()) {
            // Ignore keywords that can't possibly match the first character
            if (val.empty() || val[0] != buffer[0]) continue;
            if (buffer.size() > val.size()) continue;
            switch (get_prefix_keyword(ty, val, buffer))
            {
            case EPrefixFound::None: {
                    continue;
                }
            case EPrefixFound::All: {
                    buffer = val;
                    addToken(ty);
                    return;
                }
            case EPrefixFound::Prefix: {
                    char next = stream.peek();
                    if (next == EOF || std::iscntrl(next) || std::isspace(next)) {
                        keep_searching = false;
                        break;
                    }

                    // optimization
                    if (is_valid_prefix(next, buffer)) {
                        eat();
                        buffer += next;
                        if (buffer.size() > MAX_KEYWORD_SIZE) break;
                        keep_searching = true;
                    }
                    else {
                        while (buffer.size() > 0) {
                            if (str_is_identifier(buffer)) return;
                            if (TokTy ty = Str_to_ETokenType(buffer); ty != TokTy::UNKNOWN) {
                                addToken(ty); return;
                            }
                            char last = buffer.back();
                            buffer.pop_back();
                            stream.putback(last);
                        }
                        
                        keep_searching = false;
                    }
                    break;
                }  
            }
            if (keep_searching) break;
        }
    }

    // purge the buffer of control char
    std::string buffer_temp;
    for (char elem : buffer) {
        if (!std::iscntrl(elem) && !std::isspace(elem)) buffer_temp += elem;
    }
    buffer = buffer_temp;

    if (buffer.empty()) return;

    // no corresponding keyword found : it's an identifier 
    if (str_is_identifier(buffer)) {
        addToken(TokTy::IDENTIFIER);
    }
    // not identifier standard : error
    else {
        add_error(
            "LEX1002", 
            "Unexpected token symbol", 
            "define keywords like:"
            "\n  - Identifier: alpha or '_' first and after alphanumeric: [a-Z_][a-Z0-9_]"
            "\n  - Reserved keyword: please, refer to the language documentation.");
    }
}


void Lexer::tokenize_identifier() {
    buffer += ch;
    bool keep_tokenize = true;
    while (keep_tokenize) {
        keep_tokenize = false;
        if (std::isalnum(stream.peek()) || stream.peek() == '_') {
            eat();
            buffer += ch;
            keep_tokenize = true; 
        }

        if (!keep_tokenize) break;
    }
}


void Lexer::addToken(TokTy ty) {
    Span span(0, stream.get_line(), stream.get_column(), buffer.size());
    span.anteprocess_pos = tokens.size();
    tokens.emplace_back(Token(buffer, ty, span));
    buffer.clear();
}

bool Lexer::eat() {
    if (stream.peek() == '\n' && !tokens.empty())
        tokens.back().debug_end_of_line = true;
    if (stream.get(ch))
        return true;
    return false;
}

void Lexer::add_error(const std::string& code, const std::string& err, const std::string& hint) {
    std::string out = Error_Text(stream.get_line(), stream.get_column(), buffer.size(), lines[stream.get_line()], code, err, hint, file_path).print_error();
    errors.push_back(out);
}


TokTy Lexer::classifyNumerals(std::string& outValue) {
    outValue.clear();
    char c;

    while (stream.get(c) && std::isspace(c)) {}
    
    // Check if start of number
    if (!isdigit(c) && c != '.') {
        stream.putback(c);
        return TokTy::UNKNOWN;
    }
    

    std::string buffer;
    bool isBin = false, isOct = false, isHex = false;
    bool isFloat = false, isDecimal = false, isuDecimal = false;
    
    buffer.push_back(c);

    // Check prefix bin/oct/hex
    if (c == '0' & stream.peek() != EOF) {
        char next = stream.peek();
             if (next == 'b' || next == 'B') { isBin = true; stream.get(c); buffer += c; }
        else if (next == 'o' || next == 'O') { isOct = true; stream.get(c); buffer += c; }
        else if (next == 'x' || next == 'X') { isHex = true; stream.get(c); buffer += c; }
    }
    
    while (stream.get(c)) {
        // Binary
        if (isBin) {
            if (c == '0' || c == '1') buffer += c;
            else if (c == '\'' || c == '_') continue;
            else { stream.putback(c); break; }
        }
        // Octal
        else if (isOct) {
            if (c >= '0' && c <= '7') buffer += c;
            else if (c == '\'' || c == '_') continue;
            else { stream.putback(c); break; }
        }
        // Hexadecimal
        else if (isHex) {
            if (isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) buffer += c;
            else if (c == '\'' || c == '_') continue;
            else { stream.putback(c); break; }
        }
        // Numeric / float / decimal
        else {
            if (isdigit(c)) buffer += c;
            else if (c == '.' & stream.peek() != '.' && !isDecimal) { buffer += c; isDecimal = true; }
            else if (c == 'f') { isFloat = true; break; }
            else if (c == 'e' || c == 'E') {
                buffer += c; isFloat = true;
                if (stream.peek() == '+' || stream.peek() == '-') { stream.get(c); buffer += c; }
            }
            else if (c == 'd') { isDecimal = true; break; }
            else if (c == 'u' && stream.peek() == 'd') { isuDecimal = true; break; }
            else { stream.putback(c); break; }
        }
    }
    
    outValue = buffer;

    if (isBin) return TokTy::L_BIN;
    if (isOct) return TokTy::L_OCT;
    if (isHex) return TokTy::L_HEX;
    if (isFloat) return TokTy::L_F;
    if (isDecimal) return TokTy::L_DECIMAL;
    if (isuDecimal) return TokTy::L_UDECIMAL;

    return TokTy::L_I; // standard integral
}



TokTy Lexer::classifyKeyword(std::string& outWord) {
    char c = outWord[0];

    // Lire le mot jusqu'à espace ou caractère de contrôle
    std::string buffer(1, c);
    while (stream.peek() != EOF && !std::isspace(stream.peek()) && !std::iscntrl(stream.peek())) {
        stream.get(c);
        buffer += c;
    }

    // 1 Chercher le mot-clé exact ou le plus long possible
    for (size_t len = buffer.size(); len > 0; --len) {
        std::string candidate = buffer.substr(0, len);
        auto it = kKeywords.find(candidate);
        if (it != kKeywords.end()) {
            outWord = candidate;
            // Remettre les caractères restants
            for (int i = (int)buffer.size() - 1; i >= (int)len; --i)
                stream.putback(buffer[i]);
            return it->second;
        }
    }

    // 2 Si aucun mot-clé trouvé, tenter de former un identifiant standard C
    if (isalpha(buffer[0]) || buffer[0] == '_') {
        size_t i = 1;
        while (i < buffer.size() && (isalnum(buffer[i]) || buffer[i] == '_')) i++;

        outWord = buffer.substr(0, i);

        // Remettre les caractères restants
        for (int j = (int)buffer.size() - 1; j >= (int)i; --j)
            stream.putback(buffer[j]);

        return TokTy::IDENTIFIER;
    }

    // 3 Aucun match → UNKNOWN
    outWord = buffer;
    return TokTy::UNKNOWN;
}


TokTy Lexer::classifyFormatSpec(std::string& outFormat) {
    outFormat.clear();
    char c = outFormat[0];

    // Unsigned integrals
    if (isdigit(c)) {
        outFormat += c;

        while (stream.get(c)) {
            // concat numbers
            if (isdigit(c)) outFormat += c;
            else { stream.putback(c); break; }
        }
        return TokTy::L_U;
    }
    // Letters
    else if (isalpha(c)) {
        outFormat += c;
        return TokTy::L_ASCII;
    }

    // Symbols and operators;
    outFormat += c;
    switch (c)
    {
    case '.':   return TokTy::DOT;
    case ',':   return TokTy::COMMA;
    case '_':   return TokTy::UNDERSCORE;
    case '\'':  return TokTy::TICK;
    case '+':   return TokTy::OP_PLUS;
    case '-':   return TokTy::OP_MINUS;
    case '<':   return TokTy::OPEN_BRACKETS;
    case '>':   return TokTy::CLOSE_BRACKETS;
    case '^':   return TokTy::OP_CIRCUMFLEX;
    case '~':   return TokTy::TILDE;
    case '=':   return TokTy::ASSIGN;
    case '%':   return TokTy::OP_MODULO;
    case ' ':   return TokTy::SPACE;
    default:    return TokTy::UNKNOWN;
    }
}

