// ============================================================
// skill_chat - Offline chat skill
// Predefined Q&A pair based conversation
// Protocol: JSON line protocol (stdin/stdout)
// ============================================================

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cstdlib>

// Q&A pair structure
struct QAPair {
    std::string question;
    std::string answer;
};

// Load Q&A data from JSON file
static std::vector<QAPair> loadQAData(const std::string& filePath)
{
    std::vector<QAPair> qaList;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return qaList;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    // Simple JSON array parsing
    size_t pos = 0;
    while (true) {
        // Find next object start
        pos = content.find('{', pos);
        if (pos == std::string::npos) break;

        size_t endPos = content.find('}', pos);
        if (endPos == std::string::npos) break;

        std::string obj = content.substr(pos, endPos - pos + 1);
        pos = endPos + 1;

        // Extract question field
        std::string qKey = "\"question\":\"";
        size_t qStart = obj.find(qKey);
        if (qStart == std::string::npos) {
            qKey = "\"question\": \"";
            qStart = obj.find(qKey);
        }
        if (qStart == std::string::npos) continue;
        qStart += qKey.length();

        std::string question;
        while (qStart < obj.length() && obj[qStart] != '"') {
            if (obj[qStart] == '\\' && qStart + 1 < obj.length()) {
                qStart++;
                if (obj[qStart] == '"') question += '"';
                else if (obj[qStart] == '\\') question += '\\';
                else { question += '\\'; question += obj[qStart]; }
            } else {
                question += obj[qStart];
            }
            qStart++;
        }

        // Extract answer field
        std::string aKey = "\"answer\":\"";
        size_t aStart = obj.find(aKey);
        if (aStart == std::string::npos) {
            aKey = "\"answer\": \"";
            aStart = obj.find(aKey);
        }
        if (aStart == std::string::npos) continue;
        aStart += aKey.length();

        std::string answer;
        while (aStart < obj.length() && obj[aStart] != '"') {
            if (obj[aStart] == '\\' && aStart + 1 < obj.length()) {
                aStart++;
                if (obj[aStart] == '"') answer += '"';
                else if (obj[aStart] == '\\') answer += '\\';
                else { answer += '\\'; answer += obj[aStart]; }
            } else {
                answer += obj[aStart];
            }
            aStart++;
        }

        if (!question.empty() && !answer.empty()) {
            qaList.push_back({question, answer});
        }
    }

    return qaList;
}

// Convert string to lowercase
static std::string toLower(const std::string& str)
{
    std::string result = str;
    for (char& c : result) {
        if (c >= 'A' && c <= 'Z') {
            c = c - 'A' + 'a';
        }
    }
    return result;
}

// Simple JSON string escaping
static std::string jsonEscape(const std::string& str)
{
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c;
        }
    }
    return result;
}

// Build success JSON response
static std::string buildSuccessResponse(int id, const std::string& result)
{
    std::ostringstream oss;
    oss << "{\"id\":" << id << ",\"result\":\"" << jsonEscape(result) << "\",\"error\":null}\n";
    return oss.str();
}

// Build error JSON response
static std::string buildErrorResponse(int id, int code, const std::string& message)
{
    std::ostringstream oss;
    oss << "{\"id\":" << id << ",\"result\":null,\"error\":{\"code\":" << code
        << ",\"message\":\"" << jsonEscape(message) << "\"}}\n";
    return oss.str();
}

// Extract "input" field value from JSON string
static std::string extractInput(const std::string& json)
{
    std::string key = "\"input\":\"";
    size_t pos = json.find(key);
    if (pos == std::string::npos) {
        key = "\"input\": \"";
        pos = json.find(key);
        if (pos == std::string::npos) {
            return "";
        }
    }
    pos += key.length();
    std::string result;
    while (pos < json.length() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.length()) {
            pos++;
            if (json[pos] == '"') result += '"';
            else if (json[pos] == '\\') result += '\\';
            else if (json[pos] == 'n') result += '\n';
            else { result += '\\'; result += json[pos]; }
        } else {
            result += json[pos];
        }
        pos++;
    }
    return result;
}

