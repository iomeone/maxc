#include "token.h"
#include "error.h"
#include "maxc.h"

static enum TKIND ident2kw(String *);

struct KeyWordMap {
    char *key;
    int kind;
} kwmap[] = {
    {"int", TKIND_TInt},       {"bool", TKIND_TBool},
    {"string", TKIND_TString}, {"float", TKIND_TFloat},
    {"none", TKIND_TNone},     {"or", TKIND_KOr},
    {"and", TKIND_KAnd},       {"struct", TKIND_Struct},
    {"return", TKIND_Return},  {"if", TKIND_If},
    {"else", TKIND_Else},      {"for", TKIND_For},
    {"while", TKIND_While},    {"typedef", TKIND_Typedef},
    {"let", TKIND_Let},        {"fn", TKIND_Fn},
    {"true", TKIND_True},      {"false", TKIND_False},
    {"const", TKIND_Const},
};

Map *keywordmap;

enum TKIND tk_char1(int c) {
    switch(c) {
    case '+':
        return TKIND_Plus;
    case '-':
        return TKIND_Minus;
    case '*':
        return TKIND_Asterisk;
    case '/':
        return TKIND_Div;
    case '%':
        return TKIND_Mod;
    case '(':
        return TKIND_Lparen;
    case ')':
        return TKIND_Rparen;
    case '{':
        return TKIND_Lbrace;
    case '}':
        return TKIND_Rbrace;
    case '[':
        return TKIND_Lboxbracket;
    case ']':
        return TKIND_Rboxbracket;
    case ',':
        return TKIND_Comma;
    case ':':
        return TKIND_Colon;
    case ';':
        return TKIND_Semicolon;
    case '.':
        return TKIND_Dot;
    case '<':
        return TKIND_Lt;
    case '>':
        return TKIND_Gt;
    case '=':
        return TKIND_Assign;
    case '?':
        return TKIND_Question;
    default:
        error("internal error: %c", c);
        return -1;
    }
}

enum TKIND tk_char2(int c1, int c2) {
    switch(c1) {
    case '=':
        switch(c2) {
        case '=':
            return TKIND_Eq;
        }
    case '<':
        switch(c2) {
        case '=':
            return TKIND_Lte;
        }
    case '>':
        switch(c2) {
        case '=':
            return TKIND_Gte;
        }
    case '!':
        switch(c2) {
        case '=':
            return TKIND_Neq;
        }
    case '+':
        switch(c2) {
        case '+':
            return TKIND_Inc;
        case '=':
            return TKIND_PlusAs;
        }
    case '-':
        switch(c2) {
        case '-':
            return TKIND_Dec;
        case '=':
            return TKIND_MinusAs;
        case '>':
            return TKIND_Arrow;
        }
    case '*':
        switch(c2) {
        case '=':
            return TKIND_AsteriskAs;
        }
    case '/':
        switch(c2) {
        case '=':
            return TKIND_DivAs;
        }
    case '%':
        switch(c2) {
        case '=':
            return TKIND_ModAs;
        }
    default:
        error("internal error: %c%c", c1, c2);
        return -1;
    }
}

