#include "common.h"

KRR_WINDOW* gWindow = NULL;

double common_frameTime = 0.0f;
#ifndef DISABLE_FPS_CALC
int common_frameCount = 0;
double common_frameAccumTime = 0.0f;
double common_avgFPS = 0.0f;
#endif
