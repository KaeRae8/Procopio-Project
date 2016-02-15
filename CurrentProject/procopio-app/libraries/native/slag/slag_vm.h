//=============================================================================
//  slag_vm.h
//
//  Slag Virtual Machine Header
//
//  v3.5.0
//  ---------------------------------------------------------------------------
//
//  Copyright 2008-2011 Plasmaworks LLC
//
//  Licensed under the Apache License, Version 2.0 (the "License"); 
//  you may not use this file except in compliance with the License. 
//  You may obtain a copy of the License at 
//
//    http://www.apache.org/licenses/LICENSE-2.0 
//
//  Unless required by applicable law or agreed to in writing, 
//  software distributed under the License is distributed on an 
//  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
//  either express or implied. See the License for the specific 
//  language governing permissions and limitations under the License.
//
//  ---------------------------------------------------------------------------
//
//  History:
//    2010.11.26 / Abe Pralle - v3.2 revamp
//    2008.12.17 / Abe Pralle - Created
//=============================================================================
#ifndef SLAG_VM_H
#define SLAG_VM_H

#include "slag.h"

typedef int SlagOpcode;

//=============================================================================
//  StringBuilder
//=============================================================================
struct StringBuilder : ArrayList<char>
{
  void print( const char* st )
  {
    while (*st) add(*(st++));
  }

  void print( char ch ) { add(ch); }

  void print( int n )
  {
    if (n < 0)
    {
      print('-');
      print(-n);
    }
    else
    {
      if (n > 9)
      {
        print(n/10);
      }
      add('0'+(n%10));
    }
  }

  char* to_new_ascii()
  {
    char* buffer = new char[count+1];
    memcpy( buffer, data, count );
    buffer[count] = 0;
    return buffer;
  }
};

//=============================================================================
//  Slag Structures
//=============================================================================
struct SlagTypeInfo;

struct SlagLocalVarInfo
{
  SlagTypeInfo* type;
  int offset;

  SlagLocalVarInfo( SlagTypeInfo* type ) : type(type), offset(0)
  {
  }
};

struct SlagParameterList
{
  AllocList<SlagLocalVarInfo*> parameters;
  int num_ref_params;
  int num_data_params;

  SlagParameterList( int n ) : num_ref_params(0), num_data_params(0)
  {
    parameters.ensure_capacity(n);
  }

  inline int count() { return parameters.count; }
  inline SlagLocalVarInfo*& operator[](int index) { return parameters[index]; }
  inline SlagLocalVarInfo*& get(int index) { return parameters[index]; }
};

struct SlagPropertyInfo
{
  SlagTypeInfo* type;
  char*         name;
  int           offset;

  SlagPropertyInfo( SlagTypeInfo* type ) : type(type), name(NULL), offset(0)
  {
  }
};

struct SlagSourcePos
{
  int code_offset;
  int source_pos;
    // If pos <= 0 then |pos| is filename index, else is line #

  SlagSourcePos() : code_offset(0), source_pos(0)
  {
  }

  SlagSourcePos( int code_offset, int source_pos ) 
    : code_offset(code_offset), source_pos(source_pos)
  {
  }
};

struct SlagCatchInfo
{
  SlagTypeInfo* type_caught;
  SlagInt32 begin_offset;
  SlagInt32 end_offset;
  SlagInt32 handler;

  SlagCatchInfo( SlagTypeInfo* type_caught, SlagInt32 begin_offset, SlagInt32 end_offset,
      SlagInt32 handler ) : type_caught(type_caught), begin_offset(begin_offset),
      end_offset(end_offset), handler(handler)
  {
  }
};

struct SlagMethodInfo
{
  char* name;
  char* signature;
  int   index;
  int   qualifiers;
  int   num_local_refs;
  int   local_data_size;
  SlagTypeInfo*              type_context;
  SlagTypeInfo*              return_type;
  SlagParameterList*         parameters;
  AllocList<SlagCatchInfo*>    catches;
  AllocList<SlagLocalVarInfo*> local_vars;

  SlagOpcode*    bytecode;
  int            bytecode_offset;
  int            bytecode_limit;
  int            source_pos_offset;
  SlagNativeFn   native_handler;

