cmake_minimum_required(VERSION 3.22)
project(PetApp VERSION 1.0.0 LANGUAGES CXX)

# ============================================================
# C++ 标准
# ============================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
# Qt 自动处理
# ============================================================
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ============================================================
# 查找 Qt 6
# ============================================================
set(CMAKE_PREFIX_PATH "E:/work/QT/6.8.3/msvc2022_64")
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
)

# TextToSpeech 是可选的
find_package(Qt6 QUIET COMPONENTS TextToSpeech)

# ============================================================
# 主程序源文件
# ============================================================
set(PETAPP_HEADERS
    src/PlatformDefines.h
    src/core/EventBus.h
    src/core/MemoryModule.h
    src/core/HeartbeatModule.h
    src/core/EmotionModule.h
    src/core/CapabilityManager.h
    src/core/SkillManager.h
    src/core/CrashHandler.h
    src/ui/MainWindow.h
    src/ui/BubbleWidget.h
    src/ui/SettingsDialog.h
    src/ui/SkillMarketDialog.h
    src/ui/SkillPopupPanel.h
    src/ui/TrayManager.h
    src/ui/ChatDialog.h
    src/voice/VoiceManager.h
    src/character/CharacterWidget.h
    src/config/CharacterConfig.h
)

set(PETAPP_SOURCES
    src/main.cpp
    src/core/EventBus.cpp
    src/core/MemoryModule.cpp
    src/core/HeartbeatModule.cpp
    src/core/EmotionModule.cpp
    src/core/CapabilityManager.cpp
    src/core/SkillManager.cpp
    src/core/CrashHandler.cpp
    src/ui/MainWindow.cpp
    src/ui/BubbleWidget.cpp
    src/ui/SettingsDialog.cpp
    src/ui/SkillMarketDialog.cpp
    src/ui/SkillPopupPanel.cpp
    src/ui/TrayManager.cpp
    src/ui/ChatDialog.cpp
    src/voice/VoiceManager.cpp
    src/character/CharacterWidget.cpp
    src/config/CharacterConfig.cpp
)

# ============================================================
# 主程序目标（Windows 子系统，无控制台窗口）
# ============================================================
add_executable(PetApp WIN32
    ${PETAPP_HEADERS}
    ${PETAPP_SOURCES}
)

target_include_directories(PetApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(PetApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

# Windows 特定库
if(WIN32)
    # 查找 Windows SDK 路径
    # dbghelp.lib 在 Windows SDK 的 um 目录下
    find_library(DBGHELP_LIB dbghelp)
    if(DBGHELP_LIB)
        target_link_libraries(PetApp PRIVATE ${DBGHELP_LIB})
    else()
        # 如果找不到，尝试直接链接（需要 LIB 环境变量正确设置）
        target_link_libraries(PetApp PRIVATE dbghelp.lib)
    endif()
endif()

if(Qt6TextToSpeech_FOUND)
    target_link_libraries(PetApp PRIVATE Qt6::TextToSpeech)
    target_compile_definitions(PetApp PRIVATE HAS_TEXT_TO_SPEECH)
endif()

# ============================================================
# 全局编译定义 - Windows 宏保护
# ============================================================
target_compile_definitions(PetApp PRIVATE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

# ============================================================
# 编译选项 - W4 警告级别
# ============================================================
if(MSVC)
    target_compile_options(PetApp PRIVATE /W4 /utf-8)
endif()

# ============================================================
# 输出目录
# ============================================================
set_target_properties(PetApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# ============================================================
# 技能插件 - skill_echo
# ============================================================
add_executable(skill_echo
    skills/skill_echo/main.cpp
)

target_include_directories(skill_echo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/skills/skill_echo
)

# skill_echo 是独立 exe，不链接 Qt，不需要 AUTOMOC
set_target_properties(skill_echo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins"
    AUTOMOC OFF
    AUTORCC OFF
    AUTOUIC OFF
)

# 编译后复制 manifest.json 到 plugins/ 目录
add_custom_command(TARGET skill_echo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/skills/skill_echo/manifest.json"
        "${CMAKE_SOURCE_DIR}/plugins/skill_echo.manifest.json"
    COMMENT "Copying skill_echo manifest to plugins/"
)

# ============================================================
# 技能插件 - skill_translate
# ============================================================
add_executable(skill_translate
    skills/skill_translate/main.cpp
)

target_include_directories(skill_translate PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/skills/skill_translate
)

set_target_properties(skill_translate PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins"
    AUTOMOC OFF
    AUTORCC OFF
    AUTOUIC OFF
)

if(MSVC)
    target_compile_options(skill_translate PRIVATE /utf-8)
endif()

add_custom_command(TARGET skill_translate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/skills/skill_translate/manifest.json"
        "${CMAKE_SOURCE_DIR}/plugins/skill_translate.manifest.json"
    COMMENT "Copying skill_translate manifest to plugins/"
)

# ============================================================
# 技能插件 - skill_ocr
# ============================================================
add_executable(skill_ocr
    skills/skill_ocr/main.cpp
)

target_include_directories(skill_ocr PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/skills/skill_ocr
)

set_target_properties(skill_ocr PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins"
    AUTOMOC OFF
    AUTORCC OFF
    AUTOUIC OFF
)

if(MSVC)
    target_compile_options(skill_ocr PRIVATE /utf-8)
endif()

add_custom_command(TARGET skill_ocr POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/skills/skill_ocr/manifest.json"
        "${CMAKE_SOURCE_DIR}/plugins/skill_ocr.manifest.json"
    COMMENT "Copying skill_ocr manifest to plugins/"
)

# ============================================================
# 技能插件 - skill_chat
# ============================================================
add_executable(skill_chat
    skills/skill_chat/main.cpp
)

target_include_directories(skill_chat PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/skills/skill_chat
)

set_target_properties(skill_chat PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/plugins"
    AUTOMOC OFF
    AUTORCC OFF
    AUTOUIC OFF
)

if(MSVC)
    target_compile_options(skill_chat PRIVATE /utf-8)
endif()

add_custom_command(TARGET skill_chat POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/skills/skill_chat/manifest.json"
        "${CMAKE_SOURCE_DIR}/plugins/skill_chat.manifest.json"
    COMMENT "Copying skill_chat manifest to plugins/"
)

add_custom_command(TARGET skill_chat POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/skills/skill_chat/qa_data.json"
        "${CMAKE_SOURCE_DIR}/plugins/qa_data.json"
    COMMENT "Copying skill_chat Q&A data to plugins/"
)