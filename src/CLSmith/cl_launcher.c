// Runs a randomly generated program on a device via OpenCL.
// Usage: cl_launcher <cl_program> <platform_id> <device_id> [flags...]

#include <CL/cl.h>

#include <stdio.h>
#include <stdlib.h>

#define WORK_ITEMS 1024
#define WORK_GROUP_SIZE WORK_ITEMS

// User input.
const char *file;
cl_platform_id *platform;
cl_device_id device;

// Data to free.
char *source_text = NULL;

int run_on_platform_device(cl_platform_id *, cl_device_id *);
void error_callback(const char *, const void *, size_t, void *);
void compiler_callback(cl_program, void *);
int cl_error_check(cl_int, const char *);

int main(int argc, char **argv) {
  // Parse the input. Expect three parameters.
  if (argc < 3) {
    printf("Expected one argument \"./test <cl_program> <platform_id> <device_id> [flags...]\"\n");
    return 1;
  }

  // Source file.
  file = argv[1];

  // Platform ID, the index in the array of platforms.
  int platform_index = -1;
  sscanf(argv[2], "%d", &platform_index);
  if (platform_index < 0) {
    printf("Could not parse platform id \"%s\"\n", argv[2]);
    return 1;
  }

  // Device ID, not used atm.
  int device_index = -1;
  sscanf(argv[3], "%d", &device_index);
  if (device_index < 0) {
    printf("Could not parse device id \"%s\"\n", argv[3]);
    return 1;
  }

  // Query the OpenCL API for the given platform ID.
  cl_int err;
  cl_platform_id platforms[platform_index + 1];
  cl_int platform_count;
  err = clGetPlatformIDs(platform_index + 1, platforms, &platform_count);
  if (cl_error_check(err, "clGetPlatformIDs error"))
    return 1;
  if (platform_count <= platform_index) {
    printf("No platform for id %d\n", platform_index);
    return 1;
  }
  platform = &platforms[platform_index];

  // Find all the GPU devices for the platform.
  cl_int gpu_device_count;
  err = clGetDeviceIDs(
      *platform, CL_DEVICE_TYPE_ALL, 1, &device, &gpu_device_count);
  if (cl_error_check(err, "clGetDeviceIDs error"))
    return 1;
  if (gpu_device_count < 1) {
    printf("No GPU devices found\n");
    return 1;
  }

  int run_err = run_on_platform_device(platform, &device);
  free(source_text);

  return run_err;
}

int run_on_platform_device(cl_platform_id *platform, cl_device_id *device) {
  // Try to read source file into a string.
  FILE *source = fopen(file, "rb");
  if (source == NULL) {
    printf("Could not open %s\n", file);
    return 1;
  }

  char temp[1024];
  while (!feof(source)) fread(temp, 1, 1024, source);
  long source_size = ftell(source);
  rewind(source);

  source_text = calloc(1, source_size + 1);
  if (source_text == NULL) {
    printf("Failed to calloc %ld bytes", source_size);
    return 1;
  }
  fread(source_text, 1, source_size, source);
  fclose(source);

  // Create a context, that uses our specified platform and device.
  cl_int err;
  cl_context_properties properties[3] = {
      CL_CONTEXT_PLATFORM, (cl_context_properties)*platform, 0 };
  cl_context context =
      clCreateContext(properties, 1, device, error_callback, NULL, &err);
  if (cl_error_check(err, "Error creating context"))
    return 1;

  // Create a command queue for the device in the context just created.
  // CHANGE when cl 2.0 is released.
  //cl_command_queue com_queue =
  //    clCreateCommandQueueWithProperties(context, *device, NULL, &err);
  cl_command_queue com_queue = clCreateCommandQueue(context, *device, 0, &err);
  if (cl_error_check(err, "Error creating command queue"))
    return 1;

  // Create a kernel from the source program. This involves turning the source
  // into a program object, compiling it and creating a kernel object from it.
  const char *const_source = source_text;
  cl_program program =
      clCreateProgramWithSource(context, 1, &const_source, NULL, &err);
  if (cl_error_check(err, "Error creating program"))
    return 1;

  // Add optimisation to options later.
  err = clBuildProgram(program, 0, NULL, "-w -I../../runtime/ -g", compiler_callback, NULL);
  if (cl_error_check(err, "Error building program")) {
    size_t err_size;
    err = clGetProgramBuildInfo(
        program, *device, CL_PROGRAM_BUILD_LOG, 0, NULL, &err_size);
    if (cl_error_check(err, "Error getting build info"))
      return 1;
    char *err_code = malloc(err_size);
    if (err_code == NULL) {
      printf("Failed to malloc %ld bytes\n", err_size);
      return 1;
    }
    err = clGetProgramBuildInfo(
        program, *device, CL_PROGRAM_BUILD_LOG, err_size, err_code, &err_size);
    if (!cl_error_check(err, "Error getting build info"))
      printf("%s", err_code);
    free(err_code);
    return 1;
  }

  cl_kernel kernel = clCreateKernel(program, "entry", &err);
  if (cl_error_check(err, "Error creating kernel"))
    return 1;

  // Create the buffer that will have the results.
  cl_mem result = clCreateBuffer(
      context, CL_MEM_WRITE_ONLY, 1024 * sizeof(cl_ulong), NULL, &err);
  if (cl_error_check(err, "Error creating output buffer"))
    return 1;
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &result);
  if (cl_error_check(err, "Error setting kernel argument"))
    return 1;

  // Create command to launch the kernel.
  // For now, it is 1-dimensional
  size_t total_items = 1024;
  size_t wg_size = 32;
  err = clEnqueueNDRangeKernel(
      com_queue, kernel, 1, NULL, &total_items, &wg_size, 0, NULL, NULL);
  if (cl_error_check(err, "Error enqueueing kernel"))
    return 1;

  // Send a finish command, forcing the kernel to run.
  err = clFinish(com_queue);
  if (cl_error_check(err, "Error sending finish command"))
    return 1;

  // Read back the reults of each thread.
  cl_ulong c[1024];
  err = clEnqueueReadBuffer(
      com_queue, result, CL_TRUE, 0, 1024 * sizeof(cl_ulong), c, 0, NULL, NULL);
  if (cl_error_check(err, "Error reading output buffer"))
    return 1;

  ////
  int i;
  for (i = 0; i < 1024; ++i) printf("%#lx,", c[i]);
  ////
  
  return 0;
}

// Called if any error occurs during context creation or at kernel runtime.
// This can be called many time asynchronously, so it must be thread safe.
void error_callback(
    const char *errinfo, const void *private_info, size_t cb, void *user_data) {
  printf("Error found (callback):\n%s\n", errinfo);
}

// Called by an OpenCL compiler when building a program. Called asynchronously,
// so must be thread safe.
void compiler_callback(cl_program program, void *user_data) {
  printf("Compiler callback\n");
}

// Error checker, useful to help remove some of the boiler plate.
// Return 0 on success, 1 on error.
int cl_error_check(cl_int err, const char *err_string) {
  if (err == CL_SUCCESS)
    return 0;
  printf("%s: %d\n", err_string, err);
  return 1;
}










