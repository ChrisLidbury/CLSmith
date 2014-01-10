// Entry point to the program.

#include <iostream>

#include "AbsProgramGenerator.h"
#include "CGOptions.h"
#include "CLSmith/CLProgramGenerator.h"
#include "platform.h"

int main(int argc, char **argv) {
  unsigned long g_Seed = platform_gen_seed();
  CGOptions::set_default_settings();

  // AbsProgramGenerator does other initialisation stuff, besides itself. So we
  // call it, diregarding the returned object. Still need to delete it.
  AbsProgramGenerator *generator = AbsProgramGenerator::CreateInstance(argc, argv, g_Seed);
  if (!generator) {
    cout << "error: can't create AbsProgramGenerator. csmith init failed!"
         << std::endl;
    return -1;
  }

  // Now create our program generator for OpenCL.
  CLSmith::CLProgramGenerator cl_generator;
  cl_generator.goGenerator();

  // Calls Finalization::doFinalization(), which deletes everything, so must be
  // called after program generation.
  delete generator;

  return 0;
}
