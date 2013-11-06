#include "cs_arrayclass.h"
#include "cs_object.h"
#include "cs_error.h"

CsValue CS_CLASS_ARRAY;

// int cs_arrayclass_as_string(CsMutator* mut,
//   int argc, CsValue* args, int retc, CsValue* rets) {
//   size_t i;
//   CsValue value, element;
//   if (cs_value_toarray(SELF)->length == 0) {
//     return cs_mutator_new_string_formatted(mut, "[]");
//   }
//   value = cs_mutator_copy_string(mut, "[", 0, 1, 1);
//   for (i = 0; i < cs_value_toarray(SELF)->length-1; ++i) {
//     element = cs_mutator_value_as_string(mut,
//       cs_value_toarray(SELF)->buckets[i]);
//     if (element == NULL)
//       return 0;
//     value = cs_mutator_new_string_formatted(mut, "%s%s, ",
//       cs_value_tostring(value), cs_value_tostring(element));
//   }
//   element = cs_mutator_value_as_string(mut, cs_value_toarray(SELF)->buckets[i]);
//   if (element == NULL)
//     return 0;
//   value = cs_mutator_new_string_formatted(mut, "%s%s]",
//     cs_value_tostring(value), cs_value_tostring(element));
  
//   if (rets)
//     rets[0] = value;
//   *retc = 1;
//   return value;
// }

CsValue cs_initclass_array(CsMutator* mut) {
  CsHash* dict = cs_hash_new();
  CsArray* bases = cs_array_new();

  // cs_hash_insert(dict, "__as_string", 11,
  //   cs_mutator_new_builtin(mut, cs_arrayclass_as_string));

  cs_array_insert(bases, -1, CS_CLASS_OBJECT);
  CS_CLASS_ARRAY = cs_mutator_new_class(mut, "Array", dict, bases);
  return CS_CLASS_ARRAY;
}

void cs_freeclass_array(CsValue klass) {
  cs_hash_free(klass->dict);
  cs_array_free(klass->bases);
}