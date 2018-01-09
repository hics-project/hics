//
// Created by jallmenroeder on 07/01/18.
//

#ifndef HICS_PERFORMANCEDATA_H
#define HICS_PERFORMANCEDATA_H


#include <PlatformInfo.h>
#include <map>

class PerformanceData {
private:
    int PowerConsumption;       //saved in ws (watt seconds)
    int ComputationTime;        //saved in ms
    std::map<PlatformInfo, float> platformUsage;    //maps each platform to a percentage of usage

public:
    int getPowerConsumption() const;

    int getComputationTime() const;

    const std::map<PlatformInfo, float> &getPlatformUsage() const;
};


#endif //HICS_PERFORMANCEDATA_H