  SlagMethodInfo() : signature(0), num_local_refs(0), local_data_size(0), 
      native_handler(NULL)
  {
  }

  ~SlagMethodInfo()
  {
    if (signature)
    {
      delete signature;
      signature = NULL;
    }
  }

  void create_signature();
};

struct SlagGenericObject;

struct SlagTypeInfo : SlagTypeInfoBase
{
  SlagTypeInfo* element_type; // arrays only

  AllocList<SlagPropertyInfo*> properties;
  ArrayList<SlagMethodInfo*>   methods;
  ArrayList<SlagMethodInfo*>   dispatch_table;

  int num_reference_properties;
  int stack_slots;  // # slots on the stack (pointer size for objects)

  SlagMethodInfo* method_init_object;
  SlagMethodInfo* method_clean_up;

  SlagTypeInfo() : element_type(NULL), num_reference_properties(0)
  {
    method_init_object = NULL;
    method_clean_up = NULL;
  }

  bool is_real();
  bool is_integer();
  bool is_int32_64();
  bool is_logical();

  SlagMethodInfo* find_method( const char* sig );
  SlagMethodInfo* must_find_method( const char* sig );
  SlagPropertyInfo* find_property( const char* name, SlagTypeInfo* required_type=NULL );
  SlagPropertyInfo* must_find_property( const char* name, SlagTypeInfo* required_type=NULL );

  SlagObject* singleton();

  SlagObject* create();
  SlagArray*  create( int array_count );
};

struct SlagGenericObject : SlagObject
{
  SlagInt64  data[1];  // Int64 forces this member to have 8-byte alignment.

  int property_offset( const char* name );
  void* property_address( const char* name );
  void* property_address( int offset );
  void set_ref( const char* id, SlagObject* object );
  void set_ref( int offset, SlagObject* object );
  SlagObject* get_ref( const char* name );
  SlagObject* get_ref( int offset );
};


void slag_assert( bool value, const char* st );
void slag_assert( bool value, const char* st1, const char* st2, const char* st3 );
void slag_throw_exception_of_type( SlagTypeInfo* type );


