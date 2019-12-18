/* Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#pragma once

#include "lite/backends/fpga/KD/pe.hpp"
#include "lite/backends/fpga/KD/pe_params.hpp"

namespace paddle {
namespace zynqmp {

class OutputPE : public PE {
 public:
  bool init() {
    Tensor* output = param_.output;
    output->setAligned(false);
    DLEngine::get_instance().out_data = reinterpret_cast<float*>(
        fpga_malloc(output->shape().numel() * sizeof(float)));
    return true;
  }

  bool dispatch() {
    Tensor* input = param_.input;
    Tensor* output = param_.output;
    if (input->aligned()) {
      Tensor tmp;
      tmp.setAligned(true);
      tmp.mutableData<float16>(FP16, input->shape());
      tmp.copyFrom(input);
      tmp.unalignImage();
      output->copyFrom(&tmp);
    } else {
      output->copyFrom(input);
    }
    //
    output->syncToCPU();
    if (DLEngine::get_instance().out_data == nullptr) {
      DLEngine::get_instance().out_data = reinterpret_cast<float*>(
          fpga_malloc(output->shape().numel() * sizeof(float)));
    }
    memcpy(DLEngine::get_instance().out_data,
           output->data<void>(),
           output->shape().numel() * sizeof(float));
    return true;
  }

  OutputParam& param() { return param_; }

 private:
  OutputParam param_;
};
}  // namespace zynqmp
}  // namespace paddle