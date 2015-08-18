/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef SDL_KEYBOARD_HPP_INCLUDED
#define SDL_KEYBOARD_HPP_INCLUDED

#if !SDL_VERSION_ATLEAST(2, 0, 0)
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/**
 *  \brief Get a key code from a human-readable name
 *
 *  \return key code, or SDLK_UNKNOWN if the name wasn't recognized
 *
 *  \sa SDL_Keycode
 */
extern SDLKey SDL_GetKeyFromName(const char *name);


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif

#endif /* SDL_KEYBOARD_HPP_INCLUDED */