//=============================================================================
// SlagVM
//=============================================================================
#define SLAGOP_HALT                       0
#define SLAGOP_NOP                        1
#define SLAGOP_BREAKPOINT                 2
#define SLAGOP_MISSING_RETURN_ERROR       3
#define SLAGOP_RETURN_NIL                 4
#define SLAGOP_RETURN_REF                 5
#define SLAGOP_RETURN_8                   6
#define SLAGOP_RETURN_X                   7
#define SLAGOP_DUPLICATE_REF              8
#define SLAGOP_DUPLICATE_8                9
#define SLAGOP_POP_REF                   10
#define SLAGOP_POP_8                     11
#define SLAGOP_POP_X                     12
#define SLAGOP_JUMP                      13
#define SLAGOP_JUMP_IF_TRUE              14
#define SLAGOP_JUMP_IF_FALSE             15
#define SLAGOP_JUMP_IF_REF               16
#define SLAGOP_JUMP_IF_NULL_REF          17
#define SLAGOP_THROW                     18
#define SLAGOP_LITERAL_STRING            19
#define SLAGOP_LITERAL_NULL              20
#define SLAGOP_LITERAL_8                 21
#define SLAGOP_LITERAL_4                 22
#define SLAGOP_LITERAL_INTEGER_1         23
#define SLAGOP_LITERAL_INTEGER_0         24
#define SLAGOP_LITERAL_INTEGER_NEG1      25
#define SLAGOP_LITERAL_REAL_1            26
#define SLAGOP_LITERAL_REAL_0            27
#define SLAGOP_LITERAL_REAL_NEG1         28
#define SLAGOP_READ_THIS_REF             29
#define SLAGOP_READ_SINGLETON_REF        30
#define SLAGOP_READ_PROPERTY_REF         31
#define SLAGOP_READ_PROPERTY_1U          32
#define SLAGOP_READ_PROPERTY_2U          33
#define SLAGOP_READ_PROPERTY_4           34
#define SLAGOP_READ_PROPERTY_8           35
#define SLAGOP_READ_PROPERTY_X           36
#define SLAGOP_READ_THIS_PROPERTY_REF    37
#define SLAGOP_READ_THIS_PROPERTY_4      38
#define SLAGOP_READ_THIS_PROPERTY_8      39
#define SLAGOP_READ_THIS_PROPERTY_X      40
#define SLAGOP_READ_LOCAL_REF            41
#define SLAGOP_READ_LOCAL_8              42
#define SLAGOP_READ_LOCAL_X              43
#define SLAGOP_READ_COMPOUND_8           44
#define SLAGOP_READ_COMPOUND_X           45
#define SLAGOP_WRITE_SINGLETON_REF       46
#define SLAGOP_WRITE_PROPERTY_REF        47
#define SLAGOP_WRITE_PROPERTY_1          48
#define SLAGOP_WRITE_PROPERTY_2          49
#define SLAGOP_WRITE_PROPERTY_4          50
#define SLAGOP_WRITE_PROPERTY_8          51
#define SLAGOP_WRITE_PROPERTY_X          52
#define SLAGOP_WRITE_THIS_PROPERTY_REF   53
#define SLAGOP_WRITE_THIS_PROPERTY_4     54
#define SLAGOP_WRITE_THIS_PROPERTY_8     55
#define SLAGOP_WRITE_THIS_PROPERTY_X     56
#define SLAGOP_WRITE_LOCAL_REF           57
#define SLAGOP_WRITE_LOCAL_8             58
#define SLAGOP_WRITE_LOCAL_X             59
#define SLAGOP_FAUX_STATIC_CALL          60
#define SLAGOP_STATIC_CALL               61
#define SLAGOP_DYNAMIC_CALL              62
#define SLAGOP_NATIVE_CALL               63
#define SLAGOP_NEW_OBJECT                64
#define SLAGOP_NEW_OBJECT_NO_INIT        65
#define SLAGOP_NEW_ARRAY                 66
#define SLAGOP_ARRAY_READ_REF            67
#define SLAGOP_ARRAY_READ_1U             68
#define SLAGOP_ARRAY_READ_2U             69
#define SLAGOP_ARRAY_READ_4              70
#define SLAGOP_ARRAY_READ_8              71
#define SLAGOP_ARRAY_READ_X              72
#define SLAGOP_ARRAY_WRITE_REF           73
#define SLAGOP_ARRAY_WRITE_1             74
#define SLAGOP_ARRAY_WRITE_2             75
#define SLAGOP_ARRAY_WRITE_4             76
#define SLAGOP_ARRAY_WRITE_8             77
#define SLAGOP_ARRAY_WRITE_X             78
#define SLAGOP_ARRAY_DUPLICATE           79
#define SLAGOP_TYPECHECK                 80
#define SLAGOP_AS_REF                    81
#define SLAGOP_CAST_REAL_TO_INTEGER      82
#define SLAGOP_CAST_INTEGER_TO_REAL      83
#define SLAGOP_CAST_REAL_TO_LOGICAL      84
#define SLAGOP_CAST_INTEGER_TO_LOGICAL   85
#define SLAGOP_CAST_INTEGER_TO_I32       86
#define SLAGOP_CAST_INTEGER_TO_CHAR      87
#define SLAGOP_CAST_INTEGER_TO_BYTE      88
#define SLAGOP_CMP_INSTANCE_OF           89
#define SLAGOP_CMP_EQ_REF                90
#define SLAGOP_CMP_NE_REF                91
#define SLAGOP_CMP_EQ_REF_NULL           92
#define SLAGOP_CMP_NE_REF_NULL           93
#define SLAGOP_CMP_EQ_X                  94
#define SLAGOP_CMP_NE_X                  95
#define SLAGOP_CMP_EQ_INTEGER            96
#define SLAGOP_CMP_NE_INTEGER            97
#define SLAGOP_CMP_GT_INTEGER            98
#define SLAGOP_CMP_GE_INTEGER            99
#define SLAGOP_CMP_LT_INTEGER           100
#define SLAGOP_CMP_LE_INTEGER           101
#define SLAGOP_CMP_EQ_REAL              102
#define SLAGOP_CMP_NE_REAL              103
#define SLAGOP_CMP_GT_REAL              104
#define SLAGOP_CMP_GE_REAL              105
#define SLAGOP_CMP_LT_REAL              106
#define SLAGOP_CMP_LE_REAL              107
#define SLAGOP_NOT_INTEGER              108
#define SLAGOP_NOT_LOGICAL              109
#define SLAGOP_NEGATE_INTEGER           110
#define SLAGOP_NEGATE_REAL              111
#define SLAGOP_AND_LOGICAL              112
#define SLAGOP_OR_LOGICAL               113
#define SLAGOP_XOR_LOGICAL              114
#define SLAGOP_ADD_INTEGER              115
#define SLAGOP_SUB_INTEGER              116
#define SLAGOP_MUL_INTEGER              117
#define SLAGOP_DIV_INTEGER              118
#define SLAGOP_MOD_INTEGER              119
#define SLAGOP_EXP_INTEGER              120
#define SLAGOP_AND_INTEGER              121
#define SLAGOP_OR_INTEGER               122
#define SLAGOP_XOR_INT64                123
#define SLAGOP_SHL_INT64                124
#define SLAGOP_SHR_INT64                125
#define SLAGOP_SHRX_INT64               126
#define SLAGOP_XOR_INT32                127
#define SLAGOP_SHL_INT32                128
#define SLAGOP_SHR_INT32                129
#define SLAGOP_SHRX_INT32               130
#define SLAGOP_ADD_REAL                 131
#define SLAGOP_SUB_REAL                 132
#define SLAGOP_MUL_REAL                 133
#define SLAGOP_DIV_REAL                 134
#define SLAGOP_MOD_REAL                 135
#define SLAGOP_EXP_REAL                 136
#define SLAGOP_THIS_ADD_ASSIGN_I32      137
#define SLAGOP_THIS_SUB_ASSIGN_I32      138
#define SLAGOP_THIS_MUL_ASSIGN_I32      139
#define SLAGOP_THIS_INCREMENT_I32       140
#define SLAGOP_THIS_DECREMENT_I32       141
#define SLAGOP_THIS_ADD_ASSIGN_R64      142
#define SLAGOP_THIS_SUB_ASSIGN_R64      143
#define SLAGOP_THIS_MUL_ASSIGN_R64      144
#define SLAGOP_THIS_INCREMENT_R64       145
#define SLAGOP_THIS_DECREMENT_R64       146
#define SLAGOP_LOCAL_ADD_ASSIGN_INTEGER 147
#define SLAGOP_LOCAL_SUB_ASSIGN_INTEGER 148
#define SLAGOP_LOCAL_MUL_ASSIGN_INTEGER 149
#define SLAGOP_LOCAL_INCREMENT_INTEGER  150
#define SLAGOP_LOCAL_DECREMENT_INTEGER  151
#define SLAGOP_LOCAL_ADD_ASSIGN_REAL    152
#define SLAGOP_LOCAL_SUB_ASSIGN_REAL    153
#define SLAGOP_LOCAL_MUL_ASSIGN_REAL    154
#define SLAGOP_LOCAL_INCREMENT_REAL     155
#define SLAGOP_LOCAL_DECREMENT_REAL     156

