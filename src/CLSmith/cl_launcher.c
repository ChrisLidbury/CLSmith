// Runs a randomly generated program on a device via OpenCL.
// Usage: cl_launcher <cl_program> <platform_id> <device_id> [flags...]

#include <CL/cl.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WORK_ITEMS 1024
#define WORK_GROUP_SIZE WORK_ITEMS

// User input.
const char *file;
cl_platform_id *platform;
cl_device_id *device;
size_t binary_size = 0;

// Data to free.
char *source_text = NULL;
char *buf = NULL;

int run_on_platform_device(cl_platform_id *, cl_device_id *);
void error_callback(const char *, const void *, size_t, void *);
void compiler_callback(cl_program, void *);
int cl_error_check(cl_int, const char *);

int main(int argc, char **argv) {
  
  // Parse the input. Expect three parameters.
  if (argc < 4) {
    printf("Expected at least three arguments \"./cl_launcher <cl_program> <platform_idx> <device_idx> [flags...]\"\n");
    return 1;
  }
  
  int platform_index = -1, device_index = -1;
  int f_arg = 0, d_arg = 0, p_arg = 0;
  
  // Parsing arguments
  int arg_no = 0;
  char * curr_arg;
  while (++arg_no < argc) {
    curr_arg = argv[arg_no];
    if (!strncmp(curr_arg, "-", 1)) {
      if (arg_no + 1 == argc) {
        printf("Option with no value found.\n");
        return 1;
      }
      // Set source file
      if (!strcmp(curr_arg, "-f") || !strcmp(curr_arg, "--filename")) {
        file = argv[++arg_no];
        f_arg = 1;
      }
      // Set device index
      else if (!strcmp(curr_arg, "-d") || !strcmp(curr_arg, "--device_idx")) {
        device_index = atoi(argv[++arg_no]);
        d_arg = 1;
      }
      // Set platform index
      else if (!strcmp(curr_arg, "-p") || !strcmp(curr_arg, "--platform_idx")) {
        platform_index = atoi(argv[++arg_no]);
        p_arg = 1;
      }
      // Optionally set target
      else if (!strcmp(curr_arg, "-b") || !strcmp(curr_arg, "--binary")) {
        binary_size = atoi(argv[++arg_no]);
      }
    } else {
      printf("Expected option; found %s.\n", curr_arg);
      return 1;
    }
  }
  
  if (f_arg + d_arg + p_arg != 3) {
    if (!f_arg) 
      printf("File (-f or --file) argument required.\n");
    if (!d_arg)
      printf("Device index (-d or --device_idx) argument required.\n");
    if (!p_arg)
      printf("Platform index (-p or --platform_idx) argument required.\n");
    return 1;
  }
  
  // Platform ID, the index in the array of platforms.
//   sscanf(argv[2], "%d", &platform_index);
  if (platform_index < 0) {
    printf("Could not parse platform id \"%s\"\n", argv[2]);
    return 1;
  }

  // Device ID, not used atm.
//   sscanf(argv[3], "%d", &device_index);
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
  cl_device_id devices[device_index + 1];
  cl_int device_count;
  err = clGetDeviceIDs(
      *platform, CL_DEVICE_TYPE_ALL, device_index + 1, devices, &device_count);
  if (cl_error_check(err, "clGetDeviceIDs error"))
    return 1;
  if (device_count <= device_index) {
    printf("No device for id %d\n", device_index);
    return 1;
  }
  device = &devices[device_index];

  int run_err = run_on_platform_device(platform, device);
  free(source_text);
  free(buf);

  return run_err;
}

int run_on_platform_device(cl_platform_id *platform, cl_device_id *device) {
  
  // Try to read source file into a binary buffer
  FILE *source = fopen(file, "rb");
  if (source == NULL) {
    printf("Could not open %s\n", file);
    return 1;
  }

  size_t source_size;
  if (!binary_size) {
    char temp[1024];
    while (!feof(source)) fread(temp, 1, 1024, source);
    source_size = ftell(source);
    rewind(source);

    source_text = calloc(1, source_size + 1);
    if (source_text == NULL) {
      printf("Failed to calloc %ld bytes", source_size);
      return 1;
    }
    fread(source_text, 1, source_size, source);
    fclose(source);
  }
  else {
    buf = (char *) malloc (binary_size);
    source_size = fread(buf, 1, binary_size, source);
    fclose(source);
  }

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
  cl_program program;
  if (!binary_size) {
    const char *const_source = source_text;
    program =
      clCreateProgramWithSource(context, 1, &const_source, NULL, &err);
  }
  else {
    program =
        clCreateProgramWithBinary(context, 1, device, (const size_t *)&source_size, 
                                  (const unsigned char **)&buf, NULL, &err);
  }
  if (cl_error_check(err, "Error creating program"))
    return 1;

  // Add optimisation to options later.
  err = clBuildProgram(program, 0, NULL, "-w -I.", compiler_callback, NULL);
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
  for (i = 0; i < 1024; ++i) printf("%#"PRIx64",", c[i]);
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










