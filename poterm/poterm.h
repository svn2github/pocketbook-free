/*
 * PoTerm: primitive xterm for PocketBook - general definitions
 * ------------------------------------------------------------
 */

#ifndef POTERM_H
#define POTERM_H

#define POTERM_VERSION "1.01"

/* Keyboard buffer length */
#define KBUFFER_LEN 80

/* Init file with configuration values */
#if defined(HOST_X86)
#define INI_FILE       "./poterm.ini"
#elif defined(HOST_ARM)
#define INI_FILE       "/mnt/ext1/games/poterm.ini"
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

/*
 * I discovered some strange bug with SetOrientation - after SetOrientation() call
 * the task fails. It may be bug in the appllication itself but I suspect bug in
 * inkview because of two reasons:
 *   1. When I use SetOrientation() once on start - everything works fine
 *   2. Using SetOrientation() during normal work causes crash even on emulator
 * The second possibility - because of documentation lack I don't understand the
 * correct SetOrientation() call usage . PocketBook team, please help!
 *
 * Meanwhile I disable on the fly rotation. However application can be used
 * with any orientation - just edit poterm.ini file
 */
#define DISABLE_ROTATION

#endif
