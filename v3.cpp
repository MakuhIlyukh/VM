// v3
#include <iostream>
#include <string>
#include <list>
using std::string;
using std::cout;

enum Kind {
    FUNCTION,
    FLOAT,
    INT,
    STRING,
    CHAR,
    LIST,
    BOOL, 
    OTHER,
    NULLPTR
};

struct WSEML {
    WSEML* d;
    WSEML* s;
    Kind kind;
    WSEML() {
        kind = OTHER;
        d = nullptr;
        s = nullptr;
    }
};

struct Int : WSEML {
    int val;
    Int() {
        kind = INT;
        val = 0;
    }
    Int(int v) {
        kind = INT;
        val = v;
    }
};

struct Float : WSEML {
    float val;
    Float() {
        kind = FLOAT;
        val = 0;
    }
    Float(float f) {
        kind = FLOAT;
        val = f;
    }
};

struct Char : WSEML {
    char val;
    Char() {
        kind = CHAR;
        val = 0;
    }
    Char(char c) {
        kind = CHAR;
        val = c;
    }
};

struct Bool : WSEML {
    bool val;
    Bool() {
        kind = BOOL;
        val = false;
    }
    Bool(bool b) {
        kind = BOOL;
        val = b;
    }
};

struct String : WSEML {
    string val;
    String() {
        kind = STRING;
        val = "";
    }
    String(string s) {
        kind = STRING;
        val = s;
    }
};

struct Function : WSEML {
    void (*fun)(WSEML*);
    Function() {
        kind = FUNCTION;
        fun = nullptr;
    }
    Function(void (*f)(WSEML*)) {
        kind = FUNCTION;
        fun = f;
    }
};

struct List : WSEML {
    std::list<WSEML*> l;
    List() {
        kind = LIST;
    }
    void push_back(WSEML* w) {
        l.push_back(w);
    }
    
    void pop_back() {
        l.pop_back();
    }
    
    void push_front(WSEML* w) {
        l.push_front(w);
    }
    
    void pop_front() {
        l.pop_front();
    }
    
    WSEML* front() {
        if (size() == 0)
            return nullptr;
        return l.front();
    }
    
    WSEML* back() {
        if (size() == 0)
            return nullptr;
        return l.back();
    }
    
    WSEML* get(const int i) {
        auto it = l.begin();
        for (int j = 0; j < i; j++)
            ++it;
        return *it;
    };
    
    void set(const int i, WSEML* w) {
        auto it = l.begin();
        for (int j = 0; j < i; j++)
            ++it;
        *it = w;
    }
    
    int size() {
        return l.size();
    }
    
    bool empty() {
        return size() == 0;
    }
};

struct NullPtr : WSEML {
    NullPtr() {
        kind = NULLPTR;
    }
};

