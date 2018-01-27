//
// Created by Maximilian Franz on 08.01.18.
//

#pragma once


#include <string>
#include <map>
#include <wrapper/ImageWrapper.h>


class ImageResult {
private:
    ImageWrapper image;
    std::vector<std::pair<std::string, float>> results; // ordered list of labels and their probabilities
    std::vector<std::pair<PlatformInfo, float>> compDistribution;
public:
    /**
     * Getter for compDistribution, which Platform computed what in percentages.
     * @return compDistribution
     */
    const std::vector<std::pair<PlatformInfo, float>> &getCompDistribution() const;

    /**
     * Sets the compDistribution.
     * @param compDistribution
     */
    void setCompDistribution(const std::vector<std::pair<PlatformInfo, float>> &compDistribution);

    /**\brief Constructor
     *
     * @param results top X
     * @param image
     */
    ImageResult(std::vector<std::pair<std::string, float>> results, ImageWrapper &image);

    /**
     *
     * @return path to the image that was classified
     */
    const std::string getImagePath() const;

    /***
     * Return List of result pairs
     * @return
     */
    const std::vector<std::pair<std::string, float>> &getResults() const;

};

