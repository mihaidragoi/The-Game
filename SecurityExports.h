#pragma once

#ifdef SECURITYLIB_EXPORTS
#define SECURITYLIB_API __declspec(dllexport)
#else
#define SECURITYLIB_API __declspec(dllimport)
#endif
