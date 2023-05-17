#ifndef OPCODENATOR_H
#define OPCODENATOR_H

#include <stddef.h>
#include <stdint.h>

#define MAX_ENUM_STR 64
#define MAX_OPCODE_BITS 64

typedef struct {
  char enum_str[MAX_ENUM_STR];
  char opcode_str[MAX_OPCODE_BITS];
  uint64_t zeroed_value;
  uint64_t oned_value;
} OpcodeData;

typedef struct {
  uint8_t enum_name_width;
  uint8_t opcode_hex_width;
  uint8_t function_name_width;
  // DO NOT use this in place of opcode_bits, they are NOT the same.
  uint8_t opcode_width;
} IndentationData;

typedef struct {
  OpcodeData *opcodes;
  const uint16_t size;
  uint8_t opcode_bits;
  const char *indent_string;
  const char *function_prefix;
  const char *decode_function_name;
  const IndentationData indent_data;
} OpcodenatorData;

OpcodenatorData init_opcodenator(OpcodeData *opcodes, uint16_t n,
    const char *indentation_string, const char *function_prefix,
    const char *decode_function_name);
void print_includes();
void print_enum_declaration(OpcodenatorData d);
void print_struct_declaration(OpcodenatorData d);
void print_function_declarations(OpcodenatorData d);
void print_array_definition(OpcodenatorData d);
void print_decoder_function(OpcodenatorData d);

#endif // OPCODENATOR_H

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <math.h>

#ifdef OPCODENATOR_IMPLEMENTATION

// Column names when printing errors
static const char enum_table_col[]   = "NAME";
static const char zero_table_col[]   = "ZEROED VALUE";
static const char one_table_col[]    = "ONED VALUE";
static const char opcode_table_col[] = "OPCODE STRING";

#define MAX_SHORT_STRING 256

#define OPCODE(estr, opstr) { .enum_str = estr, .opcode_str = opstr, \
  .zeroed_value = 0, .oned_value = 0 }

#define FMT_CHECK(a, b) __attribute__ ((format(printf, a, b)))

typedef struct {
  char val[MAX_SHORT_STRING];
  int len;
} ShortString;

FMT_CHECK(1, 2) ShortString shortf(const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
 
  ShortString ret;
  int length = vsnprintf(ret.val, MAX_SHORT_STRING, fmt, va);
  assert(length >= 0);
  assert(length < MAX_SHORT_STRING - 1);
  
  ret.len = length;
  va_end(va);
  return ret;
}

FMT_CHECK(4, 5) void ind_fprintf(FILE *stream,
    const char *ind_str, int n, const char *fmt, ...) {
  va_list va;
  va_start(va, fmt);

  for (int i = 0; i < n; i++) {
    fprintf(stream, "%s",  ind_str);
  }
  
  vfprintf(stream, fmt, va);
  va_end(va);
}

uint64_t get_zeroed(const char *opcode_str, uint8_t opcode_bits) {
  uint64_t ret = 0;

  for (int i = 0; i < opcode_bits; i++) {
    if (opcode_str[i] == '1' || isalpha(opcode_str[i]))
      ret |= ((uint64_t)1 << (opcode_bits - i -1));
  }

  return ret;
}

uint64_t get_oned(const char *opcode_str, uint8_t opcode_bits) {
  uint64_t ret = 0;

  for (int i = 0; i < opcode_bits; i++) {
    if (opcode_str[i] == '1')
      ret |= ((uint64_t)1 << (opcode_bits - i -1));
  }

  return ret;
}

uint64_t get_static_bits(const OpcodeData *opcodes, uint16_t n,
    uint8_t opcode_bits) {
  uint64_t opcode_mask = ((uint64_t)1 << opcode_bits) - 1;
  uint64_t ret = 0xFFFFFFFFFFFFFFFF & opcode_mask;

  for (int i = 0; i < n; i++) {
    assert(strlen(opcodes[i].enum_str) > 0);
    assert(strlen(opcodes[i].opcode_str) > 0);
    assert(strlen(opcodes[i].opcode_str) == opcode_bits);
    for (int j = 0; j < opcode_bits; j++) {
      if (isalpha(opcodes[i].opcode_str[j])) {
        ret &= ~((uint64_t)1 << (opcode_bits - j -1));
      }
    }
  }
  
  return ret;
}

