//
// Created by Maximilian Franz on 07.01.18.
//

#include "ReLUActivationLayer.h"
#include "NotImplementedException.h"


ReLUActivationLayer::ReLUActivationLayer(std::vector<int> &inputDimension) : ActivationLayer(inputDimension) {
    this->type = "relu";
    this->outputDimensions = calcOutputDimensions();
    init();

}