#include "cs_common.h"

const char* CS_REASONS[] = {
  "UNDEFINED REASON?",
  "System out of memory x_x",
  "File can't be opened",
  "A fatal OS error occured in the threading subsystem :/",
  "Attempted to access an unimplemented feature",
  "Assembly malformed o_0",
};

void cs_exit(CsReason reason) {
  cs_error("Caesium failed with reason: %s\n", CS_REASONS[reason]);
  exit(reason);
}