uint32_t get_ids(const OpcodeData *input, size_t n, uint64_t static_bits,
    uint64_t *output) {
  uint32_t count = 0;

  for (size_t i = 0; i < n; i++) {
    bool exists = false;
    for (size_t j = 0; j < count; j++) {
      if ((input[i].zeroed_value & static_bits) == output[j]) {
        exists = true;
        break;
      }
    }

    if (!exists)
      output[count++] = input[i].zeroed_value & static_bits;
  }

  return count;
}

// id is simply the value anded with the static_bits
uint32_t get_opcodes_by_id(const OpcodeData *input, size_t n,
    uint64_t id, uint64_t static_bits, OpcodeData *output) {
  uint32_t count = 0;
  
  for (size_t i = 0; i < n; i++) {
    if ((input[i].zeroed_value & static_bits) == id) {
      output[count++] = input[i];
    }
  }

  return count;
}

IndentationData get_table_indentation(IndentationData ind) {
  uint8_t enum_width = strlen(enum_table_col) > ind.enum_name_width ?
    strlen(enum_table_col) : ind.enum_name_width;

  uint8_t zero_hex_width = strlen(zero_table_col) > ind.opcode_hex_width ?
    strlen(zero_table_col) : ind.opcode_hex_width;
  uint8_t one_hex_width = strlen(one_table_col) > ind.opcode_hex_width ?
    strlen(one_table_col) : ind.opcode_hex_width;
  uint8_t hex_width = one_hex_width > zero_hex_width ? one_hex_width + 2 :
    zero_hex_width + 2;

  uint8_t opcode_width = strlen(opcode_table_col) > ind.opcode_width ?
    strlen(opcode_table_col) : ind.opcode_width;
  
  IndentationData ret = {
    .enum_name_width = enum_width,
    .opcode_hex_width = hex_width,
    .function_name_width = ind.function_name_width,
    .opcode_width = opcode_width,
  };

  return ret;
}

void print_table_head(IndentationData ind) {
  fprintf(stderr, "%-*s %-*s %-*s %-*s\n",
      ind.enum_name_width, enum_table_col,
      ind.opcode_hex_width, zero_table_col,
      ind.opcode_hex_width, one_table_col,
      ind.opcode_width, opcode_table_col);
}

// op_ind is the indentation as initialized for the OpcodenatorData. op_int is
// the table's indentation, you gt this by using get_table_indentation()
void print_table_row(OpcodeData opcode, IndentationData op_ind,
    IndentationData table_ind) {
  ShortString zero_hex_str = shortf("0x%0*lX",
      op_ind.opcode_hex_width, opcode.zeroed_value);
  ShortString one_hex_str = shortf("0x%0*lX",
      op_ind.opcode_hex_width, opcode.oned_value);

  fprintf(stderr, "%-*s %-*s %-*s %-*s\n",
      table_ind.enum_name_width, opcode.enum_str,
      table_ind.opcode_hex_width, zero_hex_str.val,
      table_ind.opcode_hex_width, one_hex_str.val,
      table_ind.opcode_width, opcode.opcode_str);
}

bool check_for_duplicates(const OpcodeData *opcodes,
    uint16_t n, IndentationData indent_data) {
  bool ret = false;

  for (uint16_t i = 0; i < n; i++) {
    for (uint16_t j = i + 1; j < n; j++) {
      if (opcodes[i].zeroed_value == opcodes[j].zeroed_value &&
          opcodes[i].oned_value   == opcodes[j].oned_value) {
        ret = true;

        fprintf(stderr, "duplicate detected:\n");
        IndentationData table_indent_data = get_table_indentation(indent_data);
        print_table_head(table_indent_data);
        print_table_row(opcodes[i], indent_data, table_indent_data);
        print_table_row(opcodes[j], indent_data, table_indent_data);
      }
    }
  }

  return ret;
}

