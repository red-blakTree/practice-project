#include <iostream>
#include <Windows.h>
#pragma comment(lib,"winmm.lib")
using namespace std;
#include <thread>
#include <map>
#include <algorithm>
HMIDIOUT handle;
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
    _ = 0XFF,Y1= C6, Y2= D6, Y3= E6, Y4= F6, Y5 = G6, Y6 = A6, Y7 = B6,PA = 100000001,R = 1009,s = 1013,P = 1019,SPEED = 150
};
const int _V[15]={0,1033,1039,1049,1051,1061,1063,1069,1087,1091,1093,1097,1103,1109,0};
map <int,int> _mp;
//一些全局变量(用于Music函数)
int volume = 0x7F ; // 音量
int stime  = SPEED  ; // 音时
int cnt = 0; // 小节计时器

//双结构体记谱
struct v_spo {
    int _v1,_v2,_v3;
};
struct msc {
    v_spo v0,v1,v2;    
    int C;
};
//乐谱
msc DSHH_music[] = 
{
    {{G5s,R,R},{s,s,s},{B3,F4s,A4s},_V[2]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{C4s,G4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{D4s,A4s,C5s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{F3s,C4s,A4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{R,R,R},{B3,F4s,A4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{C4s,G4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{R,R,R},{D4s,A4s,C5s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{F3s,C4s,A4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{A4s,R,R},{B2,F3s,A3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{A4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{A4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{A4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{A4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{A4s,R,R},{C3s,G3s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{A4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{A4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{G4s,R,R},{D3s,A3s,C4s},_V[4]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{G4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{G4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D6s,R,R},{G4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{G4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{G4s,R,R},{F2s,C2s,A2s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5s,R,R},{G4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{G4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{A4s,R,R},{B2,F3s,A3s},_V[5]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{A4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{A4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{A4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{A4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{A4s,R,R},{C3s,G3s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{A4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{A4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{G4s,R,R},{D3s,A3s,C4s},_V[6]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{G4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{G4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D6s,R,R},{G4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{G4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{G4s,R,R},{F2s,C2s,A2s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A4s,R,R},{s,s,s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{A4s,D5s,R},{F3s,A3s,R},{B2,R,R},_V[7]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,R,R},{C4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{D4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5,R,R},{R,R,R},{C3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5s,R,R},{F3,G3s,C4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{B4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{A4s,R,R},{F3s,A3s,R},{D3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5,R,R},{C4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{D4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,R,R},{F3s,G3s,C4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{D5s,R,R},{F3s,A3s,R},{B2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,R,R},{C4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5,R,R},{D4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{R,R,R},{C3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{F3,G3s,C4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{F3s,A3s,R},{D3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{C4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{s,s,s},{D4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A4s,R,R},{F3s,A3s,C4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{A4s,D5s,R},{F3s,A3s,R},{B2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,R,R},{C4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{D4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5,R,R},{R,R,R},{C3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5s,R,R},{F3,G3s,C4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{B4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{A4s,R,R},{F3s,A3s,R},{D3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5,R,R},{C4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{D4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,R,R},{F3s,G3s,C4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{D5s,R,R},{F3s,A3s,R},{B2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,R,R},{C4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5,R,R},{D4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{R,R,R},{C3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5s,R,R},{F3,G3s,C4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{F3s,A3s,R},{D3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{C4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{s,s,s},{D4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5s,R,R},{F3s,A3s,C4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{F4s,F5s,R},{s,s,s},{B1,F2s,B2},_V[9]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F3s,A3s,C4s},0}, 
        {{s,s,s},{R,R,R},{s,s,s},0}, {{R,R,R},{R,R,R},{B2,R,R},0}, {{C5s,F5,R},{R,R,R},{F3s,A3s,C4s},0}, {{R,R,R},{R,R,R},{s,s,s},0}, 
        {{D5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,F5,R},{R,R,R},{C2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F4s,F5s,R},{R,R,R},{F3s,A3s,C4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{s,s,s},{R,R,R},{D2s,A2s,D3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,F5,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F3s,A3s,F4s},0}, 
        {{D5s,R,R},{R,R,R},{s,s,s},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, {{C5s,F5,R},{R,R,R},{F3s,A3s,F4s},0}, {{R,R,R},{R,R,R},{s,s,s},0}, 
        {{D5s,F5s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,F5,R},{R,R,R},{F2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A4s,C5s,R},{R,R,R},{F3s,A3s,F4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5s,R,R},{R,R,R},{F2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{A4s,C5s,R},{R,R,R},{B1,F2s,B2},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4s,R,R},{R,R,R},{F3s,R,R},0}, {{R,R,R},{R,R,R},{A3s,C4s,R},0}, 
        {{F4s,R,R},{R,R,R},{s,s,s},0}, {{R,R,R},{R,R,R},{B2,R,R},0}, {{D5s,R,R},{R,R,R},{F3s,A3s,C4s},0}, {{R,R,R},{R,R,R},{s,s,s},0}, 
        {{A4s,C5s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4s,R,R},{R,R,R},{C2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F4,G4s,R},{R,R,R},{F3,A3s,C4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4s,A4s,R},{R,R,R},{C2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{R,R,R},{D2s,A2s,D3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{s,s,s},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F3s,A3s,C4s},0}, 
        {{R,R,R},{R,R,R},{s,s,s},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, {{R,R,R},{R,R,R},{F3s,A3s,C4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5s,R,R},{R,R,R},{F3s,A3s,C4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5,R,R},{R,R,R},{F4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{F5s,R,R},{R,R,R},{B1,F2s,B2},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5,R,R},{R,R,R},{F3s,A3s,C4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F3s,A3s,C4s},0}, {{F5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,R,R},{R,R,R},{C3s,C4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{R,R,R},{D2s,A2s,D3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5,R,R},{R,R,R},{F3s,A3s,C4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F3s,A3s,C4s},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,A6s,R},{R,R,R},{F2s,F3s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{B5,B6,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{B3,R,R},{F3s,D4s,F4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,F6s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,F6s,R},{s,s,s},{C5s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,A6s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,G6s,R},{R,R,R},{D5s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,F6s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,F6s,R},{R,R,R},{F4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{s,s,s},{F1s,R,R},{F2s,R,R},_V[10]}, {{R,R,R},{F1s,R,R},{F2s,R,R},0}, {{R,R,R},{F1s,R,R},{F2s,R,R},0}, {{R,R,R},{F1s,R,R},{F2s,R,R},0}, 
        {{R,R,R},{F1s,R,R},{F2s,R,R},0}, {{R,R,R},{F1s,R,R},{F2s,R,R},0}, {{R,R,R},{F1s,R,R},{F2s,R,R},0}, {{R,R,R},{F1s,R,R},{F2s,R,R},0}, 
        {{R,R,R},{F1s,R,R},{F2s,R,R},0}, {{R,R,R},{F1s,R,R},{F2s,R,R},0}, {{R,R,R},{F1s,R,R},{F2s,R,R},0}, {{R,R,R},{F1s,R,R},{F2s,R,R},0}, 
        {{A4s,A5s,R},{s,s,s},{F1s,F2s,R},0}, {{s,s,s},{R,R,R},{R,R,R},0}, {{C5s,C6s,R},{R,R,R},{s,s,s},0}, {{A4s,A5s,R},{R,R,R},{R,R,R},0}, 
    {{G4s,G5s,R},{s,s,s},{B1,F2s,B2},_V[11]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4s,F5s,R},{R,R,R},{B2,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, 
        {{D4s,D5s,R},{R,R,R},{F3s,A3s,R},0}, {{F4s,F5s,R},{R,R,R},{s,s,s},0}, {{s,s,s},{R,R,R},{R,R,R},0}, {{G4s,G5s,R},{R,R,R},{F3s,A3s,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2s,R,R},0}, {{R,R,R},{R,R,R},{F3,G3s,C4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A4s,A5s,R},{R,R,R},{C3s,R,R},0}, {{R,R,R},{R,R,R},{F3,G3s,C4s},0}, {{C5s,C6s,R},{R,R,R},{R,R,R},0}, {{A4s,A5s,R},{R,R,R},{R,R,R},0}, 
    {{G4s,G5s,R},{s,s,s},{D2s,A2s,D3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4s,F5s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, 
        {{C4s,C5s,R},{R,R,R},{F3s,A3s,C4s},0}, {{F4s,F5s,R},{R,R,R},{s,s,s},0}, {{s,s,s},{R,R,R},{R,R,R},0}, {{F4s,F5s,R},{R,R,R},{F3s,A3s,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1s,R,R},0}, {{R,R,R},{A2s,R,R},{F3s,A3s,C4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A4s,A5s,R},{R,R,R},{s,s,s},0}, {{R,R,R},{s,s,s},{F3s,A3s,C4s},0}, {{C5s,C6s,R},{R,R,R},{R,R,R},0}, {{A4s,A5s,R},{R,R,R},{R,R,R},0}, 
    {{G4s,G5s,R},{s,s,s},{B1,F2s,B2},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{B2,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, 
        {{A4s,A5s,R},{R,R,R},{F3s,A3s,R},0}, {{C5s,C6s,R},{R,R,R},{s,s,s},0}, {{s,s,s},{R,R,R},{R,R,R},0}, {{C5s,C6s,R},{R,R,R},{F3s,A3s,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{D5s,D6s,R},{R,R,R},{C2s,R,R},0}, {{R,R,R},{R,R,R},{F3,G3s,C4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5s,C6s,R},{R,R,R},{F3,G3s,C4s},0}, {{B4,B5,R},{R,R,R},{R,R,R},0}, {{A4s,A5s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{s,s,s},{D2s,A2s,D3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{s,s,s},{G6s,R,R},{R,R,R},0}, {{R,R,R},{A6s,R,R},{D3s,R,R},0}, 
        {{R,R,R},{C7s,R,R},{F3s,A3s,C4s},0}, {{R,R,R},{D7s,R,R},{R,R,R},0}, {{R,R,R},{A6s,R,R},{R,R,R},0}, {{R,R,R},{G6s,R,R},{F3s,A3s,C4s},0}, 
        {{R,R,R},{A6s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F1s,F2s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A4s,A5s,R},{s,s,s},{F3s,A3s,C4s},0}, {{R,R,R},{R,R,R},{F2s,F3s,R},0}, {{C5s,C6s,R},{R,R,R},{R,R,R},0}, {{A4s,A5s,R},{R,R,R},{R,R,R},0}, 
    {{G4s,G5s,R},{s,s,s},{G1,F2s,G2},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4s,F5s,R},{R,R,R},{G2,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, 
        {{D4s,D5s,R},{R,R,R},{F4s,A4s,R},0}, {{F4s,F5s,R},{R,R,R},{s,s,s},0}, {{s,s,s},{R,R,R},{R,R,R},0}, {{G4s,G5s,R},{R,R,R},{F4s,A4s,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2s,R,R},0}, {{R,R,R},{R,R,R},{F3,G3s,C4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A4s,A5s,R},{R,R,R},{C3s,R,R},0}, {{R,R,R},{R,R,R},{F3,G3s,C4s},0}, {{C5s,C6s,R},{R,R,R},{R,R,R},0}, {{A4s,A5s,R},{R,R,R},{R,R,R},0}, 
    {{G4s,G5s,R},{s,s,s},{D2s,A2s,D3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4s,F5s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, 
        {{C4s,C5s,R},{R,R,R},{F3s,A3s,C4s},0}, {{F4s,F5s,R},{R,R,R},{s,s,s},0}, {{s,s,s},{R,R,R},{R,R,R},0}, {{F4s,F5s,R},{R,R,R},{F3s,A3s,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1s,R,R},0}, {{R,R,R},{A2s,R,R},{F3s,A3s,C4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F4s,F5s,R},{R,R,R},{s,s,s},0}, {{F4,F5,R},{s,s,s},{F3s,A3s,C4s},0}, {{D4s,D5s,R},{R,R,R},{R,R,R},0}, {{F4,F5,R},{R,R,R},{R,R,R},0}, 
    {{D4s,D5s,R},{s,s,s},{B1,F2s,B2},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{B2,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, 
        {{G4s,G5s,R},{R,R,R},{F3s,A3s,R},0}, {{R,R,R},{R,R,R},{s,s,s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F3s,A3s,R},0}, 
        {{F4,F5,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2s,R,R},0}, {{D4s,D5s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F4,F5,R},{R,R,R},{F3,G3s,C4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4,C5s,F5},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{R,R,R},{B1,B2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4s,F5s,R},{R,R,R},{F3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F4s,C5s,F5s},{R,R,R},{C2s,C3s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{G2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F4s,A4s,R},{R,R,R},{D2s,A2s,D3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, {{R,R,R},{R,R,R},{F3s,R,R},0}, 
        {{s,s,s},{R,R,R},{A3s,R,R},0}, {{R,R,R},{R,R,R},{C4s,R,R},0}, {{R,R,R},{R,R,R},{D4s,R,R},0}, {{R,R,R},{R,R,R},{F4s,R,R},0}, 
    //Part 2
{{G5s,R,R},{B3,F4s,A4s},{s,s,s},_V[5]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{C4s,G4s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{D4s,A4s,C5s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{F3s,C4s,A4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{B3,F4s,A4s},{s,s,s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{C4s,G4s,R},{A4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{A4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{A4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{D4s,F4s,R},{G4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{G4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{G4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{G4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{G4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{G4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{G4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4s,R,R},{s,s,s},{R,R,R},0}, {{G4s,R,R},{R,R,R},{s,s,s},0}, 
    {{A4s,R,R},{A2s,D3s,F3s},{s,s,s},_V[7]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G4s,R,R},{R,R,R},{R,R,R},0}, {{F4s,R,R},{R,R,R},{R,R,R},0}, 
        {{F4s,R,R},{D5s,F5s,A5s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D4s,R,R},{s,s,s},{R,R,R},0}, {{F4,R,R},{R,R,R},{R,R,R},0}, 
        {{F4s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4,R,R},{C3s,F3,G3s},{R,R,R},0}, {{D4s,R,R},{R,R,R},{R,R,R},0}, 
        {{C4s,R,R},{A5s,G5s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A3s,R,R},{C3s,R,R},{R,R,R},0}, {{C4s,R,R},{R,R,R},{R,R,R},0}, 
    {{D4s,R,R},{D3s,F3s,A3s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4,R,R},{R,R,R},{R,R,R},0}, {{F4s,R,R},{R,R,R},{R,R,R},0}, 
        {{F4,R,R},{D5s,F5s,A5s},{R,R,R},0}, {{C4s,R,R},{R,R,R},{R,R,R},0}, {{C4s,R,R},{s,s,s},{R,R,R},0}, {{D4s,R,R},{R,R,R},{R,R,R},0}, 
        {{C4s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{F2s,A3s,F3s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A3s,R,R},{A4s,C5s,G5s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A3s,R,R},{F2s,R,R},{R,R,R},0}, {{C4s,R,R},{R,R,R},{R,R,R},0}, 
    {{D4s,R,R},{B2,D3s,F3s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4,R,R},{R,R,R},{R,R,R},0}, {{F4s,R,R},{R,R,R},{R,R,R},0}, 
        {{F4,R,R},{C5s,F5s,A5s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4s,R,R},{s,s,s},{R,R,R},0}, {{G4s,R,R},{R,R,R},{R,R,R},0}, 
        {{A4s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G4s,R,R},{C3s,F3,G3s},{R,R,R},0}, {{F4s,R,R},{R,R,R},{R,R,R},0}, 
        {{F4,R,R},{C5s,G5s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C4s,R,R},{C3s,R,R},{R,R,R},0}, {{C4s,R,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{D3s,F3s,G3s},{s,s,s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F5s,R,R},0}, {{R,R,R},{R,R,R},{G5s,R,R},0}, 
        {{R,R,R},{F4s,A4s,R},{A5s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{G5s,R,R},0}, {{R,R,R},{F4s,A4s,R},{A5s,R,R},0}, 
        {{R,R,R},{R,R,R},{C6s,R,R},0}, {{R,R,R},{R,R,R},{D6s,R,R},0}, {{R,R,R},{F2s,R,R},{A5s,R,R},0}, {{R,R,R},{R,R,R},{G5s,R,R},0}, 
        {{s,s,s},{F4s,A4s,R},{A5s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4s,R,R},{F2s,R,R},{R,R,R},0}, {{G4s,R,R},{R,R,R},{R,R,R},0}, 
    {{A4s,R,R},{A2s,D3s,F3s},{s,s,s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G4s,R,R},{R,R,R},{R,R,R},0}, {{F4s,R,R},{R,R,R},{R,R,R},0}, 
        {{F4s,R,R},{D5s,F5s,A5s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D4s,R,R},{s,s,s},{R,R,R},0}, {{F4,R,R},{R,R,R},{R,R,R},0}, 
        {{F4s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4,R,R},{C3s,F3,G3s},{R,R,R},0}, {{D4s,R,R},{R,R,R},{R,R,R},0}, 
        {{C4s,R,R},{A5s,G5s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A3s,R,R},{C3s,R,R},{R,R,R},0}, {{C4s,R,R},{R,R,R},{R,R,R},0}, 
    {{D4s,R,R},{D3s,F3s,A3s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4,R,R},{R,R,R},{R,R,R},0}, {{F4s,R,R},{R,R,R},{R,R,R},0}, 
        {{F4,R,R},{F5s,D5s,A5s},{R,R,R},0}, {{C4s,R,R},{R,R,R},{R,R,R},0}, {{C4s,R,R},{s,s,s},{R,R,R},0}, {{D4s,R,R},{R,R,R},{R,R,R},0}, 
        {{C4s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{F2s,C3s,F3s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D4s,R,R},{s,s,s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{F2s,R,R},{R,R,R},0}, {{C4s,R,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{B2,D3s,F3s},{s,s,s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{B3,D4s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{D5s,F5s,A5s},{R,R,R},0}, {{D4s,F4s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F4,G4s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C3s,F3,G3s},{R,R,R},0}, {{F4s,A4s,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C5s,F5,G5s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F4,G4s,R},{C3s,R,R},{s,s,s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{D3s,F3s,A3s},{s,s,s},0}, {{D4s,F4s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{s,s,s},{R,R,R},{D5s,F5s,A5s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{D2s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F5s,R,R},0}, {{R,R,R},{R,R,R},{G5s,R,R},0}, {{F3s,R,R},{s,s,s},{A5s,R,R},0}, {{C4s,R,R},{R,R,R},{R,R,R},0}, 
        {{F4s,R,R},{R,R,R},{C6s,R,R},0}, {{G4s,R,R},{R,R,R},{R,R,R},0}, {{A4s,R,R},{R,R,R},{A5s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{D5s,A5s,F5s},{B1,F2s,B2},{D6s,R,R},_V[9]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{B2,R,R},{R,R,R},0}, {{R,R,R},{F3s,R,R},{R,R,R},0}, 
        {{C5s,C6s,R},{B3,D4s,R},{s,s,s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{B4,B5,R},{R,R,R},{R,R,R},0}, {{R,R,R},{B3,D4s,R},{R,R,R},0}, 
        {{A4s,A5s,R},{s,s,s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C2s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G4s,G5s,R},{F3,G3s,C4s},{R,R,R},0}, {{A4s,A5s,R},{R,R,R},{R,R,R},0}, {{B4,B5,R},{C2s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{D2s,A2s,D3s},{s,s,s},0}, {{A4s,A5s,R},{R,R,R},{R,R,R},0}, {{G4s,G5s,R},{D3s,R,R},{R,R,R},0}, {{F4s,F5s,R},{F3s,R,R},{R,R,R},0}, 
        {{R,R,R},{A3s,C4s,R},{R,R,R},0}, {{G4s,G5s,R},{R,R,R},{R,R,R},0}, {{A4s,A5s,R},{R,R,R},{R,R,R},0}, {{B4,B5,R},{A3s,C4s,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{A4s,A5s,R},{R,R,R},{R,R,R},0}, {{G4s,G5s,R},{C2s,R,R},{R,R,R},0}, {{F4s,F5s,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{F3s,A3s,C4s},{R,R,R},0}, {{D4s,D5s,R},{R,R,R},{R,R,R},0}, {{F4,F5,R},{C2s,R,R},{R,R,R},0}, {{F4s,F5s,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{B1,F2s,B2},{R,R,R},0}, {{F4,F5,R},{R,R,R},{R,R,R},0}, {{D4s,D5s,R},{B2,R,R},{R,R,R},0}, {{C4s,C5s,R},{F3s,R,R},{R,R,R},0}, 
        {{R,R,R},{B3,D4s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{B3,D4s,R},{R,R,R},0}, 
        {{B4,D5s,R},{s,s,s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C2s,R,R},{R,R,R},0}, {{D5s,F5s,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{F3,G3s,C4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,F4,R},{C2s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{D2s,A2s,D3s},{R,R,R},0}, {{A4s,C5s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{D3s,R,R},{R,R,R},0}, {{R,R,R},{F3s,R,R},{R,R,R},0}, 
        {{F4s,A4s,R},{A3s,C4s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4s,D5s,R},{A3s,C4s,R},{R,R,R},0}, 
        {{D4s,R,R},{R,R,R},{R,R,R},0}, {{F4s,R,R},{R,R,R},{R,R,R},0}, {{G4s,R,R},{F2s,R,R},{R,R,R},0}, {{A4s,R,R},{R,R,R},{R,R,R},0}, 
        {{C5s,R,R},{A3s,C4s,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{A5s,D6s,R},{B2,F3s,B3},{s,s,s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,C6s,R},{F4s,A3s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5s,B5,R},{F4s,A3s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,A5s,R},{F4s,A3s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{F4s,A3s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5,G5s,R},{C3s,G3s,C4s},{R,R,R},0}, {{F5s,A5s,R},{R,R,R},{R,R,R},0}, {{G5s,B5,R},{F4,G4s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{D3s,A3s,D4s},{s,s,s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{F4s,A4s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,A5s,R},{F4s,A4s,R},{R,R,R},0}, {{F5,G5s,R},{R,R,R},{R,R,R},0}, {{F5s,A5s,R},{F4s,A4s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{F4s,A4s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{F4s,A4s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{s,s,s},{F2s,C3s,F3s},{F4,G4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F4,G4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{s,s,s},{B2,F3s,B3},{F4s,A4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F4s,A4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F4s,A4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5s,B5,R},{s,s,s},{F4s,A4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,A5s,R},{R,R,R},{F4s,A4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5,G5s,R},{R,R,R},{F4s,A4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,R,R},{C3s,G3s,C4s},{F4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{F4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{s,s,s},{B1,B2,R},{F4s,A4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{B2,R,R},{R,R,R},0}, {{R,R,R},{D3s,R,R},{R,R,R},0}, 
        {{R,R,R},{F3s,A3s,R},{D4s,F4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{B2,R,R},{R,R,R},0}, {{R,R,R},{F3s,A3s,R},{D4s,F4s,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{B1,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{F3s,A3s,R},{D4s,F4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{B1,R,R},{R,R,R},0}, {{R,R,R},{F2s,R,R},{R,R,R},0}, 
    {{s,s,s},{B1,B2,R},{D4s,F4s,G4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{B2,R,R},{R,R,R},0}, {{R,R,R},{D3s,R,R},{R,R,R},0}, 
        {{R,R,R},{F3s,A3s,R},{D4s,F4s,G4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{B2,R,R},{R,R,R},0}, {{R,R,R},{F3s,A3s,R},{D4s,F4s,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{G4s,R,R},0}, {{R,R,R},{B1,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{F3s,A3s,R},{D4s,F4s,A4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{B1,R,R},{R,R,R},0}, {{R,R,R},{F2s,R,R},{R,R,R},0}, 
    {{C5s,R,R},{C2s,C3s,R},{C4s,F4s,G4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{C3s,R,R},{s,s,s},0}, {{C5s,R,R},{F3s,R,R},{R,R,R},0}, 
        {{C5s,R,R},{G3s,C4s,R},{R,R,R},0}, {{C5s,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{C3s,R,R},{R,R,R},0}, {{C5s,R,R},{G3s,C4s,R},{R,R,R},0}, 
        {{D5s,R,R},{R,R,R},{R,R,R},0}, {{D5s,R,R},{R,R,R},{R,R,R},0}, {{D5s,R,R},{C2s,R,R},{R,R,R},0}, {{D5s,R,R},{R,R,R},{R,R,R},0}, 
        {{F4s,F5s,R},{G3s,C4s,R},{R,R,R},0}, {{F4s,F5s,R},{R,R,R},{R,R,R},0}, {{F4s,F5s,R},{C3s,R,R},{R,R,R},0}, {{F4s,F5s,R},{R,R,R},{R,R,R},0}, 
    {{G4s,C5s,G5s},{C1s,C2s,R},{s,s,s},_V[10]}, {{G4s,G5s,R},{R,R,R},{R,R,R},0}, {{G4s,G5s,R},{C1s,R,R},{R,R,R},0}, {{G4s,G5s,R},{R,R,R},{R,R,R},0}, 
        {{A4s,C5s,A5s},{C1s,C2s,R},{R,R,R},0}, {{A4s,A5s,R},{R,R,R},{R,R,R},0}, {{A4s,A5s,R},{C1s,R,R},{R,R,R},0}, {{A4s,A5s,R},{R,R,R},{R,R,R},0}, 
        {{C5s,C6s,R},{C1s,C2s,R},{R,R,R},0}, {{C5s,C6s,R},{R,R,R},{R,R,R},0}, {{C5s,C6s,R},{C1s,R,R},{R,R,R},0}, {{C5s,C6s,R},{R,R,R},{R,R,R},0}, 
        {{A5s,A6s,R},{s,s,s},{R,R,R},0}, {{s,s,s},{R,R,R},{R,R,R},0}, {{C6s,C7s,R},{R,R,R},{R,R,R},0}, {{A5s,A6s,R},{R,R,R},{R,R,R},0}, 
    {{G5s,G6s,R},{B1,F2s,B2},{s,s,s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,F6s,R},{B2,R,R},{R,R,R},0}, {{R,R,R},{D3s,R,R},{R,R,R},0}, 
        {{D5s,D6s,R},{F3s,A3s,R},{R,R,R},0}, {{F5s,F6s,R},{s,s,s},{R,R,R},0}, {{s,s,s},{R,R,R},{R,R,R},0}, {{G5s,G6s,R},{F3s,A3s,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C2s,R,R},{R,R,R},0}, {{R,R,R},{F3,G3s,C4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,A6s,R},{C3s,R,R},{R,R,R},0}, {{R,R,R},{F3,G3s,C4s},{R,R,R},0}, {{C6s,C7s,R},{R,R,R},{R,R,R},0}, {{A5s,A6s,R},{R,R,R},{R,R,R},0}, 
    {{G5s,G6s,R},{D2s,A2s,D3s},{R,R,R},_V[11]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,F6s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{D3s,R,R},{R,R,R},0}, 
        {{C5s,C6s,R},{F3s,A3s,R},{R,R,R},0}, {{F5s,F6s,R},{s,s,s},{R,R,R},0}, {{s,s,s},{R,R,R},{R,R,R},0}, {{F5s,F6s,R},{F3s,A3s,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{A1s,R,R},{R,R,R},0}, {{R,R,R},{F3s,A3s,C4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,A6s,R},{s,s,s},{R,R,R},0}, {{R,R,R},{F3s,A3s,C4s},{R,R,R},0}, {{C6s,C7s,R},{R,R,R},{R,R,R},0}, {{A5s,A6s,R},{R,R,R},{R,R,R},0}, 
    {{G5s,G6s,R},{B1,F2s,B2},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{B2,R,R},{R,R,R},0}, {{R,R,R},{D3s,R,R},{R,R,R},0}, 
        {{A5s,A6s,R},{F3s,A3s,C4s},{R,R,R},0}, {{C6s,C7s,R},{s,s,s},{R,R,R},0}, {{s,s,s},{R,R,R},{R,R,R},0}, {{C6s,C7s,R},{F3s,A3s,C4s},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{D6s,D7s,R},{C2s,R,R},{R,R,R},0}, {{R,R,R},{F3,G3s,C4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,C7s,R},{F3,G3s,C4s},{R,R,R},0}, {{B5,B6,R},{R,R,R},{R,R,R},0}, {{A5s,A6s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{D2s,A2s,D3s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{D3s,R,R},{R,R,R},0}, 
        {{C6s,R,R},{F3s,A3s,C4s},{R,R,R},0}, {{D6s,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{G5s,R,R},{F3s,A3s,C4s},{R,R,R},0}, 
        {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{F1s,F2s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,A6s,R},{F3s,A3s,C4s},{R,R,R},0}, {{R,R,R},{F3s,R,R},{R,R,R},0}, {{C6s,C7s,R},{F2s,R,R},{R,R,R},0}, {{A5s,A6s,R},{R,R,R},{R,R,R},0}, 
    {{G5s,G6s,R},{B1,F2s,B2},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,F6s,R},{B2,R,R},{R,R,R},0}, {{R,R,R},{D3s,R,R},{R,R,R},0}, 
        {{D5s,D6s,R},{F3s,A3s,R},{R,R,R},0}, {{F5s,F6s,R},{s,s,s},{R,R,R},0}, {{s,s,s},{R,R,R},{R,R,R},0}, {{G5s,G6s,R},{F3s,A3s,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C2s,R,R},{R,R,R},0}, {{R,R,R},{F3,G3s,C4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,A6s,R},{C3s,R,R},{R,R,R},0}, {{R,R,R},{F3,G3s,C4s},{R,R,R},0}, {{C6s,C7s,R},{R,R,R},{R,R,R},0}, {{A5s,A6s,R},{R,R,R},{R,R,R},0}, 
    {{G5s,G6s,R},{D2s,A2s,D3s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,F6s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{D3s,R,R},{R,R,R},0}, 
        {{C5s,C6s,R},{F3s,A3s,R},{R,R,R},0}, {{F5s,F6s,R},{s,s,s},{R,R,R},0}, {{s,s,s},{R,R,R},{R,R,R},0}, {{F5s,F6s,R},{F3s,A3s,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{A1s,R,R},{R,R,R},0}, {{R,R,R},{F3s,A3s,C4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,F6s,R},{s,s,s},{R,R,R},0}, {{F5,F6,R},{F3s,A3s,C4s},{R,R,R},0}, {{D5s,D6s,R},{R,R,R},{R,R,R},0}, {{F5,F6,R},{R,R,R},{R,R,R},0}, 
    {{D5s,D6s,R},{B1,F2s,B2},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{B2,R,R},{R,R,R},0}, {{R,R,R},{D3s,R,R},{R,R,R},0}, 
        {{G5s,G6s,R},{F3s,A3s,R},{R,R,R},0}, {{R,R,R},{s,s,s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{F3s,A3s,R},{R,R,R},0}, 
        {{F5,F6,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C2s,R,R},{R,R,R},0}, {{D5s,D6s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5,F6,R},{F3,G3s,C4s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5,C6s,F6},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{R,R,R},{B1,B2,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,F6s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{F2s,R,R},{R,R,R},0}, 
        {{F5s,C6s,F6s},{C2s,C3s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{G2s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{F5s,A5s,R},{D2s,A2s,D3s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{D2s,R,R},{R,R,R},0}, {{R,R,R},{D3s,R,R},{R,R,R},0}, 
        {{F3s,A3s,C4s},{D2s,R,R},{R,R,R},0}, {{R,R,R},{D3s,R,R},{R,R,R},0}, {{R,R,R},{A2s,R,R},{R,R,R},0}, {{R,R,R},{D2s,R,R},{R,R,R},0}, 
    {{s,s,s},{B1,F2s,B2},{s,s,s},_V[9]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C5s,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{C4s,F4s,C5s},{R,R,R},0}, {{A5s,R,R},{G4s,R,R},{R,R,R},0}, {{G5s,R,R},{F4s,R,R},{R,R,R},0}, {{F5s,R,R},{G4s,R,R},{R,R,R},0}, 
        {{C6s,R,R},{G4s,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{G5s,R,R},{B2,F3s,C4s},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{A3s,R,R},{R,R,R},0}, {{A5s,R,R},{B2,G3s,R},{R,R,R},0}, {{G5s,R,R},{F3s,R,R},{R,R,R},0}, {{F5s,R,R},{G3s,R,R},{R,R,R},0}, 
    {{s,s,s},{B1,F2s,B2},{G3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C5s,R,R},{s,s,s},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{C4s,F4s,C5s},{R,R,R},0}, {{A5s,R,R},{G4s,R,R},{R,R,R},0}, {{G5s,R,R},{F4s,R,R},{R,R,R},0}, {{F5s,R,R},{G4s,R,R},{R,R,R},0}, 
        {{C6s,R,R},{G4s,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{G5s,R,R},{B2,F3s,C4s},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{A3s,R,R},{R,R,R},0}, {{A5s,R,R},{B2,G3s,R},{R,R,R},0}, {{G5s,R,R},{F3s,R,R},{R,R,R},0}, {{F5s,R,R},{G3s,R,R},{R,R,R},0}, 
    {{s,s,s},{C2s,G2s,C3s},{G3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C5s,R,R},{s,s,s},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{C4s,F4,A4s},{R,R,R},0}, {{B5,R,R},{G4s,R,R},{R,R,R},0}, {{A5s,R,R},{F4s,R,R},{R,R,R},0}, {{G5s,R,R},{G4s,R,R},{R,R,R},0}, 
        {{C6s,R,R},{G4s,R,R},{R,R,R},0}, {{B5,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{C3s,F3,C4s},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{A3s,R,R},{R,R,R},0}, {{B5,R,R},{C3s,G3s,R},{R,R,R},0}, {{A5s,R,R},{F3s,R,R},{R,R,R},0}, {{G5s,R,R},{G3s,R,R},{R,R,R},0}, 
    {{s,s,s},{C2s,G2s,C3s},{s,s,s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{D5s,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{F4,A4s,C5s},{R,R,R},0}, {{B5,R,R},{A4s,R,R},{R,R,R},0}, {{A5s,R,R},{A4s,R,R},{R,R,R},0}, {{G5s,R,R},{C5s,R,R},{R,R,R},0}, 
        {{C6s,R,R},{C5s,R,R},{R,R,R},0}, {{B5,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{C3s,F3,D4s},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{C4s,R,R},{R,R,R},0}, {{B5,R,R},{C3s,A3s,R},{R,R,R},0}, {{A5s,R,R},{C3s,A3s,R},{R,R,R},0}, {{G5s,R,R},{C3s,C4s,R},{R,R,R},0}, 
    {{s,s,s},{D2s,A2s,C3s},{C4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C5s,R,R},{s,s,s},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{D4s,F4s,A4s},{R,R,R},0}, {{B5,R,R},{F4s,R,R},{R,R,R},0}, {{A5s,R,R},{F4s,R,R},{R,R,R},0}, {{G5s,R,R},{G4s,R,R},{R,R,R},0}, 
        {{C6s,R,R},{G4s,R,R},{R,R,R},0}, {{B5,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{C3s,F3s,C4s},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{A3s,R,R},{R,R,R},0}, {{B5,R,R},{C3s,F3s,R},{R,R,R},0}, {{A5s,R,R},{C3s,F3s,R},{R,R,R},0}, {{G5s,R,R},{C3s,G3s,R},{R,R,R},0}, 
    {{s,s,s},{D2s,A2s,C3s},{G3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C5s,R,R},{s,s,s},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, 
        {{D6s,R,R},{D4s,F4s,A4s},{R,R,R},0}, {{C6s,R,R},{F4s,R,R},{R,R,R},0}, {{B5,R,R},{F4s,R,R},{R,R,R},0}, {{A5s,R,R},{G4s,R,R},{R,R,R},0}, 
        {{D6s,R,R},{G4s,R,R},{R,R,R},0}, {{C6s,R,R},{R,R,R},{R,R,R},0}, {{B5,R,R},{C3s,F3s,C4s},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, 
        {{D6s,R,R},{A3s,R,R},{R,R,R},0}, {{C6s,R,R},{C3s,F3s,R},{R,R,R},0}, {{B5,R,R},{C3s,F3s,R},{R,R,R},0}, {{A5s,R,R},{C3s,G3s,R},{R,R,R},0}, 
    {{s,s,s},{F2,C3s,F3},{G3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C5s,R,R},{s,s,s},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, 
        {{F6,R,R},{C4s,F4,A4s},{R,R,R},0}, {{D6s,R,R},{F4s,R,R},{R,R,R},0}, {{C6s,R,R},{F4s,R,R},{R,R,R},0}, {{G5s,R,R},{G4s,R,R},{R,R,R},0}, 
        {{F6,R,R},{G4s,R,R},{R,R,R},0}, {{D6s,R,R},{R,R,R},{R,R,R},0}, {{C6s,R,R},{F3s,F4s,R},{R,R,R},0}, {{G5s,R,R},{G3s,G4s,R},{R,R,R},0}, 
        {{F6,R,R},{A3s,A4s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{B3,B4,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{s,s,s},{C2s,G2s,C3s},{s,s,s},_V[10]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,F5,C6s},{G3s,G4s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5s,F5,C6s},{G3s,G4s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,F5,C6s},{C2s,C3s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5s,F5,C6s},{C2s,C3s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,F5,C6s},{C2s,C3s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5s,F5,C6s},{C2s,C3s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    //Part 3
{{G5s,R,R},{s,s,s},{B3,F4s,A4s},_V[5]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{C4s,G4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{R,R,R},{D4s,A4s,C5s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{F3s,C4s,A4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{R,R,R},{B3,F4s,A4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{C4s,G4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{R,R,R},{D4s,A4s,C5s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5s,R,R},{R,R,R},{A3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{C4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{D4s,R,R},{B2,F3s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{F4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{G4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{F4,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{C4s,R,R},{C3s,F3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{B3,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{A3s,R,R},{D3s,F3s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{C4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{F4,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D6s,R,R},{F4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{F2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5s,R,R},{F4s,R,R},{F3s,A3s,D4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{F4,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{D4s,R,R},{B2,F3s,R},_V[6]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{F4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{F4,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{C4s,R,R},{C3s,F3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{G3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{C4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{R,R,R},{D3s,F3s,A3s},_V[7]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{s,s,s},{F2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A4s,R,R},{A3s,R,R},{F3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{C4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{s,s,s},{D4s,R,R},{B2,F3s,R},_V[8]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,C6s,R},{F4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{G4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,D6s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{F4,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,C6s,R},{C4s,R,R},{C3s,F3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{B3,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{s,s,s},{A3s,R,R},{D3s,F3s,R},_V[9]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{C4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,C6s,R},{F4,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{F4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,D6s,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{F2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,C6s,R},{F4s,R,R},{F3s,A3s,D4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{F4,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{s,s,s},{D4s,R,R},{B2,F3s,R},_V[10]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,C6s,R},{F4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,D6s,R},{F4,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{C4s,R,R},{C3s,F3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,F6,R},{R,R,R},{G3s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{C4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{C6s,G6s,R},{s,s,s},{C2s,C3s,R},_V[11]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,F6s,R},{R,R,R},{C2s,C3s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,F6,R},{R,R,R},{C2s,C3s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{s,s,s},{A4s,R,R},{s,s,s},0}, {{R,R,R},{s,s,s},{R,R,R},0}, {{R,R,R},{C5s,R,R},{R,R,R},0}, {{R,R,R},{A4s,R,R},{R,R,R},0}, 
    {{s,s,s},{G4s,R,R},{B1,B2,R},_V[12]}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{C6s,R,R},{F4s,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,R,R},{D4s,R,R},{F3s,B3,R},0}, {{F5s,R,R},{F4s,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, {{D5s,R,R},{G4s,R,R},{R,R,R},0}, 
        {{F5s,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{R,R,R},{R,R,R},0}, {{D5s,R,R},{R,R,R},{C2s,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{A4s,R,R},{G3s,C4s,F4},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C5s,R,R},{R,R,R},0}, {{R,R,R},{A4s,R,R},{R,R,R},0}, 
    {{s,s,s},{G4s,R,R},{D2s,D3s,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{C6s,R,R},{F4s,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,R,R},{C4s,R,R},{F3s,A3s,R},0}, {{F5s,R,R},{F4s,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, {{D5s,R,R},{F4s,R,R},{R,R,R},0}, 
        {{F5s,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{R,R,R},{R,R,R},0}, {{D5s,R,R},{R,R,R},{A1s,A2s,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, 
        {{C5s,F5s,R},{F4s,R,R},{A4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{R,R,R},{R,R,R},0}, {{A4s,R,R},{R,R,R},{s,s,s},0}, 
    {{s,s,s},{G4s,R,R},{B1,B2,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{C6s,R,R},{R,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,R,R},{F4s,A4s,R},{s,s,s},0}, {{F5s,R,R},{F4s,C5s,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, {{D5s,R,R},{C5s,R,R},{F4s,R,R},0}, 
        {{A5s,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{D5s,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{C2s,R,R},0}, {{D6s,R,R},{R,R,R},{R,R,R},0}, 
        {{C5s,R,R},{C5s,R,R},{C4s,F4,G4s},0}, {{B4,R,R},{B4,R,R},{R,R,R},0}, {{A4s,R,R},{A4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{s,s,s},{s,s,s},{D2s,D3s,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{C6s,R,R},{R,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,R,R},{R,R,R},{D3s,F3s,A3s},0}, {{C6s,R,R},{R,R,R},{R,R,R},0}, {{D6s,R,R},{R,R,R},{R,R,R},0}, {{F6s,R,R},{R,R,R},{D3s,F3s,A3s},0}, 
        {{F6,R,R},{R,R,R},{R,R,R},0}, {{C6s,R,R},{R,R,R},{R,R,R},0}, {{D6s,R,R},{R,R,R},{F1s,F2s,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, 
        {{F4s,C6s,R},{R,R,R},{A4s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C5s,R,R},{R,R,R},0}, {{R,R,R},{A4s,R,R},{s,s,s},0}, 
    {{s,s,s},{G4s,R,R},{B1,B2,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{C6s,R,R},{F4s,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,R,R},{D4s,R,R},{F3s,B3,R},0}, {{F5s,R,R},{F4s,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, {{D5s,R,R},{G4s,R,R},{R,R,R},0}, 
        {{F5s,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{R,R,R},{R,R,R},0}, {{D5s,R,R},{R,R,R},{C2s,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{A4s,R,R},{G3s,C4s,F4},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C5s,R,R},{R,R,R},0}, {{R,R,R},{A4s,R,R},{R,R,R},0}, 
    {{s,s,s},{G4s,R,R},{D2s,D3s,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{C6s,R,R},{F4s,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,R,R},{C4s,R,R},{F3s,A3s,R},0}, {{F5s,R,R},{F4s,R,R},{R,R,R},0}, {{G5s,R,R},{R,R,R},{R,R,R},0}, {{D5s,R,R},{F4s,R,R},{R,R,R},0}, 
        {{F5s,R,R},{R,R,R},{R,R,R},0}, {{C5s,R,R},{R,R,R},{R,R,R},0}, {{D5s,R,R},{R,R,R},{A1s,A2s,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, 
        {{s,s,s},{F4s,R,R},{A3s,C4s,R},0}, {{R,R,R},{F4,R,R},{R,R,R},0}, {{R,R,R},{D4s,R,R},{R,R,R},0}, {{R,R,R},{F4,R,R},{R,R,R},0}, 
    {{s,s,s},{D4s,R,R},{B1,B2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{B2,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, 
        {{R,R,R},{G4s,G5s,R},{F3s,A3s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F3s,A3s,R},0}, 
        {{R,R,R},{F4,F5,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2s,R,R},0}, {{R,R,R},{D4s,D5s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{F4,F5,R},{F3,G3s,C4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{F4,C5s,F5},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{s,s,s},{R,R,R},{B1,B2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{F4s,F5s,R},{F2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{F4s,G4s,F5s},{C2s,C3s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{G2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{C4s,F4s,A4s},{D2s,A2s,D3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{C4s,R,R},{R,R,R},0}, {{R,R,R},{G4s,R,R},{R,R,R},0}, 
        {{R,R,R},{C5s,R,R},{F3s,A3s,R},0}, {{R,R,R},{G5s,R,R},{D3s,R,R},0}, {{R,R,R},{C6s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2s,R,R},0}, 
    {{D6s,R,R},{D5s,F5s,A5s},{B1,F2s,B2},_V[13]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{B2,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, 
        {{s,s,s},{C5s,C6s,R},{F3s,A3s,R},0}, {{R,R,R},{D5s,D6s,R},{R,R,R},0}, {{R,R,R},{F5s,F6s,R},{D3s,R,R},0}, {{R,R,R},{R,R,R},{F3s,A3s,R},0}, 
        {{R,R,R},{G5s,G6s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2s,G2s,C3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{A5s,A6s,R},{G3s,C4s,R},0}, {{R,R,R},{R,R,R},{C3s,R,R},0}, {{R,R,R},{F6s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{s,s,s},{F5s,A5s,F6s},{D2s,A2s,D3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, {{R,R,R},{R,R,R},{F3s,R,R},0}, 
        {{R,R,R},{A5s,A6s,R},{A3s,C4s,R},0}, {{R,R,R},{G5s,G6s,R},{R,R,R},0}, {{R,R,R},{F5s,F6s,R},{F3s,R,R},0}, {{R,R,R},{R,R,R},{A3s,C4s,R},0}, 
        {{R,R,R},{G5s,G6s,R},{s,s,s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1s,A2s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{C6s,C7s,R},{F3s,A3s,R},0}, {{R,R,R},{R,R,R},{A2s,R,R},0}, {{R,R,R},{A6s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{A6s,R,R},{A5s,C6s,F6s},{B1,F2s,B2},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{B2,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, 
        {{s,s,s},{F5s,F6s,R},{F3s,A3s,R},0}, {{R,R,R},{F5,F6,R},{R,R,R},0}, {{R,R,R},{C5s,C6s,R},{D3s,R,R},0}, {{R,R,R},{R,R,R},{F3s,A3s,R},0}, 
        {{R,R,R},{D5s,D6s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2s,G2s,C3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{C5s,C6s,R},{G3s,C4s,R},0}, {{R,R,R},{R,R,R},{C3s,R,R},0}, {{R,R,R},{A5s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{C6s,R,R},{C5s,F5s,G5s},{D2s,A2s,D3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{s,s,s},{F4s,R,R},{D3s,R,R},0}, {{R,R,R},{G4s,R,R},{F3s,R,R},0}, 
        {{R,R,R},{A4s,R,R},{A3s,C4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{G4s,R,R},{F3s,R,R},0}, {{R,R,R},{A4s,R,R},{A3s,C4s,R},0}, 
        {{R,R,R},{B4,R,R},{s,s,s},0}, {{R,R,R},{G4s,R,R},{R,R,R},0}, {{R,R,R},{A4s,R,R},{F1s,F2s,R},0}, {{R,R,R},{B4,R,R},{R,R,R},0}, 
        {{R,R,R},{C5s,R,R},{A3s,C4s,R},0}, {{R,R,R},{D5s,R,R},{F3s,R,R},0}, {{R,R,R},{A4s,R,R},{F2s,R,R},0}, {{R,R,R},{G4s,R,R},{R,R,R},0}, 
    {{D6s,R,R},{D5s,F5s,A5s},{B1,F2s,B2},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{B2,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, 
        {{s,s,s},{C5s,C6s,R},{F3s,A3s,R},0}, {{R,R,R},{D5s,D6s,R},{R,R,R},0}, {{R,R,R},{F5s,F6s,R},{D3s,R,R},0}, {{R,R,R},{R,R,R},{F3s,A3s,R},0}, 
        {{R,R,R},{G5s,G6s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2s,G2s,C3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{A5s,A6s,R},{G3s,C4s,R},0}, {{R,R,R},{R,R,R},{C3s,R,R},0}, {{R,R,R},{F6s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{s,s,s},{F5s,A5s,F6s},{D2s,A2s,D3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, {{R,R,R},{R,R,R},{F3s,R,R},0}, 
        {{R,R,R},{A5s,A6s,R},{A3s,C4s,R},0}, {{R,R,R},{G5s,G6s,R},{R,R,R},0}, {{R,R,R},{F5s,F6s,R},{F3s,R,R},0}, {{R,R,R},{R,R,R},{A3s,C4s,R},0}, 
        {{R,R,R},{G5s,G6s,R},{s,s,s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1s,A2s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{C6s,C7s,R},{F3s,A3s,R},0}, {{R,R,R},{R,R,R},{A2s,R,R},0}, {{R,R,R},{D6s,R,R},{R,R,R},0}, {{R,R,R},{F6s,R,R},{R,R,R},0}, 
    {{D7s,R,R},{D6s,F6s,A6s},{B1,F2s,B2},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{B2,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, 
        {{R,R,R},{R,R,R},{F3s,A3s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{s,s,s},{A5s,A6s,R},{D3s,R,R},0}, {{R,R,R},{R,R,R},{F3s,A3s,R},0}, 
        {{R,R,R},{G5s,F6,G6s},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2s,G2s,C3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{G3s,C4s,R},0}, {{R,R,R},{R,R,R},{C3s,R,R},0}, {{R,R,R},{F5s,F6s,R},{R,R,R},0}, {{R,R,R},{G5s,G6s,R},{R,R,R},0}, 
    {{A6s,R,R},{A5s,D6s,F6s},{B1,B2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2s,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C2s,C3s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{s,s,s},{G5s,G6s,R},{G2s,R,R},0}, {{R,R,R},{F5s,F6s,R},{R,R,R},0}, 
        {{F6s,R,R},{F5s,G5s,D6s},{D2s,A2s,D3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{s,s,s},{F4s,R,R},{R,R,R},0}, {{R,R,R},{G4s,R,R},{R,R,R},0}, 
        {{R,R,R},{A4s,R,R},{D3s,R,R},0}, {{R,R,R},{G4s,R,R},{F3s,R,R},0}, {{R,R,R},{A4s,R,R},{A3s,R,R},0}, {{R,R,R},{C5s,R,R},{C4s,R,R},0}, 
    {{s,s,s},{F4s,A4s,D5s},{B3,R,R},_V[10]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{C5s,R,R},{B3,R,R},0}, {{R,R,R},{D5s,R,R},{C4s,R,R},0}, {{R,R,R},{F5s,R,R},{C4s,R,R},0}, {{R,R,R},{s,s,s},{s,s,s},0}, 
        {{R,R,R},{G5s,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{C5s,F5,A5s},{F3,G3s,C4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{F5s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{s,s,s},{A4s,D5s,F5s},{D3s,F3s,A3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{A5s,R,R},{F4s,R,R},0}, {{R,R,R},{G5s,R,R},{F4,R,R},0}, {{R,R,R},{F5s,R,R},{D4s,R,R},0}, {{R,R,R},{s,s,s},{s,s,s},0}, 
        {{R,R,R},{G5s,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{F5,G5s,C6s},{A2s,F4,G3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{A5s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{s,s,s},{F5s,A5s,R},{B2,F3s,A3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{F5s,R,R},{D4s,R,R},0}, {{R,R,R},{F5,R,R},{C4s,R,R},0}, {{R,R,R},{C5s,R,R},{A3s,R,R},0}, {{R,R,R},{s,s,s},{s,s,s},0}, 
        {{R,R,R},{D5s,R,R},{B3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C4s,R,R},{F4,C5s,R},{C3s,F3,G3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{A4s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{s,s,s},{D4s,A4s,C5s},{D3s,F3s,A3s},_V[11]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{F5s,R,R},{R,R,R},0}, {{R,R,R},{G5s,R,R},{R,R,R},0}, 
        {{R,R,R},{A5s,R,R},{F4s,A4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{G5s,R,R},{R,R,R},0}, {{R,R,R},{A5s,R,R},{F4s,A4s,R},0}, 
        {{R,R,R},{B5,R,R},{R,R,R},0}, {{R,R,R},{G5s,R,R},{R,R,R},0}, {{R,R,R},{A5s,R,R},{F2s,F3s,R},0}, {{R,R,R},{B5,R,R},{R,R,R},0}, 
        {{R,R,R},{C6s,R,R},{A3s,C4s,R},0}, {{R,R,R},{D6s,R,R},{F3s,R,R},0}, {{R,R,R},{A5s,R,R},{F2s,R,R},0}, {{R,R,R},{G5s,R,R},{R,R,R},0}, 
    {{R,R,R},{D4s,F4s,D5s},{B1,F2s,B2},_V[12]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{B2,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, 
        {{R,R,R},{C4s,C5s,R},{F3s,A3s,R},0}, {{R,R,R},{D4s,D5s,R},{R,R,R},0}, {{R,R,R},{F4s,F5s,R},{D3s,R,R},0}, {{R,R,R},{R,R,R},{F3s,A3s,R},0}, 
        {{R,R,R},{G4s,G5s,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2s,G2s,C3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{A4s,A5s,R},{G3s,C4s,R},0}, {{R,R,R},{R,R,R},{C3s,R,R},0}, {{R,R,R},{F5s,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{R,R,R},{F4s,A4s,F5s},{D2s,A2s,D3s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D3s,R,R},0}, {{R,R,R},{R,R,R},{F3s,R,R},0}, 
        {{R,R,R},{A4s,A5s,R},{A3s,C4s,R},0}, {{R,R,R},{G4s,G5s,R},{R,R,R},0}, {{R,R,R},{F4s,F5s,R},{F3s,R,R},0}, {{R,R,R},{R,R,R},{A3s,A4s,R},0}, 
        {{R,R,R},{G4s,G5s,R},{s,s,s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1s,A2s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{C5s,C6s,R},{F3s,F4s,R},0}, {{R,R,R},{R,R,R},{A2s,R,R},0}, {{R,R,R},{D5s,R,R},{R,R,R},0}, {{R,R,R},{F5s,R,R},{R,R,R},0}, 
    
    {{R,R,R},{D5s,F5s,A5s},{B1,F2s,B2},_V[13]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{D5s,D6s,R},{B2,R,R},0}, {{R,R,R},{D5s,D6s,R},{D3s,R,R},0}, 
        {{R,R,R},{D5s,D6s,R},{F3s,A3s,R},0}, {{R,R,R},{D5s,D6s,R},{R,R,R},0}, {{R,R,R},{D5s,D6s,R},{D3s,R,R},0}, {{R,R,R},{D5s,D6s,R},{F3s,A3s,R},0}, 
        {{R,R,R},{F5,F6,R},{R,R,R},0}, {{R,R,R},{F5,F6,R},{R,R,R},0}, {{R,R,R},{F5,F6,R},{C2s,G2s,C3s},0}, {{R,R,R},{F5,F6,R},{R,R,R},0}, 
        {{R,R,R},{F5,F6,R},{F3,G3s,C4s},0}, {{R,R,R},{F5,F6,R},{C3s,R,R},0}, {{R,R,R},{F5s,F6s,R},{R,R,R},0}, {{R,R,R},{G5s,G6s,R},{R,R,R},0}, 
    {{R,R,R},{A5s,C6s,A6s},{B1,B2,R},0}, {{R,R,R},{A5s,C6s,A6s},{R,R,R},0}, {{R,R,R},{A5s,C6s,A6s},{F2s,R,R},0}, {{R,R,R},{A5s,C6s,A6s},{R,R,R},0}, 
        {{R,R,R},{A5s,C6s,A6s},{C2s,C3s,R},0}, {{R,R,R},{A5s,C6s,A6s},{R,R,R},0}, {{R,R,R},{G5s,G6s,R},{G2s,R,R},0}, {{R,R,R},{F5s,F6s,R},{R,R,R},0}, 
        {{R,R,R},{F5s,F6s,R},{D2s,A2s,D3s},0}, {{R,R,R},{F5s,A5s,F6s},{R,R,R},0}, {{R,R,R},{F5s,A5s,F6s},{D3s,R,R},0}, {{R,R,R},{F5s,A5s,F6s},{F3s,R,R},0}, 
        {{R,R,R},{F5s,A5s,F6s},{A3s,R,R},0}, {{R,R,R},{F5s,A5s,F6s},{C4s,R,R},0}, {{R,R,R},{F5s,A5s,F6s},{D4s,R,R},0}, {{R,R,R},{F5s,A5s,F6s},{F4s,R,R},0}, 
    {{G5s,R,R},{s,s,s},{B3,F4s,A4s},_V[5]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{C4s,G4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{R,R,R},{D4s,A4s,C5s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{F3s,C4s,A4s},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{R,R,R},{B3,F4s,A4s},_V[4]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{C4s,G4s,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{R,R,R},{D4s,A4s,C5s},_V[3]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{F5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0},
};

void DSHH_init();
void DSHH_MakeSound_0 (msc) ;
void DSHH_MakeSound_1 (msc) ;
void DSHH_MakeSound_2 (msc) ;
void DSHH_Music() ;
void DSHH_init () { //预处理指令
    _mp[R] = PA ;
    _mp[P] = PA ;
    _mp[s] = PA ;
    _mp[0] = PA ;
    _mp[ _V[1] ] = 0x1f;
    for(int i=2;i<=13;i++)
    {
        _mp[ _V[i] ] = _mp[ _V[i-1] ] + 8 ;
    }
}

void DSHH_MakeSound_0 (v_spo _m) {    //第零通道
    //特判由休止符造成的停止发声 
    if( _m._v1 == s ){
        //与else语句的区别是停止后就不发声,打算用于休止符
        midiOutShortMsg ( handle, 0x7BB0 ) ; //midiOut短消息B类
    }
    //开始发声
    else {
        //发出新的声音之前先停止上一个声音
        midiOutShortMsg ( handle, 0x7BB0 ) ; //midiOut短消息B类
        //定义声音变量voice并发声
        if ( _m._v1 != R )
        {    // "_v1" 当然是不用判断的，但是为了保证队形整齐还是这么写了
            int voice = (volume << 16) | ( (_m._v1) << 8) | 0x90 ; //音量|音高|通道
            midiOutShortMsg ( handle, voice ) ; //midiOut短消息9类//演奏
        }
        if ( _m._v2 != R )
        {
            int voice = (volume << 16) | ( (_m._v2) << 8) | 0x90 ; //音量|音高|通道
            midiOutShortMsg ( handle, voice ) ; //midiOut短消息9类//演奏
        }
        if ( _m._v3 != R )
        {
            int voice = (volume << 16) | ( (_m._v3) << 8) | 0x90 ; //音量|音高|通道
            midiOutShortMsg ( handle, voice ) ; //midiOut短消息9类//演奏
        }
    }
}
void DSHH_MakeSound_1 (v_spo _m) {    //第一通道
    //特判由休止符造成的停止发声 
    if( _m._v1 == s ){
        //与else语句的区别是停止后就不发声,打算用于休止符
        midiOutShortMsg ( handle, 0x7BB1 ) ; //midiOut短消息B类
    }
    //开始发声
    else {
        //发出新的声音之前先停止上一个声音
        midiOutShortMsg ( handle, 0x7BB1 ) ; //midiOut短消息B类
        //定义声音变量voice并发声
        if ( _m._v1 != R ) {    // "_v1" 当然是不用判断的，但是为了保证队形整齐还是这么写了
            int voice = (volume << 16) | ( (_m._v1) << 8) | 0x91 ; //音量|音高|通道
            midiOutShortMsg ( handle, voice ) ; //midiOut短消息9类//演奏
        }
        if ( _m._v2 != R ) {
            int voice = (volume << 16) | ( (_m._v2) << 8) | 0x91 ; //音量|音高|通道
            midiOutShortMsg ( handle, voice ) ; //midiOut短消息9类//演奏
        }
        if ( _m._v3 != R ) {
            int voice = (volume << 16) | ( (_m._v3) << 8) | 0x91 ; //音量|音高|通道
            midiOutShortMsg ( handle, voice ) ; //midiOut短消息9类//演奏
        }
    }
}
void DSHH_MakeSound_2 (v_spo _m) {    //第二通道
    //特判由休止符造成的停止发声 
    if( _m._v1 == s ){
        //与else语句的区别是停止后就不发声,打算用于休止符
        midiOutShortMsg ( handle, 0x7BB2 ) ; //midiOut短消息B类
    }
    //开始发声
    else {
        //发出新的声音之前先停止上一个声音
        midiOutShortMsg ( handle, 0x7BB2 ) ; //midiOut短消息B类
        //定义声音变量voice并发声
        if ( _m._v1 != R ) {    // "_v1" 当然是不用判断的，但是为了保证队形整齐还是这么写了
            int voice = (volume << 16) | ( (_m._v1) << 8) | 0x92 ; //音量|音高|通道
            midiOutShortMsg ( handle, voice ) ; //midiOut短消息9类//演奏
        }
        if ( _m._v2 != R ) {
            int voice = (volume << 16) | ( (_m._v2) << 8) | 0x92 ; //音量|音高|通道
            midiOutShortMsg ( handle, voice ) ; //midiOut短消息9类//演奏
        }
        if ( _m._v3 != R ) {
            int voice = (volume << 16) | ( (_m._v3) << 8) | 0x92 ; //音量|音高|通道
            midiOutShortMsg ( handle, voice ) ; //midiOut短消息9类//演奏
        }
    }
}
void DSHH_Music ()
{
    midiOutOpen (&handle, 0, 0, 0, CALLBACK_NULL) ; //打开MIDI设备

    midiOutShortMsg (handle, 0 << 8 | 0xC0) ; //midiOut短消息C类 //声明乐器(0:大钢琴)、通道0(右手一声部)
    midiOutShortMsg (handle, 0 << 8 | 0xC1) ; //midiOut短消息C类 //声明乐器(0:大钢琴)、通道1(右手二声部)
    midiOutShortMsg (handle, 0 << 8 | 0xC2) ; //midiOut短消息C类 //声明乐器(0:大钢琴)、通道2(左手一声部)
    //遍历乐谱
    for (auto i : DSHH_music) {
        msc m=i;
        cnt -- ; // 四分之一小节过去了
        //处理指令
        if ( _mp[m.C] != PA ){
            //cout<<1;
            volume = _mp[ m.C ] ; //调节音量
        } 
        //分通道发声
        //因为同一通道里的音同时开始和停止,所以只判断 "_v1" 的值即可 
        if ( m.v0._v1 != R ) DSHH_MakeSound_0 ( m.v0 ) ;
        if ( m.v1._v1 != R ) DSHH_MakeSound_1 ( m.v1 ) ;
        if ( m.v2._v1 != R ) DSHH_MakeSound_2 ( m.v2 ) ;
        //保持音时
        Sleep ( stime ) ; // 四分之一小节
    }
    midiOutClose (handle) ; //关闭MIDI
}
int main(void)
{
    while (1)
    {
        DSHH_init();
        DSHH_Music();
    }
}