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

#include <ctime>
#include <Client.h>
#include <thread>
#include <fstream>
#include <ResourceException.h>
#include <spdlog/spdlog.h>
#include <CommunicationException.h>

#include "Manager.h"
#include "PreProcessor.h"
#include "PerformanceCalculator.h"
#include "HostPlacer.h"

typedef unsigned int uint;

std::string getHostAdress(std::string hostname);

static std::exception_ptr exceptionptr = nullptr;

static std::shared_ptr<spdlog::logger> logger;

//function for thread to run classification in
void runClassification(ComputationHost* host,
                       std::vector<ImageResult*>& allResults,
                       std::vector<ImageWrapper*> img,
                       NetInfo net,
                       OperationMode mode,
                       std::vector<PlatformInfo*> selecedPlatforms,
                       std::chrono::steady_clock::time_point startTime,
                       int& allTimes) {
    try {
        allResults = host->classify(std::move(img), std::move(net), mode, std::move(selecedPlatforms));
    } catch (std::exception& e) {
        logger->warn("classification of host {} failed: {}", host->getName(), e.what());
        exceptionptr = std::current_exception();
    }
    std::chrono::steady_clock::time_point timeAfter = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(timeAfter - startTime);
    allTimes = (int)diff.count();
}

Manager::Manager() {

    try {
        // Create stdout logger, if not existent
        if (!logger) {
            logger = spdlog::stdout_color_mt("logger");
        }

        logger->set_level(spdlog::level::info);
        logger->flush_on(spdlog::level::info);
        logger->info("logger initialization successful");

    } catch (const spdlog::spdlog_ex& ex) {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
        exceptionptr = std::current_exception();
    }

    //try to reach the fpga server, delete fpga host if it fails
    std::string hostAdress;
    ComputationHost* client;
    std::string clientName = "fpga";
    try {
        hostAdress = getHostAdress(clientName);
        client = new Client(clientName, grpc::CreateChannel(
                hostAdress, grpc::InsecureChannelCredentials()));

        try {
            client->queryNets();
            computationHosts.push_back(client);
        } catch (CommunicationException& c) {
            logger->warn("host at {} was not reachable, will be deleted", hostAdress);
            delete client;
        }
    } catch (ResourceException& r) {
        logger->warn("error while reading host adress of host {}: {}. Host will be disabled.",
                     clientName, r.what());
    }

    //initialize the local computation host
    try {
        Executor *executor = new Executor("local");
        computationHosts.push_back(executor);
    } catch (ResourceException &r) {
        logger->critical("could not read platforms of host {}, maybe the platforms.json is corrupted?");
        exceptionptr = std::current_exception();
    }
}

void Manager::initGUI() {
    //query all nets
    std::vector<std::vector<NetInfo*>> allNets;
    for (auto host : computationHosts) {
        try {
            allNets.push_back(host->queryNets());
        } catch (CommunicationException& c) {
            logger->warn("could not query nets from remote host {}, host will be deleted.",
                         c.getFailedHost()->getName());
            //remove failed host
            computationHosts.erase(std::find(computationHosts.begin(), computationHosts.end(), c.getFailedHost()));
        } catch (ResourceException& r) {
            logger->critical("error while querying nets: {}. Aborting!", r.what());
            exceptionptr = std::current_exception();
        }
    }

    //only show nets available on all hosts
    std::vector<NetInfo*> nets;
    if (!exceptionptr) {
        nets = netIntersection(allNets);
    }

    //query platforms
    auto platforms = std::vector<PlatformInfo*>();
    for (auto host : computationHosts) {
        auto newPlatforms = host->queryPlatform();
        platforms.insert(platforms.end(), newPlatforms.begin(), newPlatforms.end());
    }
    std::vector<OperationMode> modes{OperationMode::HighPower, OperationMode::LowPower, OperationMode::EnergyEfficient};

    //initialize GUI main window
    mainWindowHandler = new MainWindowHandler(nets, platforms, modes);

    //Check if exception occured while setting up System
    if (exceptionptr) {
        mainWindowHandler->setExceptionptr(exceptionptr);

        //Null exceptionptr for next classification
        exceptionptr = nullptr;
    }
    //attach the manager as an observer to the GUI
    mainWindowHandler->attach(this);
    mainWindowHandler->init();
}

