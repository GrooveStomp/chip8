/******************************************************************************
 * File: gstest.h
 * Created: 2016-08-19
 * Updated: 2016-07-07
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

extern int GSTestNumTestsRun;
extern char GSTestErrMsg[];

#define GSRunTest(Test) \
        do { \
                char *message = Test(); \
                GSTestNumTestsRun++; \
                if (message) \
                        return message; \
        } while (0)

#define GSTestAssert(Expression, ...) \
        do { \
                char msg[256]; \
                int success = snprintf(msg, 256, __VA_ARGS__); \
                if (!success) ; \
                if (!(Expression)) { \
                        success = snprintf(GSTestErrMsg, 256, "%s:%d#%s() %s\n", __FILE__, __LINE__, __func__, msg); \
                        if (!success) ; \
                        return GSTestErrMsg; \
                } \
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
