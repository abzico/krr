#ifndef krr_window_h_
#define krr_window_h_

#include "SDL.h"
#include <stdbool.h>

#define ORIGINAL_WINDOW_TITLE_BUFFER 50

///
/// Wrapper window managmenet for SDL_Window
/// 
typedef struct {
  /// id represents this window
  Uint32 id;
  
  // display id that this window is on
  int display_id;

  /// window
  SDL_Window* window;

  /// renderer associates with this window
  SDL_Renderer* renderer;

  /// window width (read-only)
  int width;

  /// window height (read_only)
  int height;

  /// whether or not has mouse focus.
  /// internally managed, (read-only)
  bool has_mouse_focus;

  /// whether or not has keyboard focus
  /// internally managed (read-only)
  bool has_keyboard_focus;

  /// whether or not the window is minimized
  /// internally managed (read-only
  bool is_minimized;

  /// whether the window is shown
  bool is_shown;

  /// original title of window
  char original_title[ORIGINAL_WINDOW_TITLE_BUFFER];

  /// flag whether it's full screen or not.
  /// (read-only)
  bool fullscreen;

  /// callback available to be set when window's size has changed
  /// This will be called before repainting occurs.
  ///
  /// \param window_id Window id for this callback
  /// \param new_width New window's width
  /// \param new_height New window/s height
  void (*on_window_resize)(Uint32 window_id, int new_width, int new_height);

  /// handle evant callback that can be set.
  /// This function will be called after internal handling event is carried out.
  ///
  /// It's optional to use this.
  void (*handle_event)(SDL_Event* e, float delta_time);

  /// render callback function for this window
  /// Currently it's not used internally just yet, but it will make code cleaner in user's code.
  ///
  /// It's optional to use this.
  void (*render)(float delta_time);
} krr_WINDOW;

///
/// Create a new krr_WINDOW.
/// Use krr_WINDOW_free() to free krr_WINDOW later.
/// \param title Title of window
/// \param screen_width Screen width
/// \param screen_height Screen height
/// \param window_flags Additional flags to set for window. It will logical OR with SDL_WINDOW_SHOWN. Use 0 if no additional flags to be set.
/// \param renderer_flags Additional flags to set for renderer. It will logical OR with SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC. Use 0 if no additional flags to be set.
/// \return Newly created krr_WINDOW instance on heap.
///
extern krr_WINDOW* krr_WINDOW_new(const char* title, int screen_width, int screen_height, int window_flags, int renderer_flags);

///
/// Initialize allocated krr_WINDOW.
/// Use krr_WINDOW_free_internals() to free krr_WINDOW created via this function.
/// \param [in,out] window Window instance
/// \param title Title of window
/// \param screen_width Screen width
/// \param screen_height Screen height
/// \param window_flags Flags to set for window. It will logical OR with SDL_WINDOW_SHOWN. Use 0 if no additional flags to be set.
/// \param renderer_flags Additional flags to set for renderer. It will logical OR with SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC. Use 0 if no additional flags to be set.
/// \return True if initialize successfully, otherwise return false.
/// 
extern bool krr_WINDOW_init(krr_WINDOW *window, const char* title, int screen_width, int screen_height, int window_flags, int renderer_flags);

///
/// Handle event associated with window.
/// \param window Window instance
/// \param e SDL_Event to handle
/// \param delta_time Delta time since last frame
///
extern void krr_WINDOW_handle_event(krr_WINDOW *window, SDL_Event *e, float delta_time);

///
/// Set window to fullscreen or windowed mode.
///
/// \param window Window instance
/// \param fullscreen True to set to full screen, otherwise false to set to windowed mode
///
extern void krr_WINDOW_set_fullscreen(krr_WINDOW* window, bool fullscreen);

///
/// Make window focused
/// \param window Window to make focused
///
extern void krr_WINDOW_focus(krr_WINDOW* window);

///
/// Free internals of krr_WINDOW.
/// Use this to free memory created by krr_WINDOW via krr_WINDOW_init().
///
extern void krr_WINDOW_free_internals(krr_WINDOW *window);

/// Free memory of krr_WINDOW.
/// Use this to free memory created by krr_WINDOW via krr_WINDOW_new().
///
extern void krr_WINDOW_free(krr_WINDOW *window);

#endif