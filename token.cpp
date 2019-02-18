#include"maxc.h"

void Token::push_num(std::string value, int line) {
    token_v.push_back((token_t){TOKEN_TYPE::NUM, value, line});
}

void Token::push_symbol(std::string value, int line) {
    token_v.push_back((token_t){TOKEN_TYPE::SYMBOL, value, line});
}

void Token::push_ident(std::string value, int line) {
    token_v.push_back((token_t){TOKEN_TYPE::IDENTIFER, value, line});
}

void Token::push_string(std::string value, int line) {
    token_v.push_back((token_t){TOKEN_TYPE::STRING, value, line});
}

void Token::push_char(std::string value, int line) {
    token_v.push_back((token_t){TOKEN_TYPE::CHAR, value, line});
}

void Token::push_end() {
    token_v.push_back((token_t){TOKEN_TYPE::END, "", 0});
}

token_t Token::get() {
    return token_v[pos];
}

token_t Token::get_step() {
    return token_v[pos++];
}

token_t Token::see(int p) {
    return token_v[pos + p];
}

bool Token::is_value(std::string tk) {
    return token_v[pos].value == tk;
}

bool Token::is_type(TOKEN_TYPE ty) {
    return token_v[pos].type == ty;
}

bool Token::is_type() {
    if(is_value("int") || is_value("char"))
        return true;
    else
        return false;
}

void Token::step() {
    pos++;
}

bool Token::skip(std::string val) {
    if(token_v[pos].value == val) {
        pos++;
        return true;
    }
    else
        return false;
}

bool Token::abs_skip(std::string val) {
    if(token_v[pos].value == val) {
        pos++;
        return true;
    }
    else {
        error("line %d: expected token \" %s \"", get().line, val.c_str());
        return false;
    }
}

bool Token::step_to(std::string val) {
   return token_v[pos++].value == val;
}

void Token::save() {
    save_point = pos;
    issaved = true;
}

void Token::rewind() {
    if(issaved) {
        pos = save_point;
        issaved = false;
    }
    else
        puts("[warning] you don't save");
}

void Token::show() {
    std::string literal;

    for(token_t token: token_v) {
        literal = [&]() -> std::string {
            if(token.type == TOKEN_TYPE::NUM)
                return "Number";
            else if(token.type == TOKEN_TYPE::SYMBOL)
                return "Symbol";
            else if(token.type == TOKEN_TYPE::IDENTIFER)
                return "Identifer";
            else if(token.type == TOKEN_TYPE::STRING)
                return "String";
            else if(token.type == TOKEN_TYPE::CHAR)
                return "Char";
            else if(token.type == TOKEN_TYPE::END)
                return "End";
            else {
                printf("???");
                exit(1);
            }
        }();

        std::cout << "line "<< token.line << ": " << literal << "( " << token.value << " )" << std::endl;
    }
}

