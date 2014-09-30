// Runs a randomly generated program on a device via OpenCL.
// Usage: cl_launcher <cl_program> <platform_id> <device_id> [flags...]

#include <CL/cl.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define DEF_LOCAL_SIZE 32
#define DEF_GLOBAL_SIZE 1024

// User input.
const char *file;
size_t binary_size = 0;
int device_index = 0;
int platform_index = 0;
char* device_name_given = "";

bool atomics = false;
int atomic_counter_no = 0;

// Data to free.
char *source_text = NULL;
char *buf = NULL;
int *init_atomic_vals = NULL;
int *init_special_vals = NULL;
size_t *local_size = NULL;
size_t *global_size = NULL;
char* local_dims = "";
char* global_dims = "";

// Other parameters
cl_platform_id *platform;
cl_device_id *device;
int total_threads = 1;

int run_on_platform_device(cl_platform_id *, cl_device_id *, cl_uint);
void error_callback(const char *, const void *, size_t, void *);
void compiler_callback(cl_program, void *);
int cl_error_check(cl_int, const char *);

int main(int argc, char **argv) {
  
  // Parse the input. Expect three parameters.
  if (argc < 4) {
    printf("Expected at least three arguments \"./cl_launcher -f <cl_program> -p <platform_idx> -d <device_idx> [flags...]\"\n");
    return 1;
  }
  
  int req_arg = 0;
  
  // Parsing arguments
  int arg_no = 0;
  int parse_ret;
  char* curr_arg;
  char* next_arg;
  while (++arg_no < argc) {
    curr_arg = argv[arg_no];
    if (++arg_no >= argc) {
      printf("Found option %s with no value.\n", curr_arg);
      return 1;
    }
    next_arg = argv[arg_no];
    parse_ret = parse_arg(curr_arg, next_arg);
    if (!parse_ret)
      return 1;
    if (parse_ret == 2)
      req_arg++;
  }
  
  if (req_arg != 3) {
    printf("Require file (-f), device index (-d) and platform index (-p) arguments!\n");
    return 1;
  }
  
  // Parse arguments found in the given source file
  if (!parse_file_args(file)) {
    printf("Failed parsing file for arguments.\n");
    return 1;
  }
  
  // TODO function this
  // Parsing thread and group dimension information
  int l_dim = 1, g_dim = 1;
  if (local_dims == "") {
    printf("No local dimension information found; defaulting to uni-dimensional %d threads.\n", DEF_LOCAL_SIZE);
    local_size = malloc(sizeof(size_t));
    local_size[0] = DEF_LOCAL_SIZE;
  } else {
    int i = 0;
    while (local_dims[i] != '\0')
      if (local_dims[i++] == ',')
        l_dim++;
    i = 0;
    local_size = (size_t*)malloc(l_dim * sizeof(size_t));
    char* tok = strtok(local_dims, ",");
    while (tok) {
      local_size[i++] = (size_t) atoi(tok);
      tok = strtok(NULL, ",");
    }
  }
  if (global_dims == "") {
    printf("No global dimension information found; defaulting to uni-dimensional %d threads.\n", DEF_GLOBAL_SIZE);
    global_size = malloc(sizeof(size_t));
    global_size[0] = DEF_GLOBAL_SIZE;
  } else {
    int i = 0;
    while (global_dims[i] != '\0')
      if (global_dims[i++] == ',')
        g_dim++;
    i = 0;
    global_size = malloc(g_dim * sizeof(size_t));
    char* tok = strtok(global_dims, ",");
    while (tok) {
      global_size[i++] = atoi(tok);
      tok = strtok(NULL, ",");
    }
  }
  
  if (g_dim != l_dim) {
    printf("Local and global sizes must have same number of dimensions!\n");
    return 1;
  }
  if (l_dim > 3) {
    printf("Cannot have more than 3 dimensions!\n");
    return 1;
  }
  
  // Calculating total number of work-units for future use
  int i;
  for (i = 0; i < l_dim; i++)
    total_threads *= global_size[i];
  
  // Platform ID, the index in the array of platforms.
  if (platform_index < 0) {
    printf("Could not parse platform id \"%s\"\n", argv[2]);
    return 1;
  }

  // Device ID, not used atm.
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
  
  // Check to see if name of device corresponds to given name, if any
  if (device_name_given) {
    size_t name_size = 128, actual_size;
    char* device_name_get = malloc(name_size*sizeof(char));
    err = clGetDeviceInfo(
      *device, CL_DEVICE_NAME, name_size, device_name_get, &actual_size);
    if (cl_error_check(err, "clGetDeviceInfo error"))
      return 1;
    char* device_name = malloc(actual_size*sizeof(char));
    strncpy(device_name, device_name_get, actual_size);
    if (!strstr(device_name, device_name_given)) {
      printf("Given name, %s, not found in device name, %s.\n", 
          device_name_given, device_name);
      return 1;
    }
  }

  int run_err = run_on_platform_device(platform, device, (cl_uint) l_dim);
  free(source_text);
  free(buf);
  free(local_size);
  free(global_size);
  
  if (atomics) {
    free(init_atomic_vals);
    free(init_special_vals);
  }

  return run_err;
}

