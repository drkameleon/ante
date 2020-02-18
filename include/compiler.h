#ifndef COMPILER_H
#define COMPILER_H

#include <climits> //required by llvm when using clang
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ADT/StringMap.h>

#include <string>
#include <memory>
#include <list>

#include "args.h"
#include "parser.h"
#include "antype.h"
#include "lazystr.h"
#include "funcdecl.h"
#include "variable.h"
#include "antevalue.h"
#include "typedvalue.h"
#include "unification.h"

#define AN_MANGLED_SELF "_$self$"

namespace ante {

    struct CompilingVisitor : public NodeVisitor {
        TypedValue val;
        Compiler *c;

        CompilingVisitor(Compiler *cc) : c(cc){}
        virtual ~CompilingVisitor(){}

        static TypedValue compile(Compiler *c, std::unique_ptr<parser::Node> &n){
            return compile(c, n.get());
        }
        static TypedValue compile(Compiler *c, std::shared_ptr<parser::Node> &n){
            return compile(c, n.get());
        }
        static TypedValue compile(Compiler *c, parser::Node *n){
            CompilingVisitor v{c};
            n->accept(v);
            return v.val;
        }

        DECLARE_NODE_VISIT_METHODS();
    };

    struct PrintingVisitor : public NodeVisitor {
        size_t indent_level = 0;

        virtual ~PrintingVisitor(){}

        static void print(std::unique_ptr<parser::Node> &n){
            return print(n.get());
        }
        static void print(std::shared_ptr<parser::Node> &n){
            return print(n.get());
        }
        static void print(parser::Node *n){
            PrintingVisitor v;
            n->accept(v);
        }

        DECLARE_NODE_VISIT_METHODS();
    };


    /**
     * @brief Contains state information on the module being compiled
     */
    struct CompilerCtxt {
        //Stack of each called function
        std::vector<FuncDecl*> callStack;

        //Map of typevar names to concrete types whenever a generic funcion is monomorphised
        Substitutions monomorphisationMappings;

        //the continue and break labels of each for/while loop to jump out of
        //the pointer is swapped/nullified when a function is called to prevent
        //cross-function jumps
        std::unique_ptr<std::vector<llvm::BasicBlock*>> continueLabels;
        std::unique_ptr<std::vector<llvm::BasicBlock*>> breakLabels;

        CompilerCtxt() : callStack(), continueLabels(new std::vector<llvm::BasicBlock*>()), breakLabels(new std::vector<llvm::BasicBlock*>()){}

        void insertMonomorphisationMappings(Substitutions const& subs){
            for(auto &sub : subs){
                if(sub.first != sub.second){
                    monomorphisationMappings.push_back(sub);
                }
            }
        }
    };

    /**
     * @brief Contains compile-time information for user hooks and ctStores.
     */
    struct CompilerCtCtxt {
        /** @brief functions to run whenever a function is declared. */
        std::vector<std::shared_ptr<FuncDecl>> on_fn_decl_hook;

        /** @brief arguments to current ante function being called.
         * Will be empty if !isJIT */
        std::vector<TypedValue> args;
    };

    /**
     * @brief An Ante compiler responsible for a single module
     */
    struct Compiler {
        std::shared_ptr<llvm::LLVMContext> ctxt;
        std::unique_ptr<llvm::ExecutionEngine> jit;
        std::unique_ptr<llvm::Module> module;
        llvm::IRBuilder<> builder;

        Module *compUnit;

        /** The abstract syntax tree.
         *  This is gradually filled with more information
         *  during each compilation phase. */
        parser::RootNode* ast;

        std::unique_ptr<CompilerCtxt> compCtxt;

        std::shared_ptr<CompilerCtCtxt> ctCtxt;

        bool compiled, isLib, isJIT;
        std::string fileName, outFile, funcPrefix;
        unsigned int scope, optLvl, fnScope;

        /**
        * @brief The main constructor for Compiler
        *
        * @param fileName Name of the file being compiled
        * @param lib Set to true if this module should be compiled as a library
        * @param ctxt The LLVMContext possibly shared with another Compiler
        */
        Compiler(const char *fileName, bool lib=false, std::shared_ptr<llvm::LLVMContext> ctxt = nullptr);

