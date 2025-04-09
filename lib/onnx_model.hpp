#ifndef ONNX_MODEL
#define ONNX_MODEL

#include <onnxruntime/onnxruntime_cxx_api.h>
#include <string>
#include <array>

using namespace Ort;

class OnnxModel {
private:
    // ONNX related fields
    Env env;
    SessionOptions sessionOptions;
    MemoryInfo memory_info;
    Session session{nullptr};
    const char* input_names[1] = {"input"};
    const char* output_names[2] = {"value", "policy"};

    const size_t INPUT_SIZE = 3 * 64;
    const size_t OUTPUT_SIZE_VALUE = 1;
    const size_t OUTPUT_SIZE_POLICY = 65;
    const std::vector <int64_t> input_shape = {1, 3, 8, 8};
    const std::vector <int64_t> output_shape_value = {1, 1};
    const std::vector <int64_t> output_shape_policy = {1, 65};

    std::string model_path;
    std::array <float, 1> value;
    std::array <float, 65> policy;

public:
    OnnxModel(std::string model_path) 
        : env(Env(OrtLoggingLevel::ORT_LOGGING_LEVEL_WARNING, "Default")),
          memory_info(MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemType::OrtMemTypeDefault)),
          model_path(model_path)
        {
        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        OrtCUDAProviderOptions cuda_options;
        sessionOptions.AppendExecutionProvider_CUDA(cuda_options);
        
        session = Session(env, model_path.c_str(), sessionOptions);
    }

    std::pair<float, std::array <float, 65>> run_inference(std::vector <float>& board_tensor) {

        Value input_tensor = Value::CreateTensor<float>(
            memory_info,
            board_tensor.data(), 
            INPUT_SIZE,
            input_shape.data(),
            input_shape.size()
        );

        Value output_tensors[2] = {
            Value::CreateTensor<float>(
                memory_info, 
                value.data(), 
                OUTPUT_SIZE_VALUE,
                output_shape_value.data(),
                output_shape_value.size()
            ),
            Value::CreateTensor<float>(
                memory_info, 
                policy.data(), 
                OUTPUT_SIZE_POLICY,
                output_shape_policy.data(),
                output_shape_policy.size()
            )
        };

        session.Run(RunOptions{nullptr}, input_names, &input_tensor, 1, output_names, output_tensors, 2);

        return {value[0], policy};
    }

    OnnxModel(const OnnxModel&) = delete;
    OnnxModel(OnnxModel&&) = default;
    OnnxModel& operator=(const OnnxModel&) = delete;
    OnnxModel& operator=(OnnxModel&&) = default;
    ~OnnxModel() = default;
};

#endif
