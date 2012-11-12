#ifndef __MODIFIEDSTACK_H__
#define __MODIFIEDSTACK_H__
#include <iostream>

template <class T>
class StackNode {
public:
    StackNode *next;
    StackNode *prev;
    T * content;
    int level;

    StackNode(StackNode *next, int level, T * content) {
        this->next = next;
        this->level = level;
        this->content = content;
        prev = 0;
    }
};

template <class T>
class ModifiedStack {
private:
    StackNode<T> *top;
    StackNode<T> *bottom;
    int size;
    int bcount;

    int recB(StackNode<T> *nxt, const int lll) {
        if (0 != nxt)
            if (lll == nxt->level)
                return 1 + recB(nxt->prev, lll);
        return 0;
    }

public:

    ModifiedStack() {
        top = 0;
        size = 0;
    }

    bool isEmpty() const {
        return (top == 0);
    }

    int getTLevel() const {
        return top->level;
    }

    int getBLevel() const {
        return bottom->level;
    }

    int getSize() const {
        return size;
    }

    int getBcount() const {
        return bcount;
    }

    void recountBcount() {
        int blowest = getBLevel();
        bcount = recB(bottom, blowest);
    }

    void push(int ilevel, T *idata) {
        if (0 == top)
            bcount = 1;
        else
            if (getBLevel() == ilevel)
            bcount++;
        StackNode<T> *tmpt = top;
        top = new StackNode<T > (top, ilevel, idata);
        size++;
        if (0 == tmpt)
            bottom = top;
        else
            tmpt->prev = top;
    }

    T* pop() {
        if (0 == top) {
            std::cerr << "poping from empty stack" << std::endl;
            exit(1);
        }
if (getTLevel() == getBLevel())
            bcount--;
        T *out = top->content;
        StackNode<T> *next = top->next;

        delete top;
        top = next;
        if (0 == top)
            bottom = 0;
        else
            top->prev = 0;

        size--;
        return out;
    }

    T* bop() {
        if (0 == bottom) {
            std::cerr << "boping from empty stack" << std::endl;
            exit(1);
        }

        if (0 != bcount)
            bcount--;
        T *out = bottom->content;
        StackNode<T> *prev = bottom->prev;
        delete bottom;
        bottom = prev;
        if (0 != bottom)
            bottom->next = 0;

        size--;
        return out;
    }

    ModifiedStack<T>* cut() {
        StackNode<T> *pom = new StackNode<T > (0, 1, this->bop());
        return 0;
    }

    ~ModifiedStack() {
        while (0 != top)
            delete pop();
    }
};

#endif
