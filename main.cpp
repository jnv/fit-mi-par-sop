#include <cstdlib>
#include <iostream>
#include <stack>
#include <vector>
#include <set>
#include <fstream>
#include <string>
#include <sstream>
using namespace std;

#define DEBUG 1

int n = 0,
        c = 0,
        a = 0;
int * inputSet; // pole vstupnich dat
set<int> S;

struct Node
{
    set<int> inputSet; // zbyvajici vstupni mnozina
    vector<int> * sets;
    int * sums;
    int penalty;

    Node(const set<int> & givenInputSet)
    {
        inputSet = givenInputSet;

        init();
        for(int i = 0; i < a; i++)
        {
            sums[i] = 0;
            sets[i].clear();
        }
    }

    Node(const Node & o)
    {
        inputSet = o.inputSet;
        init();
        penalty = o.penalty;
        for(int i = 0; i < a; i++)
        {
            sums[i] = o.sums[i];
            vector<int> oset = o.sets[i];
            sets[i] = vector<int>(oset);
        }

    }

    void init()
    {
        sums = new int[a];
        sets = new vector<int>[a];
        penalty = maxPenalty();
    }

    ~Node()
    {
        delete [] sums;
        delete [] sets;
    }

    bool isFeasible(const int set, const int number) const
    {
        return(sums[set] + number) < c;
    }

    bool push(const int set, const int number)
    {
        /*if(!isFeasible(set, number))
            return false;*/
        sets[set].push_back(number);
        sums[set] += number;
        penalty -= number;
        return true;
    }

    int pop()
    {
        set<int>::iterator it = inputSet.begin();
        int ret = *it;
        inputSet.erase(it);

        return ret;
    }

    bool empty()
    {
        return inputSet.empty();
    }

    string toString() const
    {
        stringstream ret;
        ret << "Penalty " << penalty << endl;
        for(int i = 0; i < a; i++)
        {
            ret << "Set " << i << " ";
            ret << "[" << sums[i] << "]";

            vector<int>::iterator it;

            for(it = sets[i].begin(); it < sets[i].end(); it++)
                ret << " " << *it;
            ret << endl;
        }

        return ret.str();
    }

    bool isBetterThan(const Node * o) const
    {
        return(penalty < o->penalty);
    }

    inline int maxPenalty() const
    {
        return a * (c - 1);
    }

    bool hasMaxPenalty() const
    {
        return(penalty == maxPenalty());
    }


};

void doSolve()
{
    stack<Node*> stack;
    //bool found = false;

    Node * solution = new Node(S);
    stack.push(solution);

    while(!stack.empty())
    {
        Node * node = stack.top();
        stack.pop();

        if(node->penalty == 0)
        {
            cout << "0 penalty solution" << endl;
            delete solution;
            solution = node;
            break;
        }

        if(node->isBetterThan(solution))
        {
            cout << "Better solution." << endl;
#ifndef DEBUG
            cout << node->toString() << endl;
#endif

            delete solution;
            solution = node;
        }

        if(node->empty())
        {
            cout << "Empty set." << endl;
            continue;
        }

        while(!node->empty())
        {
            int add = node->pop();
            for(int i = 0; i < a; i++)
            {
                if(node->isFeasible(i, add))
                {
                    Node * newNode = new Node(*node);
                    newNode->push(i, add);
                    stack.push(newNode);
                }
            }
        }

#ifdef DEBUG
        cout << node->toString();
#endif
        if(node != solution)
            delete node;
    }

    cout << "----------" << endl;

    if(solution->hasMaxPenalty())
    {
        cout << "Found nothing." << endl;
    }
    else
    {
        cout << "Found solution: " << endl << solution->toString() << endl;
    }

    delete solution;
}

bool loadSet(char * fname)
{
    ifstream f;
    f.open(fname);

    if(!f.is_open())
    {
        cout << "Cannot open input file: " << fname << endl;
        return false;
    }

    f >> n;
#ifndef DEBUG
    if(n < 20)
    {
        cout << "n must be 20 or more";
        f.close();
        return false;
    }
#endif

    f >> c;
    f >> a;
#ifndef DEBUG
    if(a < 2 || a > n / 10)
    {
        cout << "a must be more than 1 and less than n/10.";
        f.close();
        return 0;
    }
#endif

    inputSet = new int[n];

    cout << "Loading " << n << " numbers" << endl;

    for(int i = 0; i < n; i++)
    {
        int in;
        f >> in;
        inputSet[i] = in;
        S.insert(in);
    }

    cout << "Loaded: ";
    for(int i = 0; i < n; i++)
    {
        cout << inputSet[i] << " ";
    }
    cout << endl;
    return true;
}

int main(int argc, char ** argv)
{
    if(argc != 2)
    {
        cout << "Usage: sop <file>" << endl;
        return 0;
    }

    if(!loadSet(argv[1]))
    {
        return 1;
    }

    doSolve();
    delete [] inputSet;

    return 0;

}