#include <iostream>
using std::cout;

enum Kind{
    INT, // переменная INT 
    CHAR, // переменная CHAR
    FLOAT, // переменная FLOAT
    BOOL, // переменная BOOL
    INTCONST, // константа INT
    CHARCONST, // константа CHAR
    FLOATCONST, // константа FLOAT
    BOOLCONST, // константа BOOL
    NOTCOMMAND, // не команда
    WRITE, // вывод на экран(1 параметр) -- DONE
    SETVAL, // задать переменной следующее значение(2 параметра) -- DONE
    MEMALL, // выделить память в указанном количестве -- DONE
    INTSUM, // a + b (результат - int, оба аргумента - int) -- DONE
    INTMULT, // a * b -- DONE
    INTSUB, // a - b -- DONE
    INTDIV, // a/b -- DONE
    INTMOD, // a % b -- DONE
    FLOATSUM, // a + b (результат - float, аргументы - int или float) -- DONE
    FLOATSUB, // -- DONE
    FLOATDIV, // -- DONE
    FLOATMULT, // -- DONE
    IF, // условный оператор -- DONE
    EQUALS, // ==  -- DONE
    LESS, // < -- DONE
    MORE, // > -- DONE
    NOT, //  логическое отрицание  
    AND, // логическое 'и'
    OR, // логическое 'или'
    DEF, // объявление переменной? 
    WHILE, // -- DONE
    INDEXING // индексация в массивах
};

struct WSEML {
    WSEML* d;
    WSEML* s;
    void* val;
    Kind kind;
    WSEML() {
        d = nullptr;
        s = nullptr;
        val = nullptr;
    }
};

struct Stack { // Доступ к элементам через ->d
    WSEML* tail;
    Stack() {
        tail = nullptr;
    }
    void push(WSEML* ws) {
        WSEML* a = new WSEML;
        a->d = ws;
        a->s = tail;
        tail = a;
    }
    WSEML* pop() {
        if (tail == nullptr) 
            return nullptr;
        
        WSEML* ws = tail->d;
        tail = tail->s;
        return ws;
    }
    
    WSEML* getTail() { // возращает tail->d
        if (tail == nullptr)
            return nullptr;
        
        return tail->d;
    }
    bool empty() {
        return tail == nullptr;
    }
};

struct VM {
    Stack stack;
    
    void exec(WSEML* program) {
        stack.push(program);
        WSEML* w;
        WSEML* p;
        
        while(!stack.empty()) {
label1:
            w = stack.getTail();
            
            if(w->d == nullptr) { // а как быть с условиями и циклами?
                run(w);
label2:
                stack.pop();
                while(w->s == nullptr) {
                    if (stack.empty())
                        return; // or return
                    w = stack.pop();
                    if (w->kind == WHILE) { // Мы выполнили тело цикла, пора проверить условие
                        stack.push(w); // засовываем обратно цикл
                        stack.push(w->d); // и его условие
                        goto label1;
                    }
                    run(w);
                }
                
                p = stack.getTail(); // отец w
                if (p != nullptr && p->d == w && (p->kind == IF || p->kind == WHILE) && **((bool**) w->val) != true) {
                    // тогда w обязано быть выражением булева типа
                    // и w == false => условие IF или WHILE не выполнено
                           w = p;
                           goto label2;
                }
                stack.push(w->s);
                
            } 
            else {
                stack.push(w->d);
            }
        }
    }
    
