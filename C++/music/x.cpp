#include <iostream>
#include <windows.h>
#pragma comment(lib, "winmm.lib")
using namespace std;
enum Scale
{
    Rest = 0, C8 = 108, B7 = 107, A7s = 106, A7 = 105, G7s = 104, G7 = 103, F7s = 102, F7 = 101, E7 = 100,
    D7s = 99, D7 = 98, C7s = 97, C7 = 96, B6 = 95, A6s = 94, A6 = 93, G6s = 92, G6 = 91, F6s = 90, F6 = 89,
    E6 = 88, D6s = 87, D6 = 86, C6s = 85, C6 = 84, B5 = 83, A5s = 82, A5 = 81, G5s = 80, G5 = 79, F5s = 78,
    F5 = 77, E5 = 76, D5s = 75, D5 = 74, C5s = 73, C5 = 72, B4 = 71, A4s = 70, A4 = 69, G4s = 68, G4 = 67,
    F4s = 66, F4 = 65, E4 = 64, D4s = 63, D4 = 62, C4s = 61, C4 = 60, B3 = 59, A3s = 58, A3 = 57, G3s = 56,
    G3 = 55, F3s = 54, F3 = 53, E3 = 52, D3s = 51, D3 = 50, C3s = 49, C3 = 48, B2 = 47, A2s = 46, A2 = 45,
    G2s = 44, G2 = 43, F2s = 42, F2 = 41, E2 = 40, D2s = 39, D2 = 38, C2s = 37, C2 = 36, B1 = 35, A1s = 34,
    A1 = 33, G1s = 32, G1 = 31, F1s = 30, F1 = 29, E1 = 28, D1s = 27, D1 = 26, C1s = 25, C1 = 24, B0 = 23,
    A0s = 22, A0 = 21,
};
enum Voice
{
    X1 = C2, X2 = D2, X3 = E2, X4 = F2, X5 = G2, X6 = A2, X7 = B2,
    L1 = C3, L2 = D3, L3 = E3, L4 = F3, L5 = G3, L6 = A3, L7 = B3,
    M1 = C4, M2 = D4, M3 = E4, M4 = F4, M5 = G4, M6 = A4, M7 = B4,
    H1 = C5, H2 = D5, H3 = E5, H4 = F5, H5 = G5, H6 = A5, H7 = B5,
    LOW_SPEED = 500, MIDDLE_SPEED = 400, HIGH_SPEED = 300,
    _ = 0XFF,Y1= C6, Y2= D6, Y3= E6, Y4= F6, Y5 = G6, Y6 = A6, Y7 = B6,PA = 100000001,R = 1009,s = 1013,P = 1019, S = 1013,
    _4 = 0XFF, _8 = 0XFE, _16 = 0XFD, _20 = 0XFC, _24 = 0XFB, _32 = 0XFA, _2 = 0XF9
};
void Wind()
{
    HMIDIOUT handIe;
    midiOutOpen(&handIe, 0, 0, 0, CALLBACK_NULL);
    int volume = 0x7f , voice = 0x0 , sleep = 350;
    int wind[] =
    {
        0,D4s,_,G5s,A4s,G5,A4s,G5s,A4s,G5s,A4s,
        0,D4,_,G5,A4s,G5,A4s,G5,A4s,D5s,F5,
        0,C4,_,G5,A4s,G5,A4s,G5,A4s,G5s,A4s,
        0,A3s,_,A5s,A4s,G5s,A4s,G5,A4s,700,F5,
        0,G3s,_,D5s,A4s,D5s,A4s,D5s,A4s,F5,A4s,
        0,G3,_,D5s,A4s,D5s,A4s,D5s,A4s,700,A5s,
        0,F3,_,D5s,A4s,D5s,F5,G5,100,F5,G5,F5,D5s,A4s,
        0,A3s,_,D5s,A4s,D5s,F5,G5,100,F5,G5,F5,D5s,F5,

        0,D3s,_,G5,0,A3s,_,A4s,0,G4,_,G5,A4s,0,D3s,_,G5,0,A3s,_,A4s,0,F4,_,G5s,A4s,
        0,D3s,G5,0,A3s,_,A4s,0,F4,_,G5,A4s,0,D3,_,G5,0,A3s,_,A4s,0,D4s,_,D5s,F5,
        0,C3,_,G5,0,G3,_,A4s,0,D4s,_,G5,A4s,0,C3,_,G5,0,G3,_,A4s,0,D4s,_,G5,G5s,
        0,A2s,A5s,0,G3,_,A4s,0,D4s,700,D6s,0,A2s,_,A5s,0,G3,100,G5s,A5s,G5s,0,D4s,G5,F5,
        0,G2s,_,D5s,0,D3s,_,A4s,0,C4,_,D5s,0,D3s,_,A4s,0,G2s,_,D5s,0,D3s,_,A4s,0,C4,700,A5s,
        0,G2,88,G5,A5s,500,D6s,0,D3s,0,A3s,_,A5s,0,D3s,_,G5s,0,G2,_,G5,0,D3s,_,F5,0,A3s,_,D5s,0,D3s,_,A4s,
        0,F2,_,D5s,0,C3,_,A4s,0,G3,_,D5s,F5,0,F2,_,G5,0,C3,_,100,F5,G5,F5,0,G3,_,D5s,A4s,
        0,A2s,_,D5s,0,G3,_,A4s,0,G3s,_,D5s,0,G3,_,A5s,0,A2s,_,G5s,0,G3,_,100,G5,G5s,G5,0,D4,_,F5,D5s,

        0,F2s,_,A5s,0,C3s,_,C5s,0,A3s,_,A5s,0,C3s,_,C5s,0,F2s,_,A5s,0,C3s,_,C5s,0,G3s,_,B5,0,C3s,_,C5s,
        0,F2,_,A5s,0,D3s,_,D5s,0,G3s,_,A5s,0,D3s,_,D5s,0,A2s,_,A5s,0,G3s,_,D5,0,D4,_,F5s,0,G3s,_,G5s,
        0,D2s,_,A5s,0,A2s,_,D5s,0,F3s,_,A5s,0,A2s,_,D5s,0,D2s,_,A5s,0,A2s,_,D5s,0,F3s,_,B5,0,A2s,_,D5s,
        0,C3s,_,C6s,8,G3s,_,F5,0,E4,_,F6s,G3s,0,F3s,_,C6s,0,C4s,_,100,B5,C6s,B5,0,A4s,_,A5s,G5s,
        0,B2,_,F5s,0,F3s,_,C5s,0,B3,_,F5s,0,F3s,_,C5s,0,C4s,_,F5s,0,F3s,_,C5s,0,D4s,_,700,C6s,
        0,A2s,88,F5s,A5s,C6s,F6s,_,F3s,0,C4s,_,C6s,0,F3,_,B5,0,A2s,_,A5s,0,F3s,_,100,G5s,A5s,G5s,0,C4s,_,F5s,0,_,F3s,_,C5s,
        0,G2s,_,F5s,0,F3s,_,C5s,0,B3,_,F5s,0,D4s,_,G5s,0,A4s,_,A5s,100,G5s,A5s,G5s,_,F5s,C5s,
        0,C3s,_,F5s,0,B3,_,C5s,0,F4s,_,F5s,C6s,B5,100,A5s,B5,A5s,_,G5s,F5s,
        0,F3s,88,F4s,A4s,C5s,F5s,500,C4s,A4s,C4s,A4s,700,C4s,2800,F3s,
    };
    for (auto i : wind)
    {
        switch (i)
        {
            case 500:
            case 300:
            case 400:
                sleep = i;
                continue;
            case 0:
                sleep = 0;
                continue;
            case 88:
                sleep = 88;
                continue;
            case 100:
                sleep = 100;
                continue;
            case _:
                sleep = 350;
                continue;
        }
        voice = (volume << 16) + ((i) << 8) + 0x90;
        midiOutShortMsg(handIe, voice);
        cout << voice << "\n";
        Sleep(sleep);
    }
    midiOutClose(handIe);
}
int main(void)
{
    Wind();
    return 0;
}