//
// Created by David Culley on 07.01.18.
//

#pragma once

#include "WeightLoader.h"

class AlexNetWeightLoader : public WeightLoader {
private:

public:
    AlexNetWeightLoader();

    WeightWrapper getWeights(LayerIdentifier layerId) override;
};