const char *tk2str(enum TKIND tk) {
    switch(tk) {
    case TKIND_End:
        return "End";
    case TKIND_Num:
        return "Number";
    case TKIND_String:
        return "String";
    case TKIND_Char:
        return "Char";
    case TKIND_Identifer:
        return "Identifer";
    case TKIND_TInt:
        return "int";
    case TKIND_TUint:
        return "uint";
    case TKIND_TInt64:
        return "int64";
    case TKIND_TUint64:
        return "uint64";
    case TKIND_TBool:
        return "bool";
    case TKIND_TChar:
        return "char";
    case TKIND_TString:
        return "string";
    case TKIND_TFloat:
        return "float";
    case TKIND_TNone:
        return "none";
    case TKIND_KAnd:
        return "and";
    case TKIND_KOr:
        return "or";
    case TKIND_Struct:
        return "struct";
    case TKIND_If:
        return "if";
    case TKIND_Else:
        return "else";
    case TKIND_For:
        return "for";
    case TKIND_While:
        return "while";
    case TKIND_Return:
        return "return";
    case TKIND_Typedef:
        return "typedef";
    case TKIND_Let:
        return "let";
    case TKIND_Fn:
        return "fn";
    case TKIND_True:
        return "true";
    case TKIND_False:
        return "false";
    case TKIND_Const:
        return "const";
    case TKIND_Lparen:
        return "(";
    case TKIND_Rparen:
        return ")";
    case TKIND_Lbrace:
        return "{";
    case TKIND_Rbrace:
        return "}";
    case TKIND_Lboxbracket:
        return "[";
    case TKIND_Rboxbracket:
        return "]";
    case TKIND_Comma:
        return ",";
    case TKIND_Colon:
        return ":";
    case TKIND_Dot:
        return ".";
    case TKIND_DotDot:
        return "..";
    case TKIND_Semicolon:
        return ";";
    case TKIND_Arrow:
        return "->";
    case TKIND_Inc:
        return "++";
    case TKIND_Dec:
        return "--";
    case TKIND_Plus:
        return "+";
    case TKIND_Minus:
        return "-";
    case TKIND_Asterisk:
        return "*";
    case TKIND_Div:
        return "/";
    case TKIND_Mod:
        return "%";
    case TKIND_Eq:
        return "==";
    case TKIND_Neq:
        return "!=";
    case TKIND_Lt:
        return "<";
    case TKIND_Lte:
        return "<=";
    case TKIND_Gt:
        return ">";
    case TKIND_Gte:
        return ">=";
    case TKIND_LogAnd:
        return "&&";
    case TKIND_LogOr:
        return "||";
    case TKIND_Assign:
        return "=";
    case TKIND_Question:
        return "?";
    default:
        return "error";
    }

    return "error";
}

void setup_token() {
    keywordmap = New_Map();

    int kwmap_len = sizeof(kwmap) / sizeof(kwmap[0]);

    for(int i = 0; i < kwmap_len; i++) {
        map_push(
            keywordmap, (void *)kwmap[i].key, (void *)(intptr_t)kwmap[i].kind);
    }
}

static Token *
New_Token(enum TKIND kind, String *value, Location s, Location e) {
    Token *self = malloc(sizeof(Token));

    self->kind = kind;
    self->value = value->data;
    self->len = value->len;
    self->start = s;
    self->end = e;

    return self;
}

static Token *
New_Token_With_Symbol(enum TKIND kind, uint8_t len, Location s, Location e) {
    Token *self = malloc(sizeof(Token));

    self->kind = kind;
    self->value = "";
    self->len = len;
    self->start = s;
    self->end = e;

    return self;
}

static Token *New_Token_With_End(Location s, Location e) {
    Token *self = malloc(sizeof(Token));

    self->kind = TKIND_End;
    self->value = "";
    self->len = 1;
    self->start = s;
    self->end = e;

    return self;
}

void token_push_num(Vector *self, String *value, Location s, Location e) {
    Token *tk = New_Token(TKIND_Num, value, s, e);

    vec_push(self, tk);
}

void token_push_ident(Vector *self, String *value, Location s, Location e) {
    enum TKIND kind = ident2kw(value);

    Token *tk = New_Token(kind, value, s, e);

    vec_push(self, tk);
}

void token_push_symbol(
    Vector *self, enum TKIND kind, uint8_t len, Location s, Location e) {
    Token *tk = New_Token_With_Symbol(kind, len, s, e);

    vec_push(self, tk);
}

void token_push_string(Vector *self, String *str, Location s, Location e) {
    Token *tk = New_Token(TKIND_String, str, s, e);

    vec_push(self, tk);
}

void token_push_end(Vector *self, Location s, Location e) {
    Token *tk = New_Token_With_End(s, e);

    vec_push(self, tk);
}

static enum TKIND ident2kw(String *k) {
    for(int i = 0; i < keywordmap->key->len; i++) {
        if(k->len != strlen((char *)keywordmap->key->data[i]))
            continue;

        if(strncmp(k->data,
                   (char *)keywordmap->key->data[i],
                   strlen((char *)keywordmap->key->data[i])) == 0)
            return (intptr_t)keywordmap->value->data[i];
    }

    return TKIND_Identifer;
}

#ifdef MXC_DEBUG
void tokendump(Vector *token) {
    for(int i = 0; i < token->len; ++i) {
        printf("kind: %s\t\t", tk2str(((Token *)token->data[i])->kind));
        printf("value: %s\t\t", ((Token *)token->data[i])->value);
        printf("len: %d\n", ((Token *)token->data[i])->len);
    }
}
#endif
