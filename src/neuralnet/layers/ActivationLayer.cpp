//
// Created by Maximilian Franz on 07.01.18.
//

#include "ActivationLayer.h"
#include "../../NotImplementedException.h"


void ActivationLayer::forward(DataWrapper &input, DataWrapper &output) {
    throw NotImplementedException();
}

ActivationLayer::ActivationLayer(std::vector<int> inputDimensions) : inputDimensions(inputDimensions) {
    //
}

