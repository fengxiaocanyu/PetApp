// ============================================================
// skill_echo/main.cpp - Echo skill
// Returns input text with "Hello: " prefix
// ============================================================

#include <iostream>
#include <string>

// Note: Skill plugin is a standalone exe without Qt, using pure C++

int main()
{
    std::string line;

    // Read one line of JSON request from stdin
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            continue;
        }

        // Simple JSON parsing to extract "input" field
        std::string input;
        std::string::size_type inputPos = line.find("\"input\"");
        if (inputPos != std::string::npos) {
            std::string::size_type colonPos = line.find(':', inputPos);
            if (colonPos != std::string::npos) {
                std::string::size_type quotePos = line.find('"', colonPos + 1);
                if (quotePos != std::string::npos) {
                    std::string::size_type endQuotePos = line.find('"', quotePos + 1);
                    if (endQuotePos != std::string::npos) {
                        input = line.substr(quotePos + 1, endQuotePos - quotePos - 1);
                    }
                }
            }
        }

        // Build response
        std::string result = "Hello: " + input;

        // Output JSON response
        std::cout << "{\"id\":1,\"result\":\"" << result << "\",\"error\":null}" << std::endl;

        // Exit after single task completion
        return 0;
    }

    return 0;
}