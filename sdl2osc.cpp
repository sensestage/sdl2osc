//  sdl-jstest - Joystick Test Program for SDL
//  Copyright (C) 2007 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.

#include <SDL/SDL.h>
#include <assert.h>
// #include <curses.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <lo/lo.h>


#include <map>

typedef std::map<int,SDL_Joystick*> joy_map_t;

joy_map_t joysticks;    // declares a vector of integers


int done = 0;

lo_address t;
lo_server s;
lo_server_thread st;


/// initialize joysticks vector:

// void init_joysticks_vector(){
//   int num_joysticks = SDL_NumJoysticks();
//   joysticks.reserve( num_joysticks );
// }

void close_all_joysticks(){
  joy_map_t::const_iterator it;
  for(it=joysticks.begin(); it!=joysticks.end(); ++it){
    SDL_JoystickClose( it->second );
  }
  joysticks.clear();
}

void open_joystick( int joy_idx ){
  if ( SDL_JoystickOpened( joy_idx ) ){
      return;
  }
  SDL_Joystick * newjoy = SDL_JoystickOpen( joy_idx );
  if (!newjoy)
  {
    fprintf(stderr, "Unable to open joystick %d\n", joy_idx);
    lo_send_from( t, s, LO_TT_IMMEDIATE, "/joystick/open/error", "i", joy_idx );
  }
  else
  {
    joysticks[ joy_idx ] = newjoy;
    lo_send_from( t, s, LO_TT_IMMEDIATE, "/joystick/open", "is", joy_idx, SDL_JoystickName( joy_idx ) );
//     /// add to the vector:
//     joysticks.push_back( newjoy );
  }
}

void close_joystick( int joy_idx ){
  if ( !SDL_JoystickOpened( joy_idx ) ){
      lo_send_from( t, s, LO_TT_IMMEDIATE, "/joystick/close/error", "i", joy_idx );
      return;
  }
//   joy_map_t::const_iterator it;
  lo_send_from( t, s, LO_TT_IMMEDIATE, "/joystick/closed", "is", joy_idx, SDL_JoystickName( joy_idx ) );
  SDL_JoystickClose( joysticks.find( joy_idx )->second );
  joysticks.erase( joysticks.find( joy_idx ) );
}


/// OSC bits

void error(int num, const char *m, const char *path);
int info_handler(const char *path, const char *types, lo_arg **argv,
		    int argc, void *data, void *user_data);

// int openjoystick_handler(const char *path, const char *types, lo_arg **argv,
// 		    int argc, void *data, void *user_data);

int joy_open_handler(const char *path, const char *types, lo_arg **argv,
			 int argc, void *data, void *user_data);
int joy_close_handler(const char *path, const char *types, lo_arg **argv,
			 int argc, void *data, void *user_data);
int joy_info_handler(const char *path, const char *types, lo_arg **argv,
			 int argc, void *data, void *user_data);

int generic_handler(const char *path, const char *types, lo_arg **argv,
		    int argc, void *data, void *user_data);
int quit_handler(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data);

int init_osc( char * ip, char *outport, char * port ){
     /* create liblo addres */
    t = lo_address_new(ip, outport); // change later to use other host

    lo_server_thread st = lo_server_thread_new(port, error);

    lo_server_thread_add_method(st, "/joystick/open", "i", joy_open_handler, NULL);
    lo_server_thread_add_method(st, "/joystick/info", "i", joy_info_handler, NULL);
    lo_server_thread_add_method(st, "/joystick/close", "i", joy_close_handler, NULL);

    lo_server_thread_add_method(st, "/sdl2osc/info", "", info_handler, NULL);
    lo_server_thread_add_method(st, "/sdl2osc/quit", "", quit_handler, NULL);
    lo_server_thread_add_method(st, NULL, NULL, generic_handler, NULL);

    lo_server_thread_start(st);
 
    lo_server s = lo_server_thread_get_server( st );

    lo_send_from( t, s, LO_TT_IMMEDIATE, "/sdl2osc/started", "" ); 
}

lo_message get_joystick_info_msg(int joy_idx, SDL_Joystick * joy )
{
//   SDL_Joystick * joy = joysticks.find( joy_idx )->second;
  
  lo_message m1 = lo_message_new();
  lo_message_add_string( m1, SDL_JoystickName(joy_idx) );
  lo_message_add_int32( m1, joy_idx );
  lo_message_add_int32( m1, SDL_JoystickNumAxes(joy) );
  lo_message_add_int32( m1, SDL_JoystickNumButtons(joy) );
  lo_message_add_int32( m1, SDL_JoystickNumHats(joy) );
  lo_message_add_int32( m1, SDL_JoystickNumBalls(joy) );
  
//   lo_send_message_from( t, s, "/joystick/info", m1 ); 
  return m1;
}


