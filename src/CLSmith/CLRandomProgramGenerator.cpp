// Entry point to the program.

#include <cassert>
#include <cstdlib>
#include <iostream>

#include "AbsProgramGenerator.h"
#include "CGOptions.h"
#include "CLSmith/CLProgramGenerator.h"
#include "platform.h"

int main(int argc, char **argv) {
  // Temporary, check first param for seed.
  unsigned long g_seed;
  if (argc > 1) { g_seed = strtoul(argv[1], NULL, 0); assert(g_seed); }
  else g_seed = platform_gen_seed();

  // Force options that would otherwide produce an invalid OpenCL program.
  CGOptions::set_default_settings();
  CGOptions::force_globals_static(false);
  CGOptions::bitfields(false);

  // AbsProgramGenerator does other initialisation stuff, besides itself. So we
  // call it, diregarding the returned object. Still need to delete it.
  AbsProgramGenerator *generator =
      AbsProgramGenerator::CreateInstance(argc, argv, g_seed);
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
