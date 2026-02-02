#pragma once

#include <sstream>

class StreamTracker {
	std::istringstream stream;
	size_t line = 1;
	size_t column = 0;
	char lastChar = '\0';

public:
	StreamTracker(const std::string& s) : stream(std::istringstream(s)) {}

	// Read char and update line and column
	bool get(char& c) {
		if (!stream.get(c)) return false;
		if (lastChar == '\n') {
			line++;
			column = 1;
		} else {
			column++;
		}
		lastChar = c;
		return true;
	}

	char peek() {
		return stream.peek();
	}

	size_t get_line() const { return line; }
	size_t get_column() const { return column; }

	void putback(char c) {
		if (!stream) stream.clear();  // clear le flux avant putback
		stream.putback(c);
		if (c == '\n') {
			line--;
			column = 1;
		} else if (!std::iscntrl(c)) {
			column--;
			if (column < 1) column = 1;
		}
	}

	char get_last_ch() { return lastChar; }
};