void error(int num, const char *msg, const char *path)
{
     printf("liblo server error %d in path %s: %s\n", num, path, msg);
     fflush(stdout);
}

int info_handler(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
//      printf("info message\n");
//      fflush(stdout);

  lo_bundle b = lo_bundle_new( LO_TT_IMMEDIATE );

      int num_joysticks = SDL_NumJoysticks();
//       printf("Found %d joystick(s)\n\n", num_joysticks);
	lo_message m1 = lo_message_new();
	lo_message_add_int32( m1, num_joysticks );
	lo_bundle_add_message( b, "/joystick/number", m1 );
	int joy_idx;
        for(joy_idx = 0; joy_idx < num_joysticks; ++joy_idx)
        {
          SDL_Joystick* joy = SDL_JoystickOpen(joy_idx);
          if (!joy)
          {
            fprintf(stderr, "Unable to open joystick %d\n", joy_idx);
          }
          else
          {
	    lo_message m2 = get_joystick_info_msg( joy_idx, joy );
	    lo_bundle_add_message( b, "/joystick/info", m2 );
//             print_joystick_info(joy_idx, joy);
            SDL_JoystickClose(joy);
          }
//         }
      }

 	if ( lo_send_bundle_from( t, s, b )  == -1 ){
 		{ printf("sd2osc/info: OSC error %d: %s\n", lo_address_errno(t), lo_address_errstr(t)); }
 	}
 	lo_bundle_free( b );
	fflush(stdout);

    return 0;
}


/* catch any incoming messages and display them. returning 1 means that the
 * message has not been fully handled and the server should try other methods */
int generic_handler(const char *path, const char *types, lo_arg **argv,
		    int argc, void *data, void *user_data)
{
    int i;

    printf("path: <%s>\n", path);
    for (i=0; i<argc; i++) {
	printf("arg %d '%c' ", i, types[i]);
	lo_arg_pp( (lo_type) types[i], argv[i]);
	printf("\n");
    }
    printf("\n");
    fflush(stdout);

    return 1;
}

int quit_handler(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
    done = 1;
    printf("sdl2osc: allright, that's it, I quit\n");
    fflush(stdout);

    return 0;
}

void send_joystick_info(int joy_idx)
{
  SDL_Joystick * joy = joysticks.find( joy_idx )->second;
  
  lo_message m1 = lo_message_new();
  lo_message_add_string( m1, SDL_JoystickName(joy_idx) );
  lo_message_add_int32( m1, joy_idx );
  lo_message_add_int32( m1, SDL_JoystickNumAxes(joy) );
  lo_message_add_int32( m1, SDL_JoystickNumButtons(joy) );
  lo_message_add_int32( m1, SDL_JoystickNumHats(joy) );
  lo_message_add_int32( m1, SDL_JoystickNumBalls(joy) );
  
  lo_send_message_from( t, s, "/joystick/info", m1 ); 
}

int joy_info_handler(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
  printf("sdl2osc: joystick info handler\n");

  send_joystick_info( argv[0]->i );
}

int joy_open_handler(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
  printf("sdl2osc: joystick open handler\n");
  open_joystick( argv[0]->i );
}

int joy_close_handler(const char *path, const char *types, lo_arg **argv, int argc,
		 void *data, void *user_data)
{
  printf("sdl2osc: joystick close handler\n");
  close_joystick( argv[0]->i );
}


 
 
/// end OSC stuff

int str2int(const char* str, int* val)
{
  char* endptr;
  errno = 0;    /* To distinguish success/failure after call */

  *val = strtol(str, &endptr, 10);

  /* Check for various possible errors */
  if ((errno == ERANGE && (*val == LONG_MAX || *val == LONG_MIN))
      || (errno != 0 && *val == 0)) {
    return 0;
  }

  if (endptr == str) {
    return 0;
  }

  return 1;
}

void print_joystick_info(int joy_idx, SDL_Joystick* joy)
{
  printf("Joystick Name:     '%s'\n", SDL_JoystickName(joy_idx));
  printf("Joystick Number:   %2d\n", joy_idx);
  printf("Number of Axes:    %2d\n", SDL_JoystickNumAxes(joy));
  printf("Number of Buttons: %2d\n", SDL_JoystickNumButtons(joy));
  printf("Number of Hats:    %2d\n", SDL_JoystickNumHats(joy));
  printf("Number of Balls:   %2d\n", SDL_JoystickNumBalls(joy));
  printf("\n");
}