// Extract "id" field value from JSON string
static int extractId(const std::string& json)
{
    std::string key = "\"id\":";
    size_t pos = json.find(key);
    if (pos == std::string::npos) {
        return 0;
    }
    pos += key.length();
    while (pos < json.length() && json[pos] == ' ') pos++;
    int id = 0;
    while (pos < json.length() && json[pos] >= '0' && json[pos] <= '9') {
        id = id * 10 + (json[pos] - '0');
        pos++;
    }
    return id;
}

// Find best matching answer
static std::string findAnswer(const std::string& input, const std::vector<QAPair>& qaList)
{
    if (input.empty()) {
        return "请说点什么吧！";
    }

    std::string lowerInput = toLower(input);

    // Try exact match first
    for (const auto& qa : qaList) {
        std::string lowerQ = toLower(qa.question);
        if (lowerInput == lowerQ) {
            return qa.answer;
        }
    }

    // Try substring match (input contains question or vice versa)
    for (const auto& qa : qaList) {
        std::string lowerQ = toLower(qa.question);
        if (lowerInput.find(lowerQ) != std::string::npos ||
            lowerQ.find(lowerInput) != std::string::npos) {
            return qa.answer;
        }
    }

    // Try keyword matching
    struct KeywordAnswer {
        std::vector<std::string> keywords;
        std::string answer;
    };

    std::vector<KeywordAnswer> fallbacks = {
        {{"hello", "hi", "hey"}, "Hello! How can I help you today?"},
        {{"bye", "goodbye", "see you", "cya"}, "Goodbye! Have a nice day!"},
        {{"thank", "thanks", "thx"}, "You're welcome! Happy to help!"},
        {{"help", "?"}, "I can chat with you! Try asking me about myself, or just say hello."},
        {{"name", "who"}, "I'm PetApp, your desktop pet companion!"},
        {{"joke", "funny", "laugh"}, "Why do programmers always mix up Halloween and Christmas? Because Oct 31 equals Dec 25!"},
        {{"sing", "song", "music"}, "La la la~ I'm a cute little desktop pet~ La la la~"}
    };

    for (const auto& fb : fallbacks) {
        for (const auto& kw : fb.keywords) {
            if (lowerInput.find(kw) != std::string::npos) {
                return fb.answer;
            }
        }
    }

    // Default response
    return "嗯... 我还没学会怎么回答这个。试试说'你好'或者'help'吧！";
}

int main(int argc, char* argv[])
{
    // Determine Q&A data file path
    std::string qaPath = "qa_data.json";

    // Check if running from plugins/ directory, adjust path
    if (argc > 0) {
        std::string exePath(argv[0]);
        size_t pos = exePath.find_last_of("/\\");
        if (pos != std::string::npos) {
            std::string dir = exePath.substr(0, pos);
            std::string altPath = dir + "/qa_data.json";
            std::ifstream testFile(altPath);
            if (testFile.is_open()) {
                testFile.close();
                qaPath = altPath;
            }
        }
    }

    // Also try skills/skill_chat/qa_data.json relative to working dir
    {
        std::string altPath = "skills/skill_chat/qa_data.json";
        std::ifstream testFile(altPath);
        if (testFile.is_open()) {
            testFile.close();
            qaPath = altPath;
        }
    }

    auto qaList = loadQAData(qaPath);
    if (qaList.empty()) {
        // Fallback: use built-in minimal Q&A
        qaList.push_back({"hello", "Hello! How can I help you today?"});
        qaList.push_back({"你好", "你好呀！我是你的桌宠小伙伴！"});
        qaList.push_back({"help", "I can chat with you! Try saying hello."});
    }

    std::string line;

    // Read JSON request from stdin
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            continue;
        }

        int id = extractId(line);
        std::string input = extractInput(line);

        if (input.empty()) {
            std::cout << buildErrorResponse(id, -1, "Missing input parameter");
            return 0;
        }

        std::string answer = findAnswer(input, qaList);
        std::cout << buildSuccessResponse(id, answer);
        return 0;
    }

    return 0;
}