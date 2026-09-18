#pragma once

#include <SDL3/SDL.h>

void Port_CheckForUpdates(SDL_Window* window);
int Port_Update_Check(void);
int Port_Update_Download(void);
const char* Port_Update_CurrentVersion(void);
const char* Port_Update_LatestVersion(void);
const char* Port_Update_Status(void);
int Port_Update_IsAvailable(void);
