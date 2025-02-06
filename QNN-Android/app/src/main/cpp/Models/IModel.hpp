//
// Created by user on 2025-02-06.
//

#ifndef APP_IMODEL_HPP
#define APP_IMODEL_HPP


#pragma once
#include <string>
#include <opencv2/opencv.hpp>

/**
 * A generic interface for any model that follows:
 *  - initialize(...)
 *  - preprocess(...)
 *  - inference()
 *  - postprocess()
 */
class IModel {
public:
    virtual ~IModel() = default;

    /**
     * Initialize resources (load weights, set up runtimes, etc.).
     */
    virtual bool initialize(const std::string& device,
                            const std::string& modelName,
                            const std::string& backend,
                            const std::string& precision,
                            const std::string& framework) = 0;

    /**
     * Preprocess input data (e.g., YUV->RGB, resize, normalize, etc.).
     *
     * @param inputData  raw pointer to input (e.g., YUV buffer).
     * @param width      image width.
     * @param height     image height.
     * @return           true on success, false on error.
     */
    virtual bool preprocess(const uint8_t* inputData, int width, int height) = 0;

    /**
     * Run forward inference on the preprocessed data.
     *
     * @return true on success, false on error.
     */
    virtual bool inference() = 0;

    /**
     * Postprocess the outputs. Typically returns pointers
     * or data structures with final results (bounding boxes, depth map, etc.).
     *
     * Because the results can vary by model, we use a void* here
     * or a generic pointer. You could also define a union or a polymorphic
     * return type if desired.
     *
     * @return pointer to postprocessed results (must be cast or interpreted).
     */
    virtual void* postprocess() = 0;
};


#endif //APP_IMODEL_HPP
