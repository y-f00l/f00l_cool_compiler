#include <assert.h>
#include <stdio.h>
#include "emit.h"
#include "cool-tree.h"
#include "symtab.h"

enum Basicness     {Basic, NotBasic};
#define TRUE 1
#define FALSE 0

class CgenClassTable;
typedef CgenClassTable *CgenClassTableP;

class CgenNode;
typedef CgenNode *CgenNodeP;

class CgenEnvTopLevel;
typedef CgenEnvTopLevel *CgenEnvTopLevelP;

class CgenEnvClassLevel;
typedef CgenEnvClassLevel *CgenEnvClassLevelP;

class CgenEnvironment;
typedef CgenEnvironment *CgenEnvironmentP;

class CgenClassTable : public SymbolTable<Symbol,CgenNode> {
private:
   int num_classes;
   List<CgenNode> *nds;
   SymbolTable<Symbol,int> class_to_tag_table;
   SymbolTable<Symbol,int> class_to_max_child_table;
   SymbolTable<int,StringEntry> tag_to_class_table;
   SymbolTable<Symbol,SymbolTable<Symbol,int>> table_of_method_tables;
   ostream& str;
   int stringclasstag;
   int intclasstag;
   int boolclasstag;


// The following methods emit code for
// constants and global declarations.

   void code_global_data();
   void code_global_text();
   void code_bools();
   void code_ints();
   void code_strings();
   void code_select_gc();
   void code_constants();
   void code_class_table();
   void code_object_table();
   void code_class_featrues();

// The following creates an inheritance graph from
// a list of classes.  The graph is implemented as
// a tree of `CgenNode', and class names are placed
// in the base class symbol table.

   void install_basic_classes();
   void install_class(CgenNodeP nd);
   void install_classes(Classes cs);
   void build_inheritance_tree();
   void set_relations(CgenNodeP nd);
public:
   CgenClassTable(Classes, ostream& str);
   int assign_tag(Symbol name);
   void set_max_child(Symbol name, int tag);
   void add_to_method_table(Symbol name, SymbolTable<Symbol,int>* method_table);
   int last_tag();
   void code();
   CgenNodeP root();
};

class VarBinding {
protected:
   int offset;          //the var location on the stack -> offeset from $fp;
public:
   VarBinding(int i);
   virtual ~VarBinding() {}

   virtual char *code_ref(char *optional_dest, ostream&) = 0;
   //code_ref  show the name of the register which hold the value of the var

   virtual void code_ref_force_dest(char *dest, ostream&) = 0;
   //code_ref_force_dest force the result move to dest

   virtual void code_update(char *source, ostream&) = 0;

   virtual Register get_register() = 0;
   //return the register which store the local var
   //if on the stack, return null
};

class AttributeBinding : public VarBinding {
public:
   AttributeBinding(int i);
   virtual ~AttributeBinding() {}

   char *code_ref(char *optional_dest, ostream&);
   void code_ref_force_dest(char *dest, ostream& os) {
      code_ref(dest, os);
   }
   void code_update(char *source, ostream&);

   Register get_register() {
      return NULL;
   }
};

class SelfBinding : public VarBinding {
public:
   SelfBinding();
   virtual ~SelfBinding();

   char *code_ref(char *optional_dest, ostream&);
   void code_ref_force_dest(char *dest, ostream& os) {
      code_ref(dest, os);
   }
   void code_update(char *source, ostream&);

   Register get_register() {
      return NULL;
   }
};

class LocalBinding : public VarBinding {
protected:
   CgenEnvironmentP env;
public:
   LocalBinding(int i, CgenEnvironmentP __env);
   virtual ~LocalBinding();

   char *code_ref(char *optional_dest, ostream&);
   void code_ref_force_dest(char *dest, ostream&);
   void code_update(char *source, ostream&);

   Register get_register();
};

class MethodBinding {
private:
   Symbol method_name;
   Symbol class_name;
public:
   MethodBinding(Symbol method_name, Symbol class_name);
   void code_ref(ostream&);
};

class CgenNode : public class__class {
private: 
   int class_tag;                             // the tag of class
   int max_child;                             // the max num of the tag of any child
   CgenNodeP parentnd;                        // Parent of class
   List<CgenNode> *children;                  // Children of class
   Basicness basic_status;                    // `Basic' if class is basic

   //method info
   int num_methods;
   SymbolTable<Symbol,int> method_name_to_offset_table;
   SymbolTable<Symbol,int> method_name_to_numtemps_table;
   SymbolTable<int,MethodBinding> method_offset_to_binding_table;

