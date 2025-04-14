#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <iostream>
#include <unistd.h>

int main() {
  // 打开与X服务器的连接
  Display* display = XOpenDisplay(nullptr);
  if (!display) {
    std::cerr << "无法连接到X服务器" << std::endl;
    return 1;
  }

  // 获取根窗口
  Window root = DefaultRootWindow(display);

  // 获取当前活动窗口
  Window focus_window;
  int revert_to;
  XGetInputFocus(display, &focus_window, &revert_to);

  // 监听键盘事件
  XEvent event;
  XSelectInput(display, root, KeyPressMask);

  bool is_topmost = false;
  while (true) {
    XNextEvent(display, &event);
    if (event.type == KeyPress) {
      KeySym key = XLookupKeysym(&event.xkey, 0);

      // 检测是否按下Ctrl+T
      if (key == XK_t && (event.xkey.state & ControlMask)) {
        is_topmost = !is_topmost;

        // 设置或取消窗口置顶
        Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
        Atom wm_state_above = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);

        XClientMessageEvent xclient;
        xclient.type = ClientMessage;
        xclient.window = focus_window;
        xclient.message_type = wm_state;
        xclient.format = 32;
        xclient.data.l[0] = is_topmost ? 1 : 0; // 1表示添加，0表示移除
        xclient.data.l[1] = wm_state_above;
        xclient.data.l[2] = 0;
        xclient.data.l[3] = 0;
        xclient.data.l[4] = 0;

        XSendEvent(display, root, False, SubstructureRedirectMask | SubstructureNotifyMask, (XEvent*)&xclient);
        XFlush(display);

        std::cout << "Window topmost: " << is_topmost << std::endl;
      }
    }

    // 稍微延迟一下，避免CPU占用过高
    usleep(10000);
  }

  // 关闭与X服务器的连接
  XCloseDisplay(display);
  return 0;
}