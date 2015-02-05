// Options for CLSmith. Like CGOptions, but specific to OpenCL C generation.

#ifndef _CLSMITH_CLOPTIONS_H_
#define _CLSMITH_CLOPTIONS_H_

namespace CLSmith {

// Encapsulates all the options used throughout the program such that they can
// easily be accessed anywhere.
class CLOptions {
 public:
  CLOptions() = delete;

  // Eww macro, used here just to make the flags easier to write.
  #define DEFINE_CLFLAG(name, type) \
    private: static type name##_; \
    public: \
    static type name(); \
    static void name(type x);
  DEFINE_CLFLAG(atomic_reductions, bool)
  DEFINE_CLFLAG(atomics, bool)
  DEFINE_CLFLAG(barriers, bool)
  DEFINE_CLFLAG(divergence, bool)
  DEFINE_CLFLAG(embedded, bool)
  DEFINE_CLFLAG(emi, bool)
  DEFINE_CLFLAG(emi_p_compound, int)
  DEFINE_CLFLAG(emi_p_leaf, int)
  DEFINE_CLFLAG(emi_p_lift, int)
  DEFINE_CLFLAG(fake_divergence, bool)
  DEFINE_CLFLAG(group_divergence, bool)
  DEFINE_CLFLAG(inter_thread_comm, bool)
  DEFINE_CLFLAG(output, const char*)
  DEFINE_CLFLAG(safe_math, bool)
  DEFINE_CLFLAG(small, bool)
  DEFINE_CLFLAG(track_divergence, bool)
  DEFINE_CLFLAG(vectors, bool)
  #undef DEFINE_CLFLAG

  // Reset any option changes, even those specified by the user.
  static void set_default_settings();

  // Automagically sets flags in CGOptions to prevent producing invalid OpenCL C
  // programs.
  static void ResolveCGOptions();

  // Checks for conflicts in user arguments.
  static bool Conflict();
};

}  // namespace CLSmith

#endif  // _CLSMITH_CLOPTIONS_H_
