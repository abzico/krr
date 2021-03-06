#include "krr/ui/button.h"
#include <stdbool.h>

KRR_BUTTON* KRR_BUTTON_Create(void)
{
  KRR_BUTTON* ptr = malloc(sizeof(KRR_BUTTON));
  ptr->state = KRR_BUTTON_MOUSE_OUT;

  return ptr;
}

void KRR_BUTTON_HandleEvent(KRR_BUTTON* button, SDL_Event* e, SDL_Rect buttonRect)
{
  // check for mouse event
  if (e->type == SDL_MOUSEMOTION ||
      e->type == SDL_MOUSEBUTTONDOWN ||
      e->type == SDL_MOUSEBUTTONUP)
  {
    // get mouse's position
    int x, y;
    SDL_GetMouseState(&x, &y);

    // check if mouse is in button
    bool inside = false;

    if (x >= buttonRect.x &&
        x <= buttonRect.x + buttonRect.w &&
        y >= buttonRect.y &&
        y <= buttonRect.y + buttonRect.h)
    {
      inside = true;
    }

    // mouse is outside button
    if (!inside)
    {
      button->state = KRR_BUTTON_MOUSE_OUT;	
    }
    // mouse is inside button
    else {
      // set mouse state
      switch (e->type)
      {
        case SDL_MOUSEMOTION:
          button->state = KRR_BUTTON_MOUSE_OVER_MOTION;
          break;	
        case SDL_MOUSEBUTTONDOWN:
          button->state = KRR_BUTTON_MOUSE_DOWN;
          break;
        case SDL_MOUSEBUTTONUP:
          button->state = KRR_BUTTON_MOUSE_UP;
          break;
      }
    }
  }
}

void KRR_BUTTON_Free(KRR_BUTTON* ptr)
{
  free(ptr);
  ptr = NULL;
}
