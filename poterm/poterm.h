/*
 * PoTerm: primitive xterm for PocketBook - general definitions
 * ------------------------------------------------------------
 */

#ifndef POTERM_H
#define POTERM_H

#define POTERM_VERSION "1.03"

/* Keyboard buffer length */
#define KBUFFER_LEN 80

/* Init file with configuration values */
#if defined(__EMU__)
#define INI_FILE       "./poterm.ini"
#elif defined(__ARM__)
#define INI_FILE       "/mnt/ext1/system/config/poterm.ini"
#else
#error "Invalid architecture"
#endif

// Debug printouts
//#define P(args...)      do { fprintf(stderr, args); fflush(stderr); } while (0)
#define P(args...)

/*
 * How to call underlayed shell - define one of them
 * PTY_SHELL is much better, but PocketBook team disables access to /dev/pty*
 * files for some strange reason. So right now only PIPE_SHELL works
 */
#define PIPE_SHELL
//#define PTY_SHELL

#endif
