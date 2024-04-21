#ifndef ACTIVATE_CONSTANTS_H
#define ACTIVATE_CONSTANTS_H 1

//存放本程序所使用到的各个变参数

#include "globals.hh"
#include "G4SystemOfUnits.hh"
namespace myConsts {

static constexpr G4double gGountBeginTime = 10.*CLHEP::s; // 统计沉积能量最小起始时刻，若小于该时刻的沉积能量，则不统计
static constexpr G4double gTimeWidth = 10.*CLHEP::ns; // 统计一个时间的时间窗，若大于该时间，则认为是两个事件
static constexpr G4double gTarckMaxTimeLimit = 40.*24.*3600.*CLHEP::s; //统计径迹的最大允许时间，超过改时间，则杀死粒子。
static constexpr G4double gEventEdepThreshold = 0.1*keV; //单个事件测量能量下限
static constexpr G4double gTrackEdepThreshold = 0.1*keV; //单个粒子径迹测量能量下限

}

#endif