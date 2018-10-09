#include "constraintfindingvisitor.h"
#include "unification.h"
#include "antype.h"
#include "funcdecl.h"
#include "compiler.h"
#include "types.h"

using namespace std;

namespace ante {
    using namespace parser;

    std::list<std::pair<AnType*, AnType*>> ConstraintFindingVisitor::getConstraints() const {
        return constraints;
    }

    /** Annotate all nodes with placeholder types */
    void ConstraintFindingVisitor::visit(RootNode *n){
        for(auto &m : n->imports)
            m->accept(*this);
        for(auto &m : n->types)
            m->accept(*this);
        for(auto &m : n->traits)
            m->accept(*this);
        for(auto &m : n->extensions)
            m->accept(*this);
        for(auto &m : n->funcs)
            m->accept(*this);

        for(auto &m : n->main){
            m->accept(*this);
        }
    }

    void ConstraintFindingVisitor::visit(IntLitNode *n){}

    void ConstraintFindingVisitor::visit(FltLitNode *n){}

    void ConstraintFindingVisitor::visit(BoolLitNode *n){}

    void ConstraintFindingVisitor::visit(StrLitNode *n){}

    void ConstraintFindingVisitor::visit(CharLitNode *n){}

    void ConstraintFindingVisitor::visit(ArrayNode *n){
        auto arrty = try_cast<AnArrayType>(n->getType());
        if(!n->exprs.empty()){
            auto t1 = n->exprs[0]->getType();
            for(auto it = ++n->exprs.begin(); it != n->exprs.end(); it++){
                (*it)->accept(*this);
                constraints.emplace_back(t1, (*it)->getType());
            }
            constraints.emplace_back(arrty->extTy, t1);
        }else{
            constraints.emplace_back(arrty->extTy, AnType::getVoid());
        }
    }

    void ConstraintFindingVisitor::visit(TupleNode *n){
        for(auto &e : n->exprs)
            e->accept(*this);
    }

    void ConstraintFindingVisitor::visit(ModNode *n){
        if(n->expr)
            n->expr->accept(*this);
    }

    void ConstraintFindingVisitor::visit(TypeNode *n){}

    void ConstraintFindingVisitor::visit(TypeCastNode *n){
        n->rval->accept(*this);
        constraints.emplace_back(n->typeExpr->getType(), n->rval->getType());
    }

    void ConstraintFindingVisitor::visit(UnOpNode *n){
        n->rval->accept(*this);
        auto tv = nextTypeVar();
        switch(n->op){
        case '@':
            constraints.emplace_back(n->rval->getType(), AnPtrType::get(tv));
            constraints.emplace_back(n->getType(), tv);
            break;
        case '&':
            constraints.emplace_back(n->getType(), AnPtrType::get(tv));
            constraints.emplace_back(n->rval->getType(), tv);
            break;
        case '-': //negation
            constraints.emplace_back(n->getType(), AnType::getI32());
            constraints.emplace_back(n->rval->getType(), AnType::getI32());
            break;
        case Tok_Not:
            constraints.emplace_back(n->rval->getType(), AnType::getBool());
            constraints.emplace_back(n->getType(), AnType::getBool());
            break;
        case Tok_New:
            constraints.emplace_back(n->getType(), AnPtrType::get(tv));
            constraints.emplace_back(n->rval->getType(), tv);
            break;
        }
    }

    void ConstraintFindingVisitor::visit(SeqNode *n){
        for(auto &stmt : n->sequence){
            stmt->accept(*this);
        }
    }

