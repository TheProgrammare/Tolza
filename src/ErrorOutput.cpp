#include "ErrorOutput.hpp"

#include <iomanip>
#include <sstream>

#include "Globals.hpp"

// errorCode : 'AAAwxyz'
// AAA = LEX: lexer, PAR: parser, SYM: symbol resolution, TYP: type resolution, SEM: semantic resolution
// w = 0: warning, 1: error, 2: fatal
// xy = 0-9 0-9 error type
// z = 0-9 sub error type
std::string make_error_output(
    size_t line, 
    size_t col, 
    size_t cursor_len, 
    const std::string& lineStr, 
    const std::string& errCode, 
    const std::string& errMsg, 
    const std::string& hintMsg, 
    const std::string& f) {
    //[file] file:LL:CC
    //[code] | code line
    //       |      ^^^^
    //[error] [AAwxyz] blabla
    //[hint] blabla
    
    size_t finalCursorSize = cursor_len == 0 ? 1 : cursor_len;
    // Trim code
    std::string trimmedLine = lineStr.empty() ? "NO LINE FOUND" : trim(lineStr);
    size_t trimSize = abs(int(trimmedLine.size() - lineStr.size()));

    std::string line_offset_str = std::string(6 - std::to_string(line).size(), ' ');
    std::string cursor = std::string(finalCursorSize, '^');
    size_t cursor_offset = col < cursor_len + trimSize ? 0 : col - (cursor_len + trimSize);
    std::string cursor_offset_str = std::string(cursor_offset, ' ');


    std::ostringstream headMsg;
    headMsg << "[file] " << f << ":" << line << ":" << col << "\n" << color_GREEN;                
    headMsg << "[code] | " << trimmedLine << "\n";                                                           
    headMsg <<  line_offset_str << line << " | " color_RED << cursor_offset_str << cursor << color_RESET << "\n";    
   
    if (!errMsg.empty()) {
        // Color according type

        if (errCode.substr(3, 1) == "0")    headMsg << "[warning] ["    color_MAGENTA << errCode << color_RESET "] " color_GREEN    << errMsg << color_RESET "\n";
        else                                headMsg << "[error] ["      color_MAGENTA << errCode << color_RESET "] " color_RED      << errMsg << color_RESET "\n";
    }
    if (!hintMsg.empty())                   headMsg << "[hint] " color_CYAN << hintMsg << color_RESET "\n";

    return headMsg.str();
}

inline std::string escapeChar(unsigned char c)
{
    switch (c) {
        case '\a': return "\\a";
        case '\b': return "\\b";
        case '\f': return "\\f";
        case '\n': return "\\n";
        case '\r': return "\\r";
        case '\t': return "\\t";
        case '\v': return "\\v";
        case '\\': return "\\\\";
        case '\'': return "\\\'";
        case '\"': return "\\\"";
        default:
            if (isprint(c))
                return std::string(1, c); // caractère visible
            else {
                // caractère de contrôle non standard → représentation hexadécimale
                std::ostringstream oss;
                oss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c;
                return oss.str();
            }
    }
}

std::string trim(const std::string& str) {
    const char* whitespace = " \t\n\r\f\v";

    // Find first position who is not white space
    size_t start = str.find_first_not_of(whitespace);
    if (start == std::string::npos) {
        // string is null or contains only spaces
        return "";
    }

    // Find the last position who is not white space
    size_t end = str.find_last_not_of(whitespace);

    // Extract the sub string without spaces around
    return str.substr(start, end - start + 1);
}
