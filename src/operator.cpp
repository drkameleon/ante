#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include "unification.h"
#include "scopeguard.h"
#include "antevalue.h"
#include "compiler.h"
#include "function.h"
#include "compapi.h"
#include "target.h"
#include "tokens.h"
#include "types.h"
#include "trait.h"
#include "util.h"

using namespace std;
using namespace llvm;
using namespace ante::parser;

namespace ante {

TypedValue Compiler::compAdd(TypedValue &l, TypedValue &r, BinOpNode *op){
    switch(l.type->typeTag){
        case TT_I8:  case TT_U8:  case TT_C8:
        case TT_I16: case TT_U16:
        case TT_I32: case TT_U32:
        case TT_I64: case TT_U64:
        case TT_Isz: case TT_Usz:
        case TT_Ptr:
            return TypedValue(builder.CreateAdd(l.val, r.val), l.type);
        case TT_F16:
        case TT_F32:
        case TT_F64:
            return TypedValue(builder.CreateFAdd(l.val, r.val), l.type);

        default:
            error("binary operator + is undefined for the type " + anTypeToColoredStr(l.type) + " and " + anTypeToColoredStr(r.type), op->loc);
            return {};
    }
}

TypedValue Compiler::compSub(TypedValue &l, TypedValue &r, BinOpNode *op){
    switch(l.type->typeTag){
        case TT_I8:  case TT_U8:  case TT_C8:
        case TT_I16: case TT_U16:
        case TT_I32: case TT_U32:
        case TT_I64: case TT_U64:
        case TT_Isz: case TT_Usz:
        case TT_Ptr:
            return TypedValue(builder.CreateSub(l.val, r.val), l.type);
        case TT_F16:
        case TT_F32:
        case TT_F64:
            return TypedValue(builder.CreateFSub(l.val, r.val), l.type);

        default:
            error("binary operator - is undefined for the type " + anTypeToColoredStr(l.type) + " and " + anTypeToColoredStr(r.type), op->loc);
            return {};
    }
}

TypedValue Compiler::compMul(TypedValue &l, TypedValue &r, BinOpNode *op){
    switch(l.type->typeTag){
        case TT_I8:  case TT_U8:  case TT_C8:
        case TT_I16: case TT_U16:
        case TT_I32: case TT_U32:
        case TT_I64: case TT_U64:
        case TT_Isz: case TT_Usz:
            return TypedValue(builder.CreateMul(l.val, r.val), l.type);
        case TT_F16:
        case TT_F32:
        case TT_F64:
            return TypedValue(builder.CreateFMul(l.val, r.val), l.type);

        default:
            error("binary operator * is undefined for the type " + anTypeToColoredStr(l.type) + " and " + anTypeToColoredStr(r.type), op->loc);
            return {};
    }
}

TypedValue Compiler::compDiv(TypedValue &l, TypedValue &r, BinOpNode *op){
    switch(l.type->typeTag){
        case TT_I8:
        case TT_I16:
        case TT_I32:
        case TT_I64:
        case TT_Isz:
            return TypedValue(builder.CreateSDiv(l.val, r.val), l.type);
        case TT_U8: case TT_C8:
        case TT_U16:
        case TT_U32:
        case TT_U64:
        case TT_Usz:
            return TypedValue(builder.CreateUDiv(l.val, r.val), l.type);
        case TT_F16:
        case TT_F32:
        case TT_F64:
            return TypedValue(builder.CreateFDiv(l.val, r.val), l.type);

        default:
            error("binary operator / is undefined for the type " + anTypeToColoredStr(l.type) + " and " + anTypeToColoredStr(r.type), op->loc);
            return {};
    }
}

TypedValue Compiler::compRem(TypedValue &l, TypedValue &r, BinOpNode *op){
    switch(l.type->typeTag){
        case TT_I8:
        case TT_I16:
        case TT_I32:
        case TT_I64:
        case TT_Isz:
            return TypedValue(builder.CreateSRem(l.val, r.val), l.type);
        case TT_U8: case TT_C8:
        case TT_U16:
        case TT_U32:
        case TT_U64:
        case TT_Usz:
            return TypedValue(builder.CreateURem(l.val, r.val), l.type);
        case TT_F16:
        case TT_F32:
        case TT_F64:
            return TypedValue(builder.CreateFRem(l.val, r.val), l.type);

        default:
            error("binary operator % is undefined for the types " + anTypeToColoredStr(l.type) + " and " + anTypeToColoredStr(r.type), op->loc);
            return {};
    }
}


/*
 *  Compiles the extract operator, #
 */
TypedValue Compiler::compExtract(TypedValue &l, TypedValue &r, BinOpNode *op){
    if(auto *arrty = try_cast<AnArrayType>(l.type)){
        //check for alloca
        Value *arr = dyn_cast<LoadInst>(l.val) ?
                cast<LoadInst>(l.val)->getPointerOperand() :
                addrOf(this, l).val;

        vector<Value*> indices;
        indices.push_back(ConstantInt::get(*ctxt, APInt(64, 0, true)));
        indices.push_back(r.val);
        auto *retty = (AnType*)l.type->addModifiersTo(arrty->extTy);
        return TypedValue(builder.CreateLoad(builder.CreateGEP(arr, indices)), retty);

    }else if(auto *ptrty = try_cast<AnPtrType>(l.type)){
        auto *retty = (AnType*)l.type->addModifiersTo(ptrty->extTy);
        return TypedValue(builder.CreateLoad(builder.CreateGEP(l.val, r.val)), retty);

    }else if(l.type->typeTag == TT_Tuple){
        auto indexval = dyn_cast<ConstantInt>(r.val);
        if(!indexval)
            error("Tuple indices must always be known at compile time.", op->loc);

        auto index = indexval->getZExtValue();

        auto *aggty = try_cast<AnTupleType>(l.type);

        AnType *indexTy = (AnType*)l.type->addModifiersTo(aggty->fields[index]);

        Value *tup = l.getType()->isPointerTy() ? builder.CreateLoad(l.val) : l.val;
        return TypedValue(builder.CreateExtractValue(tup, index), indexTy);
    }
    error("Type " + anTypeToColoredStr(l.type) + " does not have elements to access", op->loc);
    return {};
}


/*
 *  Compiles an insert statement for arrays or tuples.
 *  An insert statement would look similar to the following:
 *
 *  var tuple = ("one", 2.0, 4)
 *  tuple#2 = 3
 *
 *  This method works on lvals and returns a void value.
 */
TypedValue Compiler::compInsert(BinOpNode *op, Node *assignExpr){
    CompilingVisitor cv{this};
    auto tmp = compileRefExpr(cv, op->lval.get(), assignExpr);

    auto *var = dyn_cast<LoadInst>(tmp.val);

    auto index =  CompilingVisitor::compile(this, op->rval);
    auto newVal = CompilingVisitor::compile(this, assignExpr);

    //see if insert operator # = is overloaded already
    /*
    string basefn = "#";
    vector<AnType*> args = {tmp.type, AnType::getI32(), newVal.type};
    auto fn = getMangledFn(basefn, args);
    if(fn){
        vector<Value*> args = {var, index.val, newVal.val};
        auto *retty = try_cast<AnAggregateType>(fn.type)->extTys[0];
        auto *call = builder.CreateCall(fn.val, args);
        return TypedValue(call, retty);
    }
    */

    switch(tmp.type->typeTag){
        case TT_Array: {
            Value *cast = builder.CreateBitCast(var, var->getType()->getPointerElementType()->getArrayElementType()->getPointerTo());
            Value *dest = builder.CreateInBoundsGEP(cast, index.val);
            builder.CreateStore(newVal.val, dest);
            return getUnitLiteral();
        }
        case TT_Ptr: {
            Value *load = builder.CreateLoad(tmp.val);
            Value *dest = builder.CreateInBoundsGEP(load, index.val);
            builder.CreateStore(newVal.val, dest);
            return getUnitLiteral();
        }
        case TT_Tuple: case TT_Data: {
            ConstantInt *tupIndexVal = dyn_cast<ConstantInt>(index.val);
            if(!tupIndexVal){
                error("Tuple indices must always be known at compile time", op->loc);
            }else{
                auto tupIndex = tupIndexVal->getZExtValue();
                auto *aggty = try_cast<AnTupleType>(tmp.type);

                if(tupIndex >= aggty->fields.size())
                    error("Index of " + to_string(tupIndex) + " exceeds the maximum index of the tuple, "
                            + to_string(aggty->fields.size()-1), op->loc);

                auto *ins = builder.CreateInsertValue(builder.CreateLoad(var), newVal.val, tupIndex);
                builder.CreateStore(ins, var);
                return getUnitLiteral();
            }
        }
        default:
            error("Variable being indexed must be an Array or Tuple, but instead is a(n) " +
                    anTypeToColoredStr(tmp.type), op->loc);
            return {};
    }
}


TypedValue createUnionVariantCast(Compiler *c, vector<TypedValue> const& args,
        string &tagName, AnSumType *unionDataTy){

    vector<Type*> unionTys;
    vector<Constant*> unionVals;

    auto tagVal = unionDataTy->getTagVal(tagName);
    unionTys.push_back(Type::getInt8Ty(*c->ctxt));
    unionVals.push_back(ConstantInt::get(*c->ctxt, APInt(8, tagVal, true))); //tag

    for(auto &tval : args){
        unionTys.push_back(tval.getType());
        unionVals.push_back(UndefValue::get(tval.getType()));
    }

    Type *unionTy = c->anTypeToLlvmType(unionDataTy);

    //create a struct of (u8 tag, <union member type>)
    Value *taggedUnion = ConstantStruct::get(StructType::get(*c->ctxt, unionTys, true), unionVals);
    size_t i = 0;
    for(auto &arg : args){
        taggedUnion = c->builder.CreateInsertValue(taggedUnion, arg.val, ++i);
    }

    //allocate for the largest possible union member
    auto *alloca = c->builder.CreateAlloca(unionTy);

    //but bitcast it the the current member
    auto *castTo = c->builder.CreateBitCast(alloca, taggedUnion->getType()->getPointerTo());
    c->builder.CreateStore(taggedUnion, castTo);

    //load the original alloca, not the bitcasted one
    Value *unionVal = c->builder.CreateLoad(alloca);

    return TypedValue(unionVal, unionDataTy);
}


string getCastFnBaseName(AnType *t){
    if(auto *dt = try_cast<AnDataType>(t)){
        return dt->name + "_init";
    }
    return anTypeToStr(t) + "_init";
}

vector<AnType*> toTuple(AnType *ty){
    if(ty->typeTag == TT_Tuple){
        return try_cast<AnTupleType>(ty)->fields;
    }else if(ty->typeTag == TT_Unit){
        return {};
    }else{
        return {ty};
    }
}

/**
 *  Reinterpret a value as a tuple value when casting to a tuple type.
 *
 *  This function handles instances when a casted type is equal to the
 *  casting type's rhs in its definition.
 *
 *  For example, given the definition type T = U and a variable u: U
 *  the cast T u will be managed by this function with from = u and to = T
 */
TypedValue reinterpretTuple(Compiler *c, vector<TypedValue> const& args, AnType *to){
    auto *structTy = c->anTypeToLlvmType(to);
    Value *rstruct = UndefValue::get(structTy);

    auto nElems = rstruct->getType()->getStructNumElements();
    for(size_t i = 0; i < nElems; i++){
        rstruct = c->builder.CreateInsertValue(rstruct, args[i].val, i);
    }

    return TypedValue(rstruct, to);
}

TypedValue doReinterpretCast(Compiler *c, AnType *castTy, vector<TypedValue> const& args){
    auto *dt = try_cast<AnProductType>(castTy);
    if(dt){
        if(dt->parentUnionType){
            return createUnionVariantCast(c, args, dt->name, dt->parentUnionType);
        }else{
            return reinterpretTuple(c, args, dt);
        }
    }
    // cast to primitive
    return TypedValue(args[0].val, castTy);
}

void CompilingVisitor::visit(TypeCastNode *n){
    auto args = vecOf<TypedValue>(n->args.size());
    for(auto &arg : n->args){
        arg->accept(*this);
        args.push_back(this->val);
    }
    //n->typeExpr->getType() is used here instead of n->getType() in case
    //of union variants, only the typeExpr will have the specific variant used
    this->val = doReinterpretCast(c, n->typeExpr->getType(), args);
}


TypedValue compIf(Compiler *c, IfNode *ifn, BasicBlock *mergebb, vector<pair<TypedValue,BasicBlock*>> &branches){
    auto cond = CompilingVisitor::compile(c, ifn->condition);

    Function *f = c->builder.GetInsertBlock()->getParent();
    auto &blocks = f->getBasicBlockList();

    auto *thenbb = BasicBlock::Create(*c->ctxt, "then");

    //only create the else block if this ifNode actually has an else clause
    BasicBlock *elsebb = 0;

    if(ifn->elseN){
        if(dynamic_cast<IfNode*>(ifn->elseN.get())){
            elsebb = BasicBlock::Create(*c->ctxt, "elif");
            c->builder.CreateCondBr(cond.val, thenbb, elsebb);

            blocks.push_back(thenbb);
            c->builder.SetInsertPoint(thenbb);
            auto thenVal = CompilingVisitor::compile(c, ifn->thenN);

            //If a break, continue, or return was encountered then this branch doesn't merge to the endif
            if(!dyn_cast<ReturnInst>(thenVal.val) && !dyn_cast<BranchInst>(thenVal.val)){
                auto *thenretbb = c->builder.GetInsertBlock();
                c->builder.CreateBr(mergebb);

                //save the 'then' value for the PhiNode after all the elifs
                branches.push_back({thenVal, thenretbb});

                blocks.push_back(elsebb);
            }

            c->builder.SetInsertPoint(elsebb);
            return compIf(c, (IfNode*)ifn->elseN.get(), mergebb, branches);
        }else{
            elsebb = BasicBlock::Create(*c->ctxt, "else");
            c->builder.CreateCondBr(cond.val, thenbb, elsebb);

            blocks.push_back(thenbb);
            blocks.push_back(elsebb);
            blocks.push_back(mergebb);
        }
    }else{
        c->builder.CreateCondBr(cond.val, thenbb, mergebb);
        blocks.push_back(thenbb);
        blocks.push_back(mergebb);
    }

    c->builder.SetInsertPoint(thenbb);
    auto thenVal = CompilingVisitor::compile(c, ifn->thenN);
    if(!thenVal) return thenVal;
    auto *thenretbb = c->builder.GetInsertBlock(); //bb containing final ret of then branch.


    if(!dyn_cast<ReturnInst>(thenVal.val) && !dyn_cast<BranchInst>(thenVal.val))
        c->builder.CreateBr(mergebb);

    if(ifn->elseN){
        //save the final 'then' value for the upcoming PhiNode
        branches.push_back({thenVal, thenretbb});

        c->builder.SetInsertPoint(elsebb);
        auto elseVal = CompilingVisitor::compile(c, ifn->elseN);
        auto *elseretbb = c->builder.GetInsertBlock();

        if(!elseVal) return {};

        //save the final else
        if(!dyn_cast<ReturnInst>(elseVal.val) && !dyn_cast<BranchInst>(elseVal.val))
            branches.push_back({elseVal, elseretbb});

        if(!thenVal) return {};

        if(!dyn_cast<ReturnInst>(elseVal.val) && !dyn_cast<BranchInst>(elseVal.val))
            c->builder.CreateBr(mergebb);

        c->builder.SetInsertPoint(mergebb);

        //finally, create the ret value of this if expr, unless it is of void type
        if(thenVal.type->typeTag != TT_Unit){
            auto *phi = c->builder.CreatePHI(thenVal.getType(), branches.size());

            for(auto &pair : branches){
                if(!dyn_cast<ReturnInst>(pair.first.val)){
                    phi->addIncoming(pair.first.val, pair.second);
                }
            }

            return TypedValue(phi, thenVal.type);
        }else{
            return c->getUnitLiteral();
        }
    }else{
        c->builder.SetInsertPoint(mergebb);
        return c->getUnitLiteral();
    }
}

void CompilingVisitor::visit(IfNode *n){
    auto branches = vector<pair<TypedValue,BasicBlock*>>();
    auto *mergebb = BasicBlock::Create(*c->ctxt, "endif");
    this->val = compIf(c, n, mergebb, branches);
}

string toModuleName(const AnType *t){
    if(auto *dt = try_cast<AnDataType>(t)){
        return dt->name;
    }else if(t->isModifierType()){
        return toModuleName(static_cast<const AnModifier*>(t)->extTy);
    }else{
        return anTypeToStr(t);
    }
}

void attachTraitImpl(Declaration *decl, AnType *fnTy, Module *m, LOC_TY &loc){
    if(decl->isTraitFuncDecl()){
        auto trait = try_cast<AnFunctionType>(fnTy)->typeClassConstraints.front();
        if(!trait->hasTrivialImpl()){
            auto impl = m->lookupTraitImpl(trait->name, trait->typeArgs);
            if(!impl){
                error("No impl for " + traitToColoredStr(trait) + " in scope", loc);
            }else if(impl){
                trait->impl = impl->impl;
            }
        }
    }
}

Declaration* findFnInImpl(string const& fnName, TraitImpl *impl){
    auto extNode = impl->impl;
    if(extNode && extNode->methods){
        for(Node& n : *extNode->methods){
            if(auto fdn = dynamic_cast<FuncDeclNode*>(&n)){
                if(fdn->name == fnName){
                    return fdn->decl;
                }
            }
        }
    }
    std::cerr << "Could not find " << fnName << " in impl " << traitToColoredStr(impl) << '\n';
    ASSERT_UNREACHABLE();
}

TypedValue monomorphise(Compiler *c, FuncDecl *fd, AnFunctionType *boundType, LOC_TY &loc){
    auto fnTy = try_cast<AnFunctionType>(fd->definition->getType());

    //To be monomorphised, the function must be both generic and a definition, ie external
    //decls like printf: (ref c8) ... -> i32  are generic but cannot be monomorphised.
    auto isGenericDef = fnTy->isGeneric && static_cast<FuncDeclNode*>(fd->definition)->child;
    if(isGenericDef){
        TypeError err{"Error in monomorphisation of " + fd->name + ", types are "
            + anTypeToColoredStr(fnTy) + " bound to " + anTypeToColoredStr(boundType), loc};
        auto subs = unify({{fnTy, boundType, err}});
        c->compCtxt->insertMonomorphisationMappings(subs);
    }
    auto ret = c->compFn(fd);
    fd->tval.val = isGenericDef ? nullptr : ret.val;
    return ret;
}

AnFunctionType* applyMonomorphisationBindings(AnFunctionType *type, Substitutions const& bindings){
    return static_cast<AnFunctionType*>(ante::applySubstitutions(bindings, type));
}

TypedValue compForLoopTraitFn(Compiler *c, string const& fnName, TraitImpl *impl, AnType *argTy, LOC_TY &loc){
    FuncDecl *fn = static_cast<FuncDecl*>(findFnInImpl(fnName, impl));

    auto fnTy = try_cast<AnFunctionType>(fn->tval.type);
    auto boundTy = AnFunctionType::get(fnTy->retTy, {argTy}, fnTy->typeClassConstraints);

    TypeError err{"Error in for loop monomorphisation of " + fnName + ", types are "
            + anTypeToColoredStr(fnTy) + " bound to " + anTypeToColoredStr(boundTy), loc};

    auto subs = unify({{fnTy, boundTy, err}});

    c->compCtxt->insertMonomorphisationMappings(subs);

    fnTy = applyMonomorphisationBindings(fnTy, c->compCtxt->monomorphisationMappings);
    return monomorphise(c, fn, fnTy, loc);
}

AnFunctionType* getBuiltinFnType(TraitImpl *trait){
    auto argT = trait->typeArgs[0];
    vector<TraitImpl*> traits = { trait };
    vector<AnType*> args;

    if(trait->name == "Add" || trait->name == "Sub" || trait->name == "Mul" || trait->name == "Div" || trait->name == "Mod"){
        args.push_back(argT);
        args.push_back(argT);
        return AnFunctionType::get(argT, args, traits);

    }else if(trait->name == "Cmp" || trait->name == "Eq" || trait->name == "Is"){
        args.push_back(argT);
        args.push_back(argT);
        return AnFunctionType::get(AnType::getBool(), args, traits);

    }else if(trait->name == "Neg"){
        args.push_back(argT);
        return AnFunctionType::get(argT, args, traits);

    }else if(trait->name == "Cast"){
        auto retT = trait->typeArgs[1];
        args.push_back(argT);
        return AnFunctionType::get(retT, args, traits);

    }else if(trait->name == "Extract"){
        auto uszT = AnType::getUsz();
        auto ptrT = try_cast<AnPtrType>(argT);
        args.push_back(ptrT);
        args.push_back(uszT);
        return AnFunctionType::get(ptrT->extTy, args, traits);

    }else if(trait->name == "Insert"){
        auto uszT = AnType::getUsz();
        auto ptrT = try_cast<AnPtrType>(argT);
        args.push_back(ptrT);
        args.push_back(uszT);
        args.push_back(ptrT->extTy);
        return AnFunctionType::get(AnType::getUnit(), args, traits);

    }else if(trait->name == "Deref"){
        auto ptrT = try_cast<AnPtrType>(argT);
        args.push_back(ptrT);
        return AnFunctionType::get(ptrT->extTy, args, traits);

    }else if(trait->name == "Not"){
        args.push_back(argT);
        return AnFunctionType::get(argT, args, traits);
    }
    ASSERT_UNREACHABLE("getBuiltinFnType called on a non-builtin trait");
}

TypedValue findBuiltinFn(Compiler *c, AnFunctionType *fnTy){
    if(fnTy->typeClassConstraints.empty()){
        return {};
    }

    TraitImpl *trait = fnTy->typeClassConstraints.front();
    if(!trait->hasTrivialImpl()){
        return {};
    }

    const auto &traitName = trait->name;
    const auto &typeArgs = trait->typeArgs;

    AnFunctionType *type = getBuiltinFnType(trait);
    FunctionType *ft = dyn_cast<FunctionType>(c->anTypeToLlvmType(type)->getPointerElementType());

    Function *f = Function::Create(ft, Function::ExternalLinkage, "Builtin", c->module.get());
    BasicBlock *oldInsertBlock = c->builder.GetInsertBlock();
    BasicBlock *newInsertBlock = BasicBlock::Create(*c->ctxt, "entry", f);
    c->builder.SetInsertPoint(newInsertBlock);

    Value *ret = nullptr;
    std::array<Value*,3> args = {
        f->args().begin(),
        f->args().begin() + 1,
        f->args().begin() + 2
    };

    int i = 0;
    for(auto &arg : f->args()){
        if(type->paramTys[i]->hasModifier(Tok_Mut)){
            args[i] = c->builder.CreateLoad(&arg);
        }
        i++;
    }

    if(traitName == "Add" || traitName == "Sub" || traitName == "Mul" || traitName == "Div" || traitName == "Mod" || traitName == "Cmp" || traitName == "Neg"){
        ret = c->builder.CreateAdd(args[0], args[1]);

    }else if(traitName == "Cast"){
        if(typeArgs[0]->typeTag == TT_Ptr && type->retTy->typeTag == TT_Ptr){
            ret = c->builder.CreateBitCast(args[0], f->getReturnType());
        }else if(typeArgs[0]->typeTag == TT_Ptr && isIntTypeTag(type->retTy->typeTag)){
            ret = c->builder.CreatePtrToInt(args[0], f->getReturnType());
        }else if(isIntTypeTag(typeArgs[0]->typeTag) && type->retTy->typeTag == TT_Ptr){
            ret = c->builder.CreateIntToPtr(args[0], f->getReturnType());
        }else if(isIntTypeTag(typeArgs[0]->typeTag) && isIntTypeTag(type->retTy->typeTag)){
            ret = c->builder.CreateIntCast(args[0], f->getReturnType(), isUnsignedTypeTag(typeArgs[0]->typeTag));
        }else{
            typeArgs[0]->dump();
            type->retTy->dump();
            ASSERT_UNREACHABLE("Invalid types passed to Cast primitive");
        }

    }else if(traitName == "Eq" || traitName == "Is"){
        if(isFPTypeTag(typeArgs[0]->typeTag)){
            ret = c->builder.CreateFCmpOEQ(args[0], args[1]);
        }else{
            ret = c->builder.CreateICmpEQ(args[0], args[1]);
        }

    }else if(traitName == "Extract"){
        ret = c->builder.CreateLoad(c->builder.CreateGEP(args[0], args[1]));

    }else if(traitName == "Insert"){
        c->builder.CreateStore(args[2], c->builder.CreateGEP(args[0], args[1]));

    }else if(traitName == "Deref"){
        ret = c->builder.CreateLoad(args[0]);

    }else if(traitName == "Not"){
        ret = c->builder.CreateNot(args[0]);
    }else{
        ASSERT_UNREACHABLE("Called findBuiltinFn on non-builtin trait");
    }

    if(ret){
        c->builder.CreateRet(ret);
    }else{
        c->builder.CreateRetVoid();
    }
    c->builder.SetInsertPoint(oldInsertBlock);
    return {f, type};
}

TypedValue getFunction(Compiler *c, BinOpNode *bop){
    auto vn = dynamic_cast<VarNode*>(bop->lval.get());
    Declaration *decl = bop->decl;
    if(decl->tval.val){
        return decl->tval;
    }else if(decl->isTraitFuncDecl()){
        auto fnTy = try_cast<AnFunctionType>(bop->lval->getType());
        fnTy = applyMonomorphisationBindings(fnTy, c->compCtxt->monomorphisationMappings);
        if(TypedValue f = findBuiltinFn(c, fnTy)){
            return f;
        }

        attachTraitImpl(decl, fnTy, c->compUnit, bop->loc);
        TraitImpl *trait = fnTy->typeClassConstraints.front();
        Declaration *fn = findFnInImpl(static_cast<VarNode*>(bop->lval.get())->name, trait);
        return monomorphise(c, static_cast<FuncDecl*>(fn), fnTy, bop->loc);
    }else if(decl->isFuncDecl()){
        auto fnTy = try_cast<AnFunctionType>(bop->lval->getType());
        return monomorphise(c, static_cast<FuncDecl*>(decl), fnTy, bop->loc);
    }else{
        return CompilingVisitor::compile(c, decl->definition);
    }
}

/*
 *  Compiles the member access operator, .  eg. struct.field
 */
TypedValue Compiler::compMemberAccess(Node *ln, VarNode *field, BinOpNode *binop){
    if(!ln) throw CtError();

    if(binop->decl){
        if(binop->decl->tval.type->typeTag == TT_Function){
            return ante::getFunction(this, binop);
        }else{
            return binop->decl->tval;
        }
    }

    Value *val;
    AnType *ltyn;
    AnType *tyn;

    //prevent l from being used after this scope; only val and tyn should be used as only they
    //are updated with the automatic pointer dereferences.
    {
        auto l = CompilingVisitor::compile(this, ln);
        if(!l) return {};

        val = l.val;
        tyn = ltyn = l.type;
    }

    //the . operator automatically dereferences pointers, so update val and tyn accordingly.
    while(tyn->typeTag == TT_Ptr){
        val = builder.CreateLoad(val);
        tyn = try_cast<AnPtrType>(tyn)->extTy;
    }

    //check to see if this is a field index
    if(auto *dataTy = try_cast<AnProductType>(tyn)){
        auto index = dataTy->getFieldIndex(field->name);

        if(index != -1){
            AnType *retTy = dataTy->fields[index];

            if(!dataTy->llvmType){
                updateLlvmTypeBinding(this, dataTy);
            }

            retTy = (AnType*)tyn->addModifiersTo(retTy);

            //If dataTy is a single value tuple then val may not be a tuple at all. In this
            //case, val should be returned without being extracted from a nonexistant tuple
            if(index == 0 && !val->getType()->isStructTy())
                return TypedValue(val, retTy);

            auto ev = builder.CreateExtractValue(val, index);
            auto ret = TypedValue(ev, retTy);
            return ret;
        }
    }
    ASSERT_UNREACHABLE();
}


template<typename T>
void push_front(vector<T> &vec, T val){
    auto cpy = vecOf<T>(vec.size() + 1);
    cpy.push_back(val);

    for(auto &v : vec)
        cpy.push_back(v);

    vec = cpy;
}


vector<AnType*> toAnTypeVector(vector<TypedValue> &tvs){
    auto ret = vecOf<AnType*>(tvs.size());
    for(const auto &tv : tvs){
        ret.push_back(tv.type);
    }
    return ret;
}


string getName(Node *n){
    if(VarNode *vn = dynamic_cast<VarNode*>(n))
        return vn->name;
    else if(BinOpNode *op = dynamic_cast<BinOpNode*>(n))
        return getName(op->lval.get()) + "_" + getName(op->rval.get());
    else if(TypeNode *tn = dynamic_cast<TypeNode*>(n))
        return tn->params.empty() ? typeNodeToStr(tn) : tn->typeName;
    else
        return "";
}

#ifdef _WIN32
void* lookupCFn(string name){
    static map<string,void*> fnMap = {
        {"printf",  (void*)printf},
        {"puts",    (void*)puts},
        {"putchar", (void*)putchar},
        {"getchar", (void*)getchar},
        {"exit",    (void*)exit},
        {"malloc",  (void*)malloc},
        {"realloc", (void*)realloc},
        {"free",    (void*)free},
        {"memcpy",  (void*)memcpy},
        {"system",  (void*)system},
        {"strlen",  (void*)strlen},
        {"fopen",   (void*)fopen},
        {"fclose",  (void*)fclose},
        {"fputs",   (void*)fputs},
        {"fputc",   (void*)fputc},
        {"fgetc",   (void*)fgetc},
        {"fgets",   (void*)fgets},
        {"ungetc",  (void*)ungetc},
        {"fgetpos", (void*)fgetpos},
        {"ftell",   (void*)ftell},
        {"fsetpos", (void*)fsetpos},
        {"fseek",   (void*)fseek},
        {"feof",    (void*)feof},
        {"ferror",  (void*)ferror}
    };

    return fnMap[name];
}
#endif


TypedValue createMallocAndStore(Compiler *c, TypedValue &val){
    array<Type*, 1> args{Type::getIntNTy(*c->ctxt, AN_USZ_SIZE)};
    auto *mallocTy = FunctionType::get(Type::getIntNPtrTy(*c->ctxt, 8), args, false);
    Function* mallocFn = Function::Create(mallocTy, Function::ExternalLinkage, "malloc");

    auto size_result = val.type->getSizeInBits(c);
    if(!size_result){
        cerr << size_result.getErr() << endl;
        size_result = 0;
    }
    auto size = size_result.getVal() / 8;

    Value *sizeVal = ConstantInt::get(*c->ctxt, APInt(AN_USZ_SIZE, size, true));

    Value *voidPtr = c->builder.CreateCall(mallocFn, sizeVal);
    Type *ptrTy = val.getType()->getPointerTo();
    Value *typedPtr = c->builder.CreatePointerCast(voidPtr, ptrTy);

    //finally store val1 into the malloc'd slot
    c->builder.CreateStore(val.val, typedPtr);

    auto *tyn = AnPtrType::get(val.type);
    return TypedValue(typedPtr, tyn);
}


/*
 * Unwrap the single i8* argument given to AnteCall into a vector of each value the
 * function it should call requires.
 */
vector<Value*> unwrapVoidPtrArgs(Compiler *c, Value *anteCallArg, vector<TypedValue> const& typedArgs, FuncDecl *fd){
    vector<Value*> ret;
    bool varargs = cast<Function>(fd->tval.val)->isVarArg();

    auto *fnTy = cast<Function>(fd->tval.val)->getFunctionType();
    if(fnTy->getNumParams() == 0 && !varargs) return ret;

    size_t argc = fnTy->getNumParams();
    for(size_t i = 0; i < argc or (varargs && i < typedArgs.size()); i++){
        llvm::Type *castTy = varargs ?
            typedArgs[i].getType()->getPointerTo() :
            fnTy->getParamType(i)->getPointerTo();

        Value *cast = c->builder.CreateBitCast(anteCallArg, castTy);
        ret.push_back(c->builder.CreateLoad(cast));

        if(i != argc - 1 or (varargs && i != typedArgs.size() - 1))
            anteCallArg = c->builder.CreateInBoundsGEP(cast, c->builder.getInt64(1));
    }

    return ret;
}


/**
 * Creates a function AnteCall that unpacks the given arguments from a void*,
 * and returns the result of a call to the given FuncDecl with those arguments.
 *
 * AnteCall has the type 't*->'u* where 't is a tuple of fd's parameter types (to
 * be unpacked within AnteCall) and 'u is the return type of fd.
 */
void createDriverFunction(Compiler *c, FuncDecl *fd, vector<TypedValue> const& typedArgs){
    Type *voidPtrTy = Type::getInt8Ty(*c->ctxt)->getPointerTo();
    FunctionType *fnTy = FunctionType::get(voidPtrTy, voidPtrTy, false);

    //preFn is the predecessor to fn because we do not yet know its return type, so its body must be compiled,
    //then the type must be checked and the new function with correct return type created, and their bodies swapped.
    Function *fn = Function::Create(fnTy, Function::ExternalLinkage, "AnteCall", c->module.get());
    BasicBlock *entry = BasicBlock::Create(*c->ctxt, "entry", fn);
    c->builder.SetInsertPoint(entry);

    auto *fnArg1 = fn->arg_begin();
    auto args = unwrapVoidPtrArgs(c, fnArg1, typedArgs, fd);

    Value *call = c->builder.CreateCall(fd->tval.val, args);
    AnType *retTy = fd->tval.type->getFunctionReturnType();
    if(retTy->typeTag == TT_Unit){
        c->builder.CreateRetVoid();
    }else{
        auto callTv = TypedValue(call, fd->tval.type->getFunctionReturnType());

        auto store = createMallocAndStore(c, callTv);
        auto ret = c->builder.CreateBitCast(store.val, voidPtrTy);
        c->builder.CreateRet(ret);
    }
}

void createDriverFunction(Compiler *c, Function *f, AnType *retTy){
    if(f->arg_size() != 0){
        cerr << "createDriverFunction(Compiler*, Function*, AnType*) can only be used if the function takes no arguments\n"; 
        exit(1);
    }

    Type *voidPtrTy = Type::getInt8Ty(*c->ctxt)->getPointerTo();
    FunctionType *fnTy = FunctionType::get(voidPtrTy, {}, false);

    Function *fn = Function::Create(fnTy, Function::ExternalLinkage, "AnteCall", c->module.get());
    BasicBlock *entry = BasicBlock::Create(*c->ctxt, "entry", fn);
    c->builder.SetInsertPoint(entry);

    Value *call = c->builder.CreateCall(f, {});

    if(retTy->typeTag == TT_Unit){
        c->builder.CreateRetVoid();
    }else{
        auto callTv = TypedValue(call, retTy);

        auto store = createMallocAndStore(c, callTv);
        auto ret = c->builder.CreateBitCast(store.val, voidPtrTy);
        c->builder.CreateRet(ret);
    }
}

/**
 * Creates an empty nullary function
 */
Function* createFunctionShell(Compiler *c){
    auto *fnty = FunctionType::get(Type::getVoidTy(*c->ctxt), {}, false);
    llvm::Function *f = Function::Create(fnty, Function::LinkageTypes::PrivateLinkage, "EmptyShell", c->module.get());
    BasicBlock::Create(*c->ctxt, "entry", f);
    return f;
}


/**
 * TODO: should fail when tracing mutable variables with stores
 *       that are dependent on previous stores, eg.
 *
 *  mut a = 1
 *  a += 2
 *  ante a    => gives dependency {a = a + 2} but not previous assignment of a
 *
 *  also fails to remember a var is mutable
 */
vector<tuple<string, AnType*, Node*>> traceDependenciesOfAnteExpr(Compiler *c, Node *anteExpr){
    vector<Node*> deps;

    AnteVisitor a{c};
    anteExpr->accept(a);

    return a.dependencies;
}


void insertDependencies(CompilingVisitor &cv, Function *f,
        vector<tuple<string, AnType*, Node*>> const& deps){

    BasicBlock *oldBlock = cv.c->builder.GetInsertBlock();
    cv.c->builder.SetInsertPoint(&f->getBasicBlockList().back());

    //compile a let-binding for each dependency
    for(auto &tup : deps){
        auto   &name = get<0>(tup);
        AnType *type = get<1>(tup);
        Node   *expr = get<2>(tup);

        Compiler *c = cv.c;
        TypedValue val = CompilingVisitor::compile(c, expr);

        TypedValue result;
        if(type->hasModifier(Tok_Mut)){
            Value *alloca = c->builder.CreateAlloca(val.getType(), nullptr, name);
            val.val = c->builder.CreateStore(val.val, alloca);
            result = {alloca, type};
        }else{
            result = val;
        }

        //TODO: re-add
        //Assignment assignment{Assignment::Normal, expr};
        //c->stoVar(name, new Variable(name, result, c->scope, assignment, type->hasModifier(Tok_Mut)));
        cv.val.val = val.val;
        cv.val.type = type;
    }

    cv.c->builder.SetInsertPoint(oldBlock);
}


pair<Function*, AnType*> compileShellFunction(CompilingVisitor &cv, Function *f, Node *expr){
    BasicBlock *oldBlock = cv.c->builder.GetInsertBlock();
    cv.c->builder.SetInsertPoint(&f->getBasicBlockList().back());

    expr->accept(cv);

    // error in repl, caught by RootNode but must be handled again
    if(!cv.val.val){
        cv.val = cv.c->getUnitLiteral();
    }

    if(!dyn_cast<ReturnInst>(cv.val.val)){
        if(cv.val.type->typeTag == TT_Unit)
            cv.c->builder.CreateRetVoid();
        else
            cv.c->builder.CreateRet(cv.val.val);
    }

    auto *fnty = FunctionType::get(cv.c->anTypeToLlvmType(cv.val.type), {}, false);
    llvm::Function *fFinal = Function::Create(fnty, Function::LinkageTypes::PrivateLinkage, "Shell", cv.c->module.get());
    moveFunctionBody(f, fFinal);
    f->eraseFromParent();

    cv.c->builder.SetInsertPoint(oldBlock);
    return {fFinal, cv.val.type};
}


TypedValue callAnteFunction(Compiler *c, Function *main, BasicBlock *originalInsertPoint,
        vector<TypedValue> const& typedArgs, vector<unique_ptr<Node>> const& argExprs,
        unique_ptr<orc::LLLazyJIT> &jit, AnType *retTy){

    auto symbol = jit->lookup("AnteCall").get().getAddress();

    if(symbol){
        void *res;
        if(typedArgs.empty()){
            auto fn = (void*(*)())symbol;
            res = fn();
        }else{
            auto fn = (void*(*)(void*))symbol;
            auto arg = AnteValue(c, typedArgs, argExprs);
            res = fn(arg.asRawData());
        }
        return AnteValue(res, retTy).asTypedValue(c);
    }else{
        LOC_TY loc;
        if(!argExprs.empty()) loc = argExprs[0]->loc;
        error("Could not find entry symbol while JITing ante function", loc);
        return c->getUnitLiteral(); //unreachable
    }
}


template<typename T>
pair<Function*, AnType*> compileAnonAnteFunction(CompilingVisitor &cv, ModNode *n, T &&cleanup){
    TMP_SET(cv.c->isJIT, true);
    auto shell = createFunctionShell(cv.c);
    try{
        auto deps = traceDependenciesOfAnteExpr(cv.c, n->expr.get());
        insertDependencies(cv, shell, deps);
        return compileShellFunction(cv, shell, n->expr.get());
    }catch(...){
        //error tracing dependencies, need to cleanup callstack before unwinding
        cleanup();
        throw;
    }
}


/*
FuncDecl* compileAnteFunction(Compiler *c, string const& baseName, string const& mangledName,
        vector<TypedValue> const& typedArgs){

    FuncDecl *ret;
    TMP_SET(c->isJIT, true);
    ret = c->getFuncDecl(baseName, mangledName);
    auto argTys = toTypeVector(typedArgs);
    compFnWithArgs(c, ret, argTys);
    return ret;
}



TypedValue compileAndCallAnteFunction(Compiler *c, ModNode *n){
    CompilingVisitor cv{c};

    //and will crash llvm if we try to clone it without a ReturnInst
    auto mainFnName = "main";
    auto main = c->module->getFunction(mainFnName);
    if(main){
        main->removeFromParent();
    }

    auto originalInsertPoint = c->builder.GetInsertBlock();

    auto cleanup = [&]{
        if(main)
            c->module->getFunctionList().push_front(main);
        if(auto f1 = c->module->getFunction("AnteCall")) f1->eraseFromParent();
        if(auto f2 = c->module->getFunction("EmptyShell")) f2->eraseFromParent();
        c->builder.SetInsertPoint(originalInsertPoint);
    };

    //compile ante function and a driver to run it
    auto shellAndType = compileAnonAnteFunction(cv, n, cleanup);
    auto shellName = shellAndType.first->getName();

    createDriverFunction(c, shellAndType.first, shellAndType.second);

    auto clone = llvm::CloneModule(*c->module.get());
    auto tsm = orc::ThreadSafeModule(move(clone), std::unique_ptr<LLVMContext>(c->ctxt.get()));

    auto triple = Triple(AN_NATIVE_ARCH, AN_NATIVE_VENDOR, AN_NATIVE_OS);
    DataLayout dl{clone.get()};
    orc::JITTargetMachineBuilder b{triple};

    auto &jit = orc::LLLazyJIT::Create(b, dl).get();

    jit->addLazyIRModule(move(tsm)).success();

    cleanup();
    c->module->getFunction(shellName)->removeFromParent();
    return callAnteFunction(c, main, originalInsertPoint, {}, {}, jit, shellAndType.second);
}

TypedValue compileAndCallAnteFunction(Compiler *c, string const& baseName,
        string const& mangledName, vector<TypedValue> const& typedArgs,
        vector<unique_ptr<Node>> const& argExprs){

    //temporarily remove main function since it is unfinished
    //and will crash llvm if we try to clone it without a ReturnInst
    auto mainFnName = "main";
    auto main = c->module->getFunction(mainFnName);
    if(main){
        main->removeFromParent();
    }

    auto originalInsertPoint = c->builder.GetInsertBlock();

    //compile ante function and a driver to run it
    FuncDecl *fd = compileAnteFunction(c, baseName, mangledName, typedArgs);

    createDriverFunction(c, fd, typedArgs);
    auto *retTy = fd->tval.type->getFunctionReturnType();

    auto clone = llvm::CloneModule(*c->module);
    auto tsm = orc::ThreadSafeModule(move(clone), std::unique_ptr<LLVMContext>(c->ctxt.get()));

    auto triple = Triple(AN_NATIVE_ARCH, AN_NATIVE_VENDOR, AN_NATIVE_OS);
    DataLayout dl{clone.get()};
    orc::JITTargetMachineBuilder b{triple};

    auto &jit = orc::LLLazyJIT::Create(b, dl).get();

    jit->addLazyIRModule(move(tsm)).success();

    if(main){
        c->module->getFunctionList().push_front(main);
    }

    c->module->getFunction("AnteCall")->eraseFromParent();
    c->builder.SetInsertPoint(originalInsertPoint);
    return callAnteFunction(c, main, originalInsertPoint, typedArgs, argExprs, jit, retTy);
}
*/

/*
 *  Compile a compile-time function/macro which should not return a function call,
 *  just a compile-time constant.
 *
 *  Ex: A call to Ante.getAST() would be a meta function as it wouldn't make sense
 *      to get the parse tree during runtime.
 *
 *  - Assumes arguments are already type-checked
 *
TypedValue compMetaFunctionResult(Compiler *c, LOC_TY const& loc, string const& baseName,
        string const& mangledName, vector<TypedValue> const& ta, vector<unique_ptr<Node>> const& argExprs){

    capi::CtFunc* fn = capi::lookup(baseName);

    //fn not found, this is a user-defined ante function
    if(!fn){
        return compileAndCallAnteFunction(c, baseName, mangledName, ta, argExprs);
    }

    if(ta.size() != fn->params.size())
        error("Called function was given " + to_string(ta.size()) + " argument"
            + plural(ta.size()) + " but was declared to take " + to_string(fn->params.size()), loc);

    using A = AnteValue;

    TypedValue *res;
    switch(fn->params.size()){
        case 0: res = (*fn)(c); break;
        case 1: res = (*fn)(c, A(c, ta[0], argExprs[0])); break;
        case 2: res = (*fn)(c, A(c, ta[0], argExprs[0]), A(c, ta[1], argExprs[1])); break;
        case 3: res = (*fn)(c, A(c, ta[0], argExprs[0]), A(c, ta[1], argExprs[1]), A(c, ta[2], argExprs[2])); break;
        case 4: res = (*fn)(c, A(c, ta[0], argExprs[0]), A(c, ta[1], argExprs[1]), A(c, ta[2], argExprs[2]), A(c, ta[3], argExprs[3])); break;
        case 5: res = (*fn)(c, A(c, ta[0], argExprs[0]), A(c, ta[1], argExprs[1]), A(c, ta[2], argExprs[2]), A(c, ta[3], argExprs[3]), A(c, ta[4], argExprs[4])); break;
        case 6: res = (*fn)(c, A(c, ta[0], argExprs[0]), A(c, ta[1], argExprs[1]), A(c, ta[2], argExprs[2]), A(c, ta[3], argExprs[3]), A(c, ta[4], argExprs[4]), A(c, ta[5], argExprs[5])); break;
        default:
            cerr << "CtFuncs with more than 6 parameters are unimplemented." << endl;
            return {};
    }

    if(res){
        TypedValue ret = *res;
        delete res;
        return ret;
    }else{
        return c->getVoidLiteral();
    }
}
*/


bool isUnsizedType(Type *t){
    return t->isArrayTy();
}


//Computes the address of operator &
//
//Returns a TypedValue that is a reference to the given tv.
//If the given tv is not mutable and does not have an existing
//reference one is created on the stack.
TypedValue addrOf(Compiler *c, TypedValue &tv){
    auto *ptrTy = AnPtrType::get(tv.type);

    if(LoadInst* li = dyn_cast<LoadInst>(tv.val)){
        return TypedValue(li->getPointerOperand(), ptrTy);
    }else if(ExtractValueInst *evi = dyn_cast<ExtractValueInst>(tv.val)){
        Value *agg = evi->getAggregateOperand();
        size_t index = evi->getIndices()[0];
        if(LoadInst *li = dyn_cast<LoadInst>(agg)){
            return TypedValue(c->builder.CreateStructGEP(agg->getType(),
                        li->getPointerOperand(), index), ptrTy);
        }
    }
    //if it is not stack-allocated already, allocate it on the stack
    auto *alloca = c->builder.CreateAlloca(tv.getType());
    c->builder.CreateStore(tv.val, alloca);
    return TypedValue(alloca, ptrTy);
}


Value* Compiler::tupleOf(vector<Value*> const& elems, bool packed){
    vector<int> nonConstIndices;
    auto constVals = vecOf<Constant*>(elems.size());

    for(size_t i = 0; i < elems.size(); i++){
        if(Constant *con = dyn_cast<Constant>(elems[i])){
            constVals.push_back(con);
        }else{
            constVals.push_back(UndefValue::get(elems[i]->getType()));
            nonConstIndices.push_back(i);
        }
    }

    Value* tuple = ConstantStruct::getAnon(constVals, packed);

    for(int i : nonConstIndices){
        tuple = builder.CreateInsertValue(tuple, elems[i], i);
    }
    return tuple;
}


Value* Compiler::ptrTo(void* val){
    auto *cint = builder.getIntN(AN_USZ_SIZE, (size_t)val);
    Type *ptrTy = Type::getInt8Ty(*ctxt)->getPointerTo();
    return builder.CreateIntToPtr(cint, ptrTy);
}


vector<Value*> adaptArgsToCompilerAPIFn(Compiler *c, vector<Value*> &args, vector<TypedValue> &typedArgs){
    auto ret = vecOf<Value*>(args.size() + 1);

    //Compiler API functions take an implicit Compiler* parameter
    Value *cArg = c->ptrTo(c);
    ret.push_back(cArg);

    int i = 0;
    for(auto *val : args){
        auto valref = c->builder.CreateAlloca(val->getType());
        c->builder.CreateStore(val, valref);
        auto valTy = c->ptrTo(typedArgs[i++].type);

        auto arg = c->tupleOf({valref, valTy}, true);
        auto argref = c->builder.CreateAlloca(arg->getType());
        c->builder.CreateStore(arg, argref);
        ret.push_back(argref);
    }
    return ret;
}

TypedValue handleAnteFn(Compiler *c, BinOpNode *bop, vector<TypedValue> &typedArgs){
    if(bop->decl->name == "sizeof" && typedArgs.size() == 1){
        auto dt = try_cast<AnDataType>(typedArgs[0].type);
        AnType *t = (dt && dt->name == "Type") ? dt->typeArgs[0] : typedArgs[0].type;

        auto size = t->getSizeInBits(c);
        Value *sizeVal = c->builder.getIntN(AN_USZ_SIZE, size.getVal() / 8);
        return {sizeVal, AnType::getUsz()};
    }
    return {};
}


TypedValue compFnCall(Compiler *c, BinOpNode *bop){
    //used to type-check each parameter later
    vector<TypedValue> typedArgs;
    vector<Value*> args;

    Node *l = bop->lval.get();
    Node *r = bop->rval.get();

    //add all remaining arguments
    if(auto *tup = dynamic_cast<TupleNode*>(r)){
        typedArgs = tup->unpack(c);

        for(TypedValue v : typedArgs){
            auto arg = v;
            if(isEmptyType(c, arg.type))
                continue;

            if(isUnsizedType(arg.getType()))
                arg = addrOf(c, arg);

            args.push_back(arg.val);
        }
    }else{ //single parameter being applied
        auto param = CompilingVisitor::compile(c, r);
        if(!param) return param;

        if(!isEmptyType(c, param.type)){
            auto arg = param;

            if(isUnsizedType(arg.getType()))
                arg = addrOf(c, arg);

            typedArgs.push_back(arg);
            args.push_back(arg.val);
        }
    }

    auto ctval = handleAnteFn(c, bop, typedArgs);
    if(ctval) return ctval;

    //try to compile the function now that the parameters are compiled.
    TypedValue tvf = getFunction(c, bop);

    /*
     * TODO: re-add
    if(!!funcs->obj){
        push_front(args, funcs->obj.val);
        is_method = true;
    }
    */

    if(!tvf)
        error("Unknown error when attempting to call function", l->loc);

    //now that we assured it is a function, unwrap it
    AnFunctionType *fty = try_cast<AnFunctionType>(tvf.type);

    //type check each parameter
    size_t argc = fty->paramTys.size();
    for(size_t i = 0; i < argc; i++){
        TypedValue tArg = typedArgs[i];
        AnType *paramTy = fty->paramTys[i];

        if(i >= typedArgs.size())
            break;

        if(isEmptyType(c, tArg.type)){
            continue;
        }

        if(!paramTy) break;

        //Mutable parameters are implicitly passed by reference
        //
        //Note that by getting the address of tArg (and not args[i-1])
        //any previous implicit references (like from the passing of an array type)
        //are not applied so no implicit references to references accidentally occur
        if(paramTy->hasModifier(Tok_Mut)){
            args[i] = addrOf(c, tArg).val;
        }

        if(tvf.val && args[i]->getType() != tvf.getType()->getPointerElementType()->getFunctionParamType(i) && paramTy->typeTag == TT_Ptr){
            args[i] = c->builder.CreateBitCast(args[i], tvf.getType()->getPointerElementType()->getFunctionParamType(i));
        }
    }

    //if tvf is a ante function or similar MetaFunction, then compile it in a separate
    //module and JIT it instead of creating a call instruction
    /*if(isCompileTimeFunction(tvf)){
        if(c->isJIT && tvf.type->typeTag == TT_MetaFunction){
            args = adaptArgsToCompilerAPIFn(c, args, typedArgs);
        }else{
            string baseName = getName(l);
            auto *fnty = try_cast<AnFunctionType>(tvf.type);
            string mangledName = mangle(baseName, fnty->extTys);
            if(auto *tup = dynamic_cast<TupleNode*>(r)){
                return compMetaFunctionResult(c, l->loc, baseName, mangledName, typedArgs, tup->exprs);
            }else{
                vector<unique_ptr<Node>> anteExpr;
                anteExpr.emplace_back(r);
                auto res = compMetaFunctionResult(c, l->loc, baseName, mangledName, typedArgs, {anteExpr});
                anteExpr[0].release();
                return res;
            }
        }
    }*/

    //Create the call to tvf.val, not f as if tvf is a function pointer,
    //passing it as f will fail.
    auto *call = c->builder.CreateCall(tvf.val, args);
    return TypedValue(call, tvf.type->getFunctionReturnType());
}

TypedValue Compiler::compLogicalOr(Node *lexpr, Node *rexpr, BinOpNode *op){
    Function *f = builder.GetInsertBlock()->getParent();
    auto &blocks = f->getBasicBlockList();

    auto lhs = CompilingVisitor::compile(this, lexpr);
    if(lhs.type->typeTag != TT_Bool)
        error("The 'or' operator's lval must be of type bool, but instead is of type "
                + anTypeToColoredStr(lhs.type), op->lval->loc);

    auto *curbbl = builder.GetInsertBlock();
    auto *orbb = BasicBlock::Create(*ctxt, "or");
    auto *mergebb = BasicBlock::Create(*ctxt, "merge");

    builder.CreateCondBr(lhs.val, mergebb, orbb);
    blocks.push_back(orbb);
    blocks.push_back(mergebb);


    builder.SetInsertPoint(orbb);
    auto rhs = CompilingVisitor::compile(this, rexpr);

    //the block must be re-gotten in case the expression contains if-exprs, while nodes,
    //or other exprs that change the current block
    auto *curbbr = builder.GetInsertBlock();
    builder.CreateBr(mergebb);

    if(rhs.type->typeTag != TT_Bool)
        error("The 'or' operator's rval must be of type bool, but instead is of type "
                + anTypeToColoredStr(rhs.type), op->rval->loc);

    builder.SetInsertPoint(mergebb);
    auto *phi = builder.CreatePHI(rhs.getType(), 2);

    //short circuit, returning true if return from the first label
    phi->addIncoming(ConstantInt::get(*ctxt, APInt(1, true, true)), curbbl);
    phi->addIncoming(rhs.val, curbbr);

    return TypedValue(phi, rhs.type);

}

TypedValue Compiler::compLogicalAnd(Node *lexpr, Node *rexpr, BinOpNode *op){
    Function *f = builder.GetInsertBlock()->getParent();
    auto &blocks = f->getBasicBlockList();

    auto lhs = CompilingVisitor::compile(this, lexpr);
    if(lhs.type->typeTag != TT_Bool)
        error("The 'and' operator's lval must be of type bool, but instead is of type "
                + anTypeToColoredStr(lhs.type), op->lval->loc);

    auto *curbbl = builder.GetInsertBlock();
    auto *andbb = BasicBlock::Create(*ctxt, "and");
    auto *mergebb = BasicBlock::Create(*ctxt, "merge");

    builder.CreateCondBr(lhs.val, andbb, mergebb);
    blocks.push_back(andbb);
    blocks.push_back(mergebb);


    builder.SetInsertPoint(andbb);
    auto rhs = CompilingVisitor::compile(this, rexpr);

    //the block must be re-gotten in case the expression contains if-exprs, while nodes,
    //or other exprs that change the current block
    auto *curbbr = builder.GetInsertBlock();
    builder.CreateBr(mergebb);

    if(rhs.type->typeTag != TT_Bool)
        error("The 'and' operator's rval must be of type bool, but instead is of type "
                + anTypeToColoredStr(rhs.type), op->rval->loc);

    builder.SetInsertPoint(mergebb);
    auto *phi = builder.CreatePHI(rhs.getType(), 2);

    //short circuit, returning false if return from the first label
    phi->addIncoming(ConstantInt::get(*ctxt, APInt(1, false, true)), curbbl);
    phi->addIncoming(rhs.val, curbbr);

    return TypedValue(phi, rhs.type);
}


TypedValue handlePrimitiveNumericOp(BinOpNode *bop, Compiler *c, TypedValue &lhs, TypedValue &rhs){
    switch(bop->op){
        case '+': return c->compAdd(lhs, rhs, bop);
        case '-': return c->compSub(lhs, rhs, bop);
        case '*': return c->compMul(lhs, rhs, bop);
        case '/': return c->compDiv(lhs, rhs, bop);
        case '%': return c->compRem(lhs, rhs, bop);
        case '<':
                    if(isFPTypeTag(lhs.type->typeTag))
                        return TypedValue(c->builder.CreateFCmpOLT(lhs.val, rhs.val), AnType::getBool());
                    else if(isUnsignedTypeTag(lhs.type->typeTag))
                        return TypedValue(c->builder.CreateICmpULT(lhs.val, rhs.val), AnType::getBool());
                    else
                        return TypedValue(c->builder.CreateICmpSLT(lhs.val, rhs.val), AnType::getBool());
        case '>':
                    if(isFPTypeTag(lhs.type->typeTag))
                        return TypedValue(c->builder.CreateFCmpOGT(lhs.val, rhs.val), AnType::getBool());
                    else if(isUnsignedTypeTag(lhs.type->typeTag))
                        return TypedValue(c->builder.CreateICmpUGT(lhs.val, rhs.val), AnType::getBool());
                    else
                        return TypedValue(c->builder.CreateICmpSGT(lhs.val, rhs.val), AnType::getBool());
        case Tok_EqEq:
        case Tok_Is:
                    if(isFPTypeTag(lhs.type->typeTag))
                        return TypedValue(c->builder.CreateFCmpOEQ(lhs.val, rhs.val), AnType::getBool());
                    else
                        return TypedValue(c->builder.CreateICmpEQ(lhs.val, rhs.val), AnType::getBool());
        case Tok_NotEq:
        case Tok_Isnt:
                    if(isFPTypeTag(lhs.type->typeTag))
                        return TypedValue(c->builder.CreateFCmpONE(lhs.val, rhs.val), AnType::getBool());
                    else
                        return TypedValue(c->builder.CreateICmpNE(lhs.val, rhs.val), AnType::getBool());
        case Tok_LesrEq:
                    if(isFPTypeTag(lhs.type->typeTag))
                        return TypedValue(c->builder.CreateFCmpOLE(lhs.val, rhs.val), AnType::getBool());
                    else if(isUnsignedTypeTag(lhs.type->typeTag))
                        return TypedValue(c->builder.CreateICmpULE(lhs.val, rhs.val), AnType::getBool());
                    else
                        return TypedValue(c->builder.CreateICmpSLE(lhs.val, rhs.val), AnType::getBool());
        case Tok_GrtrEq:
                    if(isFPTypeTag(lhs.type->typeTag))
                        return TypedValue(c->builder.CreateFCmpOGE(lhs.val, rhs.val), AnType::getBool());
                    else if(isUnsignedTypeTag(lhs.type->typeTag))
                        return TypedValue(c->builder.CreateICmpUGE(lhs.val, rhs.val), AnType::getBool());
                    else
                        return TypedValue(c->builder.CreateICmpSGE(lhs.val, rhs.val), AnType::getBool());
        default:
            error("Operator " + Lexer::getTokStr(bop->op) + " is not overloaded for types "
                   + anTypeToColoredStr(lhs.type) + " and " + anTypeToColoredStr(rhs.type), bop->loc);
            return {};
    }
}


void CompilingVisitor::visit(SeqNode *n){
    for(auto &node : n->sequence){
        try{
            node->accept(*this);
        }catch(CtError const& e){
            //Unless the final value throws, delete the error
            if(&node == &n->sequence.back()) throw e;
        }
    }
}


TypedValue handlePointerOffset(BinOpNode *n, Compiler *c, TypedValue &lhs, TypedValue &rhs){
    Value *ptr;
    Value *idx;
    AnType *ptrTy;

    if(lhs.type->typeTag == TT_Ptr && rhs.type->typeTag != TT_Ptr){
        ptr = lhs.val;
        idx = rhs.val;
        ptrTy = lhs.type;
    }else if(lhs.type->typeTag != TT_Ptr && rhs.type->typeTag == TT_Ptr){
        ptr = rhs.val;
        idx = lhs.val;
        ptrTy = rhs.type;
    }else{
        error("Operands for pointer addition must be a pointer and an integer", n->loc);
    }

    if(n->op == '+'){
        return {c->builder.CreateInBoundsGEP(ptr, idx), ptrTy};
    }else if(n->op == '-'){
        idx = c->builder.CreateNeg(idx);
        return {c->builder.CreateInBoundsGEP(ptr, idx), ptrTy};
    }else{
        error("Operator " + to_string(n->op) + " is not a primitive pointer operator", n->loc);
    }
    return {}; //unreachable
}

void handlePrimitiveOp(CompilingVisitor &cv, BinOpNode *n, TypedValue &lhs, TypedValue &rhs){
    //first, if both operands are primitive numeric types, use the default ops
    if(isNumericTypeTag(lhs.type->typeTag) && isNumericTypeTag(rhs.type->typeTag)){
        cv.val = handlePrimitiveNumericOp(n, cv.c, lhs, rhs);
        return;

    //and bools/ptrs are only compatible with == and !=
    }else if((lhs.type->typeTag == TT_Bool && rhs.type->typeTag == TT_Bool) or
             (lhs.type->typeTag == TT_Ptr && rhs.type->typeTag == TT_Ptr)){

        //= is no longer implemented for pointers by default
        if(n->op == Tok_EqEq &&lhs.type->typeTag == TT_Bool && rhs.type->typeTag == TT_Bool){
            cv.val = TypedValue(cv.c->builder.CreateICmpEQ(lhs.val, rhs.val), AnType::getBool());
            return;
        }

        if(n->op == Tok_Is){
            cv.val = TypedValue(cv.c->builder.CreateICmpEQ(lhs.val, rhs.val), AnType::getBool());
            return;
        }else if(n->op == Tok_NotEq || n->op == Tok_Isnt){
            cv.val = TypedValue(cv.c->builder.CreateICmpNE(lhs.val, rhs.val), AnType::getBool());
            return;
        }
    }

    if(n->op == '+' or n->op == '-'){
        if((lhs.type->typeTag == TT_Ptr or isNumericTypeTag(lhs.type->typeTag)) and
           (rhs.type->typeTag == TT_Ptr or isNumericTypeTag(rhs.type->typeTag))){
            cv.val = handlePointerOffset(n, cv.c, lhs, rhs);
            return;
        }
    }

    if(n->op == '#'){
        cv.val = cv.c->compExtract(lhs, rhs, n);
        return;
    }
}


bool isPrimitiveOp(BinOpNode *n, AnType *l, AnType *r){
    //first, if both operands are primitive numeric types, use the default ops
    if(isNumericTypeTag(l->typeTag) && isNumericTypeTag(r->typeTag)){
        return true;

    //and bools/ptrs are only compatible with == and !=
    }else if((l->typeTag == TT_Bool && r->typeTag == TT_Bool) or
             (l->typeTag == TT_Ptr && r->typeTag == TT_Ptr)){

        //== is no longer implemented for pointers by default
        if(n->op == Tok_EqEq && l->typeTag == TT_Bool && r->typeTag == TT_Bool){
            return true;
        }

        if(n->op == Tok_Is || n->op == Tok_NotEq || n->op == Tok_Isnt){
            return true;
        }
    }

    if(n->op == '+' || n->op == '-'){
        if((l->typeTag == TT_Ptr or isNumericTypeTag(l->typeTag)) and
           (r->typeTag == TT_Ptr or isNumericTypeTag(r->typeTag))){
            return true;
        }
    }

    return n->op == '#';
}


/*
 *  Compiles an operation along with its lhs and rhs
 */
void CompilingVisitor::visit(BinOpNode *n){
    if(n->op == '.'){
        if(dynamic_cast<IntLitNode*>(n->rval.get())){
            TypedValue l = CompilingVisitor::compile(c, n->lval);
            TypedValue r = CompilingVisitor::compile(c, n->rval);
            this->val = c->compExtract(l, r, n);
        }else{
            this->val = c->compMemberAccess(n->lval.get(), (VarNode*)n->rval.get(), n);
        }
        return;
    }else if(n->op == '('){
        this->val = compFnCall(c, n);
        return;
    }else if(n->op == Tok_And){
        this->val = c->compLogicalAnd(n->lval.get(), n->rval.get(), n);
        return;
    }else if(n->op == Tok_Or){
        this->val = c->compLogicalOr(n->lval.get(), n->rval.get(), n);
        return;
    }else if(n->op == ':'){
        n->lval->accept(*this);
        val.type = n->getType();
        return;
    }

    TypedValue lhs = CompilingVisitor::compile(c, n->lval);
    TypedValue rhs = CompilingVisitor::compile(c, n->rval);

    if(isPrimitiveOp(n, lhs.type, rhs.type)){
        handlePrimitiveOp(*this, n, lhs, rhs);
        return;
    }

    if(n->decl->isFuncDecl()){
        TypedValue fnVal = n->decl->tval;

        if(!fnVal.val){
            auto fnTy = try_cast<AnFunctionType>(n->decl->tval.type);
            auto boundTy = AnFunctionType::get(fnTy->retTy, {lhs.type, rhs.type}, fnTy->typeClassConstraints);

            TypeError err{"Error in operator monomorphisation with " + anTypeToColoredStr(fnTy)
                + " bound to " + anTypeToColoredStr(boundTy), n->loc};
            auto subs = unify({{fnTy, boundTy, err}});

            c->compCtxt->insertMonomorphisationMappings(subs);

            fnTy = applyMonomorphisationBindings(fnTy, c->compCtxt->monomorphisationMappings);
            if(TypedValue f = findBuiltinFn(c, fnTy)){
                array<Value*, 2> args{lhs.val, rhs.val};
                Value *call = c->builder.CreateCall(f.val, args);
                this->val = {call, n->getType()};
                return;
            }
            // if(isPrimitiveOp(n, fnTy->paramTys[0], fnTy->paramTys[1])){
            //     lhs.type = fnTy->paramTys[0];
            //     rhs.type = fnTy->paramTys[1];
            //     handlePrimitiveOp(*this, n, lhs, rhs);
            //     return;
            // }

            attachTraitImpl(n->decl, fnTy, c->compUnit, n->loc);
            TraitImpl *trait = fnTy->typeClassConstraints.front();
            Declaration *fn = findFnInImpl(Lexer::getTokStr(n->op), trait);
            fnVal = monomorphise(c, static_cast<FuncDecl*>(fn), fnTy, n->loc);
        }

        //call function
        array<Value*, 2> args{lhs.val, rhs.val};
        Value *call = c->builder.CreateCall(fnVal.val, args);
        this->val = {call, n->getType()};
        return;
    }

    error("Operator " + Lexer::getTokStr(n->op) + " is not overloaded for types "
            + anTypeToColoredStr(lhs.type) + " and " + anTypeToColoredStr(rhs.type), n->loc);
}


void CompilingVisitor::visit(UnOpNode *n){
    n->rval->accept(*this);

    switch(n->op){
        case '@': //pointer dereference
            if(val.type->typeTag != TT_Ptr){
                error("Cannot dereference non-pointer type " + anTypeToColoredStr(val.type), n->loc);
            }

            this->val = TypedValue(c->builder.CreateLoad(val.val),
                (AnType*)val.type->addModifiersTo(try_cast<AnPtrType>(val.type)->extTy));
            return;
        case '&': //address-of
            this->val = addrOf(c, val);
            return;
        case '-': //negation
            if(!isNumericTypeTag(val.type->typeTag))
                error("Cannot negate non-numeric type " + anTypeToColoredStr(val.type), n->loc);

            if(isFPTypeTag(val.type->typeTag))
                this->val = TypedValue(c->builder.CreateFNeg(val.val), val.type);
            else
                this->val = TypedValue(c->builder.CreateNeg(val.val), val.type);
            return;
        case Tok_Not:
            if(val.type->typeTag != TT_Bool)
                error("Unary not operator not overloaded for type " + anTypeToColoredStr(val.type), n->loc);

            this->val = TypedValue(c->builder.CreateNot(val.val), val.type);
            return;
        case Tok_New:
            //the 'new' keyword in ante creates a reference to any existing value
            this->val = createMallocAndStore(c, val);
            return;
    }

    error("Unknown unary operator " + Lexer::getTokStr(n->op), n->loc);
}

} // end of namespace ante