void print_help(const char* prg)
{
  printf("Usage: %s [OPTION]\n", prg);
  printf("List available joysticks or test a  joystick.\n");
  printf("This programm uses SDL for doing its test instead of using the raw\n"
         "/dev/input/jsX interface\n");
  printf("\n");
  printf("Options:\n");
  printf("  --help             Print this help\n");
  printf("  --version          Print version number and exit\n");
  printf("  --list             Search for available joysticks and list their properties\n");
  printf("  --event JOYNUM     Display the events that are received from the joystick\n");
  printf("  --osc JOYNUM     Send the events that are received from the joystick\n");
  printf("\n");
  printf("Examples:\n");
  printf("  %s --list\n", prg);
  printf("  %s --test 1\n", prg);
}

int main(int argc, char** argv)
{
  if (argc == 1)
  {
    print_help(argv[0]);
    exit(1);
  }
  
//     printf("argv: %s %s %s %s %s %i\n", argv[0], argv[1], argv[2], argv[3], argv[4], argc );


  // FIXME: We don't need video, but without it SDL will fail to work in SDL_WaitEvent()
  if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
  {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }
  else
  {
    atexit(SDL_Quit);

    if (argc == 2 && (strcmp(argv[1], "--help") == 0 ||
                      strcmp(argv[1], "-h") == 0))
    {
      print_help(argv[0]);
    }
    if (argc == 2 && (strcmp(argv[1], "--version") == 0))
    {
      printf("sdl2osc 0.1.0\n");
      exit(EXIT_SUCCESS);
    }
    else if (argc == 2 && (strcmp(argv[1], "--list") == 0 ||
                           (strcmp(argv[1], "-l") == 0)))
    {
      int num_joysticks = SDL_NumJoysticks();
      if (num_joysticks == 0)
      {
        printf("No joysticks were found\n");
      }
      else
      {
	int joy_idx;
        printf("Found %d joystick(s)\n\n", num_joysticks);
        for(joy_idx = 0; joy_idx < num_joysticks; ++joy_idx)
        {
          SDL_Joystick* joy = SDL_JoystickOpen(joy_idx);
          if (!joy)
          {
            fprintf(stderr, "Unable to open joystick %d\n", joy_idx);
          }
          else
          {
            print_joystick_info(joy_idx, joy);
            SDL_JoystickClose(joy);
          }
        }
      }
    }
    else if (argc == 3 && (strcmp(argv[1], "--event") == 0 ||
                           strcmp(argv[1], "-e") == 0))
    {
      int joy_idx;
      if (!str2int(argv[2], &joy_idx))
      {
        fprintf(stderr, "Error: JOYSTICKNUM argument must be a number, but was '%s'\n", argv[2]);
        exit(1);
      }

      SDL_Joystick* joy = SDL_JoystickOpen(joy_idx);
      if (!joy)
      {
        fprintf(stderr, "Unable to open joystick %d\n", joy_idx);
      }
      else
      {
        print_joystick_info(joy_idx, joy);

        printf("Entering joystick test loop, press Ctrl-c to exit\n");
        int quit = 0;
        SDL_Event event;

        while(!quit && SDL_WaitEvent(&event))
        {
          switch(event.type)
          {
            case SDL_JOYAXISMOTION:
              printf("SDL_JOYAXISMOTION: joystick: %d axis: %d value: %d\n",
                     event.jaxis.which, event.jaxis.axis, event.jaxis.value);
              break;

            case SDL_JOYBUTTONDOWN:
              printf("SDL_JOYBUTTONUP: joystick: %d button: %d state: %d\n",
                     event.jbutton.which, event.jbutton.button, event.jbutton.state);
              break;
            case SDL_JOYBUTTONUP:
              printf("SDL_JOYBUTTONDOWN: joystick: %d button: %d state: %d\n",
                     event.jbutton.which, event.jbutton.button, event.jbutton.state);
              break;

            case SDL_JOYHATMOTION:
              printf("SDL_JOYHATMOTION: joystick: %d hat: %d value: %d\n",
                     event.jhat.which, event.jhat.hat, event.jhat.value);
              break;

            case SDL_JOYBALLMOTION:
              printf("SDL_JOYBALLMOTION: joystick: %d ball: %d x: %d y: %d\n",
                     event.jball.which, event.jball.ball, event.jball.xrel, event.jball.yrel);
              break;

            case SDL_QUIT:
              quit = 1;
              printf("Recieved interrupt, exiting\n");
              break;

            default:
              fprintf(stderr, "Error: Unhandled event type: %d\n", event.type);
          }
        }
        SDL_JoystickClose(joy);

      }
      fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    }
    else if (argc == 2 && (strcmp(argv[1], "--osc") == 0 ||
                           strcmp(argv[1], "-o") == 0))
    {
      int joy_idx;
//       if (!str2int(argv[2], &joy_idx))
//       {
//         fprintf(stderr, "Error: JOYSTICKNUM argument must be a number, but was '%s'\n", argv[2]);
//         exit(1);
//       }
      
      char *port = "57151";
      char *outport = "57120";
      char *ip = "127.0.0.1";

      
      if ( argc == 5 )
	{
	ip = argv[4];
	port = argv[3];
	outport = argv[2];
	}
      else if ( argc == 4 )
	{
	port = argv[3];
	outport = argv[2];
	}
      else if ( argc == 3 )
	{
	outport = argv[2];
	}
  
      init_osc( ip, outport, port );

//       SDL_Joystick* joy = SDL_JoystickOpen(joy_idx);
//       if (!joy)
//       {
//         fprintf(stderr, "Unable to open joystick %d\n", joy_idx);
//       }
//       else
//       {
//         print_joystick_info(joy_idx, joy);

        printf("Entering joystick test loop, press Ctrl-c to exit\n");
//         int quit = 0;
        SDL_Event event;
        lo_timetag lo_now = LO_TT_IMMEDIATE;
        
        while(!done && SDL_WaitEvent(&event))
        {
          switch(event.type)
          {
            case SDL_JOYAXISMOTION:
	      lo_send_from( t, s, lo_now, "/joystick/axis", "iii", event.jaxis.which, event.jaxis.axis, event.jaxis.value );
//               printf("SDL_JOYAXISMOTION: joystick: %d axis: %d value: %d\n",
//                      event.jaxis.which, event.jaxis.axis, event.jaxis.value);
              break;
            case SDL_JOYBUTTONDOWN:
	      lo_send_from( t, s, lo_now, "/joystick/button", "iii", event.jbutton.which, event.jbutton.button, event.jbutton.state );
//               printf("SDL_JOYBUTTONUP: joystick: %d button: %d state: %d\n",
//                      event.jbutton.which, event.jbutton.button, event.jbutton.state);
              break;
            case SDL_JOYBUTTONUP:
	      lo_send_from( t, s, lo_now, "/joystick/button", "iii", event.jbutton.which, event.jbutton.button, event.jbutton.state );
//               printf("SDL_JOYBUTTONDOWN: joystick: %d button: %d state: %d\n",
//                      event.jbutton.which, event.jbutton.button, event.jbutton.state);
              break;

            case SDL_JOYHATMOTION:
	      lo_send_from( t, s, lo_now, "/joystick/hat", "iii", event.jhat.which, event.jhat.hat, event.jhat.value );
//               printf("SDL_JOYHATMOTION: joystick: %d hat: %d value: %d\n",
//                      event.jhat.which, event.jhat.hat, event.jhat.value);
              break;

            case SDL_JOYBALLMOTION:
	      lo_send_from( t, s, lo_now, "/joystick/ball", "iii", event.jball.which, event.jball.ball, event.jball.xrel, event.jball.yrel );
//               printf("SDL_JOYBALLMOTION: joystick: %d ball: %d x: %d y: %d\n",
//                      event.jball.which, event.jball.ball, event.jball.xrel, event.jball.yrel);
              break;

            case SDL_QUIT:
              done = 1;
              printf("Received interrupt, exiting\n");
              break;

            default:
              fprintf(stderr, "Error: Unhandled event type: %d\n", event.type);
          }
	}
//         SDL_JoystickClose(joy);
	  close_all_joysticks();
	  
	  lo_send_from( t, s, lo_now, "/sdl2osc/quit", "s", "nothing more to do, quitting" );
	  lo_server_thread_free( st );
	  lo_address_free( t );

//        }
//       fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    }
    else
    {
      fprintf(stderr, "%s: unknown arguments\n", argv[0]);
      fprintf(stderr, "Try '%s --help' for more informations\n", argv[0]);
    }
  }
}

/* EOF */
