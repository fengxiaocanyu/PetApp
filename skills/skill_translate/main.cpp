// ============================================================
// skill_translate - Offline translation skill
// Local dictionary based English-Chinese translation
// Protocol: JSON line protocol (stdin/stdout)
// ============================================================

#include <iostream>
#include <string>
#include <map>
#include <sstream>

// Local English-Chinese dictionary
static std::map<std::string, std::string> buildDictionary()
{
    std::map<std::string, std::string> dict;
    dict["hello"] = u8"你好";
    dict["world"] = u8"世界";
    dict["good"] = u8"好的";
    dict["bad"] = u8"坏的";
    dict["big"] = u8"大的";
    dict["small"] = u8"小的";
    dict["hot"] = u8"热的";
    dict["cold"] = u8"冷的";
    dict["happy"] = u8"快乐的";
    dict["sad"] = u8"悲伤的";
    dict["love"] = u8"爱";
    dict["hate"] = u8"恨";
    dict["yes"] = u8"是";
    dict["no"] = u8"不";
    dict["thank"] = u8"谢谢";
    dict["please"] = u8"请";
    dict["sorry"] = u8"对不起";
    dict["help"] = u8"帮助";
    dict["friend"] = u8"朋友";
    dict["family"] = u8"家庭";
    dict["water"] = u8"水";
    dict["food"] = u8"食物";
    dict["time"] = u8"时间";
    dict["day"] = u8"天";
    dict["night"] = u8"夜晚";
    dict["morning"] = u8"早上";
    dict["evening"] = u8"晚上";
    dict["today"] = u8"今天";
    dict["tomorrow"] = u8"明天";
    dict["yesterday"] = u8"昨天";
    dict["cat"] = u8"猫";
    dict["dog"] = u8"狗";
    dict["bird"] = u8"鸟";
    dict["fish"] = u8"鱼";
    dict["book"] = u8"书";
    dict["pen"] = u8"笔";
    dict["school"] = u8"学校";
    dict["home"] = u8"家";
    dict["work"] = u8"工作";
    dict["play"] = u8"玩";
    dict["eat"] = u8"吃";
    dict["drink"] = u8"喝";
    dict["sleep"] = u8"睡觉";
    dict["run"] = u8"跑";
    dict["walk"] = u8"走";
    dict["talk"] = u8"说话";
    dict["see"] = u8"看见";
    dict["hear"] = u8"听见";
    dict["think"] = u8"思考";
    dict["know"] = u8"知道";
    dict["want"] = u8"想要";
    dict["need"] = u8"需要";
    dict["can"] = u8"能";
    dict["will"] = u8"将";
    dict["must"] = u8"必须";
    dict["may"] = u8"可以";
    dict["should"] = u8"应该";
    dict["beautiful"] = u8"美丽的";
    dict["ugly"] = u8"丑陋的";
    dict["rich"] = u8"富有的";
    dict["poor"] = u8"贫穷的";
    dict["young"] = u8"年轻的";
    dict["new"] = u8"新的";
    dict["fast"] = u8"快的";
    dict["slow"] = u8"慢的";
    dict["high"] = u8"高的";
    dict["low"] = u8"低的";
    dict["long"] = u8"长的";
    dict["short"] = u8"短的";
    dict["easy"] = u8"容易的";
    dict["hard"] = u8"困难的";
    dict["light"] = u8"光";
    dict["dark"] = u8"黑暗";
    dict["sun"] = u8"太阳";
    dict["moon"] = u8"月亮";
    dict["star"] = u8"星星";
    dict["sky"] = u8"天空";
    dict["earth"] = u8"地球";
    dict["fire"] = u8"火";
    dict["wind"] = u8"风";
    dict["rain"] = u8"雨";
    dict["snow"] = u8"雪";
    dict["cloud"] = u8"云";
    dict["mountain"] = u8"山";
    dict["river"] = u8"河";
    dict["sea"] = u8"海";
    dict["tree"] = u8"树";
    dict["flower"] = u8"花";
    dict["grass"] = u8"草";
    dict["animal"] = u8"动物";
    dict["human"] = u8"人类";
    dict["man"] = u8"男人";
    dict["woman"] = u8"女人";
    dict["child"] = u8"孩子";
    dict["baby"] = u8"婴儿";

    return dict;
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

// Extract "input" field value from JSON string (simple parser, no JSON lib)
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

// Translate function
static std::string translate(const std::string& input, const std::map<std::string, std::string>& dict)
{
    if (input.empty()) {
        return "Please enter text to translate";
    }

    std::string lower = toLower(input);
    auto it = dict.find(lower);
    if (it != dict.end()) {
        return it->second;
    }

    // Try word-by-word translation
    std::string result;
    std::string word;
    for (char c : lower) {
        if (c == ' ' || c == '\t' || c == '\n') {
            if (!word.empty()) {
                auto w = dict.find(word);
                if (w != dict.end()) {
                    result += w->second;
                } else {
                    result += word;
                }
                word.clear();
            }
            result += c;
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        auto w = dict.find(word);
        if (w != dict.end()) {
            result += w->second;
        } else {
            result += word;
        }
    }

    if (result.empty() || result == lower) {
        return "Translation not found: " + input;
    }

    return result;
}

int main()
{
    auto dictionary = buildDictionary();
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

        std::string result = translate(input, dictionary);
        std::cout << buildSuccessResponse(id, result);
        return 0;
    }

    return 0;
}
