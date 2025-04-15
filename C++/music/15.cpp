#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <mmsystem.h>    
#include <mutex>    
#pragma comment(lib,"winmm.lib")
using namespace std;

enum Scale {
	Rest = 0, C8 = 108, B7 = 107, A7s = 106, A7 = 105, G7s = 104, G7 = 103, F7s = 102, F7 = 101, E7 = 100,
	D7s = 99, D7 = 98, C7s = 97, C7 = 96, B6 = 95, A6s = 94, A6 = 93, G6s = 92, G6 = 91, F6s = 90, F6 = 89,
	E6 = 88, D6s = 87, D6 = 86, C6s = 85, C6 = 84, B5 = 83, A5s = 82, A5 = 81, G5s = 80, G5 = 79, F5s = 78,
	F5 = 77, E5 = 76, D5s = 75, D5 = 74, C5s = 73, C5 = 72, B4 = 71, A4s = 70, A4 = 69, G4s = 68, G4 = 67,
	F4s = 66, F4 = 65, E4 = 64, D4s = 63, D4 = 62, C4s = 61, C4 = 60, B3 = 59, A3s = 58, A3 = 57, G3s = 56,
	G3 = 55, F3s = 54, F3 = 53, E3 = 52, D3s = 51, D3 = 50, C3s = 49, C3 = 48, B2 = 47, A2s = 46, A2 = 45,
	G2s = 44, G2 = 43, F2s = 42, F2 = 41, E2 = 40, D2s = 39, D2 = 38, C2s = 37, C2 = 36, B1 = 35, A1s = 34,
	A1 = 33, G1s = 32, G1 = 31, F1s = 30, F1 = 29, E1 = 28, D1s = 27, D1 = 26, C1s = 25, C1 = 24, B0 = 23,
	A0s = 22, A0 = 21
};
//落凡尘
void People(void);
void People(void)
{
 	HMIDIOUT handle;
 	midiOutOpen(&handle, 0, 0, 0, 0);

 	int peoplee[] =
 	{
		0,0,1000,
		F4s,1000, G4s,500, B4,500, C5s,2000, //1
		B4,500, G4s,500, G4s,500, A5s,500, D5s,2000, //2
		G4s,500, B4,500, B4,500, D5s,500, C5s,500, B4,500, C5s,1000, //3
		C5s,500, B4,500, C5s,500, B4,500, D5s,1000, D5s,500, C5s,500, //4
		B4,1500, G5s,500, F5s,2000, //5
		G4s,500, B4,500, B4,500, E5,500, D5s,2000, //6
		D5s,500, F5s,500, F5s,500, G5s,500, F5s,500, D5s,500, C5s,500, B4,500, //7
		G4s,500, B4,500, B4,500, C5s,500, B4,2000, 0,2000, //8
		F4s,1000, G4s,500, B4,500, C5s,2000, //9
		B4,500, G4s,500, G4s,500, A5s,500, D5s,2000, //10
		G4s,500, B4,500, B4,500, D5s,500, C5s,250, B4,250, B4,500, C5s,1000, //11
		C5s,500, B4,500, C5s,500, B4,500, D5s,1000, D5s,500, C5s,500, //12
		B4,1500, G5s,500, F5s,2000, //13
		G5s,500, B5,500, B5,500, D5s,500, C5s,500, B4,1500, //14
		G4s,500, B4,500, B4,500, G5s,500, F5s,500, D5s,500, C5s,500, B4,500, //15
		G4s,500, B4,500, B4,500, C5s,1000, B4,2500, //16
		0,4000, //17
		D5s,500, F5s,500, B5,1500, A5s,500, G5s,500, F5s,500, //18
		D5s,500, G5s,250, F5s,750, F5s,2500, //19
		D5s,500, F5s,500, G5s,1500, A5s,500, B5,500, F5s,500, //20
		B4,500, F5s,500, D5s,1750, C5s,250, C5s,1000, //21
		D5s, 500, F5s, 500, B5, 1500, A5s, 500, G5s, 500, F5s, 500, //22
		D5s,500, D6s,500, A5s,500, A5s,2500, //23
		D5s, 500, F5s, 500, G5s, 1500, A5s, 500, B5, 500, A5s, 500, //20
		G5s,250, F5s,500, D5s,250, F5s,2000, 0,1000, //21
		D5s,1000, C5s,250, B4,250, B4,2500, //22
		0,1000,-1
 	};
	int i = 1;
	int back;
	while (peoplee[i] != -1)
	{
		if (peoplee[i] != 0)
		{
			back = ((0x7f) << 16) + ((peoplee[i]) << 8) + (0x90);
			midiOutShortMsg(handle, back);
		}
		Sleep(peoplee[i+1]/1.1);
		i = i + 2;
	}
	midiOutClose(handle);
}

int main(void)
{
	People();
	return 0;
}