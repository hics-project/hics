/* Copyright 2018 The HICS Authors
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall
 * be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>
#include <wrapper/DataWrapper.h>
#include <wrapper/WeightWrapper.h>
#include <platforms/Platform.h>

#include "LayerType.h"


/**
 * Abstract class Layer defines the public interface for all layers contained in a NeuralNet.
 */
class Layer {
protected:

    Layer* previousLayer = nullptr;
    Layer* nextLayer = nullptr;
    DataWrapper* inputWrapper; //! == previousLayer.getOutputWrapper()
    DataWrapper* outputWrapper; //! == nextLayer.getInputWrapper()

    bool computed = false;
    bool functionSet = false;
    int difficulty = 0; //! A relative sign for the difficulty of this layer / amount of computation

    LayerType type;
    std::vector<int> inputDimensions;
    std::vector<int> outputDimensions;


public:
    /**
     * Dimension positions in the dimension vectors according to convention in 3D
     */
    const int D3_X_DIM = 2;
    const int D3_Y_DIM = 1;
    const int D3_Z_DIM = 0;


    /**
     * Triggers the computation of the forward propagation. Takes input from inputWrapper and writes to outputWrapper.
     * The Executor knows the size the output Wrapper needs by querying getOutputDimensions()
     */
    virtual void forward() = 0;

    /**
     * Set the platform to be used to create the function that performs the computations of the layer.
     *
     * @param platform      The platform to be used as a LayerFunction factory.
     */
    virtual void setPlatform(Platform *platform) = 0;

    /**
     *
     * @return
     */
    virtual bool isPlatformSet();

    /**
     * Returns an approximation of the number of necessary computations in this layer, which indicates the difficulty of this layer.
     *
     * @return long value: difficulty of this layer
     */
    virtual int getDifficulty() = 0;

    /**
     * Initializes the default values of this layer
     *
     * This means: functionsSet is false
     *             computed is false
     *
     */
    void init();

    /**
     * Set the preceeding layer of a layer.
     *
     * @param previousLayer     A pointer to the preceeding layer.
     */
    virtual void setPreviousLayer(Layer *previousLayer);

    /**
     * Set next layer by providing a pointer
     *
     * @param nextLayer which to append to this one.
     */
    virtual void setNextLayer(Layer *nextLayer);

    /**
     * Calculates the output dimensions of the layer given the inputDimensions of this instance and it's parameters.
     *
     * Different implementation for each layer type.
     *
     * @return the Dimensions the output Wrapper has to have.
     */
    virtual std::vector<int> calcOutputDimensions() = 0;

    /**
     * Returns the dimensions of the output wrapper.
     *
     * @return dimensions of the output Wrapper.
     */
    virtual std::vector<int> getOutputDimensions() const;

    /**
     * Returns a pointer to the previous layer
     * @return a pointer to the previous layer.
     */
    virtual Layer *getPreviousLayer() const;

    /**
     *
     * @return a pointer to the next Layer after this one.
     */
    Layer *getNextLayer() const;

    /**
     *
     * @return type enum of this layer
     */
    LayerType getType() const;

    /**
     *  Getter
     * @return dimensions of the inputWrapper to this
     */
    const std::vector<int> &getInputDimensions() const;

    /**
     * Set inputWrapper explicitly
     * @param inputWrapper
     */
    void setInputWrapper(DataWrapper *inputWrapper);

    /**
     * Getter for outputWrapper
     * @return outputWrapper
     */
    DataWrapper *getOutputWrapper() const;


    /**
     * Resets the status of this Layer
     */
    void reset();

    /**
     * Remove obsolete DataWrapper instances
     *
     * Call only after forward() has been comuted
     *
     */
    void deleteGarbage();

    /**
     * Destructor of Layer
     */
    virtual ~Layer();

};