#define SLAG_PUSH_REF( obj ) svm.regs.stack.refs[-1] = (obj); --svm.regs.stack.refs
#define SLAG_PEEK_REF() (*(svm.regs.stack.refs))
#define SLAG_POP_REF() *(svm.regs.stack.refs++)
#define SLAG_DUPLICATE_REF() svm.regs.stack.refs[-1] = *svm.regs.stack.refs; --svm.regs.stack.refs

#define SLAG_POP_INTEGER()   *(svm.regs.stack.data++)
#define SLAG_PUSH_INTEGER(i) *(--svm.regs.stack.data) = i
#define SLAG_PEEK_INTEGER() *(svm.regs.stack.data)

#define SLAG_POP_REAL()   *(SlagReal64*)(svm.regs.stack.data++)
#define SLAG_PUSH_REAL(r) *(SlagReal64*)(--svm.regs.stack.data) = r
#define SLAG_PEEK_REAL() *(SlagReal64*)(svm.regs.stack.data)

#define SLAG_PUSH(type,value) *((type*)(svm.regs.stack.data -= sizeof(type)/8)) = (value)
#define SLAG_PEEK(type) *((type*)svm.regs.stack.data)
#define SLAG_POP(type) ((type*)(svm.regs.stack.data += sizeof(type)/8))[-1]


