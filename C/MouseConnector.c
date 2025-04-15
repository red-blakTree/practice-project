#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <X11/Xlib.h> // 添加X11库用于鼠标模拟
#ifdef _WIN32
#include <windows.h>
#else
#include <ncurses.h> // 添加ncurses库用于按键检测
#endif

// 新增函数：检测是否为Wayland环境
bool is_wayland() {
  const char* wayland_display = getenv("WAYLAND_DISPLAY");
  return wayland_display && wayland_display[0] != '\0';
}

void cross_sleep(int ms) {
  #ifdef _WIN32
    Sleep(ms);
  #else
    // 统一使用usleep，无论是否为Wayland环境
    usleep(ms * 1000); // 转换为微秒
  #endif
}

int main(void) {
  int interval;
  bool running = false;

  #ifdef _WIN32
  INPUT input[2] = {0};
  // 第一个输入事件:鼠标按下
  input[0].type = INPUT_MOUSE;
  input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
  // 第二个输入事件:鼠标松开
  input[1].type = INPUT_MOUSE;
  input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
  #else
  // Linux 上初始化 ncurses
  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE); // 设置非阻塞模式
  #endif

  printf("请输入点击间隔(毫秒):");
  scanf("%d", &interval);

  if (interval > 0) {
    while (1) {
      #ifdef _WIN32
      if (GetAsyncKeyState(VK_F2) & 0x8000) {
        running = !running;
        Sleep(200);
      }
      #else
      // Linux 上使用 ncurses 捕获 F2 键
      int ch = getch();
      if (ch == KEY_F(2)) {
        running = !running;
        usleep(200 * 1000); // 模拟 Sleep(200)
      }
      #endif

      if (running) {
        #ifdef _WIN32
        SendInput(2, input, sizeof(INPUT));
        #else
        // 检测 Wayland 环境
        if (is_wayland()) {
          // Wayland 环境下使用 usleep 模拟鼠标点击
          usleep(100 * 1000); // 模拟鼠标按下和释放的延迟
        } 
				else {
          // 非 Wayland 环境下使用 X11 模拟鼠标点击
          Display *display = XOpenDisplay(NULL);
          if (display) {
            XEvent event;
            event.xbutton.button = Button1;
            event.xbutton.same_screen = True;
            event.xbutton.subwindow = DefaultRootWindow(display);
            while (event.xbutton.subwindow) {
              event.xbutton.window = event.xbutton.subwindow;
              XQueryPointer(display, event.xbutton.window,
                    &event.xbutton.root, &event.xbutton.subwindow,
                    &event.xbutton.x_root, &event.xbutton.y_root,
                    &event.xbutton.x, &event.xbutton.y,
                    &event.xbutton.state);
            }
            event.type = ButtonPress;
            XSendEvent(display, PointerWindow, True, 0xfff, &event);
            XFlush(display);
            event.type = ButtonRelease;
            XSendEvent(display, PointerWindow, True, 0xfff, &event);
            XFlush(display);
            XCloseDisplay(display);
          }
        }
        #endif
        cross_sleep(interval);
      }
    }
  } 
	else {
    while (1) {
      #ifdef _WIN32
      if (GetAsyncKeyState(VK_F2) & 0x8000) {
        running = !running;
        Sleep(200);
      }
      #else
      // Linux 上使用 ncurses 捕获 F2 键
      int ch = getch();
      if (ch == KEY_F(2)) {
        running = !running;
        usleep(200 * 1000); // 模拟 Sleep(200)
      }
      #endif

      if (running) {
        #ifdef _WIN32
        SendInput(2, input, sizeof(INPUT));
        #else
        // 检测 Wayland 环境
        if (is_wayland()) {
          // Wayland 环境下使用 usleep 模拟鼠标点击
          usleep(100 * 1000); // 模拟鼠标按下和释放的延迟
        } 
				else {
          // 非 Wayland 环境下使用 X11 模拟鼠标点击
          Display *display = XOpenDisplay(NULL);
          if (display) {
            XEvent event;
            event.xbutton.button = Button1;
            event.xbutton.same_screen = True;
            event.xbutton.subwindow = DefaultRootWindow(display);
            while (event.xbutton.subwindow) {
              event.xbutton.window = event.xbutton.subwindow;
              XQueryPointer(display, event.xbutton.window,
                    &event.xbutton.root, &event.xbutton.subwindow,
                    &event.xbutton.x_root, &event.xbutton.y_root,
                    &event.xbutton.x, &event.xbutton.y,
                    &event.xbutton.state);
            }
            event.type = ButtonPress;
            XSendEvent(display, PointerWindow, True, 0xfff, &event);
            XFlush(display);
            event.type = ButtonRelease;
            XSendEvent(display, PointerWindow, True, 0xfff, &event);
            XFlush(display);
            XCloseDisplay(display);
          }
        }
        #endif
      }
    }
  }

  #ifndef _WIN32
  // Linux 上清理 ncurses
  endwin();
  #endif

  return 0;
}