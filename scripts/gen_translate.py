# Generate skill_translate/main.cpp with Chinese characters
import os

dict_data = {
    'hello': '你好', 'world': '世界', 'good': '好的', 'bad': '坏的',
    'big': '大的', 'small': '小的', 'hot': '热的', 'cold': '冷的',
    'happy': '快乐的', 'sad': '悲伤的', 'love': '爱', 'hate': '恨',
    'yes': '是', 'no': '不', 'thank': '谢谢', 'please': '请',
    'sorry': '对不起', 'help': '帮助', 'friend': '朋友', 'family': '家庭',
    'water': '水', 'food': '食物', 'time': '时间', 'day': '天',
    'night': '夜晚', 'morning': '早上', 'evening': '晚上',
    'today': '今天', 'tomorrow': '明天', 'yesterday': '昨天',
    'cat': '猫', 'dog': '狗', 'bird': '鸟', 'fish': '鱼',
    'book': '书', 'pen': '笔', 'school': '学校', 'home': '家',
    'work': '工作', 'play': '玩', 'eat': '吃', 'drink': '喝',
    'sleep': '睡觉', 'run': '跑', 'walk': '走', 'talk': '说话',
    'see': '看见', 'hear': '听见', 'think': '思考', 'know': '知道',
    'want': '想要', 'need': '需要', 'can': '能', 'will': '将',
    'must': '必须', 'may': '可以', 'should': '应该',
    'beautiful': '美丽的', 'ugly': '丑陋的', 'rich': '富有的', 'poor': '贫穷的',
    'young': '年轻的', 'new': '新的', 'fast': '快的', 'slow': '慢的',
    'high': '高的', 'low': '低的', 'long': '长的', 'short': '短的',
    'easy': '容易的', 'hard': '困难的', 'light': '光', 'dark': '黑暗',
    'sun': '太阳', 'moon': '月亮', 'star': '星星', 'sky': '天空',
    'earth': '地球', 'fire': '火', 'wind': '风', 'rain': '雨',
    'snow': '雪', 'cloud': '云', 'mountain': '山', 'river': '河',
    'sea': '海', 'tree': '树', 'flower': '花', 'grass': '草',
    'animal': '动物', 'human': '人类', 'man': '男人', 'woman': '女人',
    'child': '孩子', 'baby': '婴儿'
}

cpp_code = '''// ============================================================
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
'''

for eng, chn in dict_data.items():
    cpp_code += f'    dict["{eng}"] = u8"{chn}";\n'

cpp_code += '''
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
            case '"': result += "\\\\\\""; break;
            case '\\\\': result += "\\\\\\\\"; break;
            case '\\n': result += "\\\\n"; break;
            case '\\r': result += "\\\\r"; break;
            case '\\t': result += "\\\\t"; break;
            default: result += c;
        }
    }
    return result;
}

// Build success JSON response
static std::string buildSuccessResponse(int id, const std::string& result)
{
    std::ostringstream oss;
    oss << "{\\"id\\":" << id << ",\\"result\\":\\"" << jsonEscape(result) << "\\",\\"error\\":null}\\n";
    return oss.str();
}

// Build error JSON response
static std::string buildErrorResponse(int id, int code, const std::string& message)
{
    std::ostringstream oss;
    oss << "{\\"id\\":" << id << ",\\"result\\":null,\\"error\\":{\\"code\\":" << code
        << ",\\"message\\":\\"" << jsonEscape(message) << "\\"}}\\n";
    return oss.str();
}

// Extract "input" field value from JSON string (simple parser, no JSON lib)
static std::string extractInput(const std::string& json)
{
    std::string key = "\\"input\\":\\"";
    size_t pos = json.find(key);
    if (pos == std::string::npos) {
        key = "\\"input\\": \\"";
        pos = json.find(key);
        if (pos == std::string::npos) {
            return "";
        }
    }
    pos += key.length();
    std::string result;
    while (pos < json.length() && json[pos] != '"') {
        if (json[pos] == '\\\\' && pos + 1 < json.length()) {
            pos++;
            if (json[pos] == '"') result += '"';
            else if (json[pos] == '\\\\') result += '\\\\';
            else if (json[pos] == 'n') result += '\\n';
            else { result += '\\\\'; result += json[pos]; }
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
    std::string key = "\\"id\\":";
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
        if (c == ' ' || c == '\\t' || c == '\\n') {
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
'''

# Write file with UTF-8 encoding
output_path = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'skills', 'skill_translate', 'main.cpp')
with open(output_path, 'w', encoding='utf-8') as f:
    f.write(cpp_code)

print(f'File written: {output_path}')
print('Done!')