#ifndef PTREE_H
#define PTREE_H

#include "parser.h"

#ifndef LOC_TY
#define LOC_TY yy::location
#endif

//defined in lexer.cpp
extern char* lextxt;

namespace ante {
    namespace parser {

        Node* setRoot(Node* root);
        Node* getRoot();
        Node* setNext(Node* cur, Node* nxt);
        Node* setElse(Node *ifn, Node *elseN);
        Node* addMatch(Node *matchExpr, Node *newMatch);
        Node* applyMods(Node *mods, Node *decls);

        void createRoot();
        void createRoot(LOC_TY& loc);

        Node* append_main(Node *n);
        Node* append_fn(Node *n);
        Node* append_type(Node *n);
        Node* append_extension(Node *n);
        Node* append_trait(Node *n);
        Node* append_import(Node *n);
        Node* append_modifiers(Node *modifiers, Node *modifiableNode);

        char* nextVarArgsTVName();

        Node* mkIntLitNode(LOC_TY loc, char* s);
        Node* mkFltLitNode(LOC_TY loc, char* s);
        Node* mkStrLitNode(LOC_TY loc, char* s);
        Node* mkCharLitNode(LOC_TY loc, char* s);
        Node* mkBoolLitNode(LOC_TY loc, char b);
        Node* mkArrayNode(LOC_TY loc, Node *expr);
        Node* mkTupleNode(LOC_TY loc, Node *expr);
        Node* mkModNode(LOC_TY loc, ante::TokenType mod);
        Node* mkModExprNode(LOC_TY loc, ante::TokenType mod, Node *expr);
        Node* mkAsNode(LOC_TY loc, Node *expr, Node *type);

        //A compiler directive is represented as a ModNode
        //internally, hence the omission of Node from the name below
        Node* mkCompilerDirective(LOC_TY loc, Node *directive);
        Node* mkCompilerDirectiveExpr(LOC_TY loc, Node *directive, Node *expr);

        Node* mkTypeNode(LOC_TY loc, TypeTag type, char* typeName, Node *extTy = nullptr);
        Node* mkInferredTypeNode(LOC_TY loc);
        Node* mkTypeCastNode(LOC_TY loc, Node *l, Node *r);
        Node* mkUnOpNode(LOC_TY loc, int op, Node *r);
        Node* mkBinOpNode(LOC_TY loc, int op, Node* l, Node* r);
        Node* mkSeqNode(LOC_TY loc, Node *l, Node *r);
        Node* mkBlockNode(LOC_TY loc, Node* b);
        Node* mkNamedValNode(LOC_TY loc, Node* nodes, Node* tExpr);
        Node* mkVarNode(LOC_TY loc, char* s);
        Node* mkRetNode(LOC_TY loc, Node* expr);
        Node* mkImportNode(LOC_TY loc, Node* expr);
        Node* mkVarAssignNode(LOC_TY loc, Node* var, Node* expr, bool shouldFreeLval = true);
        Node* mkExtNode(LOC_TY loc, Node* typeExpr, Node* methods, Node* traits);
        Node* mkMatchNode(LOC_TY loc, Node* expr, Node* branch);
        Node* mkMatchBranchNode(LOC_TY loc, Node* pattern, Node* branch);
        Node* mkJumpNode(LOC_TY loc, int jumpType, Node* expr);

        Node* mkIfNode(LOC_TY loc, Node* con, Node* body, Node* els);
        Node* mkWhileNode(LOC_TY loc, Node* con, Node* body);
        Node* mkForNode(LOC_TY loc, Node* var, Node* range, Node* body);
        Node* mkFuncCallNode(LOC_TY loc, Node* nameAndArgs);
        Node* mkFuncDeclNode(LOC_TY loc, Node* nameAndParams, Node* tExpr, Node* tcc, Node* body);
        Node* mkDataDeclNode(LOC_TY loc, char* s, Node *p, Node* b, bool isAlias);
        Node* mkTraitNode(LOC_TY loc, char* s, Node* generics, Node* fns);

    }
}

#endif
