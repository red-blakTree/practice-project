#include <iostream>
#include <map>
#include <algorithm>
#include <Windows.h>

#pragma comment(lib,"winmm.lib")

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
    A0s = 22, A0 = 21 //88键,酷！
};
//常用参数
enum Voice // 不知道这个大括号是干什么用的
{
    PA = 100000001 , // Prohibition of access , 作为一个空置的参数 , 判断用
    R  = 1009 , // Rest  , 表示声音的延长
    S  = 1013 , // Stop  , 表示休止符
    P  = 1019 , // Pedal , 表示踏板的踩下(或松开)
    //找到的谱子是 183拍/分 C调,选择将速度设为324,与原速比较接近且便于计算时值,还能弹三连音
    SPEED = 77,  // 324ms/拍,这里图省事直接设置成1/4拍
                 // 2.6注:听了很多遍原曲和钢琴曲,感觉有必要加一下速 (81->79)
                 // 2.6又注:踏板性能损耗很严重,还得再加            (79->77)
};

//声音指令，每8一格,最小为0x1f,最大0x7f,共十三档,对应数组_V[1]到_V[13]
const int _V[15]={0,1033,1039,1049,1051,1061,1063,1069,1087,1091,1093,1097,1103,1109,0};
map <int,int> _mp;


//一些全局变量(用于Music函数)
HMIDIOUT handle   ;
  //在采用指令式的控制方式后下边的几个变量都很好处理了
int  volume = 0x7f ; // 音量
int  stime  = SPEED  ; // 音时
                         // 它们的修改现在都可以用一条指令解决,不弄只是因为懒
bool pedal  = 0      ; // 踏板
                         // 默认为抬起(0),踩下(1)时将声音停止改为每16单位(一小节)一次
int  cnt    = 10     ; // 小节计时器
                         // count元素的每一个单位表示四分之一拍,初值表示弱起小节,目前的唯一用途是控制踏板

//再为乐谱做一下结构上的准备,使用双结构体记谱
struct v_spo {   // "Voice_SPO"
    //作为 msc结构体 的基本组成单位
    //考虑到使用上的问题,一个通道上仅允许同时演奏三个音是完全可以接受的,再多的话泛用性也不会提高多少,所以只装三个音
    int _v1,_v2,_v3;
};
struct msc {
    //当前的发声过程共有五个参数:通道、乐器、音量、音高、时值
        //按现在这一版的处理方法,通道和时值自动处理,乐器和通道一起处理(而且我打算只用大钢琴)
        //音量难以处理 ( 不是说不行,只是有点麻烦,鉴于它只有锦上添花而不是火中送炭的作用,我决定舍弃它 ( 才不是因为懒和笨！) ) 
        //所以这里头只需要放音高就行了
    
    //本曲只用到三个通道
    v_spo v0,v1,v2  ;    
    int C           ; // Command,命令,用于处理一些指令
                      // 现在能处理的指令 : Pedal,用于改变踏板状态(是的只有这一个)
};

//上方定义参数
//#######分界线###########################################################
//下方记录乐谱