   //attribute and local var info
   int first_attribute;
   int num_attributes;
   SymbolTable<Symbol,VarBinding> var_binding_table;
   SymbolTable<int,attr_class> attribute_init_table;
   SymbolTable<int,Entry> attribute_proto_table;

   CgenClassTableP class_table;

   void code_disptable_ref(ostream&);
   void code_protoobj_ref(ostream&);
   void code_init_ref(ostream&);
   void code_method(ostream&, CgenEnvClassLevelP);

public:
   CgenNode(Class_ c,
            Basicness bstatus,
            CgenClassTableP class_table);

   void init(int,
             SymbolTable<Symbol,int> method_name_to_offset_table,
             SymbolTable<int,MethodBinding> method_offset_to_binding_table,
             int first_attribute,
             SymbolTable<Symbol,VarBinding> binding_table,
             SymbolTable<int,Entry> attribute_proto_table
             );
   
   void assign_tags(CgenClassTableP c);
   void layout_method(Symbol name, int numtemps);
   void layout_attributes(Symbol name, attr_class *a, int init);
   void code_prototype_object(ostream&);
   void code_methods(ostream&,CgenEnvTopLevelP);
   void code_init(ostream&, CgenEnvTopLevelP);
   void code_dispatch_table(ostream&);
   int get_tag();
   int get_max_child();

   void add_child(CgenNodeP child);
   List<CgenNode> *get_children() { return children; }
   void set_parentnd(CgenNodeP p);
   CgenNodeP get_parentnd() { return parentnd; }
   int basic() { return (basic_status == Basic); }
};

class BoolConst 
{
 private: 
  int val;
 public:
  BoolConst(int);
  void code_def(ostream&, int boolclasstag);
  void code_ref(ostream&) const;
};

class CgenEnvTopLevel {
protected:
   SymbolTable<Symbol,int> *class_to_tag_table;
   SymbolTable<Symbol,int> *class_to_max_child_tag_table;
   SymbolTable<Symbol,SymbolTable<Symbol,int>> *table_of_method_tables;
   int num_class;
public:
   CgenEnvTopLevel(SymbolTable<Symbol,int> *cttt,
                   SymbolTable<Symbol,int> *ctmctt,
                   SymbolTable<Symbol,SymbolTable<Symbol,int>> *tomt,
                   int nc) :
         class_to_tag_table(cttt),
         class_to_max_child_tag_table(ctmctt),
         table_of_method_tables(tomt),
         num_class(nc)
         { }
};

class CgenEnvClassLevel : public CgenEnvTopLevel {
protected:
   SymbolTable<Symbol,int> *method_name_to_offset_table;

   SymbolTable<Symbol,VarBinding> var_binding_table;
   Symbol class_name;
   Symbol file_name;

public:
   CgenEnvClassLevel(CgenEnvTopLevelP env,
            SymbolTable<Symbol,int> *mntot,
            SymbolTable<Symbol,VarBinding> vbt,
            Symbol cn,
            Symbol fn) :
         CgenEnvTopLevel(*env),
         method_name_to_offset_table(mntot),
         var_binding_table(vbt),
         class_name(cn),
         file_name(fn)
         { }
};

class CgenEnvironment : public CgenEnvClassLevel {
private:
   static int next_label; //for the jump instruction
   int next_formal;
   int num_temps;
   int next_temp_location;
public:
   CgenEnvironment(CgenEnvClassLevelP env,
            Formals formals,
            int num_temps);

   int lookup_tag(Symbol);
   int lookup_child_tag(Symbol);
   int lookup_method(Symbol classname, Symbol methodname);


   //varia table operations : enter,exit scope lookup
   VarBinding *lookup_variable(Symbol );
   void add_local(Symbol sym);
   void add_formal(Symbol sym);
   void remove_local();
   int label();

   Register get_register(int offset); //which register is used at the offset
   Register get_next_register(); //which register will allocate

   int get_num_temps();
   int get_register_temps();
   int get_stack_temps();

   int get_num_class() {
      return num_class;
   }
   Symbol get_class_name() {
      return class_name;
   }
   Symbol get_file_name() {
      return file_name;
   }
};

class CalcTemp;
typedef CalcTemp *CalcTempP;

class CalcTemp {
protected:
   int num;
   int max;

public:
   CalcTempP add() {
      ++num;
      if(num > max)
         max = num;
      return this;
   }

   CalcTempP sub() {
      --num;
      return this;
   }

   int get_max() {
      return max;
   }

   CalcTemp() : num(0), max(0) { }
}；