ClassificationResult* Manager::update() {
    std::vector<ComputationHost *> availableHosts;
    std::vector<std::vector<ImageWrapper *>> batches;
    std::vector<int> compTime;
    std::vector<std::vector<ImageResult *>> allResults;
    std::vector<std::pair<ComputationHost *, int>> hostDistribution;

    ClassificationRequest *request = mainWindowHandler->getClassificationRequestState();

    //preprocess images
    PreProcessor processor = PreProcessor();
    processor.setOutputSize(request->getSelectedNeuralNet().getImageDimension(),
                            request->getSelectedNeuralNet().getImageDimension());

    std::vector<ImageWrapper *> processedImages = processor.processImages(request->getUserImages());

    //Check which platforms of each host were selected
    auto hostPlatforms = std::vector<std::vector<PlatformInfo *>>(computationHosts.size());
    std::vector<PlatformInfo *> selectedPlatforms = request->getSelectedPlatforms();

    try {
        for (uint i = 0; i < computationHosts.size(); i++) {
            for (auto hostPlatIt : computationHosts[i]->queryPlatform()) {
                auto platIt = std::find_if(selectedPlatforms.begin(), selectedPlatforms.end(),
                                           [&hostPlatIt](PlatformInfo *temp) {
                                               return temp->getPlatformId() == hostPlatIt->getPlatformId();
                                           });
                if (platIt != selectedPlatforms.end()) {
                    hostPlatforms[i].push_back(*platIt);
                }
            }
        }
    } catch (std::exception &e) {
        exceptionptr = std::current_exception();
    }

    //Remove a computationHost if none of its Platforms are selected
    availableHosts = computationHosts;
    for (uint i = 0; i < availableHosts.size(); i++) {
        if (hostPlatforms[i].empty()) {
            ComputationHost *currentHost = availableHosts[i];
            availableHosts.erase(std::find_if(availableHosts.begin(),
                                              availableHosts.end(),
                                              [&currentHost](ComputationHost *temp) {
                                                  return temp->getName() == currentHost->getName();
                                              }));
            hostPlatforms.erase(hostPlatforms.begin() + i);
        }
    }

    //place the images to computation hosts
    try {
        hostDistribution = HostPlacer::place(availableHosts, (int) processedImages.size(),
                                             request->getSelectedOperationMode());
    } catch (ResourceException &r) {
        logger->critical(r.what());
        exceptionptr = std::current_exception();
    }

    if (!exceptionptr) {

        //create batches for each computation host according to the host placement
        batches = std::vector<std::vector<ImageWrapper *>>(availableHosts.size());
        int hostIndex = 0;
        int imageIndex = 0;
        for (auto imageIt : processedImages) {
            if (imageIndex < hostDistribution[hostIndex].second) {
                batches[hostIndex].push_back(imageIt);
                imageIndex++;
            } else {
                hostIndex++;
                imageIndex = 0;
                batches[hostIndex].push_back(imageIt);
            }
        }

        //initialize result and time variables to be set in the classification
        allResults = std::vector<std::vector<ImageResult *>>(availableHosts.size());

        std::chrono::steady_clock::time_point time, timeAfter;
        std::chrono::milliseconds diff = std::chrono::milliseconds();

        compTime = std::vector<int>(availableHosts.size());

        std::vector<std::thread> classifyThreads;

        //create a thread for each computationhost to run the classification in parallel
        //since every host represents a whole system, there are no shared resoures to worry about
        for (uint i = 0; i < availableHosts.size(); i++) {
            if (!batches[i].empty()) {
                time = std::chrono::steady_clock::now();
                std::thread t(runClassification, availableHosts[i],
                              std::ref(allResults[i]),
                              batches[i],
                              request->getSelectedNeuralNet(),
                              request->getSelectedOperationMode(),
                              hostPlatforms[i],
                              time,
                              std::ref(compTime[i]));
                classifyThreads.push_back(std::move(t));
            } else {
                //dummy value for hosts that did not compute anything
                allResults[i] = std::vector<ImageResult *>();
                compTime[i] = 1;
            }
        }

        //join all classification threads
        for (uint i = 0; i < classifyThreads.size(); i++) {
            classifyThreads[i].join();
        }
    }

    //check if the classification went wrong
    if (exceptionptr) {
        mainWindowHandler->setExceptionptr(exceptionptr);

        //if a communication exception occured, delete the associated computationHost
        try {
            std::rethrow_exception(exceptionptr);
        } catch (CommunicationException& c) {
            logger->warn("remove remote computation host {} because it failed the classification",
                         c.getFailedHost()->getName());
            auto failedHost = std::find(computationHosts.begin(), computationHosts.end(), c.getFailedHost());
            computationHosts.erase(failedHost);
            for (auto host : computationHosts) {
                logger->debug("remaining host: {}", host->getName());
            }
            logger->debug("remaining hosts size: {}", computationHosts.size());


            //Update available platforms
            auto availablePlatforms = new std::vector<PlatformInfo *>;
            for (auto host : computationHosts) {
                auto currentPlatforms = host->queryPlatform();
                for (auto available : currentPlatforms) {
                    availablePlatforms->push_back(available);
                    logger->debug("Platform {} still available", available->getDescription());
                }
            }

            std::shared_ptr<std::vector<PlatformInfo *>> updatedPlatforms(availablePlatforms);
            mainWindowHandler->updatePlatforms(updatedPlatforms);
        } catch (...) {
            //If its not a communication exception there is nothing to handle, the gui already has the exception pointer
        }

        //Null exceptionptr for next classification
        exceptionptr = nullptr;
        return nullptr;
    }

    auto hosts = std::vector<PerformanceCalculator::HostInfo*>();

    //read performance of each host
    for (uint i = 0; i < availableHosts.size(); i++) {
        auto newHost = new PerformanceCalculator::HostInfo(availableHosts[i]->getName(),
                                                           float(batches[i].size()) / float(processedImages.size()),
                                                           compTime[i]);
        hosts.push_back(newHost);
    }

    std::vector<std::vector<std::pair<PlatformInfo*, float>>> calculateInfo;

    //Add the ComputationDistribution of each host to the calculateInfo
    for (uint i = 0; i < batches.size(); i++) {
        if (!batches[i].empty()) {
            std::vector<std::pair<PlatformInfo *, float>> distr = allResults[i].front()->getCompDistribution();
            calculateInfo.push_back(distr);
        } else {
            std::vector<std::pair<PlatformInfo*, float>> distr;
            for (auto hostPlatIt : hostPlatforms[i]) {
                distr.emplace_back(hostPlatIt, 0);
            }
            calculateInfo.push_back(distr);
        }
    }

    //calculate the performance data for the classification
    PerformanceData performanceData = PerformanceCalculator::calculatePerformance(calculateInfo, hosts);

    std::vector<ImageResult> newResults;

    //merge the results from each host back together to one large result vector
    for (auto hostResults : allResults){
            for (auto singleResult : hostResults) {
                newResults.push_back(*singleResult);
                delete singleResult;
            }
    }

    ClassificationResult* result = new ClassificationResult(newResults, request->getSelectedNeuralNet(), performanceData);

    if(request->getAggregateResults()){
        result->aggregateResults();
    }

    return result;
}

