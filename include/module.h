#ifndef AN_MODULE_H
#define AN_MODULE_H

#include <string>
#include <memory>
#include <llvm/ADT/StringMap.h>
#include "funcdecl.h"
#include "typedecl.h"

namespace ante {
    struct TraitDecl;
    struct TraitImpl;
    class AnType;

    using TypeArgs = std::vector<AnType*>;

    /**
     * A virtual filesystem tree node containing information on
     * types, functions, imports, and traits of the current module.
     *
     * Unlike the actual file system, this tree has several roots,
     * including the PWD as well as the location of any included
     * libraries, including Ante's standard library.  This has the
     * effect of merging each of these directories into a single
     * larger root directory.  In addition, not all modules within
     * the tree encompass a whole file/directory.  Instead, some
     * may be submodules within the same file as their parent module.
     */
    struct Module {
        std::string name;

        /**
         * @brief All imports of the module
         */
        std::vector<Module*> imports;

        /**
         * @brief The abstract syntax tree representing the contents of the module.
         */
        std::unique_ptr<parser::RootNode> ast;

        /**
         * @brief Each declared function in the module
         */
        llvm::StringMap<FuncDecl*> fnDecls;

        /**
         * @brief Each declared DataType in the module
         */
        llvm::StringMap<TypeDecl> userTypes;

        /**
         * @brief Map of all declared traits; not including their implementations for a given type
         */
        llvm::StringMap<TraitDecl*> traitDecls;

        /**
         * @brief Map of all trait implementations keyed by name.
         */
        llvm::StringMap<std::vector<TraitImpl*>> traitImpls;

        private:
        /** The submodules of the current node */
        llvm::StringMap<Module> children;

        public:
            Module(std::string const& name) : name{name} {}
            ~Module() = default;

            /** Return the root of the virtual file/module system. */
            static Module& getRoot();

            /** Return a declared type if it is visible to the current module.
             *  This is usually an AnDataType, but may be any type if the
             *  named type is an alias to a primitive type. */
            AnType* lookupType(std::string const& name) const;

            /** Lookup the given type and return it and its location. */
            TypeDecl* lookupTypeDecl(std::string const& name) const;

            /** Lookup the given TraitDecl* and return it if found, null otherwise */
            TraitDecl* lookupTraitDecl(std::string const& name) const;

            /** Lookup the given TraitInstance* and return it if found, null otherwise */
            TraitImpl* lookupTraitImpl(std::string const& name, TypeArgs const& typeArgs) const;

            /** Lookup the TraitDecl and return a new, unimplemented instance of it */
            TraitImpl* freshTraitImpl(std::string const& name) const;

            /** For some TraitDecl  D 'a 'b  create a TraitImpl exactly matching it with no fresh typevars */
            TraitImpl* createTraitImplFromDecl(std::string const& traitName) const;

            /** Find a single direct child with the given name */
            llvm::StringMap<Module>::iterator findChild(std::string const& name);


            /**
             * Return children.end().
             *
             * Can be used to check if returned iterators from the find methods are valid.
             */
            llvm::StringMap<Module>::iterator childrenEnd();

            /** Add a single direct child with the given name. */
            Module& addChild(std::string const& childName);

            /** Find a child with the given relative path from the current node. */
            template<class StringIt>
            llvm::StringMap<Module>::iterator findPath(StringIt path) {
                Module *node = this;
                llvm::StringMap<Module>::iterator ret = children.end();

                for(std::string const& name : path){
                    ret = node->findChild(name);
                    if(ret == node->childrenEnd()){
                        return children.end();
                    }
                    node = &ret->getValue();
                }
                return ret;
            }

            /** Add a child at the given relative path, adding any intermediate children as necessary. */
            template<class StringIt>
            Module& addPath(StringIt path){
                Module *node = this;
                for(std::string const& name : path){
                    auto child = node->findChild(name);
                    if(child == node->childrenEnd()){
                        node = &node->addChild(name);
                    }else{
                        node = &child->getValue();
                    }
                }
                return *node;
            }
    };


    /**
     * Provides an iterator over a substrings of a file path.
     * each item is separated by the OS-specific file separater character.
     * When converting a path to a module path, / and \ will both be treated
     * as directory separators, regardless of OS.  Furthermore, the file named '.'
     * will be skipped, so both stdlib/prelude.an and ./stdlib/prelude.an are equivalent.
     */
    class ModulePath {
        typedef ModulePath iterator;
        typedef std::string const& reference;
        typedef std::string const* pointer;

        std::string s;
        std::string substr;
        std::string::size_type prev, cur;

        /** remove trailing .an from module names */
        void removeTrailingFileType();

    public:
        ModulePath(std::string const& s);
        ~ModulePath() = default;

        iterator begin() const;
        iterator end() const;

        iterator& operator++();
        reference operator* () const;
        bool operator!=(const iterator& rhs) const;
    };
}


#endif /* end of include guard: AN_MODULE_H */