//乐谱
msc music[] = 
{
    // 正在做了(进度%100):

    /* 表示小节线 */  
    // 用缩进区分乐句和小节
    // 如果时间充足的话,我会在每个乐句旁附上歌词,也算是增加可读性
        //时间充足，但是这个歌词加上没有意义，不整了

    // 设置速度示例 : 在第五次改板子之后,暂时不想碰这玩意

    // 每一个 四分之一小节 用一个复合结构体(其实是用三元的三元组加一个指令符表示,但我不知道怎么叫)来表示。
        // (恐怖的码量,越尝试做这种项目越能体会到科技的力量。)
        // (真的很吸引人……哪怕仅仅接触到她的冰山一角,哪怕我之前也做过比这个更有技术含量的……但只有这次真的觉得自己学的是有用的,真的投入进去了)
    
    // 每一个单元的格式示例 {{R,R,R},{R,R,R},{R,R,R},P},
    // 其中每一个R都可以用一个音来代替,每一组中第一个R可以使用S
        //最后的可用命令 :  
               //            0   无命令
               //            P   改变踏板状态
               //          _V[i] 控制音量 ( 0 < i < 14 ) 
               
    //能感觉到音多的时候速度慢了很多,考虑到踏板的性能损耗,也许我应该在踏板踩下的时候稍微加一下速

    //为了写着方便，我给它分了三段
    //Part 1 
    
{{E5,A4,R},{S,S,S},{S,S,S},_V[6]}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E4,A4,C5},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{C5,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{G1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E4,A4,C5},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{C5,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    

    {{S,S,S},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{G1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{R,R,R},{A1,R,R},_V[9]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5,A4,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{C4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{S,S,S},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{D4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5,A4,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{F4,D4,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    

    {{E6,E5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,E5,C5},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E4,A4,C5},{R,R,R},{A2,A1,R},P}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{C5,R,R},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,R,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{D4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E4,A4,C5},{R,R,R},{A2,A1,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{C5,R,R},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,R,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    

    {{S,S,S},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{D4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{R,R,R},{A2,A1,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5,A4,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{S,S,S},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{D5,A4,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{S,S,S},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{S,S,S},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{D5,A4,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{S,S,S},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{A4,F4,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{S,S,S},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{S,S,S},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5,A4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{E5,E4,R},{F3,F2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{D7,D6,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    

    {{E6,E5,R},{R,R,R},{F3,F2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{D7,D6,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,E5,C5},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{S,S,S},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{A6,A5,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{A2,A1,R},_V[12]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{S,S,S},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,C5,E4},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{S,S,S},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,A4,D4},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,C5,E4},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,G4,D4},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,D5,G4},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,D5,E4},{R,R,R},{C4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,D5,E4},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,D5,G4},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{D5,A4,D4},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    

    {{E5,E4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5,C4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F4,C4,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{A2,A1,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,C5,E4},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,A4,D4},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,C5,E4},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,G4,D4},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,D5,G4},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,D5,E4},{R,R,R},{C4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,D5,E4},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,D5,G4},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{D5,A4,D4},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    

    {{E5,E4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5,C4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F4,C4,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{R,R,R},{A2,A1,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5,A4,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{S,S,S},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{C4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5,A4,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{D5,A4,D4},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    

    {{E5,E4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5,C4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F4,C4,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},P}, 
    


    //Part 2
    


{{S,S,S},{S,S,S},{A1,R,R},_V[9]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{B1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{B1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D4,D5,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,A4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{G1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E4,A4,C5},{R,R,R},{A2,A1,R},P}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{C5,R,R},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,R,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{D4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    

    {{E4,A4,C5},{R,R,R},{A2,A1,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{C5,R,R},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,R,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A4,R,R},{R,R,R},{D4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{R,R,R},{A2,A1,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5,A4,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{S,S,S},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    

    {{E5,E4,R},{R,R,R},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5,A4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{F3,F2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{F3,F2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,E5,C5},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{S,S,S},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{A2,A1,R},_V[12]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,C5,E4},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,A4,D4},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,C5,E4},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,G4,D4},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    

    {{S,S,S},{R,R,R},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,D5,G4},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,D5,E4},{R,R,R},{C4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,D5,E4},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,D5,G4},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{D5,A4,D4},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5,C4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F4,C4,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{A2,A1,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,C5,E4},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,A4,D4},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,C5,E4},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,G4,D4},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    

    {{S,S,S},{R,R,R},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,D5,G4},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,D5,E4},{R,R,R},{C4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,D5,E4},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,D5,G4},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{D5,A4,D4},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5,C4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F4,C4,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{R,R,R},{A2,A1,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5,A4,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{S,S,S},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    

    {{E5,E4,R},{R,R,R},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{C4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5,A4,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{D5,A4,D4},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5,C4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F4,C4,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    



    //Part 3
    



{{S,S,S},{S,S,S},{A3,A2,R},_V[9]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,R,R},{R,R,R},{B3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,R,R},{R,R,R},{B3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,R,R},{R,R,R},{C4,R,R},0}, {{S,S,S},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,R,R},{R,R,R},{E4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{E4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{D5s,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{D5,R,R},{R,R,R},{D4,R,R},0}, {{S,S,S},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{D4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,R,R},{R,R,R},{E4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{E4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{C5,D5,R},{R,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,R,R},{R,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,R,R},{R,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{R,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,R,R},{S,S,S},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{E2,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,R,R},{E2,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    

    {{A4,R,R},{S,S,S},{A2,A1,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G4,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A4,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E4,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D4,R,R},{R,R,R},{B2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C4,R,R},{R,R,R},{B2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{D4,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{E4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E4,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C4,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C4,R,R},{R,R,R},{E3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C4,R,R},{R,R,R},{E3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{A4,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G4,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G4,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A4,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E4,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D4,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C4,R,R},{R,R,R},{E3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D4,R,R},{R,R,R},{E3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{D4,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C4,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C4,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C4,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{C4,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{S,S,S},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C4,C5,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C4,C5,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    

    {{A5,A4,R},{R,R,R},{A0,A1,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{A1,A2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{A1,A2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A1,A2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{A1,A2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A1,A2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{B1,B2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{B1,B2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{D5,D4,R},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,E4,R},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,E4,R},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{E3,E2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{E3,E2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{A5,A4,R},{R,R,R},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{D5,D4,R},{R,R,R},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{E3,E2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{E3,E2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{D5,D4,R},{R,R,R},{F3,F2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,F2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5,C4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,F2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,C4,R},{R,R,R},{F3,F2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{F3,F2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,F2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,F2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,F2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},P}, 
    

    {{S,S,S},{R,R,R},{A2,R,R},_V[3]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G4,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E4,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G4,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G4,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E4,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G4,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G4,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E4,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G4,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{D4,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D4,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E4,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C4,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C4,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    

    {{S,S,S},{R,R,R},{A2,R,R},_V[4]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,E4,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{S,S,S},_V[5]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{C2,R,R},_V[6]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,E4,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{S,S,S},_V[7]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{S,S,S},{R,R,R},{D2,R,R},_V[8]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{E5,E4,R},{R,R,R},{D4,F3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{S,S,S},_V[9]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,D4,R},{R,R,R},{D4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{G5,D5,G4},{R,R,R},{F2,R,R},_V[10]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,D5,G4},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,D5,G4},{R,R,R},{F4,C4,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{A5,E5,A4},{R,R,R},{S,S,S},_V[11]}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,G5,C5},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F4,C4,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6,G5,C5},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},_V[13]}, 
    

    {{E5,C5,E4},{R,R,R},{A2,A1,R},P}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5,A4,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{R,R,R},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{S,S,S},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{R,R,R},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{D4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D3,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5,A4,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{G5,D5,G4},{S,S,S},{F3,F2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,D5,G4},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,D5,G4},{R,R,R},{F4,C4,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,C5,G4},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{A5,E5,A4},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,G5,C5},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F4,C4,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C6,G5,C5},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{S,S,S},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,A1,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{E5,E4,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{E5,E4,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{G5,G4,R},{A3,E3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5,A4,R},{R,R,R},{A2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{D5,D4,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,E4,R},{S,S,S},{C3,C2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{E5,E4,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{E5,E4,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C2,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{G5,G4,R},{C4,G3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{S,S,S},{R,R,R},{C3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{D5,D4,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{C4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,C5,E4},{S,S,S},{D3,D2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{R,R,R},{D4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{G5,G4,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{A5,A4,R},{E5,E4,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E6,E5,R},{E5,E4,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D3,R,R},0}, {{E6,E5,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C6,C5,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{G5,G4,R},{D4,A3,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{A5,A4,R},{R,R,R},{D3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{G5,G4,R},{D5,D4,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{D4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{D5,G4,D4},{S,S,S},{F3,F2,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,G4,D4},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{D5,G4,D4},{R,R,R},{F4,C4,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,G4,C4},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{E5,A4,E4},{S,S,S},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{C5,G4,E4},{C4,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{F4,C4,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F3,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{C5,G4,E4},{C4,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{F4,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},P}, 
    
    {{S,S,S},{S,S,S},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{C2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{D2,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{G1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    {{R,R,R},{R,R,R},{A1,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{S,S,S},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
        {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, {{R,R,R},{R,R,R},{R,R,R},0}, 
    
    
};

//上方记录乐谱
//#######分界线###########################################################
//下方声明核心函数

//预处理
void init();
//核心函数
void MakeSound_0 (msc) ;
void MakeSound_1 (msc) ;
void MakeSound_2 (msc) ;
void Music() ;
int main() { //真是充实的主函数呐(不是)
    init();
    Music()  ;
    return 0 ;
}

//上方主函数
//#######分界线###########################################################
//下方定义函数

void init(){
    _mp[R] = PA ;
    _mp[P] = PA ;
    _mp[S] = PA ;
    _mp[0] = PA ;
    _mp[ _V[1] ] = 0x1f;
    for(int i=2;i<=13;i++) _mp[ _V[i] ] = _mp[ _V[i-1] ] + 8 ;
}

void MakeSound_0 (v_spo _m) {    //第零通道
    //特判由休止符造成的停止发声 
    if( _m._v1 == S ){
        //与else语句的区别是停止后就不发声,打算用于休止符
        if ( !pedal ) // 当踏板被踩下时不进行常规停止
            midiOutShortMsg ( handle, 0x7BB0 ) ; //midiOut短消息B类
    }
    //开始发声
    else {
        //发出新的声音之前先停止上一个声音
        if ( !pedal ) // 当踏板被踩下时不进行常规停止
            midiOutShortMsg ( handle, 0x7BB0 ) ; //midiOut短消息B类
        //定义声音变量voice并发声
        if ( _m._v1 != R ) {    // "_v1" 当然是不用判断的，但是为了保证队形整齐还是这么写了
            int voice = (volume << 16) | ( (_m._v1) << 8) | 0x90 ; //音量|音高|通道
            midiOutShortMsg ( handle, voice ) ; //midiOut短消息9类//演奏
        }
        if ( _m._v2 != R ) {
            int voice = (volume << 16) | ( (_m._v2) << 8) | 0x90 ; //音量|音高|通道
            midiOutShortMsg ( handle, voice ) ; //midiOut短消息9类//演奏
        }
        if ( _m._v3 != R ) {
            int voice = (volume << 16) | ( (_m._v3) << 8) | 0x90 ; //音量|音高|通道
            midiOutShortMsg ( handle, voice ) ; //midiOut短消息9类//演奏
        }
    }
}
void MakeSound_1 (v_spo _m) {    //第一通道
    //特判由休止符造成的停止发声 
    if( _m._v1 == S ){
        //与else语句的区别是停止后就不发声,打算用于休止符
        if ( !pedal ) // 当踏板被踩下时不进行常规停止
            midiOutShortMsg ( handle, 0x7BB1 ) ; //midiOut短消息B类
    }
    //开始发声
    else {
        //发出新的声音之前先停止上一个声音
        if ( !pedal ) // 当踏板被踩下时不进行常规停止
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
void MakeSound_2 (v_spo _m) {    //第二通道
    //特判由休止符造成的停止发声 
    if( _m._v1 == S ){
        //与else语句的区别是停止后就不发声,打算用于休止符
        if ( !pedal ) // 当踏板被踩下时不进行常规停止
            midiOutShortMsg ( handle, 0x7BB2 ) ; //midiOut短消息B类
    }
    //开始发声
    else {
        //发出新的声音之前先停止上一个声音
        if ( !pedal ) // 当踏板被踩下时不进行常规停止
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
void Music ()
{
    midiOutOpen (&handle, 0, 0, 0, CALLBACK_NULL) ; //打开MIDI设备

    midiOutShortMsg (handle, 0 << 8 | 0xC0) ; //midiOut短消息C类 //声明乐器(1:明亮的钢琴)、通道0(右手一声部)
    midiOutShortMsg (handle, 0 << 8 | 0xC1) ; //midiOut短消息C类 //声明乐器(1:明亮的钢琴)、通道1(右手二声部)
    midiOutShortMsg (handle, 0 << 8 | 0xC2) ; //midiOut短消息C类 //声明乐器(0:大钢琴)、通道2(左手一声部)
    //遍历乐谱
    for (auto i : music) {
        msc m=i;
        cnt -- ; // 四分之一小节过去了
        //分通道发声
        //因为同一通道里的音同时开始和停止,所以只判断 "_v1" 的值即可 
        if ( m.v0._v1 != R ) MakeSound_0 ( m.v0 ) ;
        if ( m.v1._v1 != R ) MakeSound_1 ( m.v1 ) ;
        if ( m.v2._v1 != R ) MakeSound_2 ( m.v2 ) ;
        //保持音时
        Sleep ( stime ) ; // 四分之一小节
        //处理指令
        if ( m.C == P )  pedal = !pedal ; //改变踏板状态
        if ( _mp[m.C] != PA ) volume = _mp[ m.C ] ; //调节音量

        // 处理小节结束
        if ( cnt == 0 ) {
            cnt = 16 ;
            // printf("One Bar Has Passed ...\n") ; // 一小节过去了
            if ( pedal ) { // 处理踏板
                midiOutShortMsg ( handle, 0X7BB0 ) ; // 0X7B是结束指令,B0是通道
                midiOutShortMsg ( handle, 0X7BB1 ) ;
                midiOutShortMsg ( handle, 0X7BB2 ) ;
            }
        }
    }
    midiOutClose (handle) ; //关闭MIDI
}
