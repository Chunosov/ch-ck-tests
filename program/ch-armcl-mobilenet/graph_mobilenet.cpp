/*
 * Copyright (c) 2017-2018 ARM Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "arm_compute/graph/Graph.h"
#include "arm_compute/graph/Nodes.h"
#include "arm_compute/runtime/CL/CLScheduler.h"
#include "support/ToolchainSupport.h"
#include "utils/GraphUtils.h"
#include "utils/Utils.h"

#include <cstdlib>

using namespace arm_compute::utils;
using namespace arm_compute::graph;
using namespace arm_compute::graph_utils;

/** Example demonstrating how to implement MobileNet's network using the Compute Library's graph API
 *
 * @param[in] argc Number of arguments
 * @param[in] argv Arguments ( [optional] Target (0 = NEON, 1 = OpenCL), [optional] Path to the weights folder, [optional] image, [optional] labels )
 */
class GraphMobilenetExample : public Example
{
public:
    void do_setup(int argc, char **argv) override
    {
        std::string data_path; /* Path to the trainable data */
        std::string image;     /* Image data */
        std::string label;     /* Label data */

        constexpr float mean = 0.f;   /* Mean value to subtract from the channels */
        constexpr float std  = 255.f; /* Standard deviation value to divide from the channels */

        // Set target. 0 (NEON), 1 (OpenCL). By default it is NEON
        TargetHint            target_hint      = set_target_hint(argc > 1 ? std::strtol(argv[1], nullptr, 10) : 0);
        ConvolutionMethodHint convolution_hint = ConvolutionMethodHint::GEMM;

        // Set model to execute. 0 (MobileNetV1_1.0_224), 1 (MobileNetV1_0.75_160)
        int model_id = (argc > 2) ? std::strtol(argv[2], nullptr, 10) : 0;
        ARM_COMPUTE_ERROR_ON_MSG(model_id > 1, "Invalid model ID. Model must be 0 (MobileNetV1_1.0_224) or 1 (MobileNetV1_0.75_160)");
        float        depth_scale  = (model_id == 0) ? 1.f : 0.75;
        unsigned int spatial_size = (model_id == 0) ? 224 : 160;
        std::string  model_path   = (model_id == 0) ? "/cnn_data/mobilenet_v1_1_224_model/" : "/cnn_data/mobilenet_v1_075_160_model/";

        // Parse arguments
        if(argc < 2)
        {
            // Print help
            std::cout << "Usage: " << argv[0] << " [target] [model] [path_to_data] [image] [labels]\n\n";
            std::cout << "No model ID provided: using MobileNetV1_1.0_224\n\n";
            std::cout << "No data folder provided: using random values\n\n";
        }
        else if(argc == 2)
        {
            std::cout << "Usage: " << argv[0] << " " << argv[1] << " [model] [path_to_data] [image] [labels]\n\n";
            std::cout << "No model ID provided: using MobileNetV1_1.0_224\n\n";
            std::cout << "No data folder provided: using random values\n\n";
        }
        else if(argc == 3)
        {
            std::cout << "Usage: " << argv[0] << " " << argv[1] << " " << argv[2] << " [path_to_data] [image] [labels]\n\n";
            std::cout << "No data folder provided: using random values\n\n";
        }
        else if(argc == 4)
        {
            data_path = argv[3];
            std::cout << "Usage: " << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " [image] [labels]\n\n";
            std::cout << "No image provided: using random values\n\n";
        }
        else if(argc == 5)
        {
            data_path = argv[3];
            image     = argv[4];
            std::cout << "Usage: " << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " [labels]\n\n";
            std::cout << "No text file with labels provided: skipping output accessor\n\n";
        }
        else
        {
            data_path = argv[3];
            image     = argv[4];
            label     = argv[5];
        }

        // Add model path to data path
        if(!data_path.empty())
        {
            data_path += model_path;
        }

        graph << target_hint
              << convolution_hint
              << Tensor(TensorInfo(TensorShape(spatial_size, spatial_size, 3U, 1U), 1, DataType::F32),
                        get_input_accessor(image,
                                           mean, mean, mean,
                                           std, std, std, false /* Do not convert to BGR */))
              << ConvolutionLayer(
                  3U, 3U, 32U * depth_scale,
                  get_weights_accessor(data_path, "Conv2d_0_weights.npy"),
                  std::unique_ptr<arm_compute::graph::ITensorAccessor>(nullptr),
                  PadStrideInfo(2, 2, 0, 1, 0, 1, DimensionRoundingType::FLOOR))
              << BatchNormalizationLayer(
                  get_weights_accessor(data_path, "Conv2d_0_BatchNorm_moving_mean.npy"),
                  get_weights_accessor(data_path, "Conv2d_0_BatchNorm_moving_variance.npy"),
                  get_weights_accessor(data_path, "Conv2d_0_BatchNorm_gamma.npy"),
                  get_weights_accessor(data_path, "Conv2d_0_BatchNorm_beta.npy"),
                  0.001f)
              << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::BOUNDED_RELU, 6.f))

              << get_dwsc_node(data_path, "Conv2d_1", 64 * depth_scale, PadStrideInfo(1, 1, 1, 1), PadStrideInfo(1, 1, 0, 0))
              << get_dwsc_node(data_path, "Conv2d_2", 128 * depth_scale, PadStrideInfo(2, 2, 0, 1, 0, 1, DimensionRoundingType::CEIL), PadStrideInfo(1, 1, 0, 0))
              << get_dwsc_node(data_path, "Conv2d_3", 128 * depth_scale, PadStrideInfo(1, 1, 1, 1, 1, 1, DimensionRoundingType::CEIL), PadStrideInfo(1, 1, 0, 0))
              << get_dwsc_node(data_path, "Conv2d_4", 256 * depth_scale, PadStrideInfo(2, 2, 0, 1, 0, 1, DimensionRoundingType::CEIL), PadStrideInfo(1, 1, 0, 0))
              << get_dwsc_node(data_path, "Conv2d_5", 256 * depth_scale, PadStrideInfo(1, 1, 1, 1, 1, 1, DimensionRoundingType::CEIL), PadStrideInfo(1, 1, 0, 0))
              << get_dwsc_node(data_path, "Conv2d_6", 512 * depth_scale, PadStrideInfo(2, 2, 0, 1, 0, 1, DimensionRoundingType::CEIL), PadStrideInfo(1, 1, 0, 0))
              << get_dwsc_node(data_path, "Conv2d_7", 512 * depth_scale, PadStrideInfo(1, 1, 1, 1, 1, 1, DimensionRoundingType::CEIL), PadStrideInfo(1, 1, 0, 0))
              << get_dwsc_node(data_path, "Conv2d_8", 512 * depth_scale, PadStrideInfo(1, 1, 1, 1, 1, 1, DimensionRoundingType::CEIL), PadStrideInfo(1, 1, 0, 0))
              << get_dwsc_node(data_path, "Conv2d_9", 512 * depth_scale, PadStrideInfo(1, 1, 1, 1, 1, 1, DimensionRoundingType::CEIL), PadStrideInfo(1, 1, 0, 0))
              << get_dwsc_node(data_path, "Conv2d_10", 512 * depth_scale, PadStrideInfo(1, 1, 1, 1, 1, 1, DimensionRoundingType::CEIL), PadStrideInfo(1, 1, 0, 0))
              << get_dwsc_node(data_path, "Conv2d_11", 512 * depth_scale, PadStrideInfo(1, 1, 1, 1, 1, 1, DimensionRoundingType::CEIL), PadStrideInfo(1, 1, 0, 0))
              << get_dwsc_node(data_path, "Conv2d_12", 1024 * depth_scale, PadStrideInfo(2, 2, 0, 1, 0, 1, DimensionRoundingType::CEIL), PadStrideInfo(1, 1, 0, 0))
              << get_dwsc_node(data_path, "Conv2d_13", 1024 * depth_scale, PadStrideInfo(1, 1, 1, 1, 1, 1, DimensionRoundingType::CEIL), PadStrideInfo(1, 1, 0, 0))
              << PoolingLayer(PoolingLayerInfo(PoolingType::AVG))
              << ConvolutionLayer(
                  1U, 1U, 1001U,
                  get_weights_accessor(data_path, "Logits_Conv2d_1c_1x1_weights.npy"),
                  get_weights_accessor(data_path, "Logits_Conv2d_1c_1x1_biases.npy"),
                  PadStrideInfo(1, 1, 0, 0))
              << ReshapeLayer(TensorShape(1001U))
              << SoftmaxLayer()
              << Tensor(get_output_accessor(label, 5));
    }
    void do_run() override
    {
        // Run graph
        graph.run();
    }

