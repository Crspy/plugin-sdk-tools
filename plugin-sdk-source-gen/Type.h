#pragma once
#include <string>
#include <vector>

using namespace std;

struct Token {
    enum Type {
        SPECIAL_CHAR,
        EPSILON,
        KEYWORD_CONST,
        INBUILT_TYPE_NAME,
        CUSTOM_TYPE_NAME,
        VOID_TYPE,
        CC_CDECL,
        CC_STDCALL,
        CC_THISCALL,
        CC_FASTCALL,
        NUMBER,
        KEYWORD_CLASS,
        KEYWORD_STRUCT,
        KEYWORD_ENUM
    };

    string value;
    Type type;

    Token() = default;
    Token(string const &_value, Type _type) : value(_value), type(_type) {}
};

class Type {
public:
    enum FunctionCC {
        CC_CDECL,
        CC_THISCALL,
        CC_STDCALL,
        CC_FASTCALL
    };

    string mName;

    // only for template types...
    bool mIsNumber = false;

    bool mIsEpsilon = false;

    bool mIsVoid = false;

    // array
    bool mIsPointerToFixedSizeArray = false;
    unsigned int mArraySize[2]; // 2D arrays are supported
    
    // const
    bool mIsConst = false;
    
    // type
    bool mIsInBuilt = false;
    bool mIsRenderWare = false;

    // pointer
    string mPointers;

    // function
    bool mIsFunction = false;
    FunctionCC mFunctionCC = CC_CDECL;
    vector<Type> mFunctionParams;
    Type *mFunctionRetType = nullptr;

    // template
    bool mIsTemplate = false;
    vector<Type> mTemplateTypes;

    // debug
    vector<Token> mDbgTokens;
    
    void SetFromString(string const &str);
    void SetFromTokens(vector<Token> const &tokens);
    string GetFullType() const;
    string BeforeName() const;
    string AfterName() const;

    bool IsPointer() const;
    bool IsTemplate() const;

    void SetFunctionTypeFromToken(Token const &t);

    void AddRetTypeForFunction();

    void DbgPrint(size_t offset = 0) const;
};