int run_on_platform_device(cl_platform_id *platform, cl_device_id *device, cl_uint work_dim) {
  
  // Try to read source file into a binary buffer
  FILE *source = fopen(file, "rb");
  if (source == NULL) {
    printf("Could not open %s.\n", file);
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
      printf("Failed to calloc %ld bytes.\n", source_size);
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
  err = clBuildProgram(program, 0, NULL, "-w -I.", NULL, NULL);
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
  } else {
    cl_build_status status;
    err = clGetProgramBuildInfo(
        program, *device, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status, NULL);
    if (cl_error_check(err, "Error getting build info"))
      return 1;
    //else
      //printf("clBuildProgram status: %d.\n", status);
  }
  
  // Parse the thread and group dimension information
  
    
  cl_kernel kernel = clCreateKernel(program, "entry", &err);
  if (cl_error_check(err, "Error creating kernel"))
    return 1;

  // Create the buffer that will have the results.
  cl_mem result = clCreateBuffer(
      context, CL_MEM_WRITE_ONLY, total_threads * sizeof(cl_ulong), NULL, &err);
  if (cl_error_check(err, "Error creating output buffer"))
    return 1;
  
  // Set the buffers as arguments
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &result);
  if (cl_error_check(err, "Error setting kernel argument 0"))
    return 1;
  
  if (atomics) {
    // Create buffer to store counters for the atomic blocks
    int total_counters = atomic_counter_no * total_threads;
    init_atomic_vals = malloc(sizeof(int) * total_counters);
    init_special_vals = malloc(sizeof(int) * total_counters);
    // TODO fill
    int i;
    for (i = 0; i < total_counters; i++) {
      init_atomic_vals[i] = 0;
      init_special_vals[i] = 0;
    }
    
    cl_mem atomic_input = clCreateBuffer(
        context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, total_counters * sizeof(int), init_atomic_vals, &err);
    if (cl_error_check(err, "Error creating atomic input buffer"))
      return 1;
    
    // Create buffer to store special values for the atomic blocks
    cl_mem special_values = clCreateBuffer(
        context, CL_MEM_WRITE_ONLY, total_counters * sizeof(int), NULL, &err);
    if (cl_error_check(err, "Error creating special values input buffer"))
      return 1;
    
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &atomic_input);
    if (cl_error_check(err, "Error setting atomic input array argument"))
      return 1;
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &special_values);
    if (cl_error_check(err, "Error setting special values array argument"))
      return 1;
  }
  

  // Create command to launch the kernel.
  // For now, it is 1-dimensional
  err = clEnqueueNDRangeKernel(
      com_queue, kernel, work_dim, NULL, global_size, local_size, 0, NULL, NULL);
  if (cl_error_check(err, "Error enqueueing kernel"))
    return 1;

  // Send a finish command, forcing the kernel to run.
  err = clFinish(com_queue);
  if (cl_error_check(err, "Error sending finish command"))
    return 1;

  // Read back the reults of each thread.
  cl_ulong c[total_threads];
  err = clEnqueueReadBuffer(
      com_queue, result, CL_TRUE, 0, total_threads * sizeof(cl_ulong), c, 0, NULL, NULL);
  if (cl_error_check(err, "Error reading output buffer"))
    return 1;

  ////
  int i;
  for (i = 0; i < total_threads; ++i) printf("%#"PRIx64",", c[i]);
  ////
  
  return 0;
}

int parse_file_args(const char* filename) {
  
  FILE* source = fopen(filename, "r");
  if (source == NULL) {
    printf("Could not open file %s for argument parsing.\n", filename);
    return 0;
  }

  char arg_buf[128];
  fgets(arg_buf, 128, source);
  int buf_len = strlen(arg_buf);
  char* new_line;
  if (new_line = strchr(arg_buf, '\n'))
    arg_buf[(int) (new_line - arg_buf)] = '\0';
  
  if (!strncmp(arg_buf, "//", 2)) {
    char* tok = strtok(arg_buf, " ");
    while (tok) {
      if (!strncmp(tok, "-", 1))
        parse_arg(tok, strtok(NULL, " "));
      tok = strtok(NULL, " ");
    }
  }

  fclose(source);
  
  return 1;
}

int parse_arg(char* arg, char* val) {
  if (!strcmp(arg, "-f") || !strcmp(arg, "--filename")) {
    file = val;
    return 2;
  }
  if (!strcmp(arg, "-d") || !strcmp(arg, "--device_idx")) {
    device_index = atoi(val);
    return 2;
  }
  if (!strcmp(arg, "-p") || !strcmp(arg, "--platform_idx")) {
    platform_index = atoi(val);
    return 2;
  }
  if (!strcmp(arg, "-b") || !strcmp(arg, "--binary")) {
    binary_size = atoi(val);
    return 1;
  }
  if (!strcmp(arg, "-l") || !strcmp(arg, "--locals")) {
    local_dims = malloc(strlen(val)*sizeof(char));
    strcpy(local_dims, val);
    return 1;
  }
  if (!strcmp(arg, "-g") || !strcmp(arg, "--groups")) {
    global_dims = malloc(strlen(val)*sizeof(char));
    strcpy(global_dims, val);
    return 1;
  }
  if (!strcmp(arg, "-n") || !strcmp(arg, "--name")) {
    device_name_given = val;
    return 1;
  }
  if (!strcmp(arg, "--atomics")) {
    atomics = true;
    atomic_counter_no = atoi(val);
    return 1;
  }
  printf("Failed parsing arg %s.", arg);
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
