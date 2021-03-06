#ifndef AN_DECLARATION_H
#define AN_DECLARATION_H

#include <string>
#include "typedvalue.h"

namespace ante {
    namespace parser {
        struct Node;
    }

    /**
     * Base class for variable declarations and function
     * declarations.  All declarations have the parser Node
     * they were defined in as well as a name.
     *
     * When Declarations are compiled the tval field is filled.
     */
    struct Declaration {
        std::string name;
        TypedValue tval;
        parser::Node *definition;

        Declaration() = delete;

        Declaration(std::string const& n, parser::Node *def)
            : name{n}, definition{def}{}

        virtual ~Declaration(){};

        bool isGlobal() const;

        /** True if this is a mutable/global var. */
        bool shouldAutoDeref() const;

        bool isParamDecl() const;

        virtual bool isFuncDecl() const {
            return false;
        }

        /** True if this is a function stub from a trait rather than an impl */
        virtual bool isTraitFuncDecl() const {
            return false;
        }
    };
}

#endif
