#include "CLSmith/FunctionInvocationBuiltIn.h"

#include <map>
#include <ostream>
#include <vector>

#include "CLSmith/Vector.h"
#include "ProbabilityTable.h"
#include "Type.h"
#include "VectorFilter.h"

namespace CLSmith {
namespace {
// Integer function tables.
DistributionTable *integer_func_table = NULL;
// Internal::ParameterType::ParameterTypeMap<enum FunctionInvocationIntegerBuiltIn::BuiltIn> *integer_param_map;
std::map<enum FunctionInvocationIntegerBuiltIn::BuiltIn,
         std::vector<Internal::ParameterType::TypeConversion>> *
    integer_param_map = NULL;

// Integer function names. The array indices line up with the enum value.
const char *const kIntegerNames[20] = {
    ""     ,  "abs"    , "abs_diff", "add_sat" , "hadd" , "rhadd", "clamp" ,
    "clz"   , "ctz"    , "mad_hi"  , "mad_sat" , "max"  , "min"  , "mul_hi",
    "rotate", "sub_sat", "upsample", "popcount", "mad24", "mul24"
};
}  // namespace

namespace Internal {
namespace ParameterType {

const Type& ConvertParameterType(
    const Type& type, enum TypeConversion conversion) {
  // Only convert scalar and vector types. Other types shpuld never get here.
  assert(type.eType == eSimple || type.eType == eVector);
  const Type *t = NULL;
  switch (conversion) {
    case kExact: return type;
    case kUnsignToSign:
      t = type.to_signed(); break;
    case kSignToUnsign:
      t = type.to_unsigned(); break;
    case kFlipSign:
      t = type.is_signed() ? type.to_unsigned() : type.to_signed(); break;
    case kDemote:
      return Vector::DemoteVectorTypeToType(&type);
    case kDemoteChance:
      return rnd_flipcoin(50) ? Vector::DemoteVectorTypeToType(&type) : type;
    case kPromote:
      return *Vector::PromoteTypeToVectorType(&type, 0);
    case kNarrow:
      t = &Type::get_simple_type(
          (eSimpleType)(type.simple_type - (type.SizeInBytes() > 1)));
      break;
    case kFlipNarrow:
      t = &ConvertParameterType(ConvertParameterType(type,
          kFlipSign), kNarrow);
      break;
    default: assert(false && "Unknown type conversion");
  }

  // For type conversions where the original type is a vector, and so the new
  // type must be promoted, break out of the above switch instead of returning.
  // For nested type conversions, must be careful this does not promote when we
  // don't want to (e.g. Conv(Conv(type, Demote), Flip), outer Conv will
  // promote).
  assert(t != NULL);
  if (type.eType == eVector && t->eType != eVector)
    t = Vector::PromoteTypeToVectorType(t, type.vector_length_);
  return *t;
}

}  // namespace ParameterType
}  // namespace Internal

FunctionInvocationBuiltIn *FunctionInvocationBuiltIn::make_random(
    CGContext& cg_context, const Type& type) {
  return FunctionInvocationIntegerBuiltIn::make_random(cg_context, type);
}

void FunctionInvocationBuiltIn::InitTables() {
  FunctionInvocationIntegerBuiltIn::InitTables();
}

void FunctionInvocationBuiltIn::Output(std::ostream& out) const {
  OutputFuncName(out);
  out << '(';
  for (size_t idx = 0; idx < param_value.size(); ++idx) {
    param_value[idx]->Output(out);
    if (idx < param_value.size() - 1) out << ", ";
  }
  out << ')';
}

void FunctionInvocationBuiltIn::indented_output(std::ostream &out, int indent)
    const {
  output_tab(out, indent);
  Output(out);
}

FunctionInvocationIntegerBuiltIn *FunctionInvocationIntegerBuiltIn::make_random(
    CGContext& cg_context, const Type& type) {
  std::vector<const Type *> param_types;
  enum BuiltIn func = FunctionSelector(type, &param_types);
  FunctionInvocationIntegerBuiltIn *fi =
      new FunctionInvocationIntegerBuiltIn(func, type);
  for (const Type *param_type : param_types)
    fi->param_value.push_back(Expression::make_random(cg_context, param_type));
  return fi;
}

enum FunctionInvocationIntegerBuiltIn::BuiltIn
    FunctionInvocationIntegerBuiltIn::FunctionSelector(
    const Type& type, std::vector<const Type *> *params) {
  assert(params != NULL);
  params->clear();
  if (type.eType != eSimple && type.eType != eVector) {
    params->push_back(&type);
    return kIdentity;
  }

  assert(integer_func_table != NULL);
  VectorFilter filter(integer_func_table);
  if (type.is_signed())
    filter.add(kAbs).add(kAbsDiff);
  if (type.simple_type == eChar || type.simple_type == eUChar)
    filter.add(kUpSample);
  int rnd = rnd_upto(filter.get_max_prob());
  enum BuiltIn func = (enum BuiltIn)filter.lookup(rnd);

  // Populate params based on the selected function.
  assert(integer_param_map != NULL);
  auto it = integer_param_map->find(func);
  assert(it != integer_param_map->end() && "Cannot find params for int func");
  for (enum Internal::ParameterType::TypeConversion conversion : it->second)
    params->push_back(
        &Internal::ParameterType::ConvertParameterType(type, conversion));
  return func;
}

void FunctionInvocationIntegerBuiltIn::InitTables() {
  integer_func_table = new DistributionTable();
  for (int func = kAbs; func <= kMul24; ++func)
    integer_func_table->add_entry(func, 10);

  integer_param_map = new std::map<
      enum BuiltIn, std::vector<Internal::ParameterType::TypeConversion>>();
  using Internal::ParameterType::kExact;
  using Internal::ParameterType::kUnsignToSign;
  using Internal::ParameterType::kDemoteChance;
  using Internal::ParameterType::kNarrow;
  using Internal::ParameterType::kFlipNarrow;
  (*integer_param_map)[kIdentity] = { kExact };
  (*integer_param_map)[kAbs]      = { kUnsignToSign };
  (*integer_param_map)[kAbsDiff]  = { kUnsignToSign, kUnsignToSign };
  (*integer_param_map)[kAddSat]   = { kExact, kExact };
  (*integer_param_map)[kHAdd]     = { kExact, kExact };
  (*integer_param_map)[kRHAdd]    = { kExact, kExact };
  (*integer_param_map)[kClamp]    = { kExact, kDemoteChance, kDemoteChance }; //either both or none?
  (*integer_param_map)[kClz]      = { kExact };
  (*integer_param_map)[kCtz]      = { kExact };
  (*integer_param_map)[kMadHi]    = { kExact, kExact, kExact };
  (*integer_param_map)[kMadSat]   = { kExact, kExact, kExact };
  (*integer_param_map)[kMax]      = { kExact, kDemoteChance };
  (*integer_param_map)[kMin]      = { kExact, kDemoteChance };
  (*integer_param_map)[kMulHi]    = { kExact, kExact };
  (*integer_param_map)[kRotate]   = { kExact, kExact };
  (*integer_param_map)[kSubSat]   = { kExact, kExact };
  (*integer_param_map)[kUpSample] = { kNarrow, kFlipNarrow };
  (*integer_param_map)[kPopCount] = { kExact };
  (*integer_param_map)[kMad24]    = { kExact, kExact, kExact };
  (*integer_param_map)[kMul24]    = { kExact, kExact };
}

FunctionInvocationIntegerBuiltIn *FunctionInvocationIntegerBuiltIn::clone()
    const {
  FunctionInvocationIntegerBuiltIn *fi =
      new FunctionInvocationIntegerBuiltIn(built_in_, type_);
  for (const Expression *expr : param_value)
    fi->param_value.push_back(expr->clone());
  return fi;
}

void FunctionInvocationIntegerBuiltIn::OutputFuncName(std::ostream& out) const {
  out << kIntegerNames[built_in_];
}

}  // namespace CLSmith