WSEML* get(WSEML* univ, WSEML* c) { // DANGER 
    // возращает значение команды
    // а также реализация индексирования по вселенной
    // ["i", [2]] <=> univ[2]
    // ["i", [3, 4]] <=> univ[3][4]
    // ["i", [ 3, 4, 5]] <=> univ[3][4][5]
    // ["i", [["i", 3], 4]] <=> univ[univ[3]->val][4]
    // ["i", [["command", ["i",...] -- res, ...]]] <=> univ[["i", ...]->val -- res]
    // ["sum", ["i", X] -- res, ...] <=> univ[X]
    /*Kind t = c->kind;
    if(t == INT || t == FLOAT || t == CHAR || t == STRING || t == BOOL || t == NULLPTR) {
        return c;
    }
    List* l = (List*) c;
    if ( ((String*)l->get(0))->val == "i") {
        if (l->size() == 2) {
            return ((List*) univ)->get( ( (Int*)(get(univ, l->get(1))) )->val  );
        } else {
            return ( (List*) get(univ, l->get(1)) )->get(  ( (Int*)(get(univ, l->get(2))) )->val );
        }
    } else {
        return get(univ, l->get(1));
    }*/
    Kind t = c->kind;
    if(t == INT || t == FLOAT || t == CHAR || t == STRING || t == BOOL || t == NULLPTR) {
        return c;
    }
    List* l = (List*) c;
    if ( ((String*)l->get(0))->val == "i") {
        List* args = (List*) l->get(1);
        WSEML* res = univ;
        for (int i = 0; i < args->size(); i++) {
            res = ((List*) res)->get( ( (Int*)(get(univ, args->get(i))) )->val  );
        }
        return res;
    } else {
        return get(univ, l->get(1));
    }
    
}
void set(WSEML* univ, WSEML* lval, WSEML* rval) { // DANGER
    List* l = (List*) lval;
    if ( ((String*)l->get(0))->val == "i") {
        List* args = (List*) l->get(1);
        WSEML* lres = univ;
        for (int i = 0; i < args->size() - 1; i++) {
            lres = ((List*) lres)->get( ( (Int*)(get(univ, args->get(i))) )->val  );
        }
        ((List*) lres)->set( ( (Int*)(get(univ, args->get(args->size()-1))) )->val, get(univ, rval));
    } else {
        set(univ, l->get(1), rval);
    }
}
// Реализация встроенных функций
void sum(WSEML* univ) { // сумма
    List* globVar = (List* ) ((List*) univ)->get(1);
    List* stack = (List* ) ((List*) univ)->get(3);
    List* com = (List*) stack->back();
    // com = ['sum', ["i"]-указатель на res, [c-указатель на lval-команду, c-указатель на rval-команду]]
    //          0             1                           2,0                        2, 1
    // получаем значение lval
    WSEML* lval = get(univ, ((List*) com->get(2))->get(0));
    WSEML* rval = get(univ, ((List*) com->get(2))->get(1));
    
    WSEML* res;
    switch(lval->kind) {
        case INT:
            switch(rval->kind) {
                case INT:
                    res = new Int(  ((Int*) lval)->val + ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Float(  ((Int*) lval)->val + ((Float*) rval)->val  );
                    break;
            }
            break;
        case FLOAT:
            switch(rval->kind) {
                case INT:
                    res = new Float(  ((Float*) lval)->val + ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Float(  ((Float*) lval)->val + ((Float*) rval)->val  );
                    break;
            }
            break;
    }
    int size = globVar->size();
    globVar->push_back(res);
    
    List* pointer = new List(); // ["i", [1, size]] <=> univ[1][size]
    pointer->push_back(new String("i"));
    pointer->push_back(new List());
    ( (List*) (pointer->get(1)) )->push_back(new Int(1));
    ( (List*) (pointer->get(1)) )->push_back(new Int(size));
    
    com->set(1, pointer);
}
void subs(WSEML* univ) { // разность
    List* globVar = (List* ) ((List*) univ)->get(1);
    List* stack = (List* ) ((List*) univ)->get(3);
    List* com = (List*) stack->back();
    // com = ['subs', указатель на res, c-указатель на lval-команду, c-указатель на rval-команду]
    //          0             1                  2                             3
    // получаем значение lval
    WSEML* lval = get(univ, ((List*) com->get(2))->get(0));
    WSEML* rval = get(univ, ((List*) com->get(2))->get(1));
    
    WSEML* res;
    switch(lval->kind) {
        case INT:
            switch(rval->kind) {
                case INT:
                    res = new Int(  ((Int*) lval)->val - ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Float(  ((Int*) lval)->val - ((Float*) rval)->val  );
                    break;
            }
            break;
        case FLOAT:
            switch(rval->kind) {
                case INT:
                    res = new Float(  ((Float*) lval)->val - ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Float(  ((Float*) lval)->val - ((Float*) rval)->val  );
                    break;
            }
            break;
    }
    int size = globVar->size();
    globVar->push_back(res);
    
    List* pointer = new List(); // ["i", [1, size]] <=> univ[1][size]
    pointer->push_back(new String("i"));
    pointer->push_back(new List());
    ( (List*) (pointer->get(1)) )->push_back(new Int(1));
    ( (List*) (pointer->get(1)) )->push_back(new Int(size));
    
    com->set(1, pointer);
}
void mult(WSEML* univ) { // умножение
    List* globVar = (List* ) ((List*) univ)->get(1);
    List* stack = (List* ) ((List*) univ)->get(3);
    List* com = (List*) stack->back();
    // com = ['mult', указатель на res, c-указатель на lval-команду, c-указатель на rval-команду]
    //          0             1                  2                             3
    // получаем значение lval
    WSEML* lval = get(univ, ((List*) com->get(2))->get(0));
    WSEML* rval = get(univ, ((List*) com->get(2))->get(1));
    
    WSEML* res;
    switch(lval->kind) {
        case INT:
            switch(rval->kind) {
                case INT:
                    res = new Int(  ((Int*) lval)->val * ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Float(  ((Int*) lval)->val * ((Float*) rval)->val  );
                    break;
            }
            break;
        case FLOAT:
            switch(rval->kind) {
                case INT:
                    res = new Float(  ((Float*) lval)->val * ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Float(  ((Float*) lval)->val * ((Float*) rval)->val  );
                    break;
            }
            break;
    }
    int size = globVar->size();
    globVar->push_back(res);
    
    List* pointer = new List(); // ["i", [1, size]] <=> univ[1][size]
    pointer->push_back(new String("i"));
    pointer->push_back(new List());
    ( (List*) (pointer->get(1)) )->push_back(new Int(1));
    ( (List*) (pointer->get(1)) )->push_back(new Int(size));
    
    com->set(1, pointer);
}
void div(WSEML* univ) { // деление
    List* globVar = (List* ) ((List*) univ)->get(1);
    List* stack = (List* ) ((List*) univ)->get(3);
    List* com = (List*) stack->back();
    // com = ['div', указатель на res, c-указатель на lval-команду, c-указатель на rval-команду]
    //          0             1                  2                             3
    // получаем значение lval
    WSEML* lval = get(univ, ((List*) com->get(2))->get(0));
    WSEML* rval = get(univ, ((List*) com->get(2))->get(1));
    
    WSEML* res;
    switch(lval->kind) {
        case INT:
            switch(rval->kind) {
                case INT:
                    res = new Int(  ((Int*) lval)->val / ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Float(  ((Int*) lval)->val / ((Float*) rval)->val  );
                    break;
            }
            break;
        case FLOAT:
            switch(rval->kind) {
                case INT:
                    res = new Float(  ((Float*) lval)->val / ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Float(  ((Float*) lval)->val / ((Float*) rval)->val  );
                    break;
            }
            break;
    }
    int size = globVar->size();
    globVar->push_back(res);
    
    List* pointer = new List(); // ["i", [1, size]] <=> univ[1][size]
    pointer->push_back(new String("i"));
    pointer->push_back(new List());
    ( (List*) (pointer->get(1)) )->push_back(new Int(1));
    ( (List*) (pointer->get(1)) )->push_back(new Int(size));
    
    com->set(1, pointer);
}
void mod(WSEML* univ) { // остаток
    List* globVar = (List* ) ((List*) univ)->get(1);
    List* stack = (List* ) ((List*) univ)->get(3);
    List* com = (List*) stack->back();
    // com = ['mod', указатель на res, c-указатель на lval-команду, c-указатель на rval-команду]
    //          0             1                  2                             3
    // получаем значение lval
    WSEML* lval = get(univ, ((List*) com->get(2))->get(0));
    WSEML* rval = get(univ, ((List*) com->get(2))->get(1));
    
    WSEML* res;
    switch(lval->kind) {
        case INT:
            switch(rval->kind) {
                case INT:
                    res = new Int(  ((Int*) lval)->val % ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    throw "MOD";
                    break;
            }
            break;
        case FLOAT:
            switch(rval->kind) {
                case INT:
                    throw "MOD";
                    break;
                case FLOAT:
                    throw "MOD";
                    break;
            }
            break;
    }
    int size = globVar->size();
    globVar->push_back(res);
    
    List* pointer = new List(); // ["i", [1, size]] <=> univ[1][size]
    pointer->push_back(new String("i"));
    pointer->push_back(new List());
    ( (List*) (pointer->get(1)) )->push_back(new Int(1));
    ( (List*) (pointer->get(1)) )->push_back(new Int(size));
    
    com->set(1, pointer);
}
void valeq(WSEML* univ) { // a->val == b->val сравнение по значению
    List* globVar = (List* ) ((List*) univ)->get(1);
    List* stack = (List* ) ((List*) univ)->get(3);
    List* com = (List*) stack->back();
    // com = ['valeq', указатель на res, c-указатель на lval-команду, c-указатель на rval-команду]
    //          0             1                  2                             3
    // получаем значение lval, rval
    WSEML* lval = get(univ, ((List*) com->get(2))->get(0));
    WSEML* rval = get(univ, ((List*) com->get(2))->get(1));
    
    WSEML* res;
    switch(lval->kind) {
        case INT:
            switch(rval->kind) {
                case INT:
                    res = new Bool(  ((Int*) lval)->val == ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Bool(  ((Int*) lval)->val == ((Float*) rval)->val  );
                    break;
                case CHAR:
                    res = new Bool(  ((Int*) lval)->val == ((Char*) rval)->val  );
                    break;
                case BOOL:
                    res = new Bool(  ((Int*) lval)->val == ((Bool*) rval)->val  );
                    break;
            }
            break;
        case FLOAT:
            switch(rval->kind) {
                case INT:
                    res = new Bool(  ((Float*) lval)->val == ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Bool(  ((Float*) lval)->val == ((Float*) rval)->val  );
                    break;
                case CHAR:
                    res = new Bool(  ((Float*) lval)->val == ((Char*) rval)->val  );
                    break;
                case BOOL:
                    res = new Bool(  ((Float*) lval)->val == ((Bool*) rval)->val  );
                    break;
            }
            break;
        case CHAR:
            switch(rval->kind) {
                case INT:
                    res = new Bool(  ((Char*) lval)->val == ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Bool(  ((Char*) lval)->val == ((Float*) rval)->val  );
                    break;
                case CHAR:
                    res = new Bool(  ((Char*) lval)->val == ((Char*) rval)->val  );
                    break;
                case BOOL:
                    res = new Bool(  ((Char*) lval)->val == ((Bool*) rval)->val  );
                    break;
            }
            break;
        case BOOL:
            switch(rval->kind) {
                case INT:
                    res = new Bool(  ((Bool*) lval)->val == ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Bool(  ((Bool*) lval)->val == ((Float*) rval)->val  );
                    break;
                case CHAR:
                    res = new Bool(  ((Bool*) lval)->val == ((Char*) rval)->val  );
                    break;
                case BOOL:
                    res = new Bool(  ((Bool*) lval)->val == ((Bool*) rval)->val  );
                    break;
            }
            break;
        case STRING:
            switch(rval->kind) {
                case STRING:
                    res = new Bool(  ((String*) lval)->val == ((String*) rval)->val  );
                    break;
            }
            break;
    }
    int size = globVar->size();
    globVar->push_back(res);
    
    List* pointer = new List(); // ["i", [1, size]] <=> univ[1][size]
    pointer->push_back(new String("i"));
    pointer->push_back(new List());
    ( (List*) (pointer->get(1)) )->push_back(new Int(1));
    ( (List*) (pointer->get(1)) )->push_back(new Int(size));
    
    com->set(1, pointer);
}
void more(WSEML* univ) { // a->val > b->val сравнение по значению
    List* globVar = (List* ) ((List*) univ)->get(1);
    List* stack = (List* ) ((List*) univ)->get(3);
    List* com = (List*) stack->back();
    // com = ['more', указатель на res, c-указатель на lval-команду, c-указатель на rval-команду]
    //          0             1                  2                             3
    // получаем значение lval, rval
    WSEML* lval = get(univ, ((List*) com->get(2))->get(0));
    WSEML* rval = get(univ, ((List*) com->get(2))->get(1));
    
    WSEML* res;
    switch(lval->kind) {
        case INT:
            switch(rval->kind) {
                case INT:
                    res = new Bool(  ((Int*) lval)->val > ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Bool(  ((Int*) lval)->val > ((Float*) rval)->val  );
                    break;
                case CHAR:
                    res = new Bool(  ((Int*) lval)->val > ((Char*) rval)->val  );
                    break;
                case BOOL:
                    res = new Bool(  ((Int*) lval)->val > ((Bool*) rval)->val  );
                    break;
            }
            break;
        case FLOAT:
            switch(rval->kind) {
                case INT:
                    res = new Bool(  ((Float*) lval)->val > ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Bool(  ((Float*) lval)->val > ((Float*) rval)->val  );
                    break;
                case CHAR:
                    res = new Bool(  ((Float*) lval)->val > ((Char*) rval)->val  );
                    break;
                case BOOL:
                    res = new Bool(  ((Float*) lval)->val > ((Bool*) rval)->val  );
                    break;
            }
            break;
        case CHAR:
            switch(rval->kind) {
                case INT:
                    res = new Bool(  ((Char*) lval)->val > ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Bool(  ((Char*) lval)->val > ((Float*) rval)->val  );
                    break;
                case CHAR:
                    res = new Bool(  ((Char*) lval)->val > ((Char*) rval)->val  );
                    break;
                case BOOL:
                    res = new Bool(  ((Char*) lval)->val > ((Bool*) rval)->val  );
                    break;
            }
            break;
        case BOOL:
            switch(rval->kind) {
                case INT:
                    res = new Bool(  ((Bool*) lval)->val > ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Bool(  ((Bool*) lval)->val > ((Float*) rval)->val  );
                    break;
                case CHAR:
                    res = new Bool(  ((Bool*) lval)->val > ((Char*) rval)->val  );
                    break;
                case BOOL:
                    res = new Bool(  ((Bool*) lval)->val > ((Bool*) rval)->val  );
                    break;
            }
            break;
        case STRING:
            switch(rval->kind) {
                case STRING:
                    res = new Bool(  ((String*) lval)->val > ((String*) rval)->val  );
                    break;
            }
            break;
    }
    int size = globVar->size();
    globVar->push_back(res);
    
    List* pointer = new List(); // ["i", [1, size]] <=> univ[1][size]
    pointer->push_back(new String("i"));
    pointer->push_back(new List());
    ( (List*) (pointer->get(1)) )->push_back(new Int(1));
    ( (List*) (pointer->get(1)) )->push_back(new Int(size));
    
    com->set(1, pointer);
}
void less(WSEML* univ) { // a->val < b->val сравнение по значению
    List* globVar = (List* ) ((List*) univ)->get(1);
    List* stack = (List* ) ((List*) univ)->get(3);
    List* com = (List*) stack->back();
    // com = ['less', указатель на res, c-указатель на lval-команду, c-указатель на rval-команду]
    //          0             1                  2                             3
    // получаем значение lval, rval
    WSEML* lval = get(univ, ((List*) com->get(2))->get(0));
    WSEML* rval = get(univ, ((List*) com->get(2))->get(1));
    
    WSEML* res;
    switch(lval->kind) {
        case INT:
            switch(rval->kind) {
                case INT:
                    res = new Bool(  ((Int*) lval)->val < ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Bool(  ((Int*) lval)->val < ((Float*) rval)->val  );
                    break;
                case CHAR:
                    res = new Bool(  ((Int*) lval)->val < ((Char*) rval)->val  );
                    break;
                case BOOL:
                    res = new Bool(  ((Int*) lval)->val < ((Bool*) rval)->val  );
                    break;
            }
            break;
        case FLOAT:
            switch(rval->kind) {
                case INT:
                    res = new Bool(  ((Float*) lval)->val < ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Bool(  ((Float*) lval)->val < ((Float*) rval)->val  );
                    break;
                case CHAR:
                    res = new Bool(  ((Float*) lval)->val < ((Char*) rval)->val  );
                    break;
                case BOOL:
                    res = new Bool(  ((Float*) lval)->val < ((Bool*) rval)->val  );
                    break;
            }
            break;
        case CHAR:
            switch(rval->kind) {
                case INT:
                    res = new Bool(  ((Char*) lval)->val < ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Bool(  ((Char*) lval)->val < ((Float*) rval)->val  );
                    break;
                case CHAR:
                    res = new Bool(  ((Char*) lval)->val < ((Char*) rval)->val  );
                    break;
                case BOOL:
                    res = new Bool(  ((Char*) lval)->val < ((Bool*) rval)->val  );
                    break;
            }
            break;
        case BOOL:
            switch(rval->kind) {
                case INT:
                    res = new Bool(  ((Bool*) lval)->val < ((Int*) rval)->val  );
                    break;
                case FLOAT:
                    res = new Bool(  ((Bool*) lval)->val < ((Float*) rval)->val  );
                    break;
                case CHAR:
                    res = new Bool(  ((Bool*) lval)->val < ((Char*) rval)->val  );
                    break;
                case BOOL:
                    res = new Bool(  ((Bool*) lval)->val < ((Bool*) rval)->val  );
                    break;
            }
            break;
        case STRING:
            switch(rval->kind) {
                case STRING:
                    res = new Bool(  ((String*) lval)->val < ((String*) rval)->val  );
                    break;
            }
            break;
    }
    int size = globVar->size();
    globVar->push_back(res);
    
    List* pointer = new List(); // ["i", [1, size]] <=> univ[1][size]
    pointer->push_back(new String("i"));
    pointer->push_back(new List());
    ( (List*) (pointer->get(1)) )->push_back(new Int(1));
    ( (List*) (pointer->get(1)) )->push_back(new Int(size));
    
    com->set(1, pointer);
}
void AND(WSEML* univ) { // a->val && b->val
    List* globVar = (List* ) ((List*) univ)->get(1);
    List* stack = (List* ) ((List*) univ)->get(3);
    List* com = (List*) stack->back();
    // com = ['and', указатель на res, c-указатель на lval-команду, c-указатель на rval-команду]
    //          0             1                  2                             3
    // получаем значение lval
    WSEML* lval = get(univ, ((List*) com->get(2))->get(0));
    WSEML* rval = get(univ, ((List*) com->get(2))->get(1));
    
    WSEML* res;
    switch(lval->kind) {
        case BOOL:
            switch(rval->kind) {
                case BOOL:
                    res = new Bool(  ((Bool*) lval)->val && ((Bool*) rval)->val  );
                    break;
                default:
                    throw "and";
                    break;
            }
            break;
        default:
            throw "and";
            break;
    }
    int size = globVar->size();
    globVar->push_back(res);
    
    List* pointer = new List(); // ["i", [1, size]] <=> univ[1][size]
    pointer->push_back(new String("i"));
    pointer->push_back(new List());
    ( (List*) (pointer->get(1)) )->push_back(new Int(1));
    ( (List*) (pointer->get(1)) )->push_back(new Int(size));
    
    com->set(1, pointer);
}
void OR(WSEML* univ) { // a->val || b->val
    List* globVar = (List* ) ((List*) univ)->get(1);
    List* stack = (List* ) ((List*) univ)->get(3);
    List* com = (List*) stack->back();
    // com = ['or', указатель на res, c-указатель на lval-команду, c-указатель на rval-команду]
    //          0             1                  2                             3
    // получаем значение lval
    WSEML* lval = get(univ, ((List*) com->get(2))->get(0));
    WSEML* rval = get(univ, ((List*) com->get(2))->get(1));
    
    WSEML* res;
    switch(lval->kind) {
        case BOOL:
            switch(rval->kind) {
                case BOOL:
                    res = new Bool(  ((Bool*) lval)->val || ((Bool*) rval)->val  );
                    break;
                default:
                    throw "or";
                    break;
            }
            break;
        default:
            throw "or";
            break;
    }
    int size = globVar->size();
    globVar->push_back(res);
    
    List* pointer = new List(); // ["i", [1, size]] <=> univ[1][size]
    pointer->push_back(new String("i"));
    pointer->push_back(new List());
    ( (List*) (pointer->get(1)) )->push_back(new Int(1));
    ( (List*) (pointer->get(1)) )->push_back(new Int(size));
    
    com->set(1, pointer);
}
void NOT(WSEML* univ) { // !(a->val)
    List* globVar = (List* ) ((List*) univ)->get(1);
    List* stack = (List* ) ((List*) univ)->get(3);
    List* com = (List*) stack->back();
    // com = ['not', указатель на res, [c-указатель на lval-команду]]
    //          0             1                  2                
    // получаем значение lval
    WSEML* lval = get(univ, ((List*) com->get(2))->get(0));
    
    WSEML* res;
    switch(lval->kind) {
        case BOOL:
            res = new Bool(  !(((Bool*) lval)->val)  );
            break;
        default:
            throw "not";
            break;
    }
    int size = globVar->size();
    globVar->push_back(res);
    
    List* pointer = new List(); // ["i", [1, size]] <=> univ[1][size]
    pointer->push_back(new String("i"));
    pointer->push_back(new List());
    ( (List*) (pointer->get(1)) )->push_back(new Int(1));
    ( (List*) (pointer->get(1)) )->push_back(new Int(size));
    
    com->set(1, pointer);
}
void IF(WSEML* univ) {
    List* globVar = (List* ) ((List*) univ)->get(1);
    List* stack = (List* ) ((List*) univ)->get(3);
    List* com = (List*) stack->back();
    // com = ['if', ["i"]-указатель на res, [    указатель на условие,  указатель на первую команду тела], указатель на команду за телом]
    //          0             1                           2,0                        2, 1
    // получаем значение lval
    WSEML* lval = get(univ, ((List*) com->get(2))->get(0));
    WSEML* rval = get(univ, ((List*) com->get(2))->get(1));
    
    if (((Bool*) lval)->val == true) {
        ((List*) univ)->set(6, rval);
    } 
    
}
void WHILE(WSEML* univ) {
    List* globVar = (List* ) ((List*) univ)->get(1);
    List* stack = (List* ) ((List*) univ)->get(3);
    List* com = (List*) stack->back();
    // com = ['while', ["i"]-указатель на res, [    указатель на условие,  указатель на первую команду тела], указатель на команду за телом]
    //          0             1                           2,0                        2, 1
    // получаем значение lval
    WSEML* lval = get(univ, ((List*) com->get(2))->get(0));
    WSEML* rval = get(univ, ((List*) com->get(2))->get(1));
    
    if (((Bool*) lval)->val == true) {
        ((List*) univ)->set(6, rval);
    } 
}
/*
WSEML* d(WSEML* w) {
    if (w->kind == LIST) {
        if ((String*) w->get(0) == "i")
            return ((List*) w->get(1))->front();
        else
            return ((List*) w->get(2))->front();
    } 
    return nullptr;
}
WSEML* s(WSEML* w) {
    if (w->kind == LIST) {
        if ((String*) w->get(0) == "i")
            return ((List*) w->get(1))->front();
        else
            return ((List*) w->get(2))->front();
    } 
    return nullptr;
}*/

struct VM {
    List* univ; // вселенная 
    List* table; //0
    List* globVar;//1
    List* locVar;//2
    List* stack;//3
    List* majorProg;//4
    List* results;//5
    List* otherData;//6
    List* prog; //7
    VM() {
        // ???
        // 0 - список команд
        // 1 - глобальные переменные 
        // 2 - локальные (или перенести их в стэк?)
        // 3 - стэк
        // 4 - главная программа
        // 5 - результаты
        // 6 - прочие данные
        univ = new List();
        
        table = new List();
        globVar = new List();
        locVar = new List();
        stack  = new List();
        majorProg = new List();
        results = new List();
        otherData = new List();
        prog = new List();
        
        univ->push_back(table);
        univ->push_back(globVar);
        univ->push_back(locVar);
        univ->push_back(stack);
        univ->push_back(majorProg);
        univ->push_back(results);
        univ->push_back(otherData);
        univ->push_back(prog);
        
        // добавление встроенных команд в таблицу
        // false - встроенная
        // true - пользовательская
        List* com;
        // sum
        com = new List();
        com->push_back(new String("sum")); // название
        com->push_back(new Bool(false)); // флаг встроенности
        com->push_back(new Function(sum));
        table->push_back(com);
        // subs
        com = new List();
        com->push_back(new String("subs")); // название
        com->push_back(new Bool(false)); // флаг встроенности
        com->push_back(new Function(subs));
        table->push_back(com);
        // mult
        com = new List(); 
        com->push_back(new String("mult")); // название
        com->push_back(new Bool(false)); // флаг встроенности
        com->push_back(new Function(mult));
        table->push_back(com);
        // div
        com = new List(); 
        com->push_back(new String("div")); // название
        com->push_back(new Bool(false)); // флаг встроенности
        com->push_back(new Function(div));
        table->push_back(com);
        // mod
        com = new List();
        com->push_back(new String("mod")); // название
        com->push_back(new Bool(false)); // флаг встроенности
        com->push_back(new Function(mod));
        table->push_back(com);
        // valeq
        com = new List();
        com->push_back(new String("valeq")); // название
        com->push_back(new Bool(false)); // флаг встроенности
        com->push_back(new Function(valeq));
        table->push_back(com);
        // less
        com = new List();
        com->push_back(new String("less")); // название
        com->push_back(new Bool(false)); // флаг встроенности
        com->push_back(new Function(less));
        table->push_back(com);
        // more
        com = new List(); 
        com->push_back(new String("more")); // название
        com->push_back(new Bool(false)); // флаг встроенности
        com->push_back(new Function(more));
        table->push_back(com);
        // and
        com = new List(); 
        com->push_back(new String("and")); // название
        com->push_back(new Bool(false)); // флаг встроенности
        com->push_back(new Function(AND));
        table->push_back(com);
        // or
        com = new List();
        com->push_back(new String("or")); // название
        com->push_back(new Bool(false)); // флаг встроенности
        com->push_back(new Function(OR));
        table->push_back(com);
        // not
        com = new List();
        com->push_back(new String("not")); // название
        com->push_back(new Bool(false)); // флаг встроенности
        com->push_back(new Function(NOT));
        table->push_back(com);
        // IF
        com = new List();
        com->push_back(new String("if")); // название
        com->push_back(new Bool(false)); // флаг встроенности
        com->push_back(new Function(NOT));
        table->push_back(com);
        //while
        com = new List();
        com->push_back(new String("while")); // название
        com->push_back(new Bool(false)); // флаг встроенности
        com->push_back(new Function(NOT));
        table->push_back(com);
        //
        // подготовка
    }
    void exec(WSEML* program) {
        // выделение памяти...
        
        //
        WSEML* w = majorProg->front();
        univ->set(7, program);
        univ->set(6, new NullPtr());
        List* c ;
        List* l;
        while(w != nullptr) {
            for (int i = 0; i < table->size(); i++)
                if ( ((String*) ((List*) table->get(i))->get(0))->val == ((String*) ((List*) w)->get(0))->val) {
                    c = (List*) table->get(i);
                    break;
                }
            if (((Bool*) c->get(1))->val == false) {
                stack->push_back(w);
                ((Function*) c->get(2))->fun(univ);
            } else {
                l = (List*) c->get(2);
                l->set(l->size() - 1,  ((List*)w)->back());
                ((List*) w)->set(((List*) w)->size() - 1, l->front());
            }
                    
            if (univ->get(6) == nullptr || univ->get(6)->kind == NULLPTR) {
                w = ((List*) w)->back();
            }
            else {
                w = otherData;
            }
        }
    }
};

int main() {
    VM v;
    List* l = new List();
    List* l2 = new List();
    l2->push_back(new Int(3));
    l2->push_back(new Int(4));
    l->push_back(new String("sum"));
    l->push_back(nullptr);
    l->push_back(l2);
    l->push_back(nullptr);
    v.majorProg->push_back(l);
    v.exec(nullptr);
    cout << ((Int*) v.globVar->get(0))->val;
}
