int ret() {
    int a;
    a = 10;
    if(a == 10)
        return 1;
    else
        return 0;
}

int main() {
    int a;
    a = 10;
    if(ret()) {
        return a;
    }
    else if(a == 20) {
        a = a * 5;
        return a;
    }
    else {
        return 200;
    }
}