        /**
        * @brief Constructor for a Compiler compiling a sub-module within the current file.  Currently only
        * used for string interpolation.
        *
        * @param root The node to set as the root node (does not need to be a RootNode already)
        * @param modName Name of the module being compiled
        * @param fName Name of the file being compiled
        * @param lib Set to true if this module should be compiled as a library
        * @param ctxt The LLVMContext shared from the parent Compiler
        */
        Compiler(Compiler *c, parser::Node *root, std::string modName, bool lib=false);
        ~Compiler();

        /** @brief Fully compiles a module into llvm bytecode */
        void compile();

        /** @brief Compiles a native binary */
        void compileNative();

        /**
        * @brief Compiles a module to an object file
        *
        * @param outName name of the file to output
        *
        * @return 0 on success
        */
        int  compileObj(std::string &outName);

        /**
        * @brief Creates the main function of a main module or creates the library_init
        * function of a lib module.
        *
        * @return The llvm::Function* of the created function.
        */
        llvm::Function* createMainFn();

        /** @brief Starts the REPL */
        void eval();

        /** @brief Dumps current contents of module to stdout */
        void emitIR();

        /** @brief Returns a pointer to the RootNode of the current Module. */
        parser::RootNode* getAST() const {
            return ast;
        }

        /**
        * @brief Sweeps through parse tree registering all functions, type
        * declarations, and traits.
        *
        * @return true on error
        */
        bool scanAllDecls(parser::RootNode *n = 0);

        /**
        * @brief Sets appropriate flags and executes operations specified by
        *        the command line arguments
        *
        * @param args The command line arguments
        */
        void processArgs(CompilerArgs *args);


        /**
         * Create a tuple of the given elements.
         *
         * @param packed True if the tuple should be represented as a packed structure.
         */
        llvm::Value* tupleOf(std::vector<llvm::Value*> const& elems, bool packed);

        /**
         * Create a ptr to the memory address specified.
         *
         * The given memory address does not change during runtime, and as such this
         * should only be used for pointers whose contents are guarenteed to also
         * be present when the module is executed.
         */
        llvm::Value* ptrTo(void* val);

        /**
         * @brief Compiles an extract operation such as array#index
         *
         * Operator overloads are not taken into account and should be handled beforehand.
         *
         * @param l The container to extract from
         * @param r The index to extract
         * @param op The # operator used for error reporting
         *
         * @return The result of the extraction
         */
        TypedValue compExtract(TypedValue &l, TypedValue &r, parser::BinOpNode *op);

        /**
         * @brief Compiles an insert operation such as array#index = 2
         *
         * @param insertOp The # operator containing the lhs of the assignment
         * @param assignExpr The rhs of the assignment
         *
         * @return A void literal
         */
        TypedValue compInsert(parser::BinOpNode *insertOp, parser::Node *assignExpr);

        /**
         * @brief Compiles a named member access such as str.len
         *
         * @param ln The value or type/module being accessed
         * @param field Name of the desired field/method
         * @param binop Location of the . operator for error reporting
         *
         * @return The extracted field or method
         */
        TypedValue compMemberAccess(parser::Node *ln, parser::VarNode *field, parser::BinOpNode *binop);
        TypedValue compLogicalOr(parser::Node *l, parser::Node *r, parser::BinOpNode *op);
        TypedValue compLogicalAnd(parser::Node *l, parser::Node *r, parser::BinOpNode *op);

        /**
        * @brief JIT compiles a function with no arguments and calls it afterward
        *
        * @param f the function to JIT
        */
        void jitFunction(llvm::Function *fnName);

        FuncDecl* getCurrentFunction() const;

        /**
         * @brief Compiles any non-generic function
         *
         * Generic functions (indicated by a typecheck returning
         * TypeCheckResult::SuccessWithTypeVars) should be compiled
         * with compTemplateFn which calls this function internally.
         */
        TypedValue compFn(FuncDecl *fn);

        /*
         * @brief Returns the current scope of the block compiling.
         */
        unsigned int getScope() const;

        /*
         * Returns the name of the singular ante::Module this
         * Compiler is in charge of compiling.
         */
        std::string getModuleName() const;

