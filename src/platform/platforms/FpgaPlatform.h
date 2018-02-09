#pragma once

#include "CL/opencl.h"

#include "Platform.h"


class FpgaPlatform : public Platform {
private:
    cl_context context;
    cl_device_id device;
    void init();

public:
    ActivationFunction *createActivationFunction(LayerType type) override;

    ConvolutionFunction *createConvolutionFunction() override;

    LossFunction *createLossFunction(LayerType type) override;

    PoolingFunction *createPoolingFunction(LayerType type) override;

    ResponseNormalizationFunction *createResponseNormalizationFunction(LayerType type) override;

    FullyConnectedFunction *createFullyConnectedFunction() override;

    PlatformInfo &getPlatformInfo() override;

    PlatformType getPlatformType() override;

    FpgaPlatform();

    FpgaPlatform(PlatformInfo &info);

    ~FpgaPlatform();
};
