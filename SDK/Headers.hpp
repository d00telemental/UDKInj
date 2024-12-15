/**
 * @file        SDK/Headers.hpp
 * @brief       This file is the primary public header for the SDK.
                It checks some core definitions and includes generated headers as required.
 */

#pragma once

#if defined(SDK_TARGET_UDK)
    #define SDK_TARGET 2015
    #if defined(SDK_TARGET_LE1) || defined(SDK_TARGET_LE2) || defined(SDK_TARGET_LE3)
        #error Conflicting SDK_TARGET definitions.
    #endif
#else
    #error No SDK_TARGET definition set.
#endif

#include "Common/Common.hpp"
#include "Includes.UDK.hpp"
