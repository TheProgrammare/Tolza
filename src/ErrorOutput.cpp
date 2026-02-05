#include "ErrorOutput.hpp"

#include <iomanip>
#include <sstream>

#include "Globals.hpp"


// [file] file:LL:CC
// [code] | code line
//        |      ^^^^
// [error] [AAwxyz] blabla
// [hint] blabla
std::string Error_Text::print_error() const
{
    return print_source() + print_line() + print_messages();
}

std::string Error_Text::print_messages() const 
{
    std::string out;
    if (!msg.empty()) {
        if (code.substr(3, 1) == "0")    
            out = "[warning] [" color_MAGENTA + code + color_RESET "] " color_GREEN + msg + color_RESET "\n";
        else                                
            out = "[error] [" color_MAGENTA + code + color_RESET "] " color_RED + msg + color_RESET "\n";
    }
    if (!hint.empty())                   
        out += "[hint] " color_CYAN + hint + color_RESET "\n";
    return out; 
}

std::string Error_Text::print_line() const 
{
    size_t finalCursorSize = cursor_len == 0 ? 1 : cursor_len;
    // Trim code
    std::string trimmedLine = line_str.empty() ? "NO LINE FOUND" : trim(line_str);
    size_t trimSize = abs(int(trimmedLine.size() - line_str.size()));

    // cursor
    std::string line_offset_str = std::string(6 - std::to_string(line).size(), ' ');
    std::string cursor = std::string(finalCursorSize, '^');
    size_t cursor_offset = column < cursor_len + trimSize ? 0 : column - (cursor_len + trimSize);
    std::string cursor_offset_str = std::string(cursor_offset, ' ');

    // final
    return line_offset_str + std::to_string(line) + " | " color_RED + cursor_offset_str + cursor + color_RESET "\n";
}

std::string Error_Text::print_source() const 
{
    return "[file] " + file + ":" + std::to_string(line) + ":" + std::to_string(column) + "\n" color_RESET;
}

std::string Error_Text::print_link_error() const
{
    return std::string();
}


inline std::string Error_Text::escapeChar(unsigned char c) const
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

std::string Error_Text::trim(const std::string& str) const 
{
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

