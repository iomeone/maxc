#pragma once

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#include<iostream>
#include<fstream>
#include<string>
#include<vector>

//main
class Maxc {
    public:
        int run(std::string src);

        void show_usage();

    private:
};

//token
enum Token_type {
    TOKEN_TYPE_END,
    TOKEN_TYPE_NUM,
    TOKEN_TYPE_SYMBOL
};

typedef struct {
    Token_type type;
    std::string value;
} token_t;

class Token {
    public:
        std::vector<token_t> token_v;

        void push_num(std::string value);
        void push_symbol(std::string value);
        void push_end();

        void show();

        void to_asm();

    private:
};

//lexer
class Lexer {
    public:
        Token run(std::string src);
};

//parser, ast
enum nd_type {
    ND_TYPE_NUM = 100,
    ND_TYPE_PLUS = 101,
    ND_TYPE_MINUS = 102
};

struct ast_t {
    nd_type type;
    std::string value;
    ast_t *left;
    ast_t *right;
};

class Ast {
    public:
        int pos;
        ast_t *node;
        
        void *make(Token token);
        ast_t *eval(std::vector<token_t> tokens);
        ast_t *make_node(std::string value, ast_t *left, ast_t *right);
        ast_t *make_num_node(token_t token);
        void show();
        void print_pos(std::string msg);
};

class Parser {
    public:
        ast_t *run(Token token);
};

