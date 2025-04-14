#include <windows.h>
#include <stdio.h>

int main(void)
{
    int interval;
    BOOL running = 0;

    INPUT input[2] = {0};
    // 第一个输入事件:鼠标按下
    input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    // 第二个输入事件:鼠标松开
    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    printf("请输入点击间隔(毫秒):");
    scanf("%d", &interval);
    if (interval > 0)
    {
        while (1)
        {
            if (GetAsyncKeyState(VK_F2) & 0x8000)
            {
                running = !running;
                Sleep(200);
            }
            if (running)
            {
                SendInput(2, input, sizeof(INPUT));
                Sleep(interval);
            }
        }
    }
    else
    {
        while (1)
        {
            if (GetAsyncKeyState(VK_F2) & 0x8000)
            {
                running = !running;
                Sleep(200);
            }
            if (running)
            {
                SendInput(2, input, sizeof(INPUT));
            }
        }
    }
    return 0;
}
