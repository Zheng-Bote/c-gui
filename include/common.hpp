/**
 * SPDX-FileComment: Common macros for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file common.hpp
 * @brief Common macros and definitions.
 */

#ifndef C_GUI_COMMON_HPP
#define C_GUI_COMMON_HPP

#ifdef _WIN32
    #ifdef CGUI_CORE_EXPORTS
        #define CGUI_API __declspec(dllexport)
    #else
        #define CGUI_API __declspec(dllimport)
    #endif
    // Suppress warning about STL types in DLL interface (C4251)
    #pragma warning(disable: 4251)
#else
    #define CGUI_API __attribute__((visibility("default")))
#endif

#endif // C_GUI_COMMON_HPP
