/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */

#ifndef regexpH
#define regexpH

#define NSUBEXP  10
typedef struct regexp {
   char *startp[NSUBEXP];
   char *endp[NSUBEXP];
   char regstart;    /* Internal use only. */
   char reganch;     /* Internal use only. */
   char *regmust;    /* Internal use only. */
   int regmlen;      /* Internal use only. */
   char program[1];  /* Unwarranted chumminess with compiler. */
} regexp;


/*
 * Everything below this point was added or modified by Ken Keys to make
 * the regexp package work better with TinyFugue.
 */

#ifdef __cplusplus
extern "C" {
#endif

regexp *v8regcomp(char *);
int     v8regexec(regexp *, char *);
void    v8regsub(regexp *, char *, char *);
void    v8regerror(char *);

#ifdef __cplusplus
}
#endif

#endif
