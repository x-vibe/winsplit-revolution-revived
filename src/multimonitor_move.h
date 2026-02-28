#ifndef __MOVE_WINDOW_H__
#define __MOVE_WINDOW_H__

enum DIRECTION {
  LEFT_SCREEN,
  RIGHT_SCREEN,
  UP_SCREEN,  // To be implemented in the future
  DOWN_SCREEN // To be implemented in the future
};

extern void MoveWindowToDirection(HWND hwnd, DIRECTION sens);

#endif // __MOVE_WINDOW_H__
