/*
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* Simple program:  Loop, watching keystrokes
   Note that you need to call SDL_PollEvent() or SDL_WaitEvent() to 
   pump the event loop and catch keystrokes.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#include "SDL_input.h"

/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    SDL_Quit();
    exit(rc);
}


int
main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Event event;
    int done;

    /* Initialize SDL */
//     if (SDL_Init(SDL_INIT_EVENT) < 0) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
        return (1);
    }

    int numinputs = SDL_GetNumInputDevices();
    fprintf(stderr, "number of input devices: %s\n", numinputs );
    
//     int numkeyboards = SDL_GetNumKeyboards();
//     fprintf(stderr, "number of keyboards: %s\n", numkeyboards );
    
   

    /* Set 640x480 video mode */
//     window = SDL_CreateWindow("CheckKeys Test",
//                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
//                               640, 480, 0);
//     if (!window) {
//         fprintf(stderr, "Couldn't create 640x480 window: %s\n",
//                 SDL_GetError());
//         quit(2);
//     }

// #if __IPHONEOS__
//     /* Creating the context creates the view, which we need to show keyboard */
//     SDL_GL_CreateContext(window);
// #endif

//     SDL_StartTextInput();
// 
//     /* Watch keystrokes */
//     done = 0;
//     while (!done) {
// //       while( SDL_PollEvent(&event) ){
//         /* Check for events */
//         SDL_WaitEvent(&event);
// 	Uint8 *state = SDL_GetKeyboardState(NULL);
// 	if ( state[SDL_SCANCODE_RETURN] ) {
// 	  printf("<RETURN> is pressed.\n");
// 	}
//         switch (event.type) {
//         case SDL_KEYDOWN:
//         case SDL_KEYUP:
//             PrintKey(&event.key.keysym, event.key.state, event.key.repeat);
//             break;
//         case SDL_TEXTINPUT:
//             PrintText(event.text.text);
//             break;
//         case SDL_MOUSEBUTTONDOWN:
//             /* Any button press quits the app... */
//         case SDL_QUIT:
//             done = 1;
//             break;
//         default:
//             break;
//         }
// //       }
//     }

    SDL_Quit();
    return (0);
}

/* vi: set ts=4 sw=4 expandtab: */
