#pragma once
// ============================================================
// PlatformDefines.h - Windows 平台宏保护
// 必须在任何 Windows 头之前包含此文件
// ============================================================

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// 清理 Windows 宏污染
#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif