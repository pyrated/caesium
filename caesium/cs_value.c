#include "cs_value.h"

static CsValueStruct _CS_NIL = {
  type: CS_VALUE_NIL,
};

static CsValueStruct _CS_TRUE = {
  type: CS_VALUE_TRUE,
};

static CsValueStruct _CS_FALSE = {
  type: CS_VALUE_FALSE,
};

static CsValueString _CS_EPSTRING = {
  size: 0,
  length: 0,
  u8str: "",
};

static CsValueStruct _CS_EPSILON = {
  type: CS_VALUE_STRING,
  hash: 0,
  string: &_CS_EPSTRING,
};

CsValue CS_NIL = &_CS_NIL;
CsValue CS_TRUE = &_CS_TRUE;
CsValue CS_FALSE = &_CS_FALSE;
CsValue CS_EPSILON = &_CS_EPSILON;

void cs_value_cleanup(CsValue value) {
  switch (value->type) {
    case CS_VALUE_STRING:
      free((char*) value->string->u8str);
      free(value->string);
      break;

    case CS_VALUE_ARRAY:
      cs_array_free(value->array);
      break;

    case CS_VALUE_INSTANCE:
      cs_hash_free(value->dict);
      break;

    case CS_VALUE_CLOSURE:
      cs_free_object(value->closure);
      break;

    default:
      break;
  }
}
