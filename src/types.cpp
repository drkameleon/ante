#include <types.h>
using namespace std;
using namespace llvm;

namespace ante {

AnTypeContainer typeArena;

char getBitWidthOfTypeTag(const TypeTag ty){
    switch(ty){
        case TT_I8:  case TT_U8: case TT_C8:  return 8;
        case TT_I16: case TT_U16: case TT_F16: return 16;
        case TT_I32: case TT_U32: case TT_F32: case TT_C32: return 32;
        case TT_I64: case TT_U64: case TT_F64: return 64;
        case TT_Isz: case TT_Usz: return AN_USZ_SIZE;
        case TT_Bool: return 1;

        case TT_Ptr:
        case TT_Function:
        case TT_MetaFunction:
        case TT_FunctionList: return AN_USZ_SIZE;

        default: return 0;
    }
}

bool isGeneric(const std::vector<AnType*> &vec){
    for(auto *t : vec)
        if(t->isGeneric)
            return true;
    return false;
}


bool AnType::hasModifier(TokenType m){
    if(auto *modty = dyn_cast<AnModifierType>(this))
        for(auto mod : modty->modifiers)
            if(mod == m)
                return true;
    return false;
}

AnModifierType* AnType::addModifier(TokenType m){
    if(auto *modty = dyn_cast<AnModifierType>(this)){
        if(std::find(modty->modifiers.cbegin(), modty->modifiers.cend(), m) != modty->modifiers.end()){
            return modty;
        }else{
            auto mods = modty->modifiers;
            mods.push_back(m);
            return AnModifierType::get(modty->extTy, mods);
        }
    }
    return AnModifierType::get(this, {m});
}
        
AnType* AnType::copyModifiersFrom(AnType *t){
    auto *ret = this;
    if(auto *modty = dyn_cast<AnModifierType>(t)){
        for(auto mod : modty->modifiers)
            ret = ret->addModifier(mod);
    }
    return ret;
}


unsigned short AnDataType::getTagVal(std::string &name){
    for(auto& tag : tags){
        if(tag->name == name){
            return tag->tag;
        }
    }
    
    std::cerr << "No value found for tag " << name
                << " of type " << this->name << std::endl;
    throw new CtError();
}
 

/*
 *  Returns the TypeNode* value of a TypedValue* of type TT_Type
 */
AnType* extractTypeValue(const TypedValue* tv){
    auto zext = dyn_cast<ConstantInt>(tv->val)->getZExtValue();
    return (AnType*) zext;
}

AnType* extractTypeValue(const unique_ptr<TypedValue> &tv){
    return extractTypeValue(tv.get());
}


AnType* AnType::getPrimitive(TypeTag tag){
    switch(tag){
        case TT_I8:           return typeArena.primitiveTypes[tag].get();
        case TT_I16:          return typeArena.primitiveTypes[tag].get();
        case TT_I32:          return typeArena.primitiveTypes[tag].get();
        case TT_I64:          return typeArena.primitiveTypes[tag].get();
        case TT_Isz:          return typeArena.primitiveTypes[tag].get();
		case TT_U8:           return typeArena.primitiveTypes[tag].get();
        case TT_U16:          return typeArena.primitiveTypes[tag].get();
        case TT_U32:          return typeArena.primitiveTypes[tag].get();
        case TT_U64:          return typeArena.primitiveTypes[tag].get();
        case TT_Usz:          return typeArena.primitiveTypes[tag].get();
        case TT_F16:          return typeArena.primitiveTypes[tag].get();
        case TT_F32:          return typeArena.primitiveTypes[tag].get();
        case TT_F64:          return typeArena.primitiveTypes[tag].get();
        case TT_C8:           return typeArena.primitiveTypes[tag].get();
        case TT_C32:          return typeArena.primitiveTypes[tag].get();
		case TT_Bool:         return typeArena.primitiveTypes[tag].get();
        case TT_Void:         return typeArena.primitiveTypes[tag].get();
        case TT_FunctionList: return typeArena.primitiveTypes[tag].get();
        default:
            cerr << "error: AnType::getPrimitive: TypeTag " << typeTagToStr(tag) << " is not primitive!\n";
    }
    throw new CtError();
}


AnType* AnType::getI8(){
    return typeArena.primitiveTypes[TT_I8].get();
}

AnType* AnType::getI16(){
    return typeArena.primitiveTypes[TT_I16].get();
}

AnType* AnType::getI32(){
    return typeArena.primitiveTypes[TT_I32].get();
}

AnType* AnType::getI64(){
    return typeArena.primitiveTypes[TT_I64].get();
}

AnType* AnType::getIsz(){
    return typeArena.primitiveTypes[TT_Isz].get();
}

AnType* AnType::getU8(){
    return typeArena.primitiveTypes[TT_U8].get();
}

AnType* AnType::getU16(){
    return typeArena.primitiveTypes[TT_U16].get();
}

AnType* AnType::getU32(){
    return typeArena.primitiveTypes[TT_U32].get();
}

AnType* AnType::getU64(){
    return typeArena.primitiveTypes[TT_U64].get();
}

AnType* AnType::getUsz(){
    return typeArena.primitiveTypes[TT_Usz].get();
}

AnType* AnType::getF16(){
    return typeArena.primitiveTypes[TT_F16].get();
}

AnType* AnType::getF32(){
    return typeArena.primitiveTypes[TT_F32].get();
}

AnType* AnType::getF64(){
    return typeArena.primitiveTypes[TT_F64].get();
}

AnType* AnType::getBool(){
    return typeArena.primitiveTypes[TT_Bool].get();
}

AnType* AnType::getVoid(){
    return typeArena.primitiveTypes[TT_Void].get();
}


AnModifierType* AnType::getModifier(AnType *e, const std::vector<TokenType> modifiers){
    return AnModifierType::get(e, modifiers);
}

string getKey(const std::vector<TokenType> &mods){
    string ret = "";
    for(auto m : mods){
        ret += Lexer::getTokStr(m) + "&";
    }
    return ret;
}

AnModifierType* AnModifierType::get(AnType *e, const std::vector<TokenType> modifiers){
    auto key = getKey(modifiers) + anTypeToStr(e);
    try{
        return typeArena.modifierTypes.at(key).get();
    }catch(out_of_range r){
        auto mod = new AnModifierType(e, modifiers);
        typeArena.modifierTypes.emplace(key, mod);
        return mod;
    }
}



AnPtrType* AnType::getPtr(AnType* ext){ return AnPtrType::get(ext); }
AnPtrType* AnPtrType::get(AnType* ext){
    try{
        return typeArena.ptrTypes.at(ext).get();
    }catch(out_of_range r){
        auto ptr = new AnPtrType(ext);
        typeArena.ptrTypes.emplace(ext, ptr);
        return ptr;
    }
}

AnArrayType* AnType::getArray(AnType* t, size_t len){ return AnArrayType::get(t,len); }
AnArrayType* AnArrayType::get(AnType* t, size_t len){
    auto key = anTypeToStr(t) + to_string(len);
    try{
        return typeArena.arrayTypes.at(key).get();
    }catch(out_of_range r){
        auto arr = new AnArrayType(t, len);
        typeArena.arrayTypes.emplace(key, arr);
        return arr;
    }
}

string getKey(const std::vector<AnType*> &exts){
    string ret = "";
    for(auto *ext : exts){
        ret += anTypeToStr(ext) + ",";
    }
    return ret;
}

AnAggregateType* AnType::getAggregate(TypeTag t, const std::vector<AnType*> exts){
    return AnAggregateType::get(t, exts);
}

AnAggregateType* AnAggregateType::get(TypeTag t, const std::vector<AnType*> exts){
    auto key = typeTagToStr(t) + getKey(exts);
    try{
        return typeArena.aggregateTypes.at(key).get();
    }catch(out_of_range r){
        auto agg = new AnAggregateType(t, exts);
        typeArena.aggregateTypes.emplace(key, agg);
        return agg;
    }
}
        
AnFunctionType* AnFunctionType::get(AnType* retty, NamedValNode* params, bool isMetaFunction){
    vector<AnType*> extTys;

    while(params && params->typeExpr.get()){
        TypeNode *pty = (TypeNode*)params->typeExpr.get();
        extTys.push_back(toAnType(pty));
        params = (NamedValNode*)params->next.get();
    }
    return AnFunctionType::get(retty, extTys, isMetaFunction);
}


AnFunctionType* AnFunctionType::get(AnType *retTy, const std::vector<AnType*> elems, bool isMetaFunction){
    auto key = getKey(elems) + "->" + anTypeToStr(retTy);
    try{
        return typeArena.functionTypes.at(key).get();
    }catch(out_of_range r){
        auto f = new AnFunctionType(retTy, elems, isMetaFunction);
        typeArena.functionTypes.emplace(key, f);
        return f;
    }
}


AnTypeVarType* AnType::getTypeVar(std::string name){
    return AnTypeVarType::get(name);
}

AnTypeVarType* AnTypeVarType::get(std::string name){
    try{
        return typeArena.typeVarTypes.at(name).get();
    }catch(out_of_range r){
        auto tvar = new AnTypeVarType(name);
        typeArena.typeVarTypes.emplace(name, tvar);
        return tvar;
    }
}

AnDataType* AnType::getDataType(string name){
    return AnDataType::get(name);
}


/*
 * Returns the unique boundName of a generic type after it is bound
 * with the specified type arguments
 */
string getBoundName(string &baseName, const vector<AnType*> &typeArgs){
    if(typeArgs.empty())
        return baseName;

    string name = baseName + "<";
    for(auto *arg : typeArgs){
        name += anTypeToStr(arg) + ",";
    }
    return name + ">";
}

string getBoundName(string &baseName, const vector<pair<string, AnType*>> &typeArgs){
    if(typeArgs.empty())
        return baseName;

    string name = baseName + "<";
    for(auto &p : typeArgs){
        name += anTypeToStr(p.second) + ",";
    }
    return name + ">";
}


AnDataType* AnDataType::get(string name){
    try{
        return typeArena.declaredTypes.at(name).get();
    }catch(out_of_range r){
        //create declaration w/out definition
        auto decl = new AnDataType(name, {});
        typeArena.declaredTypes.emplace(name, decl);
        return decl;
    }
}

AnDataType* AnDataType::create(string name, vector<AnType*> elems, bool isUnion){
    try{
        auto *dt = typeArena.declaredTypes.at(name).get();
        dt->extTys = elems;
        return dt;
    }catch(out_of_range r){
        auto tvar = new AnDataType(name, elems, isUnion);
        typeArena.declaredTypes.emplace(name, tvar);
        return tvar;
    }
}

AnTypeContainer::AnTypeContainer(){
    typeArena.primitiveTypes[TT_I8].reset(new AnType(TT_I8));
    typeArena.primitiveTypes[TT_I16].reset(new AnType(TT_I16));
    typeArena.primitiveTypes[TT_I32].reset(new AnType(TT_I32));
    typeArena.primitiveTypes[TT_I64].reset(new AnType(TT_I64));
    typeArena.primitiveTypes[TT_Isz].reset(new AnType(TT_Isz));
    typeArena.primitiveTypes[TT_U8].reset(new AnType(TT_U8));
    typeArena.primitiveTypes[TT_U16].reset(new AnType(TT_U16));
    typeArena.primitiveTypes[TT_U32].reset(new AnType(TT_U32));
    typeArena.primitiveTypes[TT_U64].reset(new AnType(TT_U64));
    typeArena.primitiveTypes[TT_Usz].reset(new AnType(TT_Usz));
    typeArena.primitiveTypes[TT_F16].reset(new AnType(TT_F16));
    typeArena.primitiveTypes[TT_F32].reset(new AnType(TT_F32));
    typeArena.primitiveTypes[TT_F64].reset(new AnType(TT_F64));
    typeArena.primitiveTypes[TT_Bool].reset(new AnType(TT_Bool));
    typeArena.primitiveTypes[TT_Void].reset(new AnType(TT_Void));
    typeArena.primitiveTypes[TT_C8].reset(new AnType(TT_C8));
    typeArena.primitiveTypes[TT_C32].reset(new AnType(TT_C32));
    typeArena.primitiveTypes[TT_FunctionList].reset(new AnType(TT_FunctionList));
}
        

AnType* AnType::getFunctionReturnType() const{
    return ((AnFunctionType*)this)->retTy;
}


AnType* toAnType(const TypeNode *tn){
    switch(tn->type){
        case TT_I8:
        case TT_I16:
        case TT_I32:
        case TT_I64:
        case TT_U8:
        case TT_U16:
        case TT_U32:
        case TT_U64:
        case TT_F16:
        case TT_F32:
        case TT_F64:
        case TT_Isz:
        case TT_Usz:
        case TT_C8:
        case TT_C32:
        case TT_Bool:
        case TT_Void:
            return AnType::getPrimitive(tn->type);

        case TT_Tuple:
        case TT_Function:
        case TT_MetaFunction:
        case TT_FunctionList: {
            TypeNode *ext = tn->extTy.get();
            vector<AnType*> tys;
            while(ext){
                tys.push_back(toAnType((TypeNode*)ext));
                ext = (TypeNode*)ext->next.get();
            }
            return AnAggregateType::get(TT_Tuple, tys);
        }

        case TT_Array: {
            TypeNode *elemTy = tn->extTy.get();
            IntLitNode *len = (IntLitNode*)elemTy->next.get();
            return AnArrayType::get(toAnType(elemTy), len ? stoi(len->val) : 0);
        }
        case TT_Ptr:
            return AnPtrType::get(toAnType(tn->extTy.get()));
        case TT_Data:
        case TT_TaggedUnion: {
            if(!tn->params.empty()){
                string key = tn->typeName + "<";
                for(auto &t : tn->params){
                    key += typeNodeToStr(t.get()) + ",";
                }
                return AnDataType::get(key + ">");
            }else{
                return AnDataType::get(tn->typeName);
            }
        }
        case TT_TypeVar:
            return AnTypeVarType::get(tn->typeName);
        default:
            cerr << "Unknown TypeTag " << typeTagToStr(tn->type) << endl;
            return nullptr;
    }
}



/*
 *  Checks to see if a type is valid to be used.
 *  To be valid the type must:
 *      - Not be recursive (contain no references to
 *        itself that are not behind a pointer)
 *      - Contain no typevars that are not declared
 *        within the rootTy's params
 *      - Contain only data types that have been declared
 */
void validateType(Compiler *c, const AnType *tn, const DataDeclNode *rootTy){
    if(!tn) return;

    if(tn->typeTag == TT_Data or tn->typeTag == TT_TaggedUnion){
        auto *dataTy = (AnDataType*)tn;

        if(dataTy->isStub()){
            if(dataTy->name == rootTy->name){
                c->compErr("Recursive types are disallowed, wrap the type in a pointer instead", rootTy->loc);
            }

            c->compErr("Type "+dataTy->name+" has not been declared", rootTy->loc);
        }

        //if(dataTy->generics.size() != tn->params.size())
        //    c->compErr("Unbound type params for type "+anTypeToColoredStr(dataTy), rootTy->loc);

        //if(dataTy->isGeneric){
        //    dataTy = bindGenericToType(dataTy, tn->params, dataTy);
        //}
        
        for(auto *t : dataTy->extTys)
            validateType(c, t, rootTy);

    }else if(tn->typeTag == TT_Tuple){
        auto *agg = (AnAggregateType*)tn;
        for(auto *ext : agg->extTys){
            validateType(c, ext, rootTy);
        }
    }else if(tn->typeTag == TT_Array){
        auto *arr = (AnArrayType*)tn;
        validateType(c, arr->extTy, rootTy);
    }else if(tn->typeTag == TT_Ptr or tn->typeTag == TT_Function or tn->typeTag == TT_MetaFunction){
        return;

    }else if(tn->typeTag == TT_TypeVar){
        auto *tvt = (AnTypeVarType*)tn;
        auto *var = c->lookup(tvt->name);
        if(var){
            return validateType(c, extractTypeValue(var->tval), rootTy);
        }

        //Typevar not found, if its not in the rootTy's params, then it is unbound
        for(auto &p : rootTy->generics){
            if(p->typeName == tvt->name) return;
        }

        c->compErr("Lookup for "+tvt->name+" not found", rootTy->loc);
    }
}

void validateType(Compiler *c, const AnType *tn, const AnDataType *dt){
    auto fakeLoc = mkLoc(mkPos(0, 0, 0), mkPos(0, 0, 0));
    auto *ddn = new DataDeclNode(fakeLoc, dt->name, 0, 0);

    for(auto &g : dt->generics){
        auto *tv = mkAnonTypeNode(TT_TypeVar);
        tv->typeName = g->name;
        ddn->generics.emplace_back(tv);
    }

    validateType(c, tn, ddn);
    ddn->generics.clear();
    delete ddn;
}


size_t AnType::getSizeInBits(Compiler *c, string *incompleteType){
    size_t total = 0;

    if(isPrimitiveTypeTag(this->typeTag))
        return getBitWidthOfTypeTag(this->typeTag);

    if(typeTag == TT_Data or typeTag == TT_TaggedUnion){
        auto *dataTy = (AnDataType*)this;

        if(dataTy->isStub()){
            if(incompleteType and dataTy->name == *incompleteType){
                cerr << "Incomplete type " << anTypeToColoredStr(this) << endl;
                throw new IncompleteTypeError();
            }

            cerr << "Type " << anTypeToColoredStr(this) << " has not been declared\n";
            return 0;
        }

        //TypeNode *tyn = dataTy->tyn.get();
        //if(!dataTy->generics.empty()){
        //    tyn = copy(tyn);
        //    bindGenericToType(tyn, params, dataTy);
        //}
        for(auto *ext : dataTy->extTys){
            total += ext->getSizeInBits(c, incompleteType);
        }
        return total;

    }else if(typeTag == TT_Tuple){
        for(auto *ext : ((AnAggregateType*)this)->extTys){
            total += ext->getSizeInBits(c, incompleteType);
        }

    }else if(typeTag == TT_Array){
        auto *arr = (AnArrayType*)this;
        return arr->len * arr->extTy->getSizeInBits(c, incompleteType);
    }else if(typeTag == TT_Ptr or typeTag == TT_Function or typeTag == TT_MetaFunction){
        return AN_USZ_SIZE;

    }else if(typeTag == TT_TypeVar){
        auto *tvt = (AnTypeVarType*)this;
        auto *var = c->lookup(tvt->name);
        if(var)
            return extractTypeValue(var->tval)->getSizeInBits(c);

        //TODO: store location data in AnTypeVarType
        cerr << "Lookup for typevar " << tvt->name << " not found" << endl;
        throw new TypeVarError();
    }

    return total;
}


vector<pair<string, AnType*>>
mapBindingsToDataType(const vector<AnType*> &bindings, AnDataType *dt){
    vector<pair<string, AnType*>> ret;
    ret.reserve(bindings.size());

    for(size_t i = 0; i < dt->generics.size(); i++){
        ret.emplace_back(dt->generics[i]->name, bindings[i]);
    }
    return ret;
}


AnType* find(string &k, const vector<pair<string, AnType*>> &bindings){
    for(auto &p : bindings)
        if(p.first == k)
            return p.second;
    return nullptr;
}


vector<pair<string, AnType*>>
filterMatchingBindings(const AnDataType *dt, const vector<pair<string, AnType*>> &bindings){
    vector<pair<string,AnType*>> matches;
    for(auto &b : dt->generics){
        AnType *arg = find(b->name, bindings);
        if(arg){
            matches.emplace_back(b->name, arg);
        }
    }
    return matches;
}


Type* updateLlvmTypeBinding(Compiler *c, AnDataType *dt){
    //create an empty type first so we dont end up with infinite recursion
    auto* structTy = StructType::create(*c->ctxt, {}, dt->name, dt->typeTag == TT_TaggedUnion);
    dt->llvmType = structTy;

    AnType *ext = dt;
    if(dt->typeTag == TT_TaggedUnion)
        ext = getLargestExt(c, dt);

    vector<Type*> tys;
    for(auto *e : dt->extTys){
        tys.push_back(c->anTypeToLlvmType(e));
    }

    structTy->setBody(tys);

    dt->llvmType = structTy;
    return structTy;
}

/*
 *  Generics can be stored and bound in two ways
 *    1. Stored as a map from name of typevar -> type bound to
 *       - Handled by this function
 *       - This is the format returned by a typeEq type check if it
 *         indicates the check would only be a success if those typevars
 *         are bound to the given types.  This is TypeCheckResult::SuccessWithTypeVars
 *    2. Stored as a vector of ordered bound types.
 *       - The ordering of type vars in this format is matched to the order of the
 *         declaration of generics when the datatype was first declared.
 *         Eg. with type Map<'k,'v> = ... and bindings {Str, i32}, 'k is bound to
 *         Str and 'v is bound to i32.
 *       - This is the format used internally by TypeNodes and DataTypes.
 *       - Before being bound in bindGenericToType this representation must be
 *         converted to the first beforehand, and to do that it needs the DataType
 *         to match the typevar ordering with.  The second function below handles
 *         this conversion
 */
AnType* bindGenericToType(Compiler *c, AnType *tn, const vector<pair<string, AnType*>> &bindings){
    if(!tn->isGeneric or bindings.empty())
        return tn;

    if(tn->typeTag == TT_Data or tn->typeTag == TT_TaggedUnion){
        auto *dty = (AnDataType*)tn;

        auto dty_bindings = filterMatchingBindings(dty, bindings);
        string boundName = getBoundName(dty->name, dty_bindings);

        auto *decl = AnDataType::get(boundName);
        if(!decl->isStub())
            return decl;

        //This variant has never been bound before so fill in the stub
        vector<AnType*> boundExts;
        boundExts.reserve(dty->extTys.size());

        for(auto *e : dty->extTys){
            boundExts.push_back(bindGenericToType(c, e, dty_bindings));
        }

        decl->unboundType = dty;
        decl->extTys.swap(boundExts);
        decl->fields = dty->fields;
        decl->tags = dty->tags;
        decl->traitImpls = dty->traitImpls;
        updateLlvmTypeBinding(c, decl);
        return decl;

    }else if(tn->typeTag == TT_TypeVar){
        auto *tv = (AnTypeVarType*)tn;
        for(auto& pair : bindings){
            if(tv->name == pair.first){
                return pair.second;
            }
        }
        cerr << "warning: unbound type var " << tv->name << " in binding\n";
        return tv;

    }else if(tn->typeTag == TT_Modifier){
        auto *mod = (AnModifierType*)tn;
        auto *ty = bindGenericToType(c, mod->extTy, bindings);
        return AnModifierType::get(ty, mod->modifiers);

    }else if(tn->typeTag == TT_Tuple or tn->typeTag == TT_Function or
             tn->typeTag == TT_MetaFunction or tn->typeTag == TT_FunctionList){
        
        auto *agg = (AnAggregateType*)tn;
        vector<AnType*> exts;
        exts.reserve(agg->extTys.size());
        for(auto *e : agg->extTys){
            exts.push_back(bindGenericToType(c, e, bindings));
        }
        return AnAggregateType::get(tn->typeTag, exts);

    }else if(tn->typeTag == TT_Ptr){
        auto *ptr = (AnPtrType*)tn;
        auto *ty = bindGenericToType(c, ptr->extTy, bindings);
        return AnPtrType::get(ty);
        
    }else if(tn->typeTag == TT_Array){
        auto *arr = (AnArrayType*)tn;
        auto *ty = bindGenericToType(c, arr->extTy, bindings);
        return AnArrayType::get(ty, arr->len);
        
    }else{
        return tn;
    }
}


AnType* bindGenericToType(Compiler *c, AnType *tn, const vector<AnType*> &bindings, AnDataType *dt){
    if(bindings.empty() or !tn->isGeneric)
        return tn;

    auto bindings_map = mapBindingsToDataType(bindings, dt);

    return bindGenericToType(c, tn, bindings_map);
}


/*
 *  Replaces already bound typevars with their declared value
 *
AnType* Compiler::searchAndReplaceBoundTypeVars(AnType *tn) const{
    if(!tn->isGeneric)
        return tn;
    
    if(tn->typeTag == TT_Data or tn->typeTag == TT_TaggedUnion){
        auto *dty = (AnDataType*)tn;

        auto dty_bindings = filterMatchingBindings(dty, bindings);
        string boundName = getBoundName(dty->name, dty_bindings);

        auto *decl = AnDataType::get(boundName);
        if(!decl->isStub())
            return decl;

        //This variant has never been bound before so fill in the stub
        vector<AnType*> boundExts;
        boundExts.reserve(dty->extTys.size());

        for(auto *e : dty->extTys){
            boundExts.push_back(bindGenericToType(e, dty_bindings));
        }

        decl->unboundType = dty;
        decl->extTys.swap(boundExts);
        decl->fields = dty->fields;
        decl->tags = dty->tags;
        decl->traitImpls = dty->traitImpls;
        return decl;

    }else if(tn->typeTag == TT_TypeVar){
        auto *tvt = (AnTypeVarType*)tn;
        auto *var = lookup(tvt->name);
        if(!var){
            cerr << "error: Lookup for "+tvt->name+" not found\n";
            return tn;
        }

        return extractTypeValue(var->tval);

    }else if(tn->typeTag == TT_Modifier){
        auto *mod = (AnModifierType*)tn;
        auto *ty = bindGenericToType(mod->extTy, bindings);
        return AnModifierType::get(ty, mod->modifiers);

    }else if(tn->typeTag == TT_Tuple or tn->typeTag == TT_Function or
             tn->typeTag == TT_MetaFunction or tn->typeTag == TT_FunctionList){
        
        auto *agg = (AnAggregateType*)tn;
        vector<AnType*> exts;
        exts.reserve(agg->extTys.size());
        for(auto *e : agg->extTys){
            exts.push_back(bindGenericToType(e, bindings));
        }
        return AnAggregateType::get(tn->typeTag, exts);

    }else if(tn->typeTag == TT_Ptr){
        auto *ptr = (AnPtrType*)tn;
        auto *ty = bindGenericToType(ptr->extTy, bindings);
        return AnPtrType::get(ty);
        
    }else if(tn->typeTag == TT_Array){
        auto *arr = (AnArrayType*)tn;
        auto *ty = bindGenericToType(arr->extTy, bindings);
        return AnArrayType::get(ty, arr->len);
        
    }else{
        return tn;
    }
}
*/


/*
 *  Checks for, and implicitly widens an integer or float type.
 *  The original value of num is returned if no widening can be performed.
 */
TypedValue* Compiler::implicitlyWidenNum(TypedValue *num, TypeTag castTy){
    bool lIsInt = isIntTypeTag(num->type->typeTag);
    bool lIsFlt = isFPTypeTag(num->type->typeTag);

    if(lIsInt or lIsFlt){
        bool rIsInt = isIntTypeTag(castTy);
        bool rIsFlt = isFPTypeTag(castTy);
        if(!rIsInt and !rIsFlt){
            cerr << "castTy argument of implicitlyWidenNum must be a numeric primitive type\n";
            exit(1);
        }

        int lbw = getBitWidthOfTypeTag(num->type->typeTag);
        int rbw = getBitWidthOfTypeTag(castTy);
        Type *ty = typeTagToLlvmType(castTy, *ctxt);

        //integer widening
        if(lIsInt and rIsInt){
            if(lbw <= rbw){
                return new TypedValue(
                    builder.CreateIntCast(num->val, ty, !isUnsignedTypeTag(num->type->typeTag)),
                    AnType::getPrimitive(castTy)
                );
            }

        //int -> flt, (flt -> int is never implicit)
        }else if(lIsInt and rIsFlt){
            return new TypedValue(
                isUnsignedTypeTag(num->type->typeTag)
                    ? builder.CreateUIToFP(num->val, ty)
                    : builder.CreateSIToFP(num->val, ty),

                AnType::getPrimitive(castTy)
            );

        //float widening
        }else if(lIsFlt and rIsFlt){
            if(lbw < rbw){
                return new TypedValue(
                    builder.CreateFPExt(num->val, ty),
                    AnType::getPrimitive(castTy)
                );
            }
        }
    }

    return num;
}


/*
 *  Assures two IntegerType'd Values have the same bitwidth.
 *  If not, one is extended to the larger bitwidth and mutated appropriately.
 *  If the extended integer value is unsigned, it is zero extended, otherwise
 *  it is sign extended.
 *  Assumes the llvm::Type of both values to be an instance of IntegerType.
 */
void Compiler::implicitlyCastIntToInt(TypedValue **lhs, TypedValue **rhs){
    int lbw = getBitWidthOfTypeTag((*lhs)->type->typeTag);
    int rbw = getBitWidthOfTypeTag((*rhs)->type->typeTag);

    if(lbw != rbw){
        //Cast the value with the smaller bitwidth to the type with the larger bitwidth
        if(lbw < rbw){
            auto *ret = new TypedValue(
                builder.CreateIntCast((*lhs)->val, (*rhs)->getType(),
                    !isUnsignedTypeTag((*lhs)->type->typeTag)),
                (*rhs)->type
            );

            *lhs = ret;

        }else{//lbw > rbw
            auto *ret = new TypedValue(
                builder.CreateIntCast((*rhs)->val, (*lhs)->getType(),
                    !isUnsignedTypeTag((*rhs)->type->typeTag)),
                (*lhs)->type
            );

            *rhs = ret;
        }
    }
}

bool isIntTypeTag(const TypeTag ty){
    return ty==TT_I8 or ty==TT_I16 or ty==TT_I32 or ty==TT_I64 or
           ty==TT_U8 or ty==TT_U16 or ty==TT_U32 or ty==TT_U64 or
           ty==TT_Isz or ty==TT_Usz or ty==TT_C8;
}

bool isFPTypeTag(const TypeTag tt){
    return tt==TT_F16 or tt==TT_F32 or tt==TT_F64;
}

bool isNumericTypeTag(const TypeTag ty){
    return isIntTypeTag(ty) or isFPTypeTag(ty);
}

/*
 *  Performs an implicit cast from a float to int.  Called in any operation
 *  involving an integer, a float, and a binop.  No matter the ints size,
 *  it is always casted to the (possibly smaller) float value.
 */
void Compiler::implicitlyCastIntToFlt(TypedValue **lhs, Type *ty){
    auto *ret = new TypedValue(
        isUnsignedTypeTag((*lhs)->type->typeTag)
            ? builder.CreateUIToFP((*lhs)->val, ty)
            : builder.CreateSIToFP((*lhs)->val, ty),

        AnType::getPrimitive(llvmTypeToTypeTag(ty))
    );
    *lhs = ret;
}


/*
 *  Performs an implicit cast from a float to float.
 */
void Compiler::implicitlyCastFltToFlt(TypedValue **lhs, TypedValue **rhs){
    int lbw = getBitWidthOfTypeTag((*lhs)->type->typeTag);
    int rbw = getBitWidthOfTypeTag((*rhs)->type->typeTag);

    if(lbw != rbw){
        if(lbw < rbw){
            auto *ret = new TypedValue(
                builder.CreateFPExt((*lhs)->val, (*rhs)->getType()),
                (*rhs)->type
            );
            *lhs = ret;
        }else{//lbw > rbw
            auto *ret = new TypedValue(
                builder.CreateFPExt((*rhs)->val, (*lhs)->getType()),
                (*lhs)->type
            );
            *rhs = ret;
        }
    }
}


/*
 *  Detects, and creates an implicit type conversion when necessary.
 */
void Compiler::handleImplicitConversion(TypedValue **lhs, TypedValue **rhs){
    bool lIsInt = isIntTypeTag((*lhs)->type->typeTag);
    bool lIsFlt = isFPTypeTag((*lhs)->type->typeTag);
    if(!lIsInt and !lIsFlt) return;

    bool rIsInt = isIntTypeTag((*rhs)->type->typeTag);
    bool rIsFlt = isFPTypeTag((*rhs)->type->typeTag);
    if(!rIsInt and !rIsFlt) return;

    //both values are numeric, so forward them to the relevant casting method
    if(lIsInt and rIsInt){
        implicitlyCastIntToInt(lhs, rhs);  //implicit int -> int (widening)
    }else if(lIsInt and rIsFlt){
        implicitlyCastIntToFlt(lhs, (*rhs)->getType()); //implicit int -> flt
    }else if(lIsFlt and rIsInt){
        implicitlyCastIntToFlt(rhs, (*lhs)->getType()); //implicit int -> flt
    }else if(lIsFlt and rIsFlt){
        implicitlyCastFltToFlt(lhs, rhs); //implicit int -> flt
    }
}


bool containsTypeVar(const TypeNode *tn){
    auto tt = tn->type;
    if(tt == TT_Array or tt == TT_Ptr){
        return tn->extTy->type == tt;
    }else if(tt == TT_Tuple or tt == TT_Data or tt == TT_TaggedUnion or
             tt == TT_Function or tt == TT_MetaFunction){
        TypeNode *ext = tn->extTy.get();
        while(ext){
            if(containsTypeVar(ext))
                return true;
        }
    }
    return tt == TT_TypeVar;
}


/*
 *  Translates an individual TypeTag to an llvm::Type.
 *  Only intended for primitive types, as there is not enough
 *  information stored in a TypeTag to convert to array, tuple,
 *  or function types.
 */
Type* typeTagToLlvmType(TypeTag ty, LLVMContext &ctxt){
    switch(ty){
        case TT_I8:  case TT_U8:  return Type::getInt8Ty(ctxt);
        case TT_I16: case TT_U16: return Type::getInt16Ty(ctxt);
        case TT_I32: case TT_U32: return Type::getInt32Ty(ctxt);
        case TT_I64: case TT_U64: return Type::getInt64Ty(ctxt);
        case TT_Isz:    return Type::getIntNTy(ctxt, AN_USZ_SIZE); //TODO: implement
        case TT_Usz:    return Type::getIntNTy(ctxt, AN_USZ_SIZE); //TODO: implement
        case TT_F16:    return Type::getHalfTy(ctxt);
        case TT_F32:    return Type::getFloatTy(ctxt);
        case TT_F64:    return Type::getDoubleTy(ctxt);
        case TT_C8:     return Type::getInt8Ty(ctxt);
        case TT_C32:    return Type::getInt32Ty(ctxt);
        case TT_Bool:   return Type::getInt1Ty(ctxt);
        case TT_Void:   return Type::getVoidTy(ctxt);
        case TT_TypeVar:
            throw new TypeVarError();
        default:
            cerr << "typeTagToLlvmType: Unknown/Unsupported TypeTag " << ty << ", returning nullptr.\n";
            return nullptr;
    }
}

AnType* getLargestExt(Compiler *c, AnDataType *tn){
    AnType *largest = 0;
    size_t largest_size = 0;

    for(auto *e : tn->extTys){
        size_t size = e->getSizeInBits(c);
        if(size > largest_size){
            largest = e;
            largest_size = size;
        }
    }
    return largest;
}



/*
 *  Translates a llvm::Type to a TypeTag. Not intended for in-depth analysis
 *  as it loses data about the type and name of UserTypes, and cannot distinguish
 *  between signed and unsigned integer types.  As such, this should mainly be
 *  used for comparing primitive datatypes, or just to detect if something is a
 *  primitive.
 */
TypeTag llvmTypeToTypeTag(Type *t){
    if(t->isIntegerTy(1)) return TT_Bool;

    if(t->isIntegerTy(8)) return TT_I8;
    if(t->isIntegerTy(16)) return TT_I16;
    if(t->isIntegerTy(32)) return TT_I32;
    if(t->isIntegerTy(64)) return TT_I64;
    if(t->isHalfTy()) return TT_F16;
    if(t->isFloatTy()) return TT_F32;
    if(t->isDoubleTy()) return TT_F64;

    if(t->isArrayTy()) return TT_Array;
    if(t->isStructTy() and !t->isEmptyTy()) return TT_Tuple; /* Could also be a TT_Data! */
    if(t->isPointerTy()) return TT_Ptr;
    if(t->isFunctionTy()) return TT_Function;

    return TT_Void;
}

/*
 *  Converts a TypeNode to an llvm::Type.  While much less information is lost than
 *  llvmTypeToTokType, information on signedness of integers is still lost, causing the
 *  unfortunate necessity for the use of a TypedValue for the storage of this information.
 */
Type* Compiler::anTypeToLlvmType(const AnType *ty){
    vector<Type*> tys;

    switch(ty->typeTag){
        case TT_Ptr: {
            auto *ptr = (AnPtrType*)ty;
            return ptr->extTy->typeTag != TT_Void ?
                anTypeToLlvmType(ptr->extTy)->getPointerTo()
                : Type::getInt8Ty(*ctxt)->getPointerTo();
        }
        case TT_Array:{
            auto *arr = (AnArrayType*)ty;
            return ArrayType::get(anTypeToLlvmType(arr->extTy), arr->len);
        }
        case TT_Tuple:
            for(auto *e : ((AnAggregateType*)ty)->extTys){
                tys.push_back(anTypeToLlvmType(e));
            }
            return StructType::get(*ctxt, tys);
        case TT_Data: case TT_TaggedUnion: {
            auto *dt = ((AnDataType*)ty);
            if(dt->isStub())
                compErr("Use of undeclared type " + dt->name);

            if(dt->llvmType)
                return dt->llvmType;
            else
                return updateLlvmTypeBinding(this, dt);
        }
        case TT_Function: case TT_MetaFunction: {
            auto *f = ((AnAggregateType*)ty);
            for(size_t i = 1; i < f->extTys.size(); i++){
                tys.push_back(anTypeToLlvmType(f->extTys[i]));
            }

            return FunctionType::get(anTypeToLlvmType(f->extTys[0]), tys, false)->getPointerTo();
        }
        case TT_TypeVar: {
            auto *tvt = (AnTypeVarType*)ty;
            Variable *typeVar = lookup(tvt->name);
            if(!typeVar){
                //compErr("Use of undeclared type variable " + ty->typeName, ty->loc);
                //compErr("tn2llvmt: TypeVarError; lookup for "+ty->typeName+" not found", ty->loc);
                //throw new TypeVarError();
                cerr << "Warning: cannot translate undeclared typevar " << tvt->name << endl;
                return Type::getInt32Ty(*ctxt);
            }

            return anTypeToLlvmType(extractTypeValue(typeVar->tval));
        }
        default:
            return typeTagToLlvmType(ty->typeTag, *ctxt);
    }
}


/*
 *  Returns true if two given types are approximately equal.  This will return
 *  true if they are the same primitive datatype, or are both pointers pointing
 *  to the same elementtype, or are both arrays of the same element type, even
 *  if the arrays differ in size.  If two types are needed to be exactly equal,
 *  pointer comparison can be used instead since llvm::Types are uniqued.
 */
bool llvmTypeEq(Type *l, Type *r){
    TypeTag ltt = llvmTypeToTypeTag(l);
    TypeTag rtt = llvmTypeToTypeTag(r);

    if(ltt != rtt) return false;

    if(ltt == TT_Ptr){
        Type *lty = l->getPointerElementType();
        Type *rty = r->getPointerElementType();

        if(lty->isVoidTy() or rty->isVoidTy()) return true;

        return llvmTypeEq(lty, rty);
    }else if(ltt == TT_Array){
        return l->getArrayElementType() == r->getArrayElementType() and
               l->getArrayNumElements() == r->getArrayNumElements();
    }else if(ltt == TT_Function or ltt == TT_MetaFunction){
        int lParamCount = l->getFunctionNumParams();
        int rParamCount = r->getFunctionNumParams();

        if(lParamCount != rParamCount)
            return false;

        for(int i = 0; i < lParamCount; i++){
            if(!llvmTypeEq(l->getFunctionParamType(i), r->getFunctionParamType(i)))
                return false;
        }
        return true;
    }else if(ltt == TT_Tuple or ltt == TT_Data){
        int lElemCount = l->getStructNumElements();
        int rElemCount = r->getStructNumElements();

        if(lElemCount != rElemCount)
            return false;

        for(int i = 0; i < lElemCount; i++){
            if(!llvmTypeEq(l->getStructElementType(i), r->getStructElementType(i)))
                return false;
        }

        return true;
    }else{ //primitive type
        return true; /* true since ltt != rtt check above is false */
    }
}


TypeCheckResult TypeCheckResult::success(){
    if(box->res != Failure){
        box->matches++;
    }
    return *this;
}

TypeCheckResult TypeCheckResult::successWithTypeVars(){
    if(box->res != Failure){
        box->res = SuccessWithTypeVars;
    }
    return *this;
}

TypeCheckResult TypeCheckResult::failure(){
    box->res = Failure;
    return *this;
}

TypeCheckResult TypeCheckResult::successIf(bool b){
    if(b) return success();
    else  return failure();
}

TypeCheckResult TypeCheckResult::successIf(Result r){
    if(r == Success)
        return success();
    else if(r == SuccessWithTypeVars)
        return successWithTypeVars();
    else
        return failure();
}

bool TypeCheckResult::failed(){
    return box->res == Failure;
}


//forward decl of typeEqHelper for extTysEq fn
TypeCheckResult typeEqHelper(const Compiler *c, const AnType *l, const AnType *r, TypeCheckResult tcr);

/*
 *  Helper function to check if each type's list of extension
 *  types are all approximately equal.  Used when checking the
 *  equality of AnTypes of type Tuple, Data, Function, or any
 *  type with multiple extTys.
 */
TypeCheckResult extTysEq(const AnType *l, const AnType *r, TypeCheckResult &tcr, const Compiler *c = 0){
    auto *lAgg = (AnAggregateType*)l;
    auto *rAgg = (AnAggregateType*)r;

    if(lAgg->extTys.size() != rAgg->extTys.size())
        return tcr.failure();

    for(size_t i = 0; i < lAgg->extTys.size(); i++){
        auto *lExt = lAgg->extTys[i];
        auto *rExt = rAgg->extTys[i];

        if(c){
            if(!typeEqHelper(c, lExt, rExt, tcr)) return tcr.failure();
        }else{
            if(!typeEqBase(lExt, rExt, tcr)) return tcr.failure();
        }
    }
    return tcr.success();
}

/*
 *  Returns 1 if two types are approx eq
 *  Returns 2 if two types are approx eq and one is a typevar
 *
 *  Does not check for trait implementation unless c is set.
 *
 *  This function is used as a base for typeEq, if a typeEq function
 *  is needed that does not require a Compiler parameter, this can be
 *  used, although it does not check for trait impls.  The optional
 *  Compiler parameter here is only used by the typeEq function.  If
 *  this function is used as a typeEq function with the Compiler ptr
 *  the outermost type will not be checked for traits.
 */
TypeCheckResult typeEqBase(const AnType *l, const AnType *r, TypeCheckResult tcr, const Compiler *c){
    if(l == r) return tcr.success();

    if(l->typeTag == TT_TaggedUnion and r->typeTag == TT_Data)
        return tcr.successIf(((AnDataType*)l)->name == ((AnDataType*)r)->name);

    if(l->typeTag == TT_Data and r->typeTag == TT_TaggedUnion)
        return tcr.successIf(((AnDataType*)l)->name == ((AnDataType*)r)->name);

    if(l->typeTag == TT_TypeVar or r->typeTag == TT_TypeVar)
        return tcr.successWithTypeVars();


    if(l->typeTag != r->typeTag)
        return tcr.failure();

    if(r->typeTag == TT_Ptr){
        auto *lptr = (AnPtrType*)l;
        auto *rptr = (AnPtrType*)r;
        if(lptr->extTy->typeTag == TT_Void or rptr->extTy->typeTag == TT_Void)
            return tcr.success();

        return c ? typeEqHelper(c, lptr->extTy, rptr->extTy, tcr)
                 : typeEqBase(lptr->extTy, rptr->extTy, tcr, c);

    }else if(r->typeTag == TT_Array){
        auto *larr = (AnArrayType*)l;
        auto *rarr = (AnArrayType*)r;
        
        if(larr->len != rarr->len) return tcr.failure();

        return c ? typeEqHelper(c, larr->extTy, rarr->extTy, tcr)
                 : typeEqBase(larr->extTy, rarr->extTy, tcr, c);

    }else if(r->typeTag == TT_Data or r->typeTag == TT_TaggedUnion){
        auto *ldt = (AnDataType*)l;
        auto *rdt = (AnDataType*)r;
        return tcr.successIf(ldt->name == rdt->name);

    }else if(r->typeTag == TT_Function or r->typeTag == TT_MetaFunction or r->typeTag == TT_Tuple){
        return extTysEq(l, r, tcr, c);
    }
    //primitive type, we already know l->type == r->type
    return tcr.success();
}

bool dataTypeImplementsTrait(AnDataType *dt, string trait){
    for(auto traitImpl : dt->traitImpls){
        if(traitImpl->name == trait)
            return true;
    }
    return false;
}

AnType* TypeCheckResult::getBindingFor(const string &name){
    for(auto &pair : box->bindings){
        if(((AnTypeVarType*)pair.second)->name == name)
            return pair.second;
    }
    return 0;
}


/*
 *  Return true if both typenodes are approximately equal
 *
 *  Compiler instance required to check for trait implementation
 */
TypeCheckResult typeEqHelper(const Compiler *c, const AnType *l, const AnType *r, TypeCheckResult tcr){
    if(l == r) return tcr.success();
    if(!r) return tcr.failure();

    if((l->typeTag == TT_Data or l->typeTag == TT_TaggedUnion) and (r->typeTag == TT_Data or r->typeTag == TT_TaggedUnion)){
        auto *ldt = (AnDataType*)l;
        auto *rdt = (AnDataType*)r;

        if(ldt->name == rdt->name)
            return tcr.success();

        if(ldt->unboundType == rdt or rdt->unboundType == ldt){
            //TODO: return bind params
        }

        //typeName's are different, check if one is a trait and the other
        //is an implementor of the trait
        Trait *t;
        AnDataType *dt;
        if((t = c->lookupTrait(ldt->name))){
            dt = c->lookupType(rdt->name);
            if(!dt) return tcr.failure();
        }else if((t = c->lookupTrait(rdt->name))){
            dt = c->lookupType(ldt->name);
            if(!dt) return tcr.failure();
        }else{
            return tcr.failure();
        }

        return tcr.successIf(dataTypeImplementsTrait(dt, t->name));

    }else if(l->typeTag == TT_TypeVar or r->typeTag == TT_TypeVar){

        //reassign l and r into typeVar and nonTypeVar so code does not have to be repeated in
        //one if branch for l and another for r
        AnTypeVarType *typeVar;
        AnType *nonTypeVar;

        if(l->typeTag == TT_TypeVar and r->typeTag != TT_TypeVar){
            typeVar = (AnTypeVarType*)l;
            nonTypeVar = (AnType*)r;
        }else if(l->typeTag != TT_TypeVar and r->typeTag == TT_TypeVar){
            typeVar = (AnTypeVarType*)r;
            nonTypeVar = (AnType*)l;
        }else{ //both type vars
            auto *ltv = (AnTypeVarType*)l;
            auto *rtv = (AnTypeVarType*)r;
            
            Variable *lv = c->lookup(ltv->name);
            Variable *rv = c->lookup(rtv->name);

            if(lv and rv){ //both are already bound
                auto lty = extractTypeValue(lv->tval);
                auto rty = extractTypeValue(rv->tval);
                return typeEqHelper(c, lty, rty, tcr);
            }else if(lv and not rv){
                typeVar = rtv;
                nonTypeVar = extractTypeValue(lv->tval);
            }else if(rv and not lv){
                typeVar = ltv;
                nonTypeVar = extractTypeValue(rv->tval);
            }else{ //neither are bound
                return tcr.successIf(ltv->name == rtv->name);
            }
        }

        auto *tv = tcr.getBindingFor(typeVar->name);
        if(!tv){
            tcr->bindings.emplace_back(typeVar->name, nonTypeVar);

            return tcr.successWithTypeVars();
        }else{ //tv is bound in same typechecking run
            return typeEqHelper(c, tv, nonTypeVar, tcr);
        }
    }
    return typeEqBase(l, r, tcr, c);
}

TypeCheckResult Compiler::typeEq(const AnType *l, const AnType *r) const{
    auto tcr = TypeCheckResult();
    typeEqHelper(this, l, r, tcr);
    return tcr;
}


TypeCheckResult Compiler::typeEq(vector<AnType*> l, vector<AnType*> r) const{
    auto tcr = TypeCheckResult();
    if(l.size() != r.size()){
        tcr.failure();
        return tcr;
    }

    for(size_t i = 0; i < l.size(); i++){
        typeEqHelper(this, l[i], r[i], tcr);
        if(tcr.failed()) return tcr;
    }
    return tcr;
}


/*
 *  Returns true if the given typetag is a primitive type, and thus
 *  accurately represents the entire type without information loss.
 *  NOTE: this function relies on the fact all primitive types are
 *        declared before non-primitive types in the TypeTag definition.
 */
bool isPrimitiveTypeTag(TypeTag ty){
    return ty >= TT_I8 and ty <= TT_Bool;
}


/*
 *  Converts a TypeTag to its string equivalent for
 *  helpful error messages.  For most cases, llvmTypeToStr
 *  should be used instead to provide the full type.
 */
string typeTagToStr(TypeTag ty){

    switch(ty){
        case TT_I8:    return "i8" ;
        case TT_I16:   return "i16";
        case TT_I32:   return "i32";
        case TT_I64:   return "i64";
        case TT_U8:    return "u8" ;
        case TT_U16:   return "u16";
        case TT_U32:   return "u32";
        case TT_U64:   return "u64";
        case TT_F16:   return "f16";
        case TT_F32:   return "f32";
        case TT_F64:   return "f64";
        case TT_Isz:   return "isz";
        case TT_Usz:   return "usz";
        case TT_C8:    return "c8" ;
        case TT_C32:   return "c32";
        case TT_Bool:  return "bool";
        case TT_Void:  return "void";

        /*
         * Because of the loss of specificity for these last four types,
         * these strings are most likely insufficient.  The llvm::Type
         * should instead be printed for these types
         */
        case TT_Tuple:        return "Tuple";
        case TT_Array:        return "Array";
        case TT_Ptr:          return "Ptr"  ;
        case TT_Data:         return "Data" ;
        case TT_TypeVar:      return "'t";
        case TT_Function:     return "Function";
        case TT_MetaFunction: return "Meta Function";
        case TT_FunctionList: return "Function List";
        case TT_TaggedUnion:  return "|";
        case TT_Type:         return "type";
        case TT_Modifier:     return "modifier";
        default:              return "(Unknown TypeTag " + to_string(ty) + ")";
    }
}

/*
 *  Converts a typeNode directly to a string with no information loss.
 *  Used in ExtNode::compile
 */
string typeNodeToStr(const TypeNode *t){
    if(!t) return "null";

    if(t->type == TT_Tuple){
        string ret = "(";
        TypeNode *elem = t->extTy.get();
        while(elem){
            if(elem->next.get())
                ret += typeNodeToStr(elem) + ", ";
            else
                ret += typeNodeToStr(elem) + ")";
            elem = (TypeNode*)elem->next.get();
        }
        return ret;
    }else if(t->type == TT_Data or t->type == TT_TaggedUnion or t->type == TT_TypeVar){
        string name = t->typeName;
        if(!t->params.empty()){
            name += "<";
            name += typeNodeToStr(t->params[0].get());
            for(unsigned i = 1; i < t->params.size(); i++){
                name += ", ";
                name += typeNodeToStr(t->params[i].get());
            }
            name += ">";
        }
        return name;
    }else if(t->type == TT_Array){
        auto *len = (IntLitNode*)t->extTy->next.get();
        return '[' + len->val + " " + typeNodeToStr(t->extTy.get()) + ']';
    }else if(t->type == TT_Ptr){
        return typeNodeToStr(t->extTy.get()) + "*";
    }else if(t->type == TT_Function or t->type == TT_MetaFunction){
        string ret = "(";
        string retTy = typeNodeToStr(t->extTy.get());
        TypeNode *cur = (TypeNode*)t->extTy->next.get();
        while(cur){
            ret += typeNodeToStr(cur);
            cur = (TypeNode*)cur->next.get();
            if(cur) ret += ",";
        }
        return ret + ")->" + retTy;
    }else{
        return typeTagToStr(t->type);
    }
}


string anTypeToStr(const AnType *t){
    if(auto *dt = dyn_cast<AnDataType>(t)){
        return dt->name;
    }else if(auto *tvt = dyn_cast<AnTypeVarType>(t)){
        return tvt->name;
    }else if(auto *f = dyn_cast<AnFunctionType>(t)){
        string ret = "(";
        string retTy = anTypeToStr(f->retTy);
        for(auto *param : f->extTys){
            ret += anTypeToStr(param);
            if(param) ret += ",";
        }
        return ret + ")->" + retTy;
    }else if(auto *tup = dyn_cast<AnAggregateType>(t)){
        string ret = "(";
        for(auto *ext : tup->extTys){
            if(ext != tup->extTys.back())
                ret += anTypeToStr(ext) + ", ";
            else
                ret += anTypeToStr(ext) + ")";
        }
        return ret;
    }else if(auto *arr = dyn_cast<AnArrayType>(t)){
        return '[' + to_string(arr->len) + " " + anTypeToStr(arr->extTy) + ']';
    }else if(auto *ptr = dyn_cast<AnPtrType>(t)){
        return anTypeToStr(ptr->extTy) + "*";
    }else{
        return typeTagToStr(t->typeTag);
    }
}

/*
 *  Returns a string representing the full type of ty.  Since it is converting
 *  from a llvm::Type, this will never return an unsigned integer type.
 *
 *  Gives output in a different terminal color intended for printing, use typeNodeToStr
 *  to get a type without print color.
 */
string llvmTypeToStr(Type *ty){
    if(!ty) return "(null)";

    TypeTag tt = llvmTypeToTypeTag(ty);
    if(isPrimitiveTypeTag(tt)){
        return typeTagToStr(tt);
    }else if(tt == TT_Tuple){
        //if(!ty->getStructName().empty())
        //    return string(ty->getStructName());

        string ret = "(";
        const unsigned size = ty->getStructNumElements();

        for(unsigned i = 0; i < size; i++){
            if(i == size-1){
                ret += llvmTypeToStr(ty->getStructElementType(i)) + ")";
            }else{
                ret += llvmTypeToStr(ty->getStructElementType(i)) + ", ";
            }
        }
        return ret;
    }else if(tt == TT_Array){
        return "[" + to_string(ty->getArrayNumElements()) + " " + llvmTypeToStr(ty->getArrayElementType()) + "]";
    }else if(tt == TT_Ptr){
        return llvmTypeToStr(ty->getPointerElementType()) + "*";
    }else if(tt == TT_Function){
        string ret = "func("; //TODO: get function return type
        const unsigned paramCount = ty->getFunctionNumParams();

        for(unsigned i = 0; i < paramCount; i++){
            if(i == paramCount-1)
                ret += llvmTypeToStr(ty->getFunctionParamType(i)) + ")";
            else
                ret += llvmTypeToStr(ty->getFunctionParamType(i)) + ", ";
        }
        return ret;
    }else if(tt == TT_TypeVar){
        return "(typevar)";
    }else if(tt == TT_Void){
        return "void";
    }
    return "(Unknown type)";
}

} //end of namespace ante