    void ConstraintFindingVisitor::visit(BinOpNode *n){
        n->lval->accept(*this);
        n->rval->accept(*this);

        if(n->op == '('){
            auto fnty = try_cast<AnFunctionType>(n->lval->getType());
            if(!fnty){
                auto args = try_cast<AnAggregateType>(n->rval->getType());
                auto params = vecOf<AnType*>(args->extTys.size());

                for(size_t i = 0; i < args->extTys.size(); i++){
                    auto param = nextTypeVar();
                    constraints.emplace_back(args->extTys[i], param);
                    params.push_back(param);
                }
                auto retTy = nextTypeVar();
                constraints.emplace_back(n->getType(), retTy);

                fnty = AnFunctionType::get(retTy, params);
                constraints.emplace_back(n->lval->getType(), fnty);
            }else{
                auto args = try_cast<AnAggregateType>(n->rval->getType());
                if(args->extTys.size() != fnty->extTys.size()){
                    error("Function takes " + to_string(fnty->extTys.size())
                            + " arguments but " + to_string(args->extTys.size())
                            + " were given", n->lval->loc);
                }

                for(size_t i = 0; i < args->extTys.size(); i++){
                    constraints.emplace_back(args->extTys[i], fnty->extTys[i]);
                }
                constraints.emplace_back(n->getType(), fnty->retTy);
            }
        }else if(n->op == '+' || n->op == '-' || n->op == '*' || n->op == '/' || n->op == '%' || n->op == '^'){
            constraints.emplace_back(n->lval->getType(), AnType::getI32());
            constraints.emplace_back(n->rval->getType(), AnType::getI32());
            constraints.emplace_back(n->getType(), AnType::getI32());
        }else if(n->op == '<' || n->op == '>' || n->op == Tok_GrtrEq || n->op == Tok_LesrEq){
            constraints.emplace_back(n->lval->getType(), AnType::getI32());
            constraints.emplace_back(n->rval->getType(), AnType::getI32());
            constraints.emplace_back(n->getType(), AnType::getBool());
        }else if(n->op == '#'){
            auto t = nextTypeVar();
            constraints.emplace_back(n->lval->getType(), AnArrayType::get(t));
            constraints.emplace_back(n->rval->getType(), AnType::getI32());
            constraints.emplace_back(n->getType(), t);
        }else if(n->op == Tok_Or || n->op == Tok_And){
            constraints.emplace_back(n->lval->getType(), AnType::getBool());
            constraints.emplace_back(n->rval->getType(), AnType::getBool());
            constraints.emplace_back(n->getType(), AnType::getBool());
        }else if(n->op == Tok_Is || n->op == Tok_Isnt || n->op == '=' || n->op == Tok_NotEq){
            constraints.emplace_back(n->lval->getType(), n->rval->getType());
            constraints.emplace_back(n->getType(), AnType::getBool());
        }else if(n->op == Tok_Range){
            constraints.emplace_back(n->lval->getType(), AnType::getI32());
            constraints.emplace_back(n->rval->getType(), AnType::getI32());
        }else if(n->op == Tok_In){
            auto tv = nextTypeVar();
            constraints.emplace_back(tv, n->lval->getType());
            constraints.emplace_back(n->rval->getType(), AnArrayType::get(tv));
            constraints.emplace_back(n->getType(), AnType::getBool());
        }
    }

    void ConstraintFindingVisitor::visit(BlockNode *n){
        n->block->accept(*this);
        constraints.emplace_back(n->getType(), n->block->getType());
    }

    void ConstraintFindingVisitor::visit(RetNode *n){
        n->expr->accept(*this);
        constraints.emplace_back(n->getType(), n->expr->getType());
    }

    void ConstraintFindingVisitor::visit(ImportNode *n){}

    void ConstraintFindingVisitor::visit(IfNode *n){
        n->condition->accept(*this);
        n->thenN->accept(*this);
        constraints.emplace_back(n->condition->getType(), AnType::getBool());
        if(n->elseN){
            n->elseN->accept(*this);
            constraints.emplace_back(n->thenN->getType(), n->elseN->getType());
            constraints.emplace_back(n->thenN->getType(), n->getType());
        }
    }

    void ConstraintFindingVisitor::visit(NamedValNode *n){
        n->typeExpr->accept(*this);
    }

    void ConstraintFindingVisitor::visit(VarNode *n){}


    void ConstraintFindingVisitor::visit(VarAssignNode *n){
        n->expr->accept(*this);
        n->ref_expr->accept(*this);
    }

    void ConstraintFindingVisitor::visit(ExtNode *n){
        for(auto *m : *n->methods)
            m->accept(*this);
    }

    void ConstraintFindingVisitor::visit(JumpNode *n){
        n->expr->accept(*this);
        constraints.emplace_back(n->expr->getType(), AnType::getI32());
    }

    void ConstraintFindingVisitor::visit(WhileNode *n){
        n->condition->accept(*this);
        n->child->accept(*this);
        constraints.emplace_back(n->condition->getType(), AnType::getBool());
    }

    void ConstraintFindingVisitor::visit(ForNode *n){
        n->range->accept(*this);
        n->pattern->accept(*this);
        n->child->accept(*this);
    }

    void ConstraintFindingVisitor::visit(MatchNode *n){
        n->expr->accept(*this);
        for(auto &b : n->branches){
            b->accept(*this);
        }
    }

    void ConstraintFindingVisitor::visit(MatchBranchNode *n){
        n->pattern->accept(*this);
        n->branch->accept(*this);
    }

    void ConstraintFindingVisitor::visit(FuncDeclNode *n){
        for(auto *p : *n->params){
            p->accept(*this);
        }
        n->child->accept(*this);

        auto fnty = try_cast<AnFunctionType>(n->getType());
        constraints.emplace_back(fnty->retTy, n->child->getType());
    }

    void ConstraintFindingVisitor::visit(DataDeclNode *n){}

    void ConstraintFindingVisitor::visit(TraitNode *n){}
}