#define SVM_DEREFERENCE(context,offset,property_type) *((property_type*)(((char*)context)+offset))

#define SLAG_FIND_TYPE( var_name, type_name ) \
  SlagTypeInfo* var_name; \
  { \
    static int type_index = -1; \
    if (type_index == -1) type_index = svm.must_find_type(type_name)->index; \
    var_name = svm.types[type_index]; \
  }

#define SLAG_GET_REF(var_name,obj,name) \
  SLAG_GET(SlagObject*,var_name,obj,name)

#define SLAG_SET_REF(obj,name,value) \
  {\
    static int property_offset = -1; \
    if (property_offset == -1) property_offset = (obj)->type->must_find_property(name)->offset; \
    ((SlagGenericObject*)(obj))->set_ref( property_offset, value ); \
  }

#define SLAG_GET(vartype,var_name,obj,name) \
  vartype var_name; \
  {\
    static int property_offset = -1; \
    if (property_offset == -1) property_offset = (obj)->type->must_find_property(name)->offset; \
    var_name = *((vartype*)(((SlagGenericObject*)(obj))->property_address( property_offset ))); \
  }

#define SLAG_SET(vartype,obj,name,value) \
  {\
    static int property_offset = -1; \
    if (property_offset == -1) property_offset = (obj)->type->must_find_property(name)->offset; \
    *((vartype*)(((SlagGenericObject*)(obj))->property_address(property_offset))) = value; \
  }

#define SLAG_CALL( type, sig ) \
  {\
    static int method_index = -1; \
    if (method_index == -1) method_index = type->must_find_method(sig)->index; \
    svm.call( svm.methods[method_index] ); \
  }

#define SLAGCODE_OPERAND_I32() *(svm.regs.ip++)
#define SLAGCODE_OPERAND_I64() svm.literal_table[*(svm.regs.ip++)]
#define SLAGCODE_OPERAND_ADDR() svm.address_table[*(svm.regs.ip++)]

#define SVM_ASSERT(obj,err_type,cmd) if (!(obj)) { svm.throw_exception(err_type); cmd; }

#define SVM_NULL_CHECK(obj,cmd) if (!(obj)) { svm.throw_exception(svm.type_null_reference_error); cmd; }

#define SVM_THROW_TYPE(err_type,cmd) { svm.throw_exception(err_type); cmd; }

#define SLAGCODE_READ_PROPERTY(property_type) \
  { \
    SlagObject* context = SLAG_POP_REF( ); \
    SVM_ASSERT( context, svm.type_null_reference_error, continue ); \
    SLAG_PUSH_INTEGER( SVM_DEREFERENCE(context,SLAGCODE_OPERAND_I32(),property_type) ); \
  }

#define SLAGCODE_WRITE_PROPERTY(property_type) \
  { \
    SlagInt64   new_value = SLAG_POP_INTEGER(); \
    SlagObject* context = SLAG_POP_REF( ); \
    SVM_ASSERT( context, svm.type_null_reference_error, continue ); \
    SVM_DEREFERENCE(context,SLAGCODE_OPERAND_I32(),property_type) = (property_type) new_value; \
  }

#define SLAGCODE_WRITE_THIS_PROPERTY(property_type) \
    SVM_DEREFERENCE(svm.regs.frame.refs[-1],SLAGCODE_OPERAND_I32(),property_type) = (property_type) SLAG_POP_INTEGER()

#define SLAG_TYPE_STRING svm.type_string
#define SLAG_TYPE_WEAK_REFERENCE svm.type_weak_reference
#define SLAG_TYPE_ARRAY_OF_CHAR svm.type_array_of_char

#define SLAG_REF_STACK_PTR svm.regs.stack.refs
#define SLAG_REF_STACK_LIMIT svm.ref_stack_limit
#define SLAG_SINGLETONS svm.singletons
#define SLAG_SINGLETONS_COUNT svm.singletons.count


