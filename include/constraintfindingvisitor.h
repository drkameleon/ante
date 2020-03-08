#ifndef AN_CONSTRAINTFINDINGVISITOR_H
#define AN_CONSTRAINTFINDINGVISITOR_H

#include "parser.h"
#include "antype.h"
#include "pattern.h"
#include "unification.h"
#include "module.h"
#include <tuple>

namespace ante {
    
    struct ConstraintFindingVisitor : public NodeVisitor {
        ConstraintFindingVisitor(Module *module) : module{module}{
            functionReturnTypes.push(AnType::getUnit());
        }

        DECLARE_NODE_VISIT_METHODS();

        UnificationList getConstraints() const;

        private:
            Module *module;

            UnificationList constraints;

            /**
             * This stack's structure is identical to the current function call stack,
             * except that it only contains the return type of each respective function.
             *
             * - Used for constraing the type of RetNodes since the function return type
             *   cant be found within the RetNode itself
             * - This is always at least length 1, with the first element always being
             *   the main function's return type of unit.
             */
            std::stack<AnType*> functionReturnTypes;

            void addConstraint(AnType *a, AnType *b, LOC_TY &loc, lazy_printer const& errMsg);

            void addTypeClassConstraint(TraitImpl *typeclass, LOC_TY &loc);

            void fnCallConstraints(parser::BinOpNode *n);

            /** Searches type for typeclasses, removes them from the type, and adds them
             *  as a separate constraint.  Returns a new type with type classes removed. */
            AnType* handleTypeClassConstraints(AnType *t, LOC_TY const& loc);

            void searchForField(parser::BinOpNode *op);

            /** Add bindings from a trait's declaration to the scope of
             *  the trait's implementation to better infer parameter/return types. */
            void addConstraintsFromTCDecl(parser::FuncDeclNode *fdn, TraitImpl *tr, parser::FuncDeclNode *decl, LOC_TY &implLoc);

            void handlePattern(parser::MatchNode *n, parser::Node *pat, AnType *expectedType, Pattern &patChecker);

            void handleTuplePattern(parser::MatchNode *n, parser::TupleNode *pat, AnType *expectedType, Pattern &patChecker);

            void handleUnionVariantPattern(parser::MatchNode *n, parser::TypeCastNode *pat, AnType *expectedType, Pattern &patChecker);
    };
}


#endif /* end of include guard: AN_CONSTRAINTFINDINGVISITOR_H */
