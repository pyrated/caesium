#include <malloc.h>

#include "cs_mutator.h"

static int mut_main(void* data) {
  CsMutator* mut = data;
  return mut->entry_point(mut, mut->data);
}

CsMutator* cs_mutator_new(CsRuntime* cs) {
  CsMutator* mut = cs_alloc_object(CsMutator);
  if (cs_unlikely(mut == NULL))
    cs_exit(CS_REASON_NOMEM);
  mut->started = false;
  mut->cs = cs;
  mut->error_stack = cs_list_new();
  mut->stack = cs_list_new();

  mut->nursery = cs_list_new();

  // Allocate initial nursery
  CsNurseryPage* page = memalign(sizeof(CsValueStruct), 16384);
  cs_list_push_back(mut->nursery, page);

  return mut;
}

void cs_mutator_free(CsMutator* mut) {
  cs_free_object(mut);
}

void cs_mutator_start(
  CsMutator* mut,
  int (*entry_point)(struct CsMutator*, void*),
  void* data)
{
  mut->entry_point = entry_point;
  mut->data = data;
  if (thrd_create(&mut->thread, mut_main, mut) != thrd_success)
    cs_exit(CS_REASON_THRDFATAL);
}

static CsStackFrame* create_stack_frame(CsByteFunction* func) {
  CsStackFrame* frame = malloc(sizeof(CsStackFrame) + sizeof(CsValue)
    * (func->nparams + func->nupvals + func->nstacks));
  if (cs_unlikely(frame == NULL))
    cs_exit(CS_REASON_NOMEM);

  // Cast frame to void* to prevent heap corruption
  frame->params = ((void*) frame) + sizeof(CsStackFrame);
  frame->upvals = &frame->params[func->nparams];
  frame->stacks = &frame->upvals[func->nupvals];

  frame->cur_func = func;
  frame->pc = 0;

  frame->ncodes = func->codes->length;
  frame->codes = (uintptr_t*) func->codes->buckets;

  return frame;
}

void cs_mutator_exec(CsMutator* mut, CsByteChunk* chunk) {
  CsStackFrame* frame = create_stack_frame(chunk->entry);
  CsByteConst* konst;
  CsPair* pair;
  int a, b, c;

  for (frame->pc = 0; frame->pc < frame->ncodes; frame->pc++) {
    CsByteCode code = frame->codes[frame->pc];
    switch (cs_bytecode_get_opcode(code)) {
      case CS_OPCODE_MOVE:
        a = cs_bytecode_get_a(code);
        b = cs_bytecode_get_b(code);
        frame->stacks[a] = frame->stacks[b];
        break;

      case CS_OPCODE_LOADK:
        a = cs_bytecode_get_a(code);
        b = cs_bytecode_get_b(code);
        konst = frame->cur_func->consts->buckets[b];
        switch (konst->type) {
          case CS_CONST_TYPE_NIL:
            frame->stacks[a] = CS_NIL;
            break;

          case CS_CONST_TYPE_TRUE:
            frame->stacks[a] = CS_TRUE;
            break;

          case CS_CONST_TYPE_FALSE:
            frame->stacks[a] = CS_FALSE;
            break;

          case CS_CONST_TYPE_INT:
            frame->stacks[a] = cs_value_fromint(konst->integer);
            break;

          case CS_CONST_TYPE_REAL:
            frame->stacks[a] = malloc(sizeof(CsValueStruct));
            frame->stacks[a]->type = CS_VALUE_REAL;
            frame->stacks[a]->real = konst->real;
            break;

          case CS_CONST_TYPE_STRING:
            frame->stacks[a] = malloc(sizeof(CsValueStruct));
            memset(frame->stacks[a], 0, sizeof(CsValueStruct));
            frame->stacks[a]->type = CS_VALUE_STRING;
            frame->stacks[a]->size = konst->size;
            frame->stacks[a]->string = konst->string;
            break;
        }
        break;
        
      case CS_OPCODE_LOADG:
        a = cs_bytecode_get_a(code);
        b = cs_bytecode_get_b(code);
        konst = frame->cur_func->consts->buckets[b];
        mtx_lock(&mut->cs->globals_lock);
        pair = cs_hash_find(mut->cs->globals, konst->string, konst->size);
        mtx_unlock(&mut->cs->globals_lock);
        if (cs_likely(pair != NULL))
          frame->stacks[a] = pair->value;
        else
          // This should raise.... but just return NIL for now
          frame->stacks[a] = CS_NIL;
        break;

      case CS_OPCODE_STORG:
        a = cs_bytecode_get_a(code);
        b = cs_bytecode_get_b(code);
        konst = frame->cur_func->consts->buckets[b];
        mtx_lock(&mut->cs->globals_lock);
        cs_hash_insert(
          mut->cs->globals,
          konst->string,
          konst->size,
          frame->stacks[a]);
        mtx_unlock(&mut->cs->globals_lock);
        break;

      case CS_OPCODE_PUTS:
        a = cs_bytecode_get_a(code);
        if (cs_value_isint(frame->stacks[a])) {
          printf("%"PRIiPTR"\n", cs_value_toint(frame->stacks[a]));
          break;
        }
        switch (frame->stacks[a]->type) {
          case CS_VALUE_NIL:
            printf("nil\n");
            break;

          case CS_VALUE_TRUE:
            printf("true\n");
            break;

          case CS_VALUE_FALSE:
            printf("false\n");
            break;

          case CS_VALUE_REAL:
            printf("%g\n", cs_value_toreal(frame->stacks[a]));
            break;

          case CS_VALUE_STRING:
            printf("%s\n", cs_value_tostring(frame->stacks[a]));
            break;
        }
        break;

        case CS_OPCODE_ADD:
          a = cs_bytecode_get_a(code);
          b = cs_bytecode_get_b(code);
          c = cs_bytecode_get_c(code);
          frame->stacks[a] = cs_value_fromint(cs_value_toint(frame->stacks[b]) + cs_value_toint(frame->stacks[c]));
          break;

        case CS_OPCODE_RET:
          break;

      default:
        cs_exit(CS_REASON_UNIMPLEMENTED);
        break;
    }
  }
}
