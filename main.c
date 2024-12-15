#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t bool;
#define true 1
#define false 0
#define not !
#define and &&
#define or ||

typedef struct exception
{
    uint8_t type;
    const char* message;
    bool cleared;
    jmp_buf ex_buf;
} exception;
enum exception_type
{
    NoError = 0,
    RuntimeError = 1,
    LogicError = 2,
};
#define error_type(err) (                                                                   \
    err == RuntimeError ? "RuntimeError"                                                    \
    : err == LogicError ? "LogicError"                                                      \
    : "UnknownError"                                                                        \
    )

#define MAX_EXCEPTIONS 0xff
struct exception * exception_context[MAX_EXCEPTIONS] = {NULL};
uint8_t exception_idx = MAX_EXCEPTIONS;

#define try                                                                                 \
    {                                                                                       \
        exception_idx++;                                                                    \
        exception_context[exception_idx] = (struct exception*)malloc(sizeof(struct exception));        \
        struct exception * pexcep = exception_context[exception_idx];                       \
        do {                                                                                \
            pexcep->type = NoError;                                                         \
            pexcep->cleared = false;                                                        \
            if( 0 == (pexcep->type = setjmp(pexcep->ex_buf) ) ) {
#define catch(err_type, e)                                                                  \
            } else if (err_type == pexcep->type) {                                          \
                pexcep->cleared = true;                                                     \
                struct exception e = *pexcep;
#define catch_(e)                                                                           \
            } else {                                                                        \
                pexcep->cleared = true;                                                     \
                struct exception e = *pexcep;
#define finalize                                                                            \
            }                                                                               \
            pexcep->cleared = true;                                                         \
        } while(0);                                                                         \
        if (pexcep->type != NoError and not pexcep->cleared) {                              \
            if (exception_idx == 0) {                                                       \
                fprintf(stderr, "terminate called after throwing an instance of %s:\n"      \
                    "  what():  %s\n"                                                       \
                    , error_type(pexcep->type)                                              \
                    , pexcep->message);                                                     \
                exit(pexcep->type);                                                         \
            }                                                                               \
            exception_context[exception_idx - 1]->type = pexcep->type;                      \
            exception_context[exception_idx - 1]->message = pexcep->message;                \
            exception_context[exception_idx - 1]->cleared = pexcep->cleared;                \
            free(pexcep);                                                                   \
            exception_context[exception_idx] = NULL;                                        \
            exception_idx--;                                                                \
            pexcep = exception_context[exception_idx];                                      \
            longjmp(pexcep->ex_buf, pexcep->type);                                          \
        }                                                                                   \
        else {                                                                              \
            free(pexcep);                                                                   \
            exception_context[exception_idx] = NULL;                                        \
            exception_idx--;                                                                \
        }                                                                                   \
    }
#define throw(err_type, msg)                                                                \
    {                                                                                       \
        struct exception * pexcep = exception_context[exception_idx];                       \
        if (pexcep->type != NoError and pexcep->cleared) {                                  \
            if (exception_idx == 0) {                                                       \
                fprintf(stderr, "terminate called after throwing an instance of %s:\n"      \
                    "  what():  %s\n"                                                       \
                    , error_type(err_type)                                                  \
                    , msg);                                                                 \
                exit(pexcep->type);                                                         \
            }                                                                               \
            free(pexcep);                                                                   \
            exception_context[exception_idx] = NULL;                                        \
            exception_idx--;                                                                \
        }                                                                                   \
        pexcep = exception_context[exception_idx];                                          \
        pexcep->message = msg;                                                              \
        pexcep->type = err_type;                                                            \
        longjmp(pexcep->ex_buf, pexcep->type);                                              \
    }
#define throw_                                                                              \
    {                                                                                       \
        if (exception_idx >= MAX_EXCEPTIONS) {                                              \
            perror("terminate called without an active exception");                         \
            abort();                                                                        \
        }                                                                                   \
        struct exception * pexcep = exception_context[exception_idx];                       \
        if (pexcep->type == NoError) {                                                      \
            perror("terminate called without an active exception");                         \
            abort();                                                                        \
        }                                                                                   \
        if (pexcep->cleared) {                                                              \
            if (exception_idx == 0) {                                                       \
                fprintf(stderr, "terminate called after throwing an instance of %s:\n"      \
                    "  what():  %s\n"                                                       \
                    , error_type(pexcep->type)                                              \
                    , pexcep->message);                                                     \
                exit(pexcep->type);                                                         \
            }                                                                               \
            exception_context[exception_idx - 1]->type = pexcep->type;                      \
            exception_context[exception_idx - 1]->message = pexcep->message;                \
            exception_context[exception_idx - 1]->cleared = false;                          \
            free(pexcep);                                                                   \
            exception_context[exception_idx] = NULL;                                        \
            exception_idx--;                                                                \
        }                                                                                   \
        pexcep = exception_context[exception_idx];                                          \
        longjmp(pexcep->ex_buf, pexcep->type);                                              \
    }
#define throw__(err_type, msg)        /* Works only in the same block as try-catch */       \
    pexcep->message = msg;                                                                  \
    pexcep->type = err_type;                                                                \
    if (pexcep->cleared) {                                                                  \
        pexcep->cleared = false;                                                            \
        break;                                                                              \
    }                                                                                       \
    longjmp(pexcep->ex_buf, pexcep->type);
#define throw___                      /* Works only in the same block as try-catch */       \
    pexcep->cleared = false;                                                                \
    if (pexcep->type == NoError) {                                                          \
        perror("terminate called without an active exception");                             \
        abort();                                                                            \
    }                                                                                       \
    break;

void func() {
    throw_;
    throw(RuntimeError, "Error in func");
}

int main(int argc, char** argv)
{
    try
    {
        printf("In Try Statement\n");
        try
        {
            printf("In Inner Try Statement\n");
            // throw_;
            throw(LogicError, "Inner Logic Error");
            printf("I do not appear\n");
        }
        catch(LogicError, e)
        {
            printf("Got Exception In: %s!\n", e.message);
            // throw_;
            // throw(RuntimeError, "Inner Runtime Error");
            // throw__(RuntimeError, "Inner Runtime Error");
            func();
        }
        catch_(e)
        {
            printf("Got Unknown Exception In: %s!\n", e.message);
        }
        finalize;
        // throw(RuntimeError, "Inner Runtime Error");
        printf("I do not appear if exception is thrown\n");
    }
    catch(LogicError, e)
    {
        printf("Got Exception Out: %s!\n", e.message);
        
    }
    catch(RuntimeError, e)
    {
        printf("Got Exception Out: %s!\n", e.message);
        throw(LogicError, "Outer Logic Error");
    }
    catch_(e)
    {
        printf("Got Unknown Exception Out: %d!\n", e.type);
    }
    finalize;

    printf("All done!!\n");

    return 0;
}