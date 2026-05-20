// ============================================================
// skill_ocr - Offline OCR skill
// Calls tesseract.exe as subprocess for text recognition
// Protocol: JSON line protocol (stdin/stdout)
// ============================================================

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#endif

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

// Extract field value from JSON string (simple parser)
static std::string extractField(const std::string& json, const std::string& fieldName)
{
    std::string key = "\"" + fieldName + "\":\"";
    size_t pos = json.find(key);
    if (pos == std::string::npos) {
        key = "\"" + fieldName + "\": \"";
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

// Find tesseract executable path
// Priority: Program Files > PATH > local data/models/
static std::string findTesseract()
{
#ifdef _WIN32
    // 1. Check Program Files (winget install path) - highest priority, has all DLLs
    const char* programFiles = getenv("ProgramFiles");
    if (programFiles) {
        std::string pfPath = std::string(programFiles) + "/Tesseract-OCR/tesseract.exe";
        if (GetFileAttributesA(pfPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
            return pfPath;
        }
    }

    // 2. Check Program Files (x86)
    const char* programFilesX86 = getenv("ProgramFiles(x86)");
    if (programFilesX86) {
        std::string pfPath = std::string(programFilesX86) + "/Tesseract-OCR/tesseract.exe";
        if (GetFileAttributesA(pfPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
            return pfPath;
        }
    }

    // 3. Check executable's directory /data/models/
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH)) {
        std::string exeDir(exePath);
        size_t pos = exeDir.find_last_of("\\/");
        if (pos != std::string::npos) {
            exeDir = exeDir.substr(0, pos);
            std::string exeLocalPath = exeDir + "/data/models/tesseract.exe";
            if (GetFileAttributesA(exeLocalPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                return exeLocalPath;
            }
        }
    }

    // 4. Check current directory's data/models/
    std::string localPath = "data/models/tesseract.exe";
    if (GetFileAttributesA(localPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
        return localPath;
    }

    // 5. Fallback to PATH
    return "tesseract";
#else
    return "tesseract";
#endif
}

// Execute tesseract on the given image path
static std::string runTesseract(const std::string& imagePath, const std::string& lang)
{
#ifdef _WIN32
    // Create temporary output file path
    char tempPath[MAX_PATH];
    char tempFile[MAX_PATH];
    if (!GetTempPathA(MAX_PATH, tempPath)) {
        return "Failed to get temp path";
    }
    if (!GetTempFileNameA(tempPath, "ocr", 0, tempFile)) {
        return "Failed to create temp file";
    }

    // Find tesseract executable
    std::string tesseractPath = findTesseract();

    // Set TESSDATA_PREFIX to find language files
    // Try multiple locations
    const char* tessdataDirs[] = {
        "data/models/tessdata",
        "data/models",
        nullptr
    };
    std::string tessdataPrefix;
    for (int i = 0; tessdataDirs[i] != nullptr; i++) {
        if (GetFileAttributesA(tessdataDirs[i]) != INVALID_FILE_ATTRIBUTES) {
            // Check if it contains eng.traineddata
            std::string checkPath = std::string(tessdataDirs[i]) + "/eng.traineddata";
            if (GetFileAttributesA(checkPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                tessdataPrefix = tessdataDirs[i];
                break;
            }
        }
    }

    // Build tesseract command
    // Use cmd /c to properly handle cd + execution
    std::string cmd = "cmd /c ";
    std::string tessDir;
    size_t lastSlash = tesseractPath.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        tessDir = tesseractPath.substr(0, lastSlash);
    }
    if (!tessDir.empty()) {
        cmd += "cd /d \"" + tessDir + "\" && ";
    }
    cmd += "\"" + tesseractPath + "\" \"" + imagePath + "\" \"" + std::string(tempFile)
        + "\" -l " + lang;
    // Add --tessdata-dir if we found a tessdata directory
    if (!tessdataPrefix.empty()) {
        cmd += " --tessdata-dir \"" + tessdataPrefix + "\"";
    }
    cmd += " 2>nul";

    // Execute tesseract
    int result = std::system(cmd.c_str());
    if (result != 0) {
        // Clean up temp file
        DeleteFileA(tempFile);
        return "Tesseract execution failed (exit code: " + std::to_string(result)
            + "). Make sure tesseract is installed and in PATH.";
    }

    // Read output file
    std::string outputPath = std::string(tempFile) + ".txt";
    FILE* fp = fopen(outputPath.c_str(), "r");
    if (!fp) {
        DeleteFileA(tempFile);
        return "Failed to read tesseract output";
    }

    std::string text;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp)) {
        text += buffer;
    }
    fclose(fp);

    // Clean up temp files
    remove(outputPath.c_str());
    DeleteFileA(tempFile);

    // Trim trailing whitespace
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r' || text.back() == ' ')) {
        text.pop_back();
    }

    if (text.empty()) {
        return "No text recognized in the image";
    }

    return text;
#else
    return "OCR is only supported on Windows";
#endif
}

int main()
{
    std::string line;

    // Read JSON request from stdin
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            continue;
        }

        int id = extractId(line);
        std::string imagePath = extractField(line, "input");
        std::string lang = "eng";  // default language

        // Check config for language
        std::string configStr = extractField(line, "config");
        if (!configStr.empty()) {
            // Simple config parsing - look for "lang" field
            std::string langField = extractField(configStr, "lang");
            if (!langField.empty()) {
                lang = langField;
            }
        }

        if (imagePath.empty()) {
            std::cout << buildErrorResponse(id, -1, "Missing input parameter (image path)");
            return 0;
        }

        std::string result = runTesseract(imagePath, lang);
        std::cout << buildSuccessResponse(id, result);
        return 0;
    }

    return 0;
}