private:
    Graph graph{};

    BranchLayer get_dwsc_node(const std::string &data_path, std::string &&param_path,
                              unsigned int  conv_filt,
                              PadStrideInfo dwc_pad_stride_info, PadStrideInfo conv_pad_stride_info)
    {
        std::string total_path = param_path + "_";
        SubGraph    sg;
        sg << DepthwiseConvolutionLayer(
               3U, 3U,
               get_weights_accessor(data_path, total_path + "depthwise_depthwise_weights.npy"),
               std::unique_ptr<arm_compute::graph::ITensorAccessor>(nullptr),
               dwc_pad_stride_info,
               true)
           << BatchNormalizationLayer(
               get_weights_accessor(data_path, total_path + "depthwise_BatchNorm_moving_mean.npy"),
               get_weights_accessor(data_path, total_path + "depthwise_BatchNorm_moving_variance.npy"),
               get_weights_accessor(data_path, total_path + "depthwise_BatchNorm_gamma.npy"),
               get_weights_accessor(data_path, total_path + "depthwise_BatchNorm_beta.npy"),
               0.001f)
           << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::BOUNDED_RELU, 6.f))
           << ConvolutionLayer(
               1U, 1U, conv_filt,
               get_weights_accessor(data_path, total_path + "pointwise_weights.npy"),
               std::unique_ptr<arm_compute::graph::ITensorAccessor>(nullptr),
               conv_pad_stride_info)
           << BatchNormalizationLayer(
               get_weights_accessor(data_path, total_path + "pointwise_BatchNorm_moving_mean.npy"),
               get_weights_accessor(data_path, total_path + "pointwise_BatchNorm_moving_variance.npy"),
               get_weights_accessor(data_path, total_path + "pointwise_BatchNorm_gamma.npy"),
               get_weights_accessor(data_path, total_path + "pointwise_BatchNorm_beta.npy"),
               0.001f)
           << ActivationLayer(ActivationLayerInfo(ActivationLayerInfo::ActivationFunction::BOUNDED_RELU, 6.f));

        return BranchLayer(std::move(sg));
    }
};