void print_decoder_switch(const char *ind_str, int ind_lvl,
    const OpcodeData *opcodes, uint16_t n, IndentationData indent_data) {
  uint64_t static_bits = get_static_bits(opcodes, n, indent_data.opcode_width);
  uint64_t ids[n];
  uint32_t ids_size = get_ids(opcodes, n, static_bits, ids);
  
  assert(ids_size > 0 && "should not be posisble to get here and have no ids");
  if (ids_size > 1) {
  ind_fprintf(stdout, ind_str, ind_lvl, "switch (opcode & 0x%0*lX) {\n",
      indent_data.opcode_hex_width, static_bits);
  }

  for (uint32_t i = 0; i < ids_size; i++) {
    OpcodeData next[n];
    uint16_t next_size = get_opcodes_by_id(opcodes, n, ids[i],
        static_bits, next);

    ind_fprintf(stdout, ind_str, ind_lvl, "case 0x%0*lX:%s\n",
        indent_data.opcode_hex_width, ids[i],
        next_size > 1 ? " {" : "");

    if (next_size == 1) {
      ind_fprintf(stdout, ind_str, ind_lvl + 1, "return %s;\n",
          next[0].enum_str);
      continue;
    }

    print_decoder_switch(ind_str, ind_lvl + 1, next, next_size, indent_data);
    ind_fprintf(stdout, ind_str, ind_lvl  + 1, "} break;\n");
  }

  ind_fprintf(stdout, ind_str, ind_lvl, "}\n");
  return;
}

bool check_for_collisions(const OpcodeData *opcodes, uint16_t n,
    IndentationData indent_data) {
  bool ret = false;

  uint64_t static_bits = get_static_bits(opcodes, n, indent_data.opcode_width);
  uint64_t ids[n];
  uint32_t ids_size = get_ids(opcodes, n, static_bits, ids);
  
  assert(ids_size > 0 && "should not be posisble to get here and have no ids");
  for (uint32_t i = 0; i < ids_size; i++) {
    OpcodeData next[n];
    int next_size = get_opcodes_by_id(opcodes, n, ids[i], static_bits, next);

    if (next_size == n && memcmp(next, opcodes, next_size) == 0) {
        fprintf(stderr, "collision detected:\n");
        IndentationData table_indent_data = get_table_indentation(indent_data);
        print_table_head(table_indent_data);
        for (int j = 0; j < next_size; j++) {
          print_table_row(next[j], indent_data, table_indent_data);
        }

      return true;
    }

    if (next_size > 1)
      ret = check_for_collisions(next, next_size, indent_data);
  }

  return ret;
}

OpcodenatorData init_opcodenator(OpcodeData *opcodes, uint16_t n,
    const char *indentation_string, const char *function_prefix,
    const char *decode_function_name) {
  assert(strlen(indentation_string) <= 32 && "indent string too big");
  assert(strlen(function_prefix) <= 32 && "function prefix too big");
  assert(strlen(decode_function_name) <= 32 && "decode function name too big");

  assert(strlen(opcodes[0].opcode_str) <= UINT8_MAX && "opcode str too big");
  uint8_t opcode_bits = strlen(opcodes[0].opcode_str);
  uint8_t max_enum_str_len = 0;

  for (size_t i = 1; i < n; i++) {
    assert(strlen(opcodes[i].opcode_str) <= UINT8_MAX && "opcode str too big");
    if (opcode_bits != strlen(opcodes[i].opcode_str)) {
      fprintf(stderr,
          "all opcodes must have the same number of bits. %s: %u bits\n",
          opcodes[i].enum_str, opcode_bits);
      exit(1);
    }
    
    assert(strlen(opcodes[i].enum_str) <= UINT8_MAX && "enum str too big");
    if (max_enum_str_len < strlen(opcodes[i].enum_str))
      max_enum_str_len = strlen(opcodes[i].enum_str);
  }
  
  for (size_t i = 0; i < n; i++) {
    opcodes[i].zeroed_value = get_zeroed(opcodes[i].opcode_str, opcode_bits);
    opcodes[i].oned_value = get_oned(opcodes[i].opcode_str, opcode_bits);
  }

  uint8_t opcode_hex_width = ceil((float)opcode_bits / 4);
  opcode_hex_width += opcode_hex_width % 2 == 0 ? 0 : 1;
  IndentationData indent_data = {
    .enum_name_width      = max_enum_str_len,
    .opcode_hex_width     = opcode_hex_width,
    .function_name_width  = max_enum_str_len + strlen(function_prefix),
    .opcode_width         = opcode_bits,
  };
    
  bool have_duplicate = check_for_duplicates(opcodes, n, indent_data);
  if (have_duplicate)
    fprintf(stderr, "\n");
  bool have_collision = check_for_collisions(opcodes, n, indent_data);
  if (have_collision)
    fprintf(stderr, "\n");
  
  if (have_duplicate || have_collision) {
    fprintf(stderr, "exiting with errors\n");
    exit(1);
  }
  
  OpcodenatorData ret = {
    .opcodes               = opcodes,
    .opcode_bits           = opcode_bits,
    .size                  = n,
    .indent_string         = indentation_string,
    .function_prefix       = function_prefix,
    .indent_data           = indent_data,
    .decode_function_name = decode_function_name,
  };

  return ret;
}