    void run(WSEML* command) {
        switch(command->kind) {
            case WRITE:
                switch(command->d->kind) {
                    case FLOAT:
                    case FLOATDIV:
                    case FLOATSUM:
                    case FLOATSUB:
                    case FLOATMULT:
                    case FLOATCONST:
                        cout << **((float**) command->d->val);
                        break;
                        
                    case INT:
                    case INTDIV:
                    case INTSUM:
                    case INTSUB:
                    case INTMULT:
                    case INTMOD:
                    case INTCONST:
                        cout << **((int**) command->d->val);
                        break;
                    
                    case CHAR:
                    case CHARCONST:
                        cout << **((char**) command->d->val);
                        break;
                        
                    case BOOL:
                    case BOOLCONST:
                    case EQUALS:
                    case LESS:
                    case MORE:
                    case NOT:
                    case AND:
                    case OR:
                        cout << **((bool**) command->d->val);
                        break;
                        
                    case INDEXING:
                        switch(command->d->d->kind) {
                            case FLOAT:
                                cout << **((float**) command->d->val);
                                break;
                            case INT:
                                cout << **((int**) command->d->val);
                                break;
                            case CHAR:
                                cout << **((char**) command->d->val);
                                break;
                            case BOOL:
                                cout << **((bool**) command->d->val);
                                break;
                        }
                        break;
                        
                    default:
                        throw("ошибка в WRITE");
                        break;
                    
                }
                break; 
                
            case SETVAL:
                switch(command->d->kind) {
                    case INT:
                        **((int**) command->d->val) = **((int**) command->d->s->val);
                        break;
                    case FLOAT:
                        **((float**) command->d->val) = **((float**) command->d->s->val);
                        break;
                    case CHAR:
                        **((char**) command->d->val) = **((char**) command->d->s->val);
                        break;
                    case BOOL:
                        **((bool**) command->d->val) = **((bool**) command->d->s->val);
                        break;
                    default:
                        throw("ОШИБКА в SETVAL");
                        break;
                }
                break;
            
            case MEMALL: // предполагается, что кол-во является выражением типом INT
                        // также предполагается что ((int**) command->d->val) != nullptr
                switch(command->d->kind) {
                    case INT:
                        *((int**) command->d->val) = new int[**((int**) command->d->s->val)];
                        break;
                    case FLOAT:
                        *((float**) command->d->val) = new float[**((int**) command->d->s->val)];
                        break;
                    case CHAR:
                        *((char**) command->d->val) = new char[**((int**) command->d->s->val)];
                        break;
                    case BOOL:
                        *((bool**) command->d->val) = new bool[**((int**) command->d->s->val)];
                        break;
                    default:
                        throw("ОШИБКА в MEMALL");
                        break;
                }
                break;
            
            case INTSUM: // предполагается, что оба аргумента являются целыми
                        // также предполагается что ((int**) command->val) != nullptr
                *((int**) command->val) = new int(**((int**) command->d->val) + **((int**) command->d->s->val)); 
                break;
                
            case INTSUB: // предполагается, что оба аргумента являются целыми
                        // также предполагается что ((int**) command->val) != nullptr
                *((int**) command->val) = new int(**((int**) command->d->val) - **((int**) command->d->s->val)); 
                break;
                
            case INTMULT: // предполагается, что оба аргумента являются целыми
                        // также предполагается что ((int**) command->val) != nullptr
                *((int**) command->val) = new int(**((int**) command->d->val) * (**((int**) command->d->s->val))); 
                break;
                
            case INTDIV: // предполагается, что оба аргумента являются целыми
                        // также предполагается что ((int**) command->val) != nullptr
                *((int**) command->val) = new int(**((int**) command->d->val) / **((int**) command->d->s->val)); 
                break; 
                
            case INTMOD: // предполагается, что оба аргумента являются целыми
                        // также предполагается что ((int**) command->val) != nullptr
                *((int**) command->val) = new int(**((int**) command->d->val) % **((int**) command->d->s->val)); 
                break;
                
            case FLOATSUM:
                switch(command->d->kind) {
                    case INT:
                        switch(command->d->s->kind) {
                            case INT:
                                *((float**) command->val) = new float(**((int**) command->d->val) + **((int**) command->d->s->val));
                                break;
                            case FLOAT:
                                *((float**) command->val) = new float(**((int**) command->d->val) + **((float**) command->d->s->val));
                                break;
                        }
                        break;
                    case FLOAT:
                        switch(command->d->s->kind) {
                            case INT:
                                *((float**) command->val) = new float(**((float**) command->d->val) + **((int**) command->d->s->val));
                                break;
                            case FLOAT:
                                *((float**) command->val) = new float(**((float**) command->d->val) + **((float**) command->d->s->val));
                                break;
                        }
                        break;
                }
                break;
            
            case FLOATSUB:
                switch(command->d->kind) {
                    case INT:
                        switch(command->d->s->kind) {
                            case INT:
                                *((float**) command->val) = new float(**((int**) command->d->val) - **((int**) command->d->s->val));
                                break;
                            case FLOAT:
                                *((float**) command->val) = new float(**((int**) command->d->val) - **((float**) command->d->s->val));
                                break;
                        }
                        break;
                    case FLOAT:
                        switch(command->d->s->kind) {
                            case INT:
                                *((float**) command->val) = new float(**((float**) command->d->val) - **((int**) command->d->s->val));
                                break;
                            case FLOAT:
                                *((float**) command->val) = new float(**((float**) command->d->val) - **((float**) command->d->s->val));
                                break;
                        }
                        break;
                }
                break;
                
            case FLOATMULT:
                switch(command->d->kind) {
                    case INT:
                        switch(command->d->s->kind) {
                            case INT:
                                *((float**) command->val) = new float(**((int**) command->d->val) * (**((int**) command->d->s->val)));
                                break;
                            case FLOAT:
                                *((float**) command->val) = new float(**((int**) command->d->val) * (**((float**) command->d->s->val)));
                                break;
                        }
                        break;
                    case FLOAT:
                        switch(command->d->s->kind) {
                            case INT:
                                *((float**) command->val) = new float(**((float**) command->d->val) * (**((int**) command->d->s->val)));
                                break;
                            case FLOAT:
                                *((float**) command->val) = new float(**((float**) command->d->val) * (**((float**) command->d->s->val)));
                                break;
                        }
                        break;
                }
                break;
                
            case FLOATDIV:
                switch(command->d->kind) {
                    case INT:
                        switch(command->d->s->kind) {
                            case INT:
                                *((float**) command->val) = new float(**((int**) command->d->val) / **((int**) command->d->s->val));
                                break;
                            case FLOAT:
                                *((float**) command->val) = new float(**((int**) command->d->val) / **((float**) command->d->s->val));
                                break;
                        }
                        break;
                    case FLOAT:
                        switch(command->d->s->kind) {
                            case INT:
                                *((float**) command->val) = new float(**((float**) command->d->val) / **((int**) command->d->s->val));
                                break;
                            case FLOAT:
                                *((float**) command->val) = new float(**((float**) command->d->val) / **((float**) command->d->s->val));
                                break;
                        }
                        break;
                }
                break;
                
            case EQUALS:
                switch(command->d->kind) {
                    case INT:
                        switch(command->d->s->kind) {
                            case INT:
                                *((bool**) command->val) = new bool(**((int**) command->d->val) == **((int**) command->d->s->val));
                                break;
                            case FLOAT:
                                *((bool**) command->val) = new bool(**((int**) command->d->val) == **((float**) command->d->s->val));
                                break;
                        }
                        break;
                    case FLOAT:
                        switch(command->d->s->kind) {
                            case INT:
                                *((bool**) command->val) = new bool(**((float**) command->d->val) == **((int**) command->d->s->val));
                                break;
                            case FLOAT:
                                *((bool**) command->val) = new bool(**((float**) command->d->val) == **((float**) command->d->s->val));
                                break;
                        }
                        break;
                    case BOOL: // bool == bool
                        *((bool**) command->val) = new bool(**((bool**) command->d->val) == **((bool**) command->d->s->val));
                        break;
                }
                break;
                
            case LESS:
                switch(command->d->kind) {
                    case INT:
                        switch(command->d->s->kind) {
                            case INT:
                                *((bool**) command->val) = new bool(**((int**) command->d->val) < **((int**) command->d->s->val));
                                break;
                            case FLOAT:
                                *((bool**) command->val) = new bool(**((int**) command->d->val) < **((float**) command->d->s->val));
                                break;
                        }
                        break;
                    case FLOAT:
                        switch(command->d->s->kind) {
                            case INT:
                                *((bool**) command->val) = new bool(**((float**) command->d->val) < **((int**) command->d->s->val));
                                break;
                            case FLOAT:
                                *((bool**) command->val) = new bool(**((float**) command->d->val) < **((float**) command->d->s->val));
                                break;
                        }
                        break;
                }
                break;
                
            case MORE:
                switch(command->d->kind) {
                    case INT:
                        switch(command->d->s->kind) {
                            case INT:
                                *((bool**) command->val) = new bool(**((int**) command->d->val) > **((int**) command->d->s->val));
                                break;
                            case FLOAT:
                                *((bool**) command->val) = new bool(**((int**) command->d->val) > **((float**) command->d->s->val));
                                break;
                        }
                        break;
                    case FLOAT:
                        switch(command->d->s->kind) {
                            case INT:
                                *((bool**) command->val) = new bool(**((float**) command->d->val) > **((int**) command->d->s->val));
                                break;
                            case FLOAT:
                                *((bool**) command->val) = new bool(**((float**) command->d->val) > **((float**) command->d->s->val));
                                break;
                        }
                        break;
                }
                break;
                
            default:
                break;
            
        }
    }
};

int main() {
    
    
    //  
    //write 3 * 2.5
    //7.5
    float* pc = new float(123545);
    int* pa = new int(3);
    float* pb = new float(2.5);
    
    WSEML* r = new WSEML;
    r->kind = WRITE;
    
    r->d = new WSEML;
    r->d->kind = FLOATMULT;
    r->d->val = (float**) &pc;
    
    r->d->d = new WSEML;
    r->d->d->kind = INT;
    r->d->d->val = (int**) &pa;
    
    r->d->d->s = new WSEML;
    r->d->d->s->kind = FLOAT;
    r->d->d->s->val = (float**) &pb;
    
    
    VM v;
    v.exec(r);
    
    return 0;
}
