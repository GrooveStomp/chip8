/******************************************************************************
 * File: gstest.h
 * Created: 2016-08-19
 * Last Updated: 2016-08-22
 * Creator: Aaron Oman (a.k.a GrooveStomp)
 * Notice: (C) Copyright 2016 by Aaron Oman
 *-----------------------------------------------------------------------------
 *
 * Library containing functions and macros to aid in testing other C code.
 *
 ******************************************************************************/
#ifndef GS_TEST_VERSION
#define GS_TEST_VERSION "0.1.0-dev"

#include <stdlib.h> // EXIT_SUCCESS, EXIT_FAILURE //
#include <stdio.h> // fprintf

extern int GSTestNumTestsRun;
extern char GSTestErrMsg[];

#define GSRunTest(Test) \
        do { \
                char *message = Test(); \
                GSTestNumTestsRun++; \
                if (message) \
                        return message; \
        } while (0)

#define GSTestAssert(Expression, Description) \
        do { \
                if (!(Expression)) { \
                        int success = snprintf(GSTestErrMsg, 256, "%s:%d#%s() %s\n", __FILE__, __LINE__, __func__, (Description)); \
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