void print_enum_declaration(OpcodenatorData d) {
  ind_fprintf(stdout, d.indent_string, 0, "typedef enum {\n");
  for (int i = 0; i < d.size; i++) {
    ind_fprintf(stdout, d.indent_string, 1, "%s,\n", d.opcodes[i].enum_str);
  }
  ind_fprintf(stdout, d.indent_string, 1, "%s,\n", "INVALID_OP");
  ind_fprintf(stdout, d.indent_string, 0, "} OpcodeType;\n");
}

void print_struct_declaration(OpcodenatorData d) {
  ind_fprintf(stdout, d.indent_string, 0, "typedef struct {\n");
  ind_fprintf(stdout, d.indent_string, 1, "char name[64];\n");
  ind_fprintf(stdout, d.indent_string, 1, "void (*function)(uint64_t);\n");
  ind_fprintf(stdout, d.indent_string, 0, "} OpcodeData;\n");
}

void print_function_declarations(OpcodenatorData d) {
  for (int i = 0; i < d.size; i++) {
    ShortString function_name = shortf("%s", d.opcodes[i].enum_str);
    for (int j = 0; j < function_name.len; j++) {
      function_name.val[j] = tolower(function_name.val[j]);
    }
    fprintf(stdout, "void %s%s(uint64_t opcode);\n", d.function_prefix,
        function_name.val); 
  }
}

void print_empty_function_definitions(OpcodenatorData d) {
  for (int i = 0; i < d.size; i++) {
    ShortString function_name = shortf("%s", d.opcodes[i].enum_str);
    for (int j = 0; j < function_name.len; j++) {
      function_name.val[j] = tolower(function_name.val[j]);
    }
    fprintf(stdout, "void %s%s(uint64_t) { }\n", d.function_prefix,
        function_name.val); 
  }
}

void print_array_definition(OpcodenatorData d) {
  ind_fprintf(stdout, d.indent_string, 0, "OpcodeData opcodes[] = {\n");

  for (int i = 0; i < d.size; i++) {
    ShortString function_name = shortf("op_%s", d.opcodes[i].enum_str);
    for (int j = 0; j < function_name.len; j++) {
      function_name.val[j] = tolower(function_name.val[j]);
    }

    ShortString enum_in_brackets = shortf("[%s]", d.opcodes[i].enum_str);
    ShortString enum_in_quotes = shortf("\"%s\",", d.opcodes[i].enum_str);
    ind_fprintf(stdout, d.indent_string, 1,
        "%-*s = { .name = %-*s .function = %-*s },\n",
        d.indent_data.enum_name_width + 2, enum_in_brackets.val,
        d.indent_data.enum_name_width + 3, enum_in_quotes.val,
        d.indent_data.function_name_width, function_name.val);

  }

  ind_fprintf(stdout, d.indent_string, 0, "};\n");
}

void print_decoder_function(OpcodenatorData d) {
  fprintf(stdout, "OpcodeType %s(uint64_t opcode) {\n", d.decode_function_name);
  print_decoder_switch(d.indent_string, 1, d.opcodes, d.size, d.indent_data);
  ind_fprintf(stdout, d.indent_string, 1, "return INVALID_OP;\n");
  fprintf(stdout, "}\n");
}

void print_includes() {
  fprintf(stdout, "#include <stdint.h>\n");
}

#endif // OPCODENATOR_IMPLEMENTATION
