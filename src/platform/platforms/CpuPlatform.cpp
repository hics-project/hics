//
// Created by David Culley on 12.01.18.
//

#include <NotImplementedException.h>

#include "CpuPlatform.h"


ActivationFunction* CpuPlatform::createActivationFunction(LayerType type) {
    throw NotImplementedException();
}

ConvolutionFunction* CpuPlatform::createConvolutionFunction() {
    throw NotImplementedException();
}

LossFunction* CpuPlatform::createLossFunction(LayerType type) {
    throw NotImplementedException();
}

PoolingFunction* CpuPlatform::createPoolingFunction(LayerType type) {
    throw NotImplementedException();
}

ResponseNormalizationFunction* CpuPlatform::createResponseNormalizationFunction(LayerType type) {
    throw NotImplementedException();
}

FullyConnectedFunction* CpuPlatform::createFullyConnectedFunction() {
    throw NotImplementedException();
}

PlatformInfo &CpuPlatform::getPlatformInfo() {
    throw NotImplementedException();
}

PlatformType CpuPlatform::getPlatformType() {
    return PlatformType::CPU;
}
