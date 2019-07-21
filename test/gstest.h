/******************************************************************************
 * File: gstest.h
 * Created: 2016-08-19
 * Updated: 2019-07-21
 * Creator: Aaron Oman
 * Notice: Creative Commons Attribution 4.0 International License (CC-BY 4.0)
 *-----------------------------------------------------------------------------
 *
 * Library containing functions and macros to aid in testing other C code.
 *
 ******************************************************************************/
#ifndef GS_TEST_VERSION
#define GS_TEST_VERSION "0.1.0-dev"

#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE //
#include <stdio.h> // fprintf, snprintf

#define GSTestErrMsgSize 1024
extern int GSTestNumTestsRun;
extern char GSTestErrMsg[];

// NOTE: This disabled diagnostic pragma should be put in the GSTestAssert
// macro, but GCC (as of version 8.3.0) appears to have a bug that doesn't allow
// this to work properly.
// To work around a plethora of warnings, I'm just disabling this diagnostic
// globally for any programs including this header.
#pragma GCC diagnostic ignored "-Wformat-truncation="

#define GSTestRun(Test)                         \
        do {                                    \
                char *message = Test();         \
                GSTestNumTestsRun++;            \
                if (message)                    \
                        return message;         \
        } while (0)

#define GSTestAssert(Expression, ...)                                   \
        do {                                                            \
                _Pragma("GCC diagnostic push");                         \
                _Pragma("GCC diagnostic ignored \"-Wformat-truncation=\""); \
                char msg[GSTestErrMsgSize];                             \
                int written = snprintf(msg, GSTestErrMsgSize, __VA_ARGS__); \
                if (!written) ;                                         \
                if (!(Expression)) {                                    \
                        written = snprintf(GSTestErrMsg, GSTestErrMsgSize, "%s:%d#%s() %s\n", __FILE__, __LINE__, __func__, msg); \
                        if (!written) ;                                 \
                        return GSTestErrMsg;                            \
                }                                                       \
                _Pragma("GCC diagnostic pop");                          \
        } while (0)

unsigned int /* Memory should be MaxLength size, at least. */
GSTestRandomString(char *Memory, unsigned int MinLength, unsigned int MaxLength)
{
        int Length = rand() % (MaxLength - MinLength) + MinLength;
        int Range = 'z' - 'a';
        for(int I=0; I<Length; I++)
        {
                Memory[I] = rand() % Range + 'a';
        }
        Memory[Length] = '\0';

        return(Length);
}

#endif /* GS_TEST_VERSION */
