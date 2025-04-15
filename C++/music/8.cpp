#include <iostream>
#include <Windows.h>
#pragma comment(lib,"winmm.lib")
using namespace std;
const int eight_note = 500;
enum Scale
{
    Rest = -1, C8 = 108, B7 = 107, A7s = 106, A7 = 105, G7s = 104, G7 = 103, F7s = 102, F7 = 101, E7 = 100,
    D7s = 99, D7 = 98, C7s = 97, C7 = 96, B6 = 95, A6s = 94, A6 = 93, G6s = 92, G6 = 91, F6s = 90, F6 = 89,
    E6 = 88, D6s = 87, D6 = 86, C6s = 85, C6 = 84, B5 = 83, A5s = 82, A5 = 81, G5s = 80, G5 = 79, F5s = 78,
    F5 = 77, E5 = 76, D5s = 75, D5 = 74, C5s = 73, C5 = 72, B4 = 71, A4s = 70, A4 = 69, G4s = 68, G4 = 67,
    F4s = 66, F4 = 65, E4 = 64, D4s = 63, D4 = 62, C4s = 61, C4 = 60, B3 = 59, A3s = 58, A3 = 57, G3s = 56,
    G3 = 55, F3s = 54, F3 = 53, E3 = 52, D3s = 51, D3 = 50, C3s = 49, C3 = 48, B2 = 47, A2s = 46, A2 = 45,
    G2s = 44, G2 = 43, F2s = 42, F2 = 41, E2 = 40, D2s = 39, D2 = 38, C2s = 37, C2 = 36, B1 = 35, A1s = 34,
    A1 = 33, G1s = 32, G1 = 31, F1s = 30, F1 = 29, E1 = 28, D1s = 27, D1 = 26, C1s = 25, C1 = 24, B0 = 23,
    A0s = 22, A0 = 21
};
enum Voice
{
    _ = 0XFF
};
void call_of_silence() {
    HMIDIOUT handle;
    midiOutOpen(&handle, 0, 0, 0, CALLBACK_NULL);
    int volume = 0x7f;
    int voice = 0x0;
    int sleep = eight_note;
    int notes[] =
    {
        F4s, _, A4, _,
        D2, A2, D3, F4s, C5s, _, _, _, _, -1, E2, B2, E3, E4, B4, _, _, G4s, _, A4, _, -1,
        F2s, C3s, F3s, E4, B4, _, _, _, _, _, _, C2s, G2s, C3s, E5, _, G4s, _,
        D2, A2, D3, D4, F4s, _, _, _, _, E2, B2, E3, E4, G4s, _, _, F4s, _, B4, _,
        F2s, C3s, F3s, _, A4, _, _, _, _, _, F4s, _, A4, _,
        D2, A2, D3, F4s, C5s, _, _, _, _, E2, B2, E3, E4, B4, _, _, G4s, _, A4, _,
        F2s, C3s, F3s, E4, B4, _, _, _, _, _, _, C2s, G2s, C3s, E5, _, B4, _,
        D2, A2, D3, D4, A4, _, _, _, _, E2, B2, E3, _, _, F4s, _, B4, _,
        F2s, C3s, F3s, _, A4, _, _, _, _, _, _, _,
        _, _, _, _, _, _, F3s, _, C4s, _,
        D2, A2, D3, C4s, _, _, A2, D3, 250, _, _, C4s, _, G3s, _, eight_note, E2, B2, E3, _, _, B2, E3, 250, _, _, F3s, _, A3, _,
        F2s, C3s, F3s, 1000, _, C3s, F3s, _, C3s, F3s, _, C3s, eight_note, F3s, _, C4s, _,
        D2, A2, D3, C4s, _, _, A2, D3, 250, _, _, C4s, _, G4s, _, eight_note, E2, B2, E3, _, _, B2, E3, 250, _, _, A4, _, C4s, _,
        F2s, C3s, F3s, 1000, _, C3s, F3s, _, C3s, F3s, _, C3s, F3s, eight_note, F4s, _, C5s, _,
        D2, A2, D3, C5s, _, _, A2, D3, 250, _, _, C5s, _, G4s, _, eight_note, E2, B2, E3, _, _, B2, E3, 250, _, _, A4, _, E4, _,
        F2s, C3s, F3s, 1000, _, C3s, F3s, _, C3s, F3s, _,C3s, F3s, 250, E4, _, _, C4s, _, E4, _,
        eight_note, D2, A2, D3, _, F4s, _, A2, D3, _, _, E2, B2, E3, _, _, B2, E3, B4, _, 250, A4, _, B4, _,
        F2s, C3s, F3s, eight_note, _, _, C3s, F3s, _, A4, 250, _, F4s,_,1000, C3s, F3s, _, C3s, F3s, _,
        F2s, C3s, F3s, _, _, _, eight_note, C5s, C4s, _, B4, B3, _,
        D2, A2, D3, B3, B4, _, A3, A4, _, D3, F3s, A3, A4, _, 250, D3, E4, E5, _, A2, _, eight_note, E2, B2, E3, A3, A4, _, _, E3, G3s, G4s, _, 250, E3, A3, A4, _, B2, _,
        F2s, F3s, _, _, C3s, _, F3s, _, A3, _, _, C3s, _, F3s, _, G3s, _, _, C3s, _, F3s, _, F2s, F3s, C5s, C4s, _, C3s, _, F2s, F3s, B3, B4, _, C3s, _,
        D2, A2, D3, B3, B4, _, _, A3, A4, _, D3, F3s, _, E3, E4, _, D3, _, A2, C5s, C4s, _, D2, _, E2, B2, E3, B3, B4, _, _, _, _, E3, G3s, B3, C4s, C5s, _, _, E3, A3, A4, _, B2, _,
        250,
        F2s, F3s, _, _, C3s, _, F3s, _, A3, _, _, C3s, _, F3s, _, G3s, _, _, C3s, _, F3s, _, F2s, F3s, C5s, C4s, _, C3s, _, F2s, F3s, B3, B4, _, C3s, _,
        D2, A2, D3, B3, B4, _, _, A3, A4, _, _, D3, F3s, A3, A4, _, _, D3, E4, E5, _, A2, _, E2, B2, E3, E4, E5, _, _, E4, E5, _, _, E3, G3s, B3, B4, _, _, E3, A3, A4, _, B2, _,
        F2s, F3s, _, _, C3s, _, F3s, _, A3, _, _, C3s, _, F3s, _, G3s, _, _, C3s, _, F3s, _, F3s, F4s, _, F2s, F3s, _, C3s, A3, A4, _, F2s, _,
        D2, A2, D3, B3, B4, _, _, _, _, D3, F3s, A3, B3, B4, _, _, D3, A3, A4, _, A2, _, E2, B2, E3, B3, B4, _, _, C4s, C5s, _, _, E3, G3s, B3, B4, _, _, E3, B3, B4, _, B2, _,
        eight_note, F2s, _, C3s, A3, A4, _, F3s, _, C3s, _, G3s, _, C3s, A5, _, F3s, G5s, _, 250, C3s, C5s, _, C5s, F5s, _,
        eight_note, D3, _, A3, _, D4,_, 250, A3, F5s, _, E5, G5s, _, eight_note, E3, _, B3, _, E4, _, 250, B3, B5, _, A5, _,
        eight_note, F3s, _, C4s, _, F4s, _, C4s, _, G4s, _, C4s, A5, _, F4s, G5s, _, 250, C4s, C5s, _, C5s, F5s, _,
        eight_note, D3, _, A3, _, D4, C5s, F5s, _, A3, _, E3, _, B3, A5, _, E4, G5s, _, 250, B3, E5, _, A4, _,
        eight_note, F3s, _, C4s, _, F4s, _, C4s, _, G4s, _, C4s, A5, _, F4s, G5s, _, 250, C4s, A5, _, G5s, B5, _,
        D3, _, _, A3, _, _, D4, _, _, A3, C6s, _, C6s, _, E3, _, _, B3, _, _, E4, _, _, B3, A5, _, G5s, _,
        eight_note, F3s, _, C4s, _, F4s, _, G4s, _, A4, _, _, _, _,
        _, _, 1000, F4s, F5s, _, _, _,
    };
    for (auto i : notes)
    {
        if (i == 1000 || i == 250 || i == eight_note)
        {
            sleep = i;
            continue;
        }
        if (i == _)
        {
            Sleep(sleep);
            continue;
        }
        voice = (volume << 16) + (i << 8) + 0x94;
        midiOutShortMsg(handle, voice);
    }
    midiOutClose(handle);
}
int main()
{
    call_of_silence();
    return 0;
}