struct SlagStackPointers
{
  SlagInt64*     data;
  SlagObject**   refs;
};

/*
struct SlagCatchInfo
{
  SlagOpcode*    begin_addr;
  SlagOpcode*    end_addr;
  SlagTypeInfo* type_caught;
  SlagOpcode*    handler;
};
*/

struct SlagCallFrame
{
  SlagMethodInfo*     called_method;
  SlagOpcode*         return_address;
  SlagStackPointers   prior_frame;
};

struct SlagRegisters
{
  SlagStackPointers stack;
  SlagStackPointers frame;
  SlagOpcode*       ip;
  SlagCallFrame*    call_frame;
};

struct SlagNativeFnHook
{
  const char* class_name;
  const char* signature;
  SlagNativeFn fn;

  SlagNativeFnHook() : class_name(NULL), signature(NULL), fn(NULL) { }

  SlagNativeFnHook( const char* _class_name, const char* _signature, SlagNativeFn _fn )
    : class_name(_class_name), signature(_signature), fn(_fn) { }
};

struct SlagVM
{
  SlagRegisters  regs;

  SlagObject**   min_ref_stack;
  SlagInt64*     min_data_stack;
  SlagCallFrame* min_call_stack;

  SlagObject**   ref_stack;
  SlagInt64*     data_stack;
  SlagCallFrame* call_stack;

  SlagObject**   ref_stack_limit;
  SlagInt64*     data_stack_limit;
  SlagCallFrame* call_stack_limit;

  ArrayList<SlagOpcode>    code;
  ArrayList<SlagInt64>     literal_table;  // 64-bit literals
  ArrayList<int>           address_offsets;
  ArrayList<void*>         address_table;
  ArrayList<SlagSourcePos> line_table;
  ArrayList<SlagObject*>   singletons;

  ArrayList<SlagNativeFnHook> native_method_hooks;

  AllocList<char*> filenames;
  AllocList<char*> identifiers;
  AllocList<SlagTypeInfo*> types;
  AllocList<SlagParameterList*> parameter_table;
  AllocList<SlagMethodInfo*>    methods;
  ArrayList<SlagObject*>        strings;
  SlagTypeInfo*  main_class;

  SlagTypeInfo* type_object;
  SlagTypeInfo* type_int64;
  SlagTypeInfo* type_int32;
  SlagTypeInfo* type_char;
  SlagTypeInfo* type_byte;
  SlagTypeInfo* type_real64;
  SlagTypeInfo* type_real32;
  SlagTypeInfo* type_logical;
  SlagTypeInfo* type_null;
  SlagTypeInfo* type_string;
  SlagTypeInfo* type_requires_cleanup;
  SlagTypeInfo* type_native_data;
  SlagTypeInfo* type_system;
  SlagTypeInfo* type_weak_reference;
  SlagTypeInfo* type_array_of_char;
  SlagTypeInfo* type_array_of_int64;
  SlagTypeInfo* type_missing_return_error;
  SlagTypeInfo* type_type_cast_error;
  SlagTypeInfo* type_out_of_bounds_error;
  SlagTypeInfo* type_divide_by_zero_error;
  SlagTypeInfo* type_null_reference_error;
  SlagTypeInfo* type_stack_limit_error;
  SlagTypeInfo* type_file_error;
  SlagTypeInfo* type_file_not_found_error;
  SlagTypeInfo* type_no_next_value_error;
  SlagTypeInfo* type_socket_error;
  SlagObject*   string_invalid_directory;
  SlagTypeInfo* type_invalid_operand_error;

  char* raw_exe_filepath;

  bool        initialized;
  SlagOpcode* exception_ip;

  SlagVM() : ref_stack(NULL), data_stack(NULL), call_stack(NULL)
  {
    initialized = false;
    init();
    raw_exe_filepath = (char*) "unavailable";
  }

  ~SlagVM();

  void init()
  {
    if (initialized) return;
    initialized = true;
    exception_ip = NULL;
    hook_stdlib();
  }

  void shut_down();

  SlagObject*    create_type( const char* name );
  SlagTypeInfo*  find_type( const char* name );
  SlagTypeInfo*  must_find_type( const char* name );

