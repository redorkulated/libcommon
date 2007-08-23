/******************************************************************************/
/*          postlicyd: a postfix policy daemon with a lot of features         */
/*          ~~~~~~~~~                                                         */
/*  ________________________________________________________________________  */
/*                                                                            */
/*  Redistribution and use in source and binary forms, with or without        */
/*  modification, are permitted provided that the following conditions        */
/*  are met:                                                                  */
/*                                                                            */
/*  1. Redistributions of source code must retain the above copyright         */
/*     notice, this list of conditions and the following disclaimer.          */
/*  2. Redistributions in binary form must reproduce the above copyright      */
/*     notice, this list of conditions and the following disclaimer in the    */
/*     documentation and/or other materials provided with the distribution.   */
/*  3. The names of its contributors may not be used to endorse or promote    */
/*     products derived from this software without specific prior written     */
/*     permission.                                                            */
/*                                                                            */
/*  THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND   */
/*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE     */
/*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR        */
/*  PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS    */
/*  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR    */
/*  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF      */
/*  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS  */
/*  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN   */
/*  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)   */
/*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF    */
/*  THE POSSIBILITY OF SUCH DAMAGE.                                           */
/******************************************************************************/

/*
 * Copyright © 2006-2007 Pierre Habouzit
 */

#include <signal.h>
#include <time.h>
#include <getopt.h>

#include "postlicyd.h"

static bool cleanexit = false;
static bool sigint = false;

static void main_sighandler(int sig)
{
    static time_t lastintr = 0;
    time_t now = time(NULL);

    switch (sig) {
      case SIGINT:
        if (sigint) {
            if (now - lastintr >= 1)
                break;
        } else {
            lastintr = now;
            sigint   = true;
        }
        return;

      case SIGTERM:
        break;

      default:
        return;
    }

    syslog(LOG_ERR, "Killed...");
    exit(-1);
}

static void main_initialize(void)
{
    openlog("postlicyd", LOG_PID, LOG_MAIL);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT,  &main_sighandler);
    signal(SIGTERM, &main_sighandler);
    syslog(LOG_INFO, "Starting...");
}

static void main_loop(void)
{
    while (!sigint) {
        int fd = accept(-1, NULL, 0);

        if (fd < 0) {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            syslog(LOG_ERR, "accept error: %m");
            return;
        }

        //pthread_create(NULL, NULL, job_run, (intptr_t)fd);
    }
}

static void main_shutdown(void)
{
    syslog(LOG_INFO, cleanexit ? "Stopping..." : "Unclean exit...");
    closelog();
}

int main(void)
{
    if (atexit(main_shutdown)) {
        fputs("Cannot hook my atexit function, quitting !\n", stderr);
        return EXIT_FAILURE;
    }

    main_initialize();
    main_loop();
    cleanexit = true;
    main_shutdown();
    return EXIT_SUCCESS;
}
