#include "CLSmith/CLOptions.h"

#include <iostream>

#include "CGOptions.h"

namespace CLSmith {

// Eww macro, used here just to make the flags easier to write.
#define DEFINE_CLFLAG(name, type, init) \
  type CLOptions::name##_ = init; \
  type CLOptions::name() { return name##_; } \
  void CLOptions::name(type x) { name##_ = x; }
DEFINE_CLFLAG(atomics, bool, false)
DEFINE_CLFLAG(barriers, bool, false)
DEFINE_CLFLAG(divergence, bool, false)
DEFINE_CLFLAG(EMI, bool, false)
DEFINE_CLFLAG(EMI_p_compound, int, 10)
DEFINE_CLFLAG(EMI_p_leaf, int, 50)
DEFINE_CLFLAG(EMI_p_lift, int, 10)
DEFINE_CLFLAG(atomic_reductions, bool, false)
DEFINE_CLFLAG(fake_divergence, bool, false)
DEFINE_CLFLAG(group_divergence, bool, false)
DEFINE_CLFLAG(inter_thread_comm, bool, false)
DEFINE_CLFLAG(output, const char*, "CLProg.c")
DEFINE_CLFLAG(small, bool, false)
DEFINE_CLFLAG(track_divergence, bool, false)
DEFINE_CLFLAG(vectors, bool, false)
#undef DEFINE_CLFLAG

void CLOptions::set_default_settings() {
  atomics_ = false;
  barriers_ = false;
  divergence_ = false;
  EMI_ = false;
  EMI_p_compound_ = 10;
  EMI_p_leaf_ = 50;
  EMI_p_lift_ = 10;
  atomic_reductions_ = false;
  fake_divergence_ = false;
  group_divergence_ = false;
  inter_thread_comm_ = false;
  output_ = "CLProg.c";
  small_ = false;
  track_divergence_ = false;
  vectors_ = false;
}

void CLOptions::ResolveCGOptions() {
  // General settings for normal OpenCL programs.
  // No static in OpenCL.
  CGOptions::force_globals_static(false);
  // No bit fields in OpenCL.
  CGOptions::bitfields(false);
  // Maybe enable in future. Has a different syntax.
  CGOptions::packed_struct(false);
  // No printf in OpenCL.
  CGOptions::hash_value_printf(false);
  // The way we currently handle globals means we need to disable consts.
  CGOptions::consts(false);
  // Reading smaller fields than the actual field is implementation-defined.
  CGOptions::union_read_type_sensitive(false);
  // Empty blocks ruin my FunctionWalker, embarassing.
  CGOptions::empty_blocks(false);

  // Setting for small programs.
  if (small_) {
    // Limit number of functions to no more than 5.
    CGOptions::max_funcs(5);
  }

  // Barrier specific stuff.
  if (track_divergence_) {
    // Must disable arrays for barrier stuff, as value is produced when printed.
    CGOptions::arrays(false);
    // Gotos are still todo.
    CGOptions::gotos(false);
  }

  // Vector specific restrictions.
  if (vectors_) {
    // Array ops try to iterate over random arrays, including vectors.
    CGOptions::array_ops(false);
  }
}

bool CLOptions::Conflict() {
  if (barriers_ && divergence_ && !track_divergence_) {
    std::cout << "Divergence tracking must be enabled when generating barriers "
                 "and divergence." << std::endl;
    return true;
  }
  if (vectors_ && track_divergence_) {
    std::cout << "Cannot track divergence with vectors enabled." << std::endl;
    return true;
  }
  if (divergence_ && fake_divergence_) {
    std::cout << "Cannot have both real and fake divergence." << std::endl;
    return true;
  }
  if (divergence_ && inter_thread_comm_) {
    std::cout << "Cannot have divergence and inter-thread communication." <<
                 std::endl;
    return true;
  }
  return false;
}

}  // namespace CLSmith