  void hook_native( const char* class_name, const char* sig, SlagNativeFn fn );
  void hook_stdlib();
  void apply_hooks();

  void set_command_line_args( char* args[], int count );
  void prep_types_and_methods();
  void configure();
  void create_singletons();
  void call( SlagMethodInfo* method );
  void invoke( SlagMethodInfo* method );
  void execute();

  void throw_exception_on_stack();
  void throw_exception( SlagTypeInfo* type, const char* mesg=NULL );
  void throw_stack_limit_error();
};

extern SlagVM svm;


//=============================================================================
//  Loader
//=============================================================================
void SlagVM_unhandled_native_method();

//-----------------------------------------------------------------------------
//  SlagReader
//-----------------------------------------------------------------------------
struct SlagReader
{
  const unsigned char* data;
  int   pos;
  int   remaining;
  int   total_size;
  bool  free_data;

  SlagReader() : data(NULL), free_data(false) { }
  ~SlagReader();

  void init( const char* filename );
  void init( const unsigned char* data, int size );
  int  read();
  int  read16();
  int  read32();
  SlagInt64 read64();
  int  readX();
  int  read_utf8();
  char* read_new_ascii();
};

struct SlagCodeLabel
{
  const char* name;
  int id;
  int n;
  int address_index;

  SlagCodeLabel() : name(NULL), id(0), n(0) { }
  SlagCodeLabel( const char* name, int id, int n, int address_index=0 ) 
    : name(name), id(id), n(n), address_index(address_index) { }

  bool equals( SlagCodeLabel other )
  {
    return (name == other.name && id == other.id && n == other.n);
  }
};

struct SlagOpInfo
{
  int op;
  int code_offset;

  SlagOpInfo() : op(0), code_offset(0)
  {
  }

  SlagOpInfo( int op, int code_offset ) : op(op), code_offset(code_offset)
  {
  }
};

struct SlagLoader
{
  SlagReader reader;
  SlagMethodInfo* this_method;
  int cur_line;
  int next_structure_id;
  int next_auto_id;
  ArrayList<SlagCodeLabel> resolved_labels;
  ArrayList<SlagCodeLabel> unresolved_labels;
  ArrayList<SlagOpInfo>    op_history;

  SlagLoader() : this_method(NULL), cur_line(0)
  {
  }

  void load( const char* filename );
  void load( const char* data, int count );

  void load();
  void load_version();
  void load_filenames();
  void load_identifiers();
  void load_type_info();
  void load_parameter_info();
  void load_method_info();
  void load_type_defs();
  void load_string_table();
  void load_method_defs();

  void load_method_body( SlagMethodInfo* m );

  SlagTypeInfo*     load_type();
  SlagMethodInfo*   load_method();
  char*             load_id();
  SlagLocalVarInfo* load_local();
  SlagPropertyInfo* load_this_property();

  void must_consume_header( const char* header_id );

  void write_op( int op );
  void write_i32( int i32 );
  void write_op_i32( int op, SlagInt32 value );
  void write_op_i64( int op, SlagInt64 value );
  int  history( int num_back=1 );
  void undo_op( int count=1 );

  void load_statement_list();
  SlagTypeInfo* load_statement();
  SlagTypeInfo* load_expression();
  SlagTypeInfo* load_ref_expression();
  void          load_logical_expression();
  void          load_int32_expression();
  SlagTypeInfo* load_compound_expression();
  int  get_next_structure_id();
  void define_label( const char* name, int id, int n ); 
  bool define_transient_label( const char* name, int id, int n, int* index_ptr=NULL ); 
  void write_label_address( const char* name, int id, int n ); 
  void close_label_id( int id );
  void write_jump_if_true( const char* name, int id, int n );
  void write_jump_if_false( const char* name, int id, int n );
};

void slag_set_raw_exe_filepath( char* filepath );
void slag_set_command_line_args( char** argv, int argc );
void slag_throw_file_error();
void slag_throw_file_error( char* filename );
void slag_throw_file_not_found_error( char* filename );
void slag_throw_no_next_value_error();
void slag_throw_socket_error();
void slag_throw_invalid_operand_error();

#endif // SLAG_VM_H