bool Manager::operator==(const ManagerObserver &managerObserver){
    return this == &managerObserver;
}

std::vector<NetInfo *> Manager::netIntersection(std::vector<std::vector<NetInfo*>> &allNets) {
    auto nets = allNets[0];

    for (auto netIt = allNets.begin() + 1; netIt != allNets.end(); netIt++) {
        auto newNets = netIt.operator*();
        for (auto net : nets) {
            if (std::find_if(newNets.begin(), newNets.end(), [&net](NetInfo* newNet) {
                return newNet->getIdentifier() == net->getIdentifier();
            }) == newNets.end()) {

                nets.erase(std::remove_if(nets.begin(), nets.end(), [&net](NetInfo* newNet) {
                    return newNet->getIdentifier() == net->getIdentifier();
                }));
            }
        }
    }

    return nets;
}

Manager::~Manager() {

}

/**
 * Reads the Adress from a computation host specified in the computations.json (only for remote hosts)
 * @param hostname  the name of the computationhost in the .json file
 * @return adress of the computationHost
 */
std::string getHostAdress(std::string hostname) {
    json computationHostFile;
    try {
        std::ifstream i(RES_DIR "computationHosts.json");
        i >> computationHostFile;
    } catch (...) {
        throw ResourceException("Error while reading computationHosots.json");
    }

    json computationHost = computationHostFile["computationHosts"];
    for (auto compHostIt : computationHost) {
        if (compHostIt["name"] == hostname) {
            return compHostIt["host"];
        }
    }
    throw ResourceException("Host name not found in .json file");
}