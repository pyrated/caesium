#include "cs_array.h"

static void array_grow(CsArray* arr) {
  size_t i;
  arr->size *= 2;
  arr->buckets = realloc(arr->buckets, sizeof(void*) * arr->size);
  if (arr->buckets == NULL)
    cs_exit(CS_REASON_NOMEM);
  for (i = arr->length; i < arr->size; i++)
    arr->buckets[i] = NULL;
}

CsArray* cs_array_new() {
  size_t i = 0;
  CsArray* arr = cs_alloc_object(CsArray);
  if (arr == NULL)
    cs_exit(CS_REASON_NOMEM);
  arr->size = 8;
  arr->length = 0;
  arr->buckets = malloc(sizeof(void*) * arr->size);
  if (arr->buckets == NULL)
    cs_exit(CS_REASON_NOMEM);
  for (i = 0; i < arr->size; i++)
    arr->buckets[i] = NULL;
  return arr;
}

void cs_array_free(CsArray* arr) {
  cs_free_object(arr->buckets);
  cs_free_object(arr);
}

bool cs_array_find(CsArray* arr, long pos, void* data) {
  long i = (pos < 0)? ((long) arr->length) + pos : pos;
  cs_return_if(i >= (long) arr->length || i < 0, false);
  data = arr->buckets + i;
  return true;
}

bool cs_array_insert(CsArray* arr, long pos, void* data) {
  long i = (pos < 0)? ((long) arr->length) + pos : pos;
  cs_return_if(i >= (long) arr->length || i < 0, false);
  memmove(
    arr->buckets + pos + 1,
    arr->buckets + pos,
    sizeof(void*) * (arr->length - pos));
  arr->length++;
  arr->buckets[pos] = data;
  if (arr->length == arr->size)
    array_grow(arr);
  return true;
}

bool cs_array_remove(CsArray* arr, long pos, void* data) {
  long i = (pos < 0)? ((long) arr->length) + pos : pos;
  cs_return_if(i >= (long) arr->length || i < 0, false);
  data = arr->buckets[pos];
  memmove(
    arr->buckets + pos,
    arr->buckets + pos + 1,
    sizeof(void*) * (arr->length - pos - 1));
  arr->length--;
  return true;
}

void cs_array_traverse(CsArray* arr, bool (*fn)(void*, void*), void* data) {
  size_t i;
  for (i = 0; i < arr->length; i++) {
    if (fn(arr->buckets[i], data))
      break;
  }
}