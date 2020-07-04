
//**************************************************************
//
// Code generator SKELETON
//
// Read the comments carefully. Make sure to
//    initialize the base class tags in
//       `CgenClassTable::CgenClassTable'
//
//    Add the label for the dispatch tables to
//       `IntEntry::code_def'
//       `StringEntry::code_def'
//       `BoolConst::code_def'
//
//    Add code to emit everyting else that is needed
//       in `CgenClassTable::code'
//
//
// The files as provided will produce code to begin the code
// segments, declare globals, and emit constants.  You must
// fill in the rest.
//
//**************************************************************

#include "cgen.h"
#include "cgen_gc.h"

extern void emit_string_constant(ostream& str, char *s);
extern int cgen_debug;
extern int disable_reg_alloc;
extern int node_lineno;

//
// Three symbols from the semantic analyzer (semant.cc) are used.
// If e : No_type, then no code is generated for e.
// Special code is generated for new SELF_TYPE.
// The name "self" also generates code different from other references.
//
//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
Symbol 
       arg,
       arg2,
       Bool,
       concat,
       cool_abort,
       copy,
       Int,
       in_int,
       in_string,
       IO,
       length,
       Main,
       main_meth,
       No_class,
       No_type,
       Object,
       out_int,
       out_string,
       prim_slot,
       self,
       SELF_TYPE,
       Str,
       str_field,
       substr,
       type_name,
       val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
  arg         = idtable.add_string("arg");
  arg2        = idtable.add_string("arg2");
  Bool        = idtable.add_string("Bool");
  concat      = idtable.add_string("concat");
  cool_abort  = idtable.add_string("abort");
  copy        = idtable.add_string("copy");
  Int         = idtable.add_string("Int");
  in_int      = idtable.add_string("in_int");
  in_string   = idtable.add_string("in_string");
  IO          = idtable.add_string("IO");
  length      = idtable.add_string("length");
  Main        = idtable.add_string("Main");
  main_meth   = idtable.add_string("main");
//   _no_class is a symbol that can't be the name of any 
//   user-defined class.
  No_class    = idtable.add_string("_no_class");
  No_type     = idtable.add_string("_no_type");
  Object      = idtable.add_string("Object");
  out_int     = idtable.add_string("out_int");
  out_string  = idtable.add_string("out_string");
  prim_slot   = idtable.add_string("_prim_slot");
  self        = idtable.add_string("self");
  SELF_TYPE   = idtable.add_string("SELF_TYPE");
  Str         = idtable.add_string("String");
  str_field   = idtable.add_string("_str_field");
  substr      = idtable.add_string("substr");
  type_name   = idtable.add_string("type_name");
  val         = idtable.add_string("_val");
}

static char *gc_init_names[] =
  { "_NoGC_Init", "_GenGC_Init", "_ScnGC_Init" };
static char *gc_collect_names[] =
  { "_NoGC_Collect", "_GenGC_Collect", "_ScnGC_Collect" };


//  BoolConst is a class that implements code generation for operations
//  on the two booleans, which are given global names here.
BoolConst falsebool(FALSE);
BoolConst truebool(TRUE);

#define TEMP1 "_1"

//*********************************************************
//
// Define method for code generation
//
// This is the method called by the compiler driver
// `cgtest.cc'. cgen takes an `ostream' to which the assembly will be
// emmitted, and it passes this and the class list of the
// code generator tree to the constructor for `CgenClassTable'.
// That constructor performs all of the work of the code
// generator.
//
//*********************************************************

void program_class::cgen(ostream &os) 
{
  // spim wants comments to start with '#'
  os << "# start of generated code\n";

  initialize_constants();
  CgenClassTable *codegen_classtable = new CgenClassTable(classes,os);

  os << "\n# end of generated code\n";
}


//////////////////////////////////////////////////////////////////////////////
//
//  emit_* procedures
//
//  emit_X  writes code for operation "X" to the output stream.
//  There is an emit_X for each opcode X, as well as emit_ functions
//  for generating names according to the naming conventions (see emit.h)
//  and calls to support functions defined in the trap handler.
//
//  Register names and addresses are passed as strings.  See `emit.h'
//  for symbolic names you can use to refer to the strings.
//
//////////////////////////////////////////////////////////////////////////////

static void emit_load(char *dest_reg, int offset, char *source_reg, ostream& s)
{
  s << LW << dest_reg << " " << offset * WORD_SIZE << "(" << source_reg << ")" 
    << endl;
}

static void emit_store(char *source_reg, int offset, char *dest_reg, ostream& s)
{
  s << SW << source_reg << " " << offset * WORD_SIZE << "(" << dest_reg << ")"
      << endl;
}

static void emit_load_imm(char *dest_reg, int val, ostream& s)
{ s << LI << dest_reg << " " << val << endl; }

static void emit_load_address(char *dest_reg, char *address, ostream& s)
{ s << LA << dest_reg << " " << address << endl; }

static void emit_partial_load_address(char *dest_reg, ostream& s)
{ s << LA << dest_reg << " "; }

static void emit_load_bool(char *dest, const BoolConst& b, ostream& s)
{
  emit_partial_load_address(dest,s);
  b.code_ref(s);
  s << endl;
}

static void emit_load_string(char *dest, StringEntry *str, ostream& s)
{
  emit_partial_load_address(dest,s);
  str->code_ref(s);
  s << endl;
}

static void emit_load_int(char *dest, IntEntry *i, ostream& s)
{
  emit_partial_load_address(dest,s);
  i->code_ref(s);
  s << endl;
}

static void emit_move(char *dest_reg, char *source_reg, ostream& s)
{ 
  if(regEq(dest_reg, source_reg)) {
    if(cgen_debug) {
      cerr << "     Omitting move from "
          << source_reg << "  to  " << dest_reg << endl;
      s << "#";
    }
    else
      return;
  }
  s << MOVE << dest_reg << " " << source_reg << endl; }

static void emit_neg(char *dest, char *src1, ostream& s)
{ s << NEG << dest << " " << src1 << endl; }

static void emit_add(char *dest, char *src1, char *src2, ostream& s)
{ s << ADD << dest << " " << src1 << " " << src2 << endl; }

static void emit_addu(char *dest, char *src1, char *src2, ostream& s)
{ s << ADDU << dest << " " << src1 << " " << src2 << endl; }

static void emit_addiu(char *dest, char *src1, int imm, ostream& s)
{ s << ADDIU << dest << " " << src1 << " " << imm << endl; }

static void emit_binop(char *op, char *dest, char *src1, char *src2,
                        ostream& s)
{ s << op << dest << " " << src1 << " " << src2 << endl; }

static void emit_div(char *dest, char *src1, char *src2, ostream& s)
{ s << DIV << dest << " " << src1 << " " << src2 << endl; }

static void emit_mul(char *dest, char *src1, char *src2, ostream& s)
{ s << MUL << dest << " " << src1 << " " << src2 << endl; }

static void emit_sub(char *dest, char *src1, char *src2, ostream& s)
{ s << SUB << dest << " " << src1 << " " << src2 << endl; }

static void emit_sll(char *dest, char *src1, int num, ostream& s)
{ s << SLL << dest << " " << src1 << " " << num << endl; }

static void emit_jalr(char *dest, ostream& s)
{ s << JALR << "\t" << dest << endl; }

static void emit_jal(char *address,ostream &s)
{ s << JAL << address << endl; }

static void emit_return(ostream& s)
{ s << RET << endl; }

static void emit_copy(ostream& s)
{ s << JAL << "Object.copy" << endl; } 

static void emit_gc_assign(ostream& s)
{ s << JAL << "_GenGC_Assign" << endl; }

static void emit_equality_test(ostream& s)
{ s << JAL << "equality_test" << endl; }

static void emit_case_abort(ostream& s) 
{ s << JAL << "_case_abort" << endl; }

static void emit_case_abort2(ostream& s)
{ s << JAL << "_case_abort2" << endl; }

static void emit_dispatch_abort(ostream& s)
{ s << JAL << "_dispatch abort" << endl; }

static void emit_disptable_ref(Symbol sym, ostream& s)
{  s << sym << DISPTAB_SUFFIX; }

static void emit_init_ref(Symbol sym, ostream& s)
{ s << sym << CLASSINIT_SUFFIX; }

static void emit_init(Symbol classname, ostream& s)
{
  s << JAL;
  emit_init_ref(classname, s);
  s << endl;
}

static void emit_label_ref(int l, ostream &s)
{ s << "label" << l; }

static void emit_protobj_ref(Symbol sym, ostream& s)
{ s << sym << PROTOBJ_SUFFIX; }

static void emit_method_ref(Symbol classname, Symbol methodname, ostream& s)
{ s << classname << METHOD_SEP << methodname; }

static void emit_label_def(int l, ostream &s)
{
  emit_label_ref(l,s);
  s << ":" << endl;
}

static void emit_beqz(char *source, int label, ostream &s)
{
  s << BEQZ << source << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_beq(char *src1, char *src2, int label, ostream &s)
{
  s << BEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bne(char *src1, char *src2, int label, ostream &s)
{
  s << BNE << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bleq(char *src1, char *src2, int label, ostream &s)
{
  s << BLEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blt(char *src1, char *src2, int label, ostream &s)
{
  s << BLT << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blti(char *src1, int imm, int label, ostream &s)
{
  s << BLT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bgti(char *src1, int imm, int label, ostream &s)
{
  s << BGT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_branch(int l, ostream& s)
{
  s << BRANCH;
  emit_label_ref(l,s);
  s << endl;
}

//
// Push a register on the stack. The stack grows towards smaller addresses.
//
static void emit_push(char *reg, ostream& str)
{
  emit_store(reg,0,SP,str);
  emit_addiu(SP,SP,-4,str);
}

//
// Fetch the integer value in an Int object.
// Emits code to fetch the integer value of the Integer object pointed
// to by register source into the register dest
//
static void emit_fetch_int(char *dest, char *source, ostream& s)
{ emit_load(dest, DEFAULT_OBJFIELDS, source, s); }

//
// Emits code to store the integer value contained in register source
// into the Integer object pointed to by dest.
//
static void emit_store_int(char *source, char *dest, ostream& s)
{ emit_store(source, DEFAULT_OBJFIELDS, dest, s); }


static void emit_gc_check(char *source, ostream &s)
{
  if (source != (char*)A1) emit_move(A1, source, s);
  s << JAL << "_gc_check" << endl;
}

//for the callee-side in the function call
//the caller has completed the argument placing,so the callee-side just 
//alloc the space to store the temp
static void function_prologue(CgenEnvironmentP env, ostream& s) {
  int num_temps = env->get_num_temps();
  int reg_temps = env->get_register_temps();
  int stk_temps = env->get_stack_temps();

  emit_addiu(SP,SP,-(3 + num_temps) * WORD_SIZE,s);
  emit_store(FP, 3 + num_temps, SP, s);
  emit_store(SELF, 2 + num_temps, SP, s);
  emit_store(RA, 1 + num_temps, SP, s);
  emit_addiu(FP, SP, 4, s);
  emit_move(SELF, ACC, s);

  for(int i = 0; i < reg_temps; ++i) {
    emit_store(regNames[i], num_temps - i - 1, FP, s);
  }

  if(cgen_Memmgr != GC_NOGC) {
    for(int i = 0; i < stk_temps; ++i) {
        emit_store(ZERO, i, FP, s);
    }
  }

  if(cgen_Memmgr_Debug == GC_DEBUG) {
    emit_gc_check(SELF,s);
  }
}


//exit the function
static void function_epilogue(CgenEnvironmentP env, int num_formals, ostream& s) {
  int num_temps = env->get_num_temps();

  if(cgen_Memmgr_Debug == GC_DEBUG) {
    emit_gc_check(ACC,s);
  }

  int reg_temps = env->get_register_temps();
  for(int i = 0; i < reg_temps; ++i) {
    emit_load(regNames[i], num_temps - i - 1, FP, s);
  }

  emit_load(RA, 1 + num_temps, SP, s);
  emit_load(SELF, 2 + num_temps, SP, s);
  emit_load(FP, 3 + num_temps, SP, s);

  emit_addiu(SP, SP, (3 + num_temps + num_formals) * WORD_SIZE, s);
  emit_return(s);
}



//Var binding
VarBinding::VarBinding(int i) : offset(i) { }
MethodBinding::MethodBinding(Symbol mn, Symbol cn) :
  method_name(mn),
  class_name(cn)
  { }

void MethodBinding::code_ref(ostream &s) {
  s << WORD;
  emit_method_ref(class_name, method_name, s);
  s << endl;
}

AttributeBinding::AttributeBinding(int i) : VarBinding(i) { }

Register AttributeBinding::code_ref(char *optional_dest, ostream& s) {
  if(cgen_debug) {
    cerr << "Attribute load from offset " << offset << endl;
  }
  emit_load(optional_dest, offset + DEFAULT_OBJFIELDS, SELF, s);
}

void AttributeBinding::code_update(char *dest, ostream& s) {
  if(cgen_debug) {
    cerr << "Attribute load from offset " << offset << endl;
  }
  emit_load(dest, offset + DEFAULT_OBJFIELDS, SELF, s);

  if(cgen_Memmgr_Debug == GC_DEBUG) {
    emit_gc_check(dest, s);
  }

  if(cgen_Memmgr == GC_GENGC) {
    emit_addiu(A1, SELF, WORD_SIZE * (DEFAULT_OBJFIELDS + offset), s);
    emit_gc_assign(s);
  }
}

SelfBinding::SelfBinding() : VarBinding(0) { }

Register SelfBinding::code_ref(char *dest, ostream& s) {
  emit_move(dest, SELF, s);
  return dest;
}

void SelfBinding::code_update(char *, ostream&) {
  cerr << "can't assign to self";
  exit(1);
}

LocalBinding::LocalBinding(int i, CgenEnvironmentP _env) : 
    VarBinding(i) {
      env = _env;
    }

Register LocalBinding::code_ref(char *dest, ostream &s) {
  Register reg = env->get_register(offset);

  if(reg != NULL) {
    cerr << " Local read from register " << reg;
    return reg;
  }
  else {
    cerr << " Local read from offset from FP " << offset;
    emit_load(dest, offset, FP, s);
    return dest;
  }
}

void LocalBinding::code_ref_force_dest(char *dest, ostream& s) {
  Register result = code_ref(dest, s);
  emit_move(dest, result, s);
}

void LocalBinding::code_update(char *source, ostream& s) {
  Register reg = env->get_register(offset);
  if(reg != NULL) {
    if(cgen_debug) {
      cerr << "   Local store to register " << reg << endl;
    }
    emit_move(reg, source, s);
  }
  else {
    if(cgen_debug) {
      cerr << "   Local store to FP offset " << offset << endl;
      emit_store(source, offset, FP, s);
    }
  }
}

Register LocalBinding::get_register() {
  return env->get_register(offset);
}

///////////////////////////////////////////////////////////////////////////////
//
// coding strings, ints, and booleans
//
// Cool has three kinds of constants: strings, ints, and booleans.
// This section defines code generation for each type.
//
// All string constants are listed in the global "stringtable" and have
// type StringEntry.  StringEntry methods are defined both for String
// constant definitions and references.
//
// All integer constants are listed in the global "inttable" and have
// type IntEntry.  IntEntry methods are defined for Int
// constant definitions and references.
//
// Since there are only two Bool values, there is no need for a table.
// The two booleans are represented by instances of the class BoolConst,
// which defines the definition and reference methods for Bools.
//
///////////////////////////////////////////////////////////////////////////////

//
// Strings
//
void StringEntry::code_ref(ostream& s)
{
  s << STRCONST_PREFIX << index;
}

//
// Emit code for a constant String.
// You should fill in the code naming the dispatch table.
//

void StringEntry::code_def(ostream& s, int stringclasstag)
{
  IntEntryP lensym = inttable.add_int(len);

  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s  << LABEL                                             // label
      << WORD << stringclasstag << endl                                 // tag
      << WORD << (DEFAULT_OBJFIELDS + STRING_SLOTS + (len+4)/4) << endl // size
      << WORD;
      emit_disptable_ref(idtable.lookup_string(STRINGNAME), s);
      s << endl;                                              // dispatch table
      s << WORD;  lensym->code_ref(s);  s << endl;            // string length
  emit_string_constant(s,str);                                // ascii string
  s << ALIGN;                                                 // align to word
}

//
// StrTable::code_string
// Generate a string object definition for every string constant in the 
// stringtable.
//
void StrTable::code_string_table(ostream& s, int stringclasstag)
{  
  for (List<StringEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,stringclasstag);
}

//
// Ints
//
void IntEntry::code_ref(ostream &s)
{
  s << INTCONST_PREFIX << index;
}

//
// Emit code for a constant Integer.
// You should fill in the code naming the dispatch table.
//

void IntEntry::code_def(ostream &s, int intclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                // label
      << WORD << intclasstag << endl                      // class tag
      << WORD << (DEFAULT_OBJFIELDS + INT_SLOTS) << endl  // object size
      << WORD; 
      emit_disptable_ref(idtable.lookup_string(INTNAME), s);
      s << endl;                                          // dispatch table
      s << WORD << str << endl;                           // integer value
}


//
// IntTable::code_string_table
// Generate an Int object definition for every Int constant in the
// inttable.
//
void IntTable::code_string_table(ostream &s, int intclasstag)
{
  for (List<IntEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,intclasstag);
}


//
// Bools
//
BoolConst::BoolConst(int i) : val(i) { assert(i == 0 || i == 1); }

void BoolConst::code_ref(ostream& s) const
{
  s << BOOLCONST_PREFIX << val;
}
  
//
// Emit code for a constant Bool.
// You should fill in the code naming the dispatch table.
//

void BoolConst::code_def(ostream& s, int boolclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                  // label
      << WORD << boolclasstag << endl                       // class tag
      << WORD << (DEFAULT_OBJFIELDS + BOOL_SLOTS) << endl   // object size
      << WORD;
      emit_disptable_ref(idtable.lookup_string(BOOLNAME), s);
      s << endl;                                            // dispatch table
      s << WORD << val << endl;                             // value (0 or 1)
}

//////////////////////////////////////////////////////////////////////////////
//
//  CgenClassTable methods
//
//////////////////////////////////////////////////////////////////////////////

//***************************************************
//
//  Emit code to start the .data segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_data()
{
  Symbol main    = idtable.lookup_string(MAINNAME);
  Symbol string  = idtable.lookup_string(STRINGNAME);
  Symbol integer = idtable.lookup_string(INTNAME);
  Symbol boolc   = idtable.lookup_string(BOOLNAME);

  str << "\t.data\n" << ALIGN;
  //
  // The following global names must be defined first.
  //
  str << GLOBAL << CLASSNAMETAB << endl;
  str << GLOBAL; emit_protobj_ref(main,str);    str << endl;
  str << GLOBAL; emit_protobj_ref(integer,str); str << endl;
  str << GLOBAL; emit_protobj_ref(string,str);  str << endl;
  str << GLOBAL; falsebool.code_ref(str);  str << endl;
  str << GLOBAL; truebool.code_ref(str);   str << endl;
  str << GLOBAL << INTTAG << endl;
  str << GLOBAL << BOOLTAG << endl;
  str << GLOBAL << STRINGTAG << endl;

  //
  // We also need to know the tag of the Int, String, and Bool classes
  // during code generation.
  //
  str << INTTAG << LABEL
      << WORD << class_to_tag_table.lookup(integer)<< endl;
  str << BOOLTAG << LABEL 
      << WORD << class_to_tag_table.lookup(boolc) << endl;
  str << STRINGTAG << LABEL 
      << WORD << class_to_tag_table.lookup(string) << endl;    
}


//***************************************************
//
//  Emit code to start the .text segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_text()
{
  str << GLOBAL << HEAP_START << endl
      << HEAP_START << LABEL 
      << WORD << 0 << endl
      << "\t.text" << endl
      << GLOBAL;
  emit_init_ref(idtable.add_string("Main"), str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Int"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("String"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Bool"),str);
  str << endl << GLOBAL;
  emit_method_ref(idtable.add_string("Main"), idtable.add_string("main"), str);
  str << endl;
}

void CgenClassTable::code_bools()
{
  int boolcclasstag = *class_to_tag_table.lookup(idtable.add_string(BOOLNAME));
  falsebool.code_def(str,boolclasstag);
  truebool.code_def(str,boolclasstag);
}

void CgenClassTable::code_select_gc()
{
  //
  // Generate GC choice constants (pointers to GC functions)
  //
  str << GLOBAL << "_MemMgr_INITIALIZER" << endl;
  str << "_MemMgr_INITIALIZER:" << endl;
  str << WORD << gc_init_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_COLLECTOR" << endl;
  str << "_MemMgr_COLLECTOR:" << endl;
  str << WORD << gc_collect_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_TEST" << endl;
  str << "_MemMgr_TEST:" << endl;
  str << WORD << (cgen_Memmgr_Test == GC_TEST) << endl;
}


//********************************************************
//
// Emit code to reserve space for and initialize all of
// the constants.  Class names should have been added to
// the string table (in the supplied code, is is done
// during the construction of the inheritance graph), and
// code for emitting string constants as a side effect adds
// the string's length to the integer table.  The constants
// are emmitted by running through the stringtable and inttable
// and producing code for each entry.
//
//********************************************************

void CgenClassTable::code_constants()
{
  //
  // Add constants that are required by the code generator.
  //
  stringtable.add_string("");
  inttable.add_string("0");

  int stringclasstag = *class_to_tag_table.lookup(idtable.lookup_string(STRINGNAME));
  int intclasstag = *class_to_tag_table.lookup(idtable.lookup_string(INTNAME));

  stringtable.code_string_table(str,stringclasstag);
  inttable.code_string_table(str,intclasstag);
  code_bools();
}

//class table is a mapping
//class tag -> class name
void CgenClassTable::code_class_table() {
  str << CLASSNAMETAB << LABEL;
  for(int i = 0; i < num_classes; ++i) {
    StringEntry* c = tag_to_class_table.lookup(i);
    str << WORD;
    c->code_ref(str);
    str << endl;
  }
}

//object table is a mapping
//class tag -> object
void CgenClassTable::code_object_table() {
  str << CLASSOBJTAB << LABEL;
  for(int i = 0; i < num_classes; ++i) {
    Symbol s = tag_to_class_table.lookup(i);
    str << WORD;
    emit_protobj_ref(s, str);
    str << endl;
    str << WORD;
    emit_init_ref(s, str);
    str << endl;
  }
}

//CgenClassTable construct actually does all work of
//code generation,such as build inheritance graph
//compute the layout information, and codes each class

CgenClassTable::CgenClassTable(Classes classes, ostream& s) : nds(NULL) , str(s)
{
  if(cgen_debug) {
    str << "Build the CgenClassTable" << endl;
  }

  num_classes = 0;
  
  class_to_tag_table.enterscope();
  class_to_max_child_table.enterscope();
  tag_to_class_table.enterscope();
  table_of_method_tables.enterscope();

  stringclasstag = 0 /* Change to your String class tag here */;
  intclasstag =    0 /* Change to your Int class tag here */;
  boolclasstag =   0 /* Change to your Bool class tag here */;

  enterscope();
  install_basic_classes();
  install_classes(classes);
  build_inheritance_tree();
  root()->init(0,
                *(new SymbolTable<Symbol,int>),
                *(new SymbolTable<int,MethodBinding>),
                0,
                *(new SymbolTable<Symbol,VarBinding>),
                *(new SymbolTable<int,Entry>));

  code();
  exitscope();
}

void CgenClassTable::install_basic_classes()
{

// The tree package uses these globals to annotate the classes built below.
  //curr_lineno  = 0;
  node_lineno = 0;
  Symbol filename = stringtable.add_string("<basic class>");

//
// A few special class names are installed in the lookup table but not
// the class list.  Thus, these classes exist, but are not part of the
// inheritance hierarchy.
// No_class serves as the parent of Object and the other special classes.
// SELF_TYPE is the self class; it cannot be redefined or inherited.
// prim_slot is a class known to the code generator.
//
  addid(No_class,
	new CgenNode(class_(No_class,No_class,nil_Features(),filename),
			    Basic,this));
  addid(SELF_TYPE,
	new CgenNode(class_(SELF_TYPE,No_class,nil_Features(),filename),
			    Basic,this));
  addid(prim_slot,
	new CgenNode(class_(prim_slot,No_class,nil_Features(),filename),
			    Basic,this));

// 
// The Object class has no parent class. Its methods are
//        cool_abort() : Object    aborts the program
//        type_name() : Str        returns a string representation of class name
//        copy() : SELF_TYPE       returns a copy of the object
//
// There is no need for method bodies in the basic classes---these
// are already built in to the runtime system.
//
  install_class(
   new CgenNode(
    class_(Object, 
	   No_class,
	   append_Features(
           append_Features(
           single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
           single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
           single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	   filename),
    Basic,this));

// 
// The IO class inherits from Object. Its methods are
//        out_string(Str) : SELF_TYPE          writes a string to the output
//        out_int(Int) : SELF_TYPE               "    an int    "  "     "
//        in_string() : Str                    reads a string from the input
//        in_int() : Int                         "   an int     "  "     "
//
   install_class(
    new CgenNode(
     class_(IO, 
            Object,
            append_Features(
            append_Features(
            append_Features(
            single_Features(method(out_string, single_Formals(formal(arg, Str)),
                        SELF_TYPE, no_expr())),
            single_Features(method(out_int, single_Formals(formal(arg, Int)),
                        SELF_TYPE, no_expr()))),
            single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
            single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	   filename),	    
    Basic,this));

//
// The Int class has no methods and only a single attribute, the
// "val" for the integer. 
//
   install_class(
    new CgenNode(
     class_(Int, 
	    Object,
            single_Features(attr(val, prim_slot, no_expr())),
	    filename),
     Basic,this));

//
// Bool also has only the "val" slot.
//
    install_class(
     new CgenNode(
      class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename),
      Basic,this));

//
// The class Str has a number of slots and operations:
//       val                                  ???
//       str_field                            the string itself
//       length() : Int                       length of the string
//       concat(arg: Str) : Str               string concatenation
//       substr(arg: Int, arg2: Int): Str     substring
//       
   install_class(
    new CgenNode(
      class_(Str, 
	     Object,
             append_Features(
             append_Features(
             append_Features(
             append_Features(
             single_Features(attr(val, Int, no_expr())),
            single_Features(attr(str_field, prim_slot, no_expr()))),
            single_Features(method(length, nil_Formals(), Int, no_expr()))),
            single_Features(method(concat, 
				   single_Formals(formal(arg, Str)),
				   Str, 
				   no_expr()))),
	    single_Features(method(substr, 
				   append_Formals(single_Formals(formal(arg, Int)), 
						  single_Formals(formal(arg2, Int))),
				   Str, 
				   no_expr()))),
	     filename),
        Basic,this));

}

// CgenClassTable::install_class
// CgenClassTable::install_classes
//
// install_classes enters a list of classes in the symbol table.
//
void CgenClassTable::install_class(CgenNodeP nd)
{
  Symbol name = nd->get_name();

  if (probe(name))
    {
      return;
    }

  // The class name is legal, so add it to the list of classes
  // and the symbol table.
  nds = new List<CgenNode>(nd,nds);
  addid(name,nd);
}

void CgenClassTable::install_classes(Classes cs)
{
  for(int i = cs->first(); cs->more(i); i = cs->next(i))
    install_class(new CgenNode(cs->nth(i),NotBasic,this));
}

//
// CgenClassTable::build_inheritance_tree
//
void CgenClassTable::build_inheritance_tree()
{
  for(List<CgenNode> *l = nds; l; l = l->tl())
      set_relations(l->hd());
}

//
// CgenClassTable::set_relations
//
// Takes a CgenNode and locates its, and its parent's, inheritance nodes
// via the class table.  Parent and child pointers are added as appropriate.
//
void CgenClassTable::set_relations(CgenNodeP nd)
{
  CgenNode *parent_node = probe(nd->get_parent());
  nd->set_parentnd(parent_node);
  parent_node->add_child(nd);
}

void CgenNode::add_child(CgenNodeP n)
{
  children = new List<CgenNode>(n,children);
}

void CgenNode::set_parentnd(CgenNodeP p)
{
  assert(parentnd == NULL);
  assert(p != NULL);
  parentnd = p;
}

int CgenClassTable::assign_tag(Symbol s) {
  assert(! class_to_tag_table.lookup(s));
  assert(! tag_to_class_table.lookup(num_classes));
  
  class_to_tag_table.addid(s, new int(num_classes));
  tag_to_class_table.addid(num_classes, stringtable.add_string(s->get_string()));

  return num_classes++;
}

void CgenClassTable::set_max_child(Symbol s, int num) {
  assert(! class_to_max_child_table.lookup(s));
  class_to_max_child_table.addid(s, new int(num));
}

int CgenClassTable::last_tag() {
  assert(num_classes != 0);
  return num_classes - 1;
}

void CgenClassTable::code()
{
  if (cgen_debug) cout << "coding global data" << endl;
  code_global_data();

  if (cgen_debug) cout << "choosing gc" << endl;
  code_select_gc();

  if (cgen_debug) cout << "coding constants" << endl;
  code_constants();

  if(cgen_debug) cout << "coding class table" << endl;
  code_class_table();

  if(cgen_debug) cout << "coding object table" << endl;
  code_object_table();

  if(cgen_debug) cout << "coding dispatch table" << endl;
  root()->code_dispatch_table(str);

  assert(inttable.lookup_string("0"));
  assert(stringtable.lookup_string(""));
  assert(idtable.lookup_string(INTNAME));
  assert(idtable.lookup_string(BOOLNAME));
  assert(idtable.lookup_string(STRINGNAME));

  if(cgen_debug) cout << "coding prototype table" << endl;
  root()->code_prototype_object(str);

  if (cgen_debug) cout << "coding global text" << endl;
  code_global_text();

  CgenEnvTopLevelP env = new CgenEnvTopLevel(&class_to_tag_table,
                          &class_to_max_child_table,
                          &table_of_method_tables,
                          num_classes);

  if(cgen_debug) cout << "coding init methods" << endl;
  root()->code_init(str, env);

  if(cgen_debug) cout << "coding methods" << endl;
  root()->code_methods(str, env);
}


CgenNodeP CgenClassTable::root()
{
   return probe(Object);
}


///////////////////////////////////////////////////////////////////////
//
// CgenNode methods
//
///////////////////////////////////////////////////////////////////////

CgenNode::CgenNode(Class_ nd, Basicness bstatus, CgenClassTableP ct) :
   class__class((const class__class &) *nd),
   parentnd(NULL),
   children(NULL),
   basic_status(bstatus)
{ 
   stringtable.add_string(name->get_string());          // Add class name to string table
}

CgenNode::CgenNode(Class_ nd, Basicness bstatus, CgenClassTableP ct) :
          class__class((const class__class&) *nd),
          parentnd(NULL),
          children(NULL),
          basic_status(bstatus),
          num_methods(0),
          method_name_to_offset_table(),
          method_offset_to_binding_table(),
          first_attribute(0),
          num_attributes(0),
          var_binding_table(),
          attribute_proto_table(),
          class_table(ct)
{ }

void CgenNode::init(int nm,
        SymbolTable<Symbol,int> mntot,
        SymbolTable<int,MethodBinding> motbt,
        int fa,
        SymbolTable<Symbol,VarBinding> vbt,
        SymbolTable<int,Entry> apt) {
  num_methods = nm;
  method_name_to_offset_table = mntot;
  method_offset_to_binding_table = motbt;
  first_attribute = fa;
  var_binding_table = vbt;
  attribute_proto_table = apt;

  if(cgen_debug) cerr << "build cgen node for " << name << endl;
  class_tag = class_table->assign_tag(name);

  method_offset_to_binding_table.enterscope();
  method_name_to_offset_table.enterscope();
  var_binding_table.enterscope();
  method_name_to_numtemps_table.enterscope();
  attribute_proto_table.enterscope();

  attribute_init_table = *(new SymbolTable<int,attr_class>);
  attribute_init_table.enterscope();

  for(int i = features->first(); features->more(i); i = features->next(i)) {
    features->nth(i)->layout_feature(this);
  }

  int next_atttibutes = first_attribute + num_attributes;
  List<CgenNode> *child = children;
  for(; child; child = child->tl()) {
    child->hd()->init(num_methods,
                      method_name_to_offset_table,
                      method_offset_to_binding_table,
                      next_atttibutes,
                      var_binding_table,
                      attribute_proto_table);
    children = new List<CgenNode>(child->hd(),child);
  }

  max_child = class_table->last_tag();
  class_table->set_max_child(name, max_child);

  if(cgen_debug) cerr << "For class" << name << "tag is "
  << class_tag << "and max child is " << max_child << endl;

  class_table->add_to_method_table(name,&method_name_to_offset_table);
}

void CgenNode::layout_method(Symbol mname, int num_temps) {
  int offset;

  if(method_name_to_offset_table.lookup(mname))
    offset = *(method_name_to_offset_table.lookup(mname));
  else
    offset = num_methods++;

  if(cgen_debug) cerr << " Method: " << mname << " Class: " << name
  << " dispatch table offset" << offset;
  method_name_to_offset_table.addid(mname, new int(offset));
  method_offset_to_binding_table.addid(offset, new MethodBinding(mname,name));
  method_name_to_numtemps_table.addid(name, new int(num_temps));
}

void CgenNode::layout_attributes(Symbol aname, attr_class *a, int init) {
  int offset = first_attribute + num_attributes++;
  var_binding_table.addid(aname, new AttributeBinding(offset));
  if(init) attribute_init_table.addid(offset, a);
  attribute_proto_table.addid(offset, a->get_type_decl());
  if(cgen_debug) cerr << " Attribute: " << aname << " Class : " << name
   << " offset: " << offset << "initialization: " << (init? "yes" : "no") << endl;
}

void CgenNode::code_disptable_ref(ostream& str) {
  emit_disptable_ref(name,str);
}

void CgenNode::code_dispatch_table(ostream& str) {
  code_disptable_ref(str);
  str << LABEL;
  for(int i = 0; i < num_methods; ++i) {
    method_offset_to_binding_table.lookup(i)->code_ref(str);
  }

  for(List<CgenNode> *l = children; l; l = l->tl()) {
    l->hd()->code_dispatch_table(str);
  }
}

void CgenNode::code_protoobj_ref(ostream& str) {
  emit_protobj_ref(name, str);
}

void CgenNode::code_prototype_object(ostream& str) {
  //for the garbage collector
  str << WORD << "-1" << endl;

  code_protoobj_ref(str);

  //The prototype object must have slots for all attributes of the class
  //including inherited attributes. The number of attributes is the index
  //of the first attributes plus the number of attributes
  int total_attributes = first_attribute + num_attributes;
  str << LABEL
      << WORD << class_tag << endl
      << WORD << (total_attributes + DEFAULT_OBJFIELDS) << endl
      << WORD;
  code_disptable_ref(str);
  str << endl;

  for(int i = 0; i < total_attributes; ++i) {
    str << WORD;

    Symbol type_decl = attribute_proto_table.lookup(i);

    if(idtable.lookup_string(INTNAME) == type_decl) {
      inttable.lookup_string("0")->code_ref(str);
    }
    else if(idtable.lookup_string(BOOLNAME) == type_decl) {
      falsebool.code_ref(str);
    }
    else if(idtable.lookup_string(STRINGNAME) == type_decl) {
      stringtable.lookup_string("")->code_ref(str);
    }
    else {
      str << EMPTYSLOT;
      str << endl;
    }
  }

  for(List<CgenNode> *l = children; l; l = l->tl()) {
    l->hd()->code_prototype_object(str);
  }
}

void CgenNode::code_methods(ostream &str, CgenEnvTopLevelP e) {
  if(basic_status == NotBasic){
    CgenEnvClassLevelP env = new CgenEnvClassLevel(e, 
                                                  &method_name_to_offset_table,
                                                  var_binding_table,
                                                  name,
                                                  filename);
    for(int i = features->first(); features->more(i); i = features->next(i)) {
      features->nth(i)->code_method(str,env);
    }
  }

  for(List<CgenNode> *l = children; l; l = l->tl()) {
    l->hd()->code_methods(str,e);
  }
}

//This is called by the `new` after the prototype has been copied
//The code must call the init function of the parent class in order
//to initialize inherited attributes . It must also overwrite the
//dispatch table pointer with the real dispatch table and excute the
//slot initializations
void CgenNode::code_init_ref(ostream& str) {
  emit_init_ref(name, str);
}

//The init method for a class is a bit tricky to get right
//First, temporaries may be needed to evaluate the initialization
//of attributes; the number of temporaries needed is the max
//over the number of temps needed to calculate any attribute of
//the class. Second, the initialization method of the parent class
//must be invoked prior to initializing any of the attributes
//of the class
void CgenNode::code_init(ostream& str, CgenEnvTopLevelP e) {
  int i;

  if(cgen_debug) cerr << "Coding init method of class " << name
                  << " first attribute: " << first_attribute
                  << " # attributes: " << num_attributes << endl;
  code_init_ref(str);
  str << LABEL;

  CalcTempP n = new CalcTemp();
  for(i = first_attribute; i < num_attributes; ++i)
    if(attribute_init_table.lookup(i))
      attribute_init_table.lookup(i)->calc_temps(n);
  int num_temps = n->get_max();
  if(cgen_debug) cerr << "Number of temporaries = " << num_temps << endl;

  CgenEnvClassLevelP classenv = new CgenEnvClassLevel(e,
                                                      &method_name_to_offset_table,
                                                      var_binding_table,
                                                      name,
                                                      filename);

  CgenEnvironmentP env = new CgenEnvironment(classenv,nil_Formals(),num_temps);

  function_prologue(env,str);
  if(this != class_table->root()) //root has no parent
    emit_init(parent,str);
  
  for(i = first_attribute; i < first_attribute + num_attributes; ++i)
    if(attribute_init_table.lookup(i))
        attribute_init_table.lookup(i)->code_init(str,env);
  
  emit_move(ACC,SELF,str);
  function_epilogue(env,0,str);

  for(List<CgenNode> *l = children; l; l = l->tl()) {
    l->hd()->code_init(str, e);
  }
}

//next_label is a member of all CgenEnvironments. It is declared static
//(shared by all instances) to guarantee that labels are unique.
//
int CgenEnvironment::next_label = 0;

CgenEnvironment::CgenEnvironment(CgenEnvClassLevelP env,
                                Formals formals,
                                int num_temporaries) :
                                CgenEnvClassLevel(*env),
                                num_temps(num_temporaries) {

    next_temp_location = -1 * get_register_temps();

    next_formal = num_temporaries + 2 + formals->len();

    for(int i = formals->first(); formals->more(i); i = formals->next(i)) {
      add_formal(formals->nth(i)->get_name());
    }

    var_binding_table.addid(self, new SelfBinding());
}

int CgenEnvironment::lookup_tag(Symbol sym) {
  int *tag = class_to_tag_table->lookup(sym);
  assert(tag);
  return *tag;
}

int CgenEnvironment::lookup_child_tag(Symbol sym) {
  int *tag = class_to_max_child_tag_table->lookup(sym);
  assert(tag);
  return *tag;
}

int CgenEnvironment::lookup_method(Symbol classname, Symbol methodname) {
  SymbolTable<Symbol,int> *table = 
    (classname == SELF_TYPE) ? method_name_to_offset_table :
                               table_of_method_tables->lookup(classname);
  assert(table);
  int *offset = table->lookup(methodname);
  assert(offset);
  return *offset;
}

VarBinding *CgenEnvironment::lookup_variable(Symbol sym) {
  if(cgen_debug) cerr << "  looking up binding for " << sym << endl;
  VarBinding *v = var_binding_table.lookup(sym);
  assert(v);
  return v;
}

//add a local variable
//A negative location is a register, a positive location is a stack slot
void CgenEnvironment::add_local(Symbol sym) {
  if(cgen_debug) cerr << "    Adding local binding " << sym << " " << next_temp_location << endl;
  var_binding_table.enterscope();

  assert(next_temp_location < get_stack_temps());
  var_binding_table.addid(sym,new LocalBinding(next_temp_location, this));
  next_temp_location++;
}

//Add a formal parameter. Formals are allocated at decreasing memory
//address in the formal area of the stack frame
void CgenEnvironment::add_formal(Symbol sym) {
  if(cgen_debug) cerr << "    Adding formal binding " << sym << " " << next_formal << endl;
  var_binding_table.enterscope();
  var_binding_table.addid(sym,new LocalBinding(next_formal--,this));
}

//remove a local variable, freeing up its slot in the stack frame
void CgenEnvironment::remove_local() {
  if(cgen_debug) cerr << "    Removing local binding." << endl;
  var_binding_table.exitscope();
  --next_temp_location;
}

int CgenEnvironment::label() {
  return next_label++;
}

Register CgenEnvironment::get_register(int offset) {
  assert(offset >= -1 * get_register_temps());
  if(!disable_reg_alloc && offset < 0) {
    int reg = -1 * offset;
    assert(reg < NUM_REGS);
    return regNames[reg];
  }
  else
    return NULL;
}

//return the unallocated register that we expect to allocate to the next variable
Register CgenEnvironment::get_next_register() {
  return get_register(next_temp_location);
}

//For the function prologues and epilogues: how many register/stack slots do we need

//Total number of temps
int CgenEnvironment::get_num_temps() {
  return num_temps;
}

//How many of these temps will be put in the registers
int CgenEnvironment::get_register_temps() {
  if(disable_reg_alloc)
    return 0;
  else if(NUM_REGS < num_temps)
    return NUM_REGS;
  else 
    return num_temps;
}

//How many of these temps will be put in the stack 
int CgenEnvironment::get_stack_temps() {
  int result = num_temps - get_register_temps();
  assert(result >= 0);
  return result;
}


//APS class method 

void method_class::layout_feature(CgenNodeP nd) {
  nd->layout_method(name,this->calc_temps());
}

void method_class::code_method(ostream& s, CgenEnvClassLevelP e) {
  if(cgen_debug) cerr << "  Coding method: " << name << endl;
  int num_temps = this->calc_temps();

  CgenEnvironmentP env = new CgenEnvironment(e, formals, num_temps);

  emit_method_ref(env->get_class_name(), name, s);
  s << LABEL;
  function_prologue(env, s);

  if(cgen_Memmgr_Debug == GC_DEBUG)
    for(int i = formals->len() - 1; i >= 0; --i) {
      emit_load(A1, 3 + num_temps + i, FP, s);
      emit_gc_check(A1, s);
    }
  
  Register result = expr->code(s, env, ACC);
  emit_move(ACC, result, s);
  function_epilogue(env, formals->len(), s);
}

void attr_class::layout_feature(CgenNodeP nd) {
  nd->layout_attributes(name, this, !(init->no_code()));
}

void attr_class::code_method(ostream& s, CgenEnvClassLevelP) 
{  }

void attr_class::code_init(ostream& str, CgenEnvironmentP env) {
  assign(name,init)->code(str,env,ACC);
}

static 
void code_force_dest(Expression e, ostream& s, CgenEnvironmentP env, Register target) {
  Register r = e->code(s, env, target);
  emit_move(target, r, s);
}
///////////////////////////////////////////
//code methods
//expr.code(ostream s, CgenEnvironmentP env, Register target)
//s:  the assembly code ouput
//env: the context
//target: if code has a choice, it will put the result here. There
//are no guarantees; so use code_force_dest insteadif you really care.
//On the other hand if you don't care, then ACC is a good choice.
//NB: target maybe a local variable. So dont write to it until after all other variable access.
//Return value: the name of the register holding the result.
//Possibly the same as the target
///////////////////////////////////////////

//For an assignment
//  1.the expression is evaluated
//  2.the value is saved at the variables locations
Register assign_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  VarBinding* lhs = env->lookup_variable(name);
  Register dest = lhs->get_register();
  if(dest == NULL) {
    dest = target;
  }

  Register rhs_value = expr->code(s, env, dest);
  lhs->code_update(rhs_value, s);
  return rhs_value;
}

Register static_dispatch_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  for(int i = actual->first(); actual->more(i); i = actual->next(i)) {
    Register r_actual = actual->nth(i)->code(s, env, target);
    emit_push(r_actual, s);
  }
  code_force_dest(expr, s, env, ACC);
  if(cgen_debug) cerr << "    Static dispatch to " << name << endl;
  int method_offset = env->lookup_method(type_name,name);

  int lab = env->label();
  emit_bne(ACC,ZERO,lab,s); //check for void
  emit_load_string(ACC,
        stringtable.lookup_string(env->get_file_name()->get_string()),s);
  emit_load_imm(T1, line_number, s);
  emit_dispatch_abort(s);
  emit_label_def(lab, s);
  emit_partial_load_address(T1, s);
  emit_disptable_ref(type_name, s);
  s << endl;
  emit_load(T1, method_offset, T1, s);
  emit_jalr(T1, s);
  return ACC;
}

Register cond_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  int else_label = env->label();
  int out_label = env->label();

  Register r_pred = pred->code(s, env, target);
  emit_fetch_int(T1, r_pred, s);
  emit_beqz(T1,else_label, s);
  code_force_dest(then_exp, s, env, ACC);
  emit_branch(out_label, s);
  emit_label_def(else_label, s);
  code_force_dest(else_exp, s, env, ACC);
  emit_label_def(out_label, s);
  return ACC;
}

Register loop_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  int loop_label = env->label();
  int out_lable = env->label();

  emit_label_def(loop_label, s);
  Register r_pred = pred->code(s, env, target);
  emit_fetch_int(T1, r_pred, s);
  emit_beq(T1, ZERO, out_lable, s);
  body->code(s, env, target);
  emit_branch(loop_label, s);
  emit_label_def(out_lable, s);
  emit_move(ACC, ZERO, s);
  return ACC;
}

//for the case expression, the cases are tested in the order
//of most specific to least specific. Since tags are assigned
// in depth-first order with the root being assigned 0,higher-numbered
//classes should be coded before lower-numbered classes.
Register typcase_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  int out_label = env->label();

  Register r_expr = expr->code(s, env, ACC);
  int lab = env->label();
  emit_bne(r_expr, ZERO, lab, s);
  emit_load_string(ACC,
                    stringtable.lookup_string(env->get_file_name()->get_string()),s);
  emit_load_imm(T1, line_number, s);
  emit_case_abort2(s);
  emit_label_def(lab,s);
  emit_load(T2, TAG_OFFSET, r_expr, s);

  for(int class_num = env->get_num_class(); class_num >= 0; --class_num) 
    for(int i = cases->first(); cases->more(i); i = cases->more(i)) {
      int tag = env->lookup_tag(cases->nth(i)->get_type_decl());
      if(class_num == tag) {
        if(cgen_debug) cerr << "    Coding case " << 
        cases->nth(i)->get_type_decl() << endl;
        cases->nth(i)->code(s, env, out_label, r_expr);
      }
    }
  emit_case_abort(s);
  emit_label_def(out_label, s);
  return ACC;
}

void branch_class::code(ostream& s, CgenEnvironmentP env, int outlabel, Register r_newvar) {
  int lab = env->label();
  int class_tag = env->lookup_tag(type_decl);
  int last_tag = env->lookup_child_tag(type_decl);
  emit_blti(T2, class_tag, lab, s);
  emit_bgti(T2, last_tag, lab, s);
  env->add_local(name);
  env->lookup_variable(name)->code_update(r_newvar, s);
  code_force_dest(expr, s, env, ACC);
  env->remove_local();
  emit_branch(outlabel, s);
  emit_label_def(lab, s);
}

Register block_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  Register r_lastvalue = ACC;
  for(int i = body->first(); body->more(i); i = body->next(i)) {
    r_lastvalue = body->nth(i)->code(s,env,target);
  }
  return r_lastvalue;
}

Register let_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  Register r_newvar = env->get_next_register();
  Register r_init = r_newvar;
  if(r_init == NULL) {
    r_init = ACC;
  }
  if(init->no_code()) {
    if(idtable.lookup_string(INTNAME) == type_decl) {
      emit_partial_load_address(r_init, s);
      inttable.lookup_string("0")->code_ref(s);
      s << endl;
    }
    else if(idtable.lookup_string(STRINGNAME) == type_decl) {
      emit_partial_load_address(r_init, s);
      stringtable.lookup_string("")->code_ref(s);
      s << endl;
    }
    else if(idtable.lookup_string(BOOLNAME) == type_decl) {
      emit_partial_load_address(r_init, s);
      falsebool.code_ref(s);
      s << endl;
    }
    else
    {
      r_init = ZERO;
    }
  }
  else
  {
    r_init = init->code(s, env, target);
  }

  env->add_local(identifier);
  VarBinding *newvar = env->lookup_variable(identifier);
  newvar->code_update(r_init, s);

  Register r_body = body->code(s, env, target);
  env->remove_local();
  return r_body;
}

//Helper for 'e1 op e2'
//
//The contents of the register that holds e1 could change when
//e2 is excuted, so we need to save the result of the first computation
//This function:
//1.evaluate e1
//2.allocates a new var
//3.puts the result of e1 in the new var
static void store_operand(Symbol temp_var, Expression e1, ostream& s, CgenEnvironmentP env) {
  Register dest = env->get_next_register();
  if(dest == 0)  {
    dest = ACC;
  }
  Register r_e1 = e1->code(s, env, dest);
  env->add_local(temp_var);
  env->lookup_variable(temp_var)->code_update(r_e1, s);
}

//Helper for plus_class::code and other integar binary ops
static Register binop_int_code(char *binop, Expression e1, Expression e2, 
                        ostream& s, CgenEnvironmentP env) {
  Symbol temp1 = idtable.add_string(TEMP1);
  store_operand(temp1, e1, s, env);

  code_force_dest(e2, s, env, ACC);
  emit_copy(s);
  
  Register r_temp1 = env->lookup_variable(temp1)->code_ref(T1, s);
  emit_fetch_int(T2, ACC, s);
  emit_fetch_int(T1, r_temp1, s);
  emit_binop(binop, T1, T1, T2, s);
  emit_store_int(T1, ACC, s);
  env->remove_local();
  return ACC;
}

Register plus_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  return binop_int_code(ADD, e1, e2, s, env);
}

Register sub_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  return binop_int_code(SUB, e1, e2, s, env);
}

Register mul_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  return binop_int_code(MUL, e1, e2, s, env);
}

Register divide_class::code(ostream& s , CgenEnvironmentP env, Register target) {
  return binop_int_code(DIV, e1, e2, s, env);
}

Register eq_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  int lab = env->label();
  Symbol temp1 = idtable.add_string(TEMP1);
  store_operand(temp1, e1, s, env);

  Register r_e2 = e2->code(s, env, target);
  Register r_e1 = env->lookup_variable(temp1)->code_ref(T1, s);
  emit_move(T1, r_e1, s);
  emit_move(T2, r_e2, s);
  emit_load_bool(ACC, truebool, s);
  emit_beq(T1, T2, lab, s);
  emit_load_bool(A1, falsebool, s);
  emit_equality_test(s);
  emit_label_def(lab, s);
  env->remove_local();
  return ACC;
}

Register leq_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  int lab_done = env->label();
  Symbol temp1 = idtable.add_string(TEMP1);
  store_operand(temp1, e1, s, env);  //temp  <-  e1

  Register r_e2 = e2->code(s, env, ACC);
  Register r_e1 = env->lookup_variable(temp1)->code_ref(T1, s);
  emit_fetch_int(T1, r_e1, s);
  emit_fetch_int(T2, r_e1, s);
  emit_load_bool(ACC, truebool, s);
  emit_bleq(T1, T2, lab_done, s);
  emit_load_bool(ACC, falsebool, s);
  emit_label_def(lab_done, s);
  env->remove_local();
  return ACC;
}

Register neg_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  code_force_dest(e1, s, env, target);
  emit_copy(s);
  emit_fetch_int(T1, ACC, s);
  emit_neg(T1, T1, s);
  emit_store_int(T1, ACC, s);
  return ACC;
}

Register comp_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  int lab = env->label();
  Register r_e1 = e1->code(s, env, ACC);
  emit_fetch_int(T1, r_e1, s);
  emit_load_bool(ACC, truebool, s);
  emit_beqz(T1, lab, s);
  emit_load_bool(ACC, falsebool, s);
  emit_label_def(lab, s);
  return ACC;
}

Register int_const_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  emit_load_int(target, inttable.lookup_string(token->get_string()), s);
}

Register string_const_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  emit_load_string(target, stringtable.lookup_string(token->get_string()), s);
}

Register bool_const_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  emit_load_bool(target, BoolConst(val), s);
}

Register new__class::code(ostream& s, CgenEnvironmentP env, Register target) {
  if(type_name == SELF_TYPE) {
    emit_load_address(T1, CLASSOBJTAB, s);
    emit_load(T2, TAG_OFFSET, SELF, s);
    emit_sll(T2, T2, LOG_WORD_SIZE, s);

    emit_addu(T1, T1, T2, s);                     //index into the table
    Symbol temp1 = idtable.add_string(TEMP1);
    env->add_local(temp1);
    env->lookup_variable(temp1)->code_update(T1, s);  //save $t1 before calling copy
    emit_load(ACC, 0, T1, s);                         //get self prototype object
    emit_copy(s);                                     //copy the prototype
    Register r_temp1 = 
    env->lookup_variable(temp1)->code_ref(T1, s);     //reload temp1
    emit_load(T1, 1, r_temp1, s);                     //address of init code
    env->remove_local();
    emit_jalr(T1, s);                                 //init the object
  }
  else
  {
    emit_partial_load_address(ACC, s);
    emit_protobj_ref(type_name, s);
    s << endl;
    emit_copy(s);
    emit_init(type_name, s);
  }
  return ACC;
}

Register object_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  return env->lookup_variable(name)->code_ref(target, s);
}

Register no_expr_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  cerr << "Reached code() of no_expr." << endl;
  exit(1);
}

Register isvoid_class::code(ostream& s, CgenEnvironmentP env, Register target) {
  int lab = env->label();

  Register r_e1 = e1->code(s, env, ACC);
  if(regEq(r_e1, target)) {
    emit_move(T1, r_e1, s);
    r_e1 = T1;
  }
  emit_load_bool(target, truebool, s);
  emit_beqz(r_e1, lab, s);
  emit_load_bool(target, falsebool, s);
  emit_label_def(lab, s);
  return target;
}


//Calculate the temporaries
CalcTempP attr_class::calc_temps(CalcTempP n) {
  return init->calc_temp(n);
}

int method_class::calc_temps() {
  return expr->calc_temp(new CalcTemp())->get_max();
}

CalcTempP int_const_class::calc_temp(CalcTempP n) {
  return n;
}

CalcTempP string_const_class::calc_temp(CalcTempP n) {
  return n;
}

CalcTempP bool_const_class::calc_temp(CalcTempP n) {
  return n;
}

CalcTempP plus_class::calc_temp(CalcTempP n) {
  return e2->calc_temp(e1->calc_temp(n)->add())->sub();
}

CalcTempP sub_class::calc_temp(CalcTempP n) {
  return e2->calc_temp(e1->calc_temp(n)->add())->sub();
}

CalcTempP mul_class::calc_temp(CalcTempP n) {
  return e2->calc_temp(e1->calc_temp(n)->add())->sub();
}

CalcTempP divide_class::calc_temp(CalcTempP n) {
  return e2->calc_temp(e1->calc_temp(n)->add())->sub();
}

CalcTempP neg_class::calc_temp(CalcTempP n) {
  return e1->calc_temp(n);
}

CalcTempP lt_class::calc_temp(CalcTempP n) {
  return e2->calc_temp(e1->calc_temp(n)->add())->sub();
}

CalcTempP leq_class::calc_temp(CalcTempP n) {
  return e2->calc_temp(e1->calc_temp(n)->add())->sub();
}

CalcTempP comp_class::calc_temp(CalcTempP n) {
  return e1->calc_temp(n);
}

CalcTempP object_class::calc_temp(CalcTempP n) {
  return n;
}

CalcTempP no_expr_class::calc_temp(CalcTempP n) {
  return n;
}

CalcTempP new__class::calc_temp(CalcTempP n) {
  if(type_name == SELF_TYPE) {
    return n->add()->sub();
  }
  else
  {
    return n;
  }
}