void run_mobilenet();

void printf_callback(const char *buffer, unsigned int len, size_t complete, void *user_data) {
    printf("%.*s", len, buffer);
}

void set_kernel_path() {
     const char* kernel_path = getenv("CK_ENV_LIB_ARMCL_CL_KERNELS");
     if (kernel_path) {
         printf("Kernel path: %s\n", kernel_path);
         arm_compute::CLKernelLibrary::get().set_kernel_path(kernel_path);
     }
}

void init_armcl(arm_compute::ICLTuner *cl_tuner = nullptr) {
    cl_context_properties properties[] =
    {
        CL_PRINTF_CALLBACK_ARM, reinterpret_cast<cl_context_properties>(printf_callback),
        CL_PRINTF_BUFFERSIZE_ARM, static_cast<cl_context_properties>(0x100000),
        CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(cl::Platform::get()()),
        0
    };
    cl::Context::setDefault(cl::Context(CL_DEVICE_TYPE_DEFAULT, properties));
    arm_compute::CLScheduler::get().default_init(cl_tuner);

    // Should be called after initialization
    set_kernel_path();
}

/** Main program for MobileNetV1
 *
 * @param[in] argc Number of arguments
 * @param[in] argv Arguments ( [optional] Target (0 = NEON, 1 = OpenCL),
 *                             [optional] Model ID (0 = MobileNetV1_1.0_224, 1 = MobileNetV1_0.75_160),
 *                             [optional] Path to the weights folder,
 *                             [optional] image,
 *                             [optional] labels )
 */
int main(int argc, char **argv)
{
    init_armcl();

    for (int i = 0; i < argc; i++) std::cout << argv[i] << std::endl;

    return arm_compute::utils::run_example<GraphMobilenetExample>(argc, argv);
}
