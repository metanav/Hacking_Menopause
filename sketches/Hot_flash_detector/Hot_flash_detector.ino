#include <TensorFlowLite.h>
#include "main_functions.h"
#include "thermal_camera_handler.h"
#include "person_predictor.h"
#include "model_data.h"
#include "output_handler.h"
#include "tensorflow/lite/experimental/micro/kernels/micro_ops.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/experimental/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"


// Globals, used for compatibility with Arduino-style sketches.
namespace {
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* model_input = nullptr;
int input_length;

// Create an area of memory to use for input, output, and intermediate arrays.
// The size of this will depend on the model you're using, and may need to be
// determined by experimentation.
constexpr int kTensorArenaSize = 60 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

float last_max_readings[20];
int begin_index = 0;

// Whether we should clear the buffer next time we fetch data
bool should_clear_buffer = false;
}  // namespace

// The name of this function is important for Arduino compatibility.
void setup() {

  // Set up logging. Google style is to avoid globals or statics because of
  // lifetime uncertainty, but since this has a trivial destructor it's okay.
  static tflite::MicroErrorReporter micro_error_reporter;  // NOLINT
  error_reporter = &micro_error_reporter;
  Serial.begin(115200);
  Serial.println("Setup");
  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Error1");
    error_reporter->Report(
      "Model provided is schema version %d not equal "
      "to supported version %d.",
      model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  Serial.println("A");

  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  static tflite::MicroMutableOpResolver micro_mutable_op_resolver;  // NOLINT
  micro_mutable_op_resolver.AddBuiltin(
    tflite::BuiltinOperator_DEPTHWISE_CONV_2D,
    tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
  micro_mutable_op_resolver.AddBuiltin(
    tflite::BuiltinOperator_MAX_POOL_2D,
    tflite::ops::micro::Register_MAX_POOL_2D());
  micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D,
                                       tflite::ops::micro::Register_CONV_2D());
  micro_mutable_op_resolver.AddBuiltin(
    tflite::BuiltinOperator_FULLY_CONNECTED,
    tflite::ops::micro::Register_FULLY_CONNECTED());
  micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX,
                                       tflite::ops::micro::Register_SOFTMAX());

  // Build an interpreter to run the model with
  static tflite::MicroInterpreter static_interpreter(
    model, micro_mutable_op_resolver, tensor_arena, kTensorArenaSize,
    error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors
  interpreter->AllocateTensors();

  //Obtain pointer to the model's input tensor
  model_input = interpreter->input(0);
  if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||
      (model_input->dims->data[1] != 24) ||
      (model_input->dims->data[2] != 32) ||
      (model_input->type != kTfLiteFloat32)) {
    Serial.println("Bad input tensor parameters in model");
    return;
  }

  Serial.println("B");

  input_length = model_input->bytes / sizeof(float);

  TfLiteStatus setup_status = InitThermalCamera(error_reporter);
  if (setup_status != kTfLiteOk) {
    Serial.println("Setup failed");
    //error_reporter->Report("Set up failed\n");
  }

  Serial.println("Setup completed");
}

void loop() {
  // Attempt to read new data from the Thermal Camera
  float max_reading = GetThermalImage(error_reporter, model_input->data.f, input_length);

  // If there was no new data, wait until next time
  if (max_reading == -100.0) return;

  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    Serial.println("Invoke failed");
    return;
  }

  // Analyze the results to obtain a prediction
  int index = PredictPerson(interpreter->output(0)->data.f);

  if (index > -1) {
    Serial.print("Predicted class: ");
    Serial.println(index);
    Serial.println("Output class probabilities:");
    for (uint8_t i = 0;  i < 3; i++) {
      Serial.print(i);
      Serial.print(": ");
      Serial.println(interpreter->output(0)->data.f[i]);
    }
    Serial.println("------------------------------------");
  } 
  
  if (begin_index == 20) {
    float readings[20];
    for (uint8_t i = 0; i < 20; i++) {
      readings[i] = last_max_readings[i];
    }

    for (uint8_t i = 0; i < 19; i++) {
      last_max_readings[i] = readings[i+1];
    }
    last_max_readings[19] = max_reading;
    
  } else {
    last_max_readings[begin_index++] = max_reading;
  }

  // Produce an output
  HandleOutput(error_reporter, index, last_max_readings, 20);

  delay(500);
}