CalcTempP isvoid_class::calc_temp(CalcTempP n) {
  return e1->calc_temp(n);
}

CalcTempP eq_class::calc_temp(CalcTempP n) {
  return e2->calc_temp(e1->calc_temp(n)->add())->sub();
}

CalcTempP let_class::calc_temp(CalcTempP n) {
  return body->calc_temp(init->calc_temp(n)->add())->sub();
}

CalcTempP block_class::calc_temp(CalcTempP n) {
  for(int i = body->first(); body->more(i); i = body->next(i))
    n = body->nth(i)->calc_temp(n);
  return n;
}

CalcTempP assign_class::calc_temp(CalcTempP n) {
  return expr->calc_temp(n);
}

CalcTempP dispatch_class::calc_temp(CalcTempP n) {
  n = expr->calc_temp(n);
  for(int i = actual->first(); actual->more(i); i = actual->next(i))
    n = actual->nth(i)->calc_temp(n);
  return n;
}

CalcTempP static_dispatch_class::calc_temp(CalcTempP n) {
  n = expr->calc_temp(n);
  for(int i = actual->first(); actual->more(i); i = actual->next(i))
    n = actual->nth(i)->calc_temp(n);
  return n;
}

CalcTempP cond_class::calc_temp(CalcTempP n) {
  return else_exp->calc_temp(then_exp->calc_temp(pred->calc_temp(n)));
}

CalcTempP loop_class::calc_temp(CalcTempP n) {
  return body->calc_temp(pred->calc_temp(n));
}

CalcTempP typcase_class::calc_temp(CalcTempP n) {
  n = expr->calc_temp(n);
  for(int i = cases->first(); cases->more(i); i = cases->next(i))
    n = cases->nth(i)->calc_temps(n);
  return n;
}

CalcTempP branch_class::calc_temps(CalcTempP n) {
  return expr->calc_temp(n->add())->sub();
}

bool isAllocatedReg(Register reg) {
  for(int i = 0; i < NUM_REGS; ++i) {
    if(reg == regNames[i])
      return true;
  }
  return false;
}
