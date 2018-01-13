//
// Created by Maximilian Franz on 07.01.18.
//

#include "ConvolutionLayer.h"
#include "NotImplementedException.h"

const int X_DIM = 2;
const int Y_DIM = 1;
const int Z_DIM = 0;

void ConvolutionLayer::forward(DataWrapper &input, DataWrapper &output) {
    throw NotImplementedException();
}


//TODO: Test this!
std::vector<int> ConvolutionLayer::calcOutputDimensions() {
    int outputWidth = (this->inputDimensions[D3_X_DIM] - filterSize) / stride + 1;
    std::vector<int> outDim(3); // three dimensional output
    outDim[X_DIM] = outputWidth; //X, Y are the same!
    outDim[Y_DIM] = outputWidth;
    outDim[Z_DIM] = numFilters;

}


void ConvolutionLayer::setWeights(WeightWrapper* weights) {

}

void ConvolutionLayer::setBias(WeightWrapper* bias) {

}

void ConvolutionLayer::setFunction(LayerFunction *function) {

}

bool ConvolutionLayer::verifyWeights() {
    return false;
}

ConvolutionLayer::ConvolutionLayer(int numFilters, int filterSize, int zeroPadding, int stride,
                                   std::vector<int> &inputDimensions)
        : numFilters(numFilters),
          filterSize(filterSize),
          zeroPadding(zeroPadding),
          stride(stride) {
    this->inputDimensions = inputDimensions;
    this->type = "convolution";
    this->outputDimensions = calcOutputDimensions();
    init();
}

ConvolutionLayer::ConvolutionLayer(int numFilters, int filterSize, int zeroPadding, int stride,
                                   std::vector<int> &inputDimensions, WeightWrapper *weights, WeightWrapper *bias)
        : numFilters(numFilters),
          filterSize(filterSize),
          zeroPadding(zeroPadding),
          stride(stride),
          weights(weights),
          bias(bias)
{
    this->inputDimensions = inputDimensions;
    this->type = "convolution";
    this->outputDimensions = calcOutputDimensions();
    init();
}

int ConvolutionLayer::getNumFilters() const {
    return numFilters;
}

int ConvolutionLayer::getFilterSize() const {
    return filterSize;
}

int ConvolutionLayer::getZeroPadding() const {
    return zeroPadding;
}

int ConvolutionLayer::getStride() const {
    return stride;
}