        /**
         * @brief Translates an AnType* to an llvm::Type*.
         *
         * Translation fails if the type contains an undeclared data type or an undeclared
         * type variable unless the force flag is set.  If the force flag is
         * set each undeclared type var is replaced with a void* and undeclared
         * data types remain an error.
         *
         * The force flag should generally be avoided unless type inferencing is
         * needed/guarenteed to be performed at a later step to retractively
         * fix the translated type.
         */
        llvm::Type* anTypeToLlvmType(const AnType *ty, int recursionLimit = 1000);

        /**
        * @brief Compiles a module into an obj file to be used for linking.
        *
        * @param mod The already-compiled module
        * @param outFile Name of the file to output
        *
        * @return 0 on success
        */
        int compileIRtoObj(llvm::Module *mod, std::string outFile);

        TypedValue getUnitLiteral();

        /**
        * @brief Invokes the linker specified by AN_LINKER (in target.h) to
        *        link each object file
        *
        * @param inFiles String containing each obj file to link separated with spaces
        * @param outFile Name of the file to output
        *
        * @return 0 on success
        */
        static int linkObj(std::string inFiles, std::string outFile);
    };

    /*
     * @brief Compiles and returns the address of an lval or expression
     */
    TypedValue addrOf(Compiler *c, TypedValue &tv);


    /**
    *  Compile a compile-time function/macro which should not return a function call, just a compile-time constant.
    *  Ex: A call to Ante.getAST() would be a meta function as it wouldn't make sense to get the parse tree
    *      during runtime
    *
    *  - Assumes arguments are already type-checked
    */
    TypedValue compMetaFunctionResult(Compiler *c, LOC_TY const& loc, std::string const& baseName,
            std::string const& mangledName, std::vector<TypedValue> const& typedArgs,
            std::vector<std::unique_ptr<parser::Node>> const& argExprs);


    /**
     * Compile and call an ante function with the given arguments.
     *
     * The result of the call will be translated into a TypedValue.
     * This function will throw a CtError on error
     */
    TypedValue compileAndCallAnteFunction(Compiler *c, std::string const& baseName,
        std::string const& mangledName, std::vector<TypedValue> const& typedArgs,
        std::vector<std::unique_ptr<parser::Node>> const& argExprs);

    /**
     * Compile and call an ante expression from its node in the AST.
     *
     * The result of the call will be translated into a TypedValue.
     * This function will throw a CtError on error
     */
    TypedValue compileAndCallAnteFunction(Compiler *c, parser::ModNode *n);

    /**
    * Compiles the given Node and catches any CtError
    * exceptions it may throw.
    */
    template<typename T>
    TypedValue safeCompile(Compiler *c, T &n){
        CompilingVisitor v{c};
        try{
            n->accept(v);
        }catch(CtError err){}
        return v.val;
    }


    /**
    * @brief Retrieves the Nth node of a list
    *
    * Does not check if list contains at least n nodes
    *
    * @param node The head of the list
    * @param n Index of the node to return
    *
    * @return The nth node from the list
    */
    parser::Node* getNthNode(parser::Node *node, size_t n);

    /** @brief Counts the amount of Nodes in the list */
    size_t getTupleSize(parser::Node *tup);

    TypedValue compileRefExpr(CompilingVisitor &cv, parser::Node *refExpr, parser::Node *assignExpr);

    /** @brief Converts the Node list argument into a vector */
    template<typename T> std::vector<T*> vectorize(T *args);

    /** @brief Extract elements of an AnAggregateType to a vector, otherwise
     * return a single element vector consisting of the type itself. */
    std::vector<AnType*> toTuple(AnType *ty);

    /** @brief Extracts the type of each arg into a TypeNode vector */
    std::vector<AnType*> toTypeVector(std::vector<TypedValue> const& tvs);

    std::string mangle(std::string const& base, std::vector<AnType*> const& params);
    std::string mangle(FuncDecl *fd, std::vector<AnType*> const& params);
    std::string mangle(std::string const& base, std::shared_ptr<parser::NamedValNode> const& paramTys);
    std::string mangle(std::string const& base, parser::TypeNode *paramTys);
    std::string mangle(std::string const& base, parser::TypeNode *p1, parser::TypeNode *p2);
    std::string mangle(std::string const& base, parser::TypeNode *p1, parser::TypeNode *p2, parser::TypeNode *p3);

    std::string removeFileExt(std::string file);

}

#endif
