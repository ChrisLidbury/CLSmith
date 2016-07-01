// Entry point to the program.

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "AbsProgramGenerator.h"
#include "CGOptions.h"
#include "CLSmith/CLOptions.h"
#include "CLSmith/CLOutputMgr.h"
#include "CLSmith/CLProgramGenerator.h"
#include "platform.h"

// Generator seed.
static unsigned long g_Seed = 0;

bool CheckArgExists(int idx, int argc) {
  if (idx >= argc) std::cout << "Expected another argument" << std::endl;
  return idx < argc;
}

bool ParseIntArg(const char *arg, unsigned long *value) {
  bool res = sscanf(arg, "%lu", value);
  if (!res) std::cout << "Expected integer arg for " << arg << std::endl;
  return res;
}

int main(int argc, char **argv) {
  g_Seed = platform_gen_seed();
  CGOptions::set_default_settings();
  CLSmith::CLOptions::set_default_settings();
  std::string output_filename = "";

  // Parse command line arguments.
  for (int idx = 1; idx < argc; ++idx) {
    if (!strcmp(argv[idx], "--seed") ||
        !strcmp(argv[idx], "-s")) {
      ++idx;
      if (!CheckArgExists(idx, argc)) return -1;
      if (!ParseIntArg(argv[idx], &g_Seed)) return -1;
      continue;
    }

    if (!strcmp(argv[idx], "--no-arrays")) {
      CGOptions::arrays(false);
      continue;
    }

    if (!strcmp(argv[idx], "--no-loops")) {
      CGOptions::loops(false);
      CGOptions::arrays(false);
      continue;
    }

    if (!strcmp(argv[idx], "--atomic_reductions")) {
      CLSmith::CLOptions::atomic_reductions(true);
      continue;
    }

    if (!strcmp(argv[idx], "--atomics")) {
      CLSmith::CLOptions::atomics(true);
      continue;
    }

    if (!strcmp(argv[idx], "--barriers")) {
      CLSmith::CLOptions::barriers(true);
      continue;
    }

    if (!strcmp(argv[idx], "--divergence")) {
      CLSmith::CLOptions::divergence(true);
      continue;
    }

    if (!strcmp(argv[idx], "--embedded")) {
      CLSmith::CLOptions::embedded(true);
      continue;
    }

    if (!strcmp(argv[idx], "--emi")) {
      CLSmith::CLOptions::emi(true);
      continue;
    }

    if (!strcmp(argv[idx], "--emi_p_compound")) {
      ++idx;
      if (!CheckArgExists(idx, argc)) return -1;
      unsigned long value;
      if (!ParseIntArg(argv[idx], &value)) return -1;
      CLSmith::CLOptions::emi_p_compound(value);
      continue;
    }

    if (!strcmp(argv[idx], "--emi_p_leaf")) {
      ++idx;
      if (!CheckArgExists(idx, argc)) return -1;
      unsigned long value;
      if (!ParseIntArg(argv[idx], &value)) return -1;
      CLSmith::CLOptions::emi_p_leaf(value);
      continue;
    }

    if (!strcmp(argv[idx], "--emi_p_lift")) {
      ++idx;
      if (!CheckArgExists(idx, argc)) return -1;
      unsigned long value;
      if (!ParseIntArg(argv[idx], &value)) return -1;
      CLSmith::CLOptions::emi_p_lift(value);
      continue;
    }

    if (!strcmp(argv[idx], "--fake_divergence")) {
      CLSmith::CLOptions::fake_divergence(true);
      continue;
    }

    if (!strcmp(argv[idx], "--group_divergence")) {
      CLSmith::CLOptions::group_divergence(true);
      continue;
    }

    if (!strcmp(argv[idx], "--inter_thread_comm")) {
      CLSmith::CLOptions::inter_thread_comm(true);
      continue;
    }

    if (!strcmp(argv[idx], "--message_passing")) {
      CLSmith::CLOptions::message_passing(true);
      continue;
    }

    if (!strcmp(argv[idx], "--output_file") ||
        !strcmp(argv[idx], "-o")) {
      ++idx;
      if (!CheckArgExists(idx, argc)) return -1;
      CLSmith::CLOptions::output(argv[idx]);
      continue;
    }

    if (!strcmp(argv[idx], "--no-safe_math")) {
      CLSmith::CLOptions::safe_math(false);
      continue;
    }

    if (!strcmp(argv[idx], "--small")) {
      CLSmith::CLOptions::small(true);
      continue;
    }

    if (!strcmp(argv[idx], "--track_divergence")) {
      CLSmith::CLOptions::track_divergence(true);
      continue;
    }

    if (!strcmp(argv[idx], "--vectors")) {
      CLSmith::CLOptions::vectors(true);
      continue;
    }

    std::cout << "Invalid option \"" << argv[idx] << '"' << std::endl;
    return -1;
  }
  // End parsing.

  // Resolve any options in CGOptions that must change as a result of options
  // that the user has set.
  CLSmith::CLOptions::ResolveCGOptions();
  // Check for conflicting options
  if (CLSmith::CLOptions::Conflict()) return -1;

  // AbsProgramGenerator does other initialisation stuff, besides itself. So we
  // call it, disregarding the returned object. Still need to delete it.
  AbsProgramGenerator *generator =
      AbsProgramGenerator::CreateInstance(argc, argv, g_Seed);
  if (!generator) {
    cout << "error: can't create AbsProgramGenerator. csmith init failed!"
         << std::endl;
    return -1;
  }

  // Now create our program generator for OpenCL.
  CLSmith::CLProgramGenerator cl_generator(g_Seed);
  cl_generator.goGenerator();

  // Calls Finalization::doFinalization(), which deletes everything, so must be
  // called after program generation.
  delete generator;

  return 0;
}
