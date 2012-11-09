/*
 ============================================================================
 Name        : MI-PAR-SOP Sekv
 Author      :
 Description :
 ============================================================================
 */
#include <cstdlib>
#include <iostream>
#include <stack>
#include <vector>
#include <set>
#include <fstream>
#include <string>
#include <sstream>
#include "mpi.h"
using namespace std;

#define DEBUG 1

int n = 0,
        c = 0,
        a = 0;
int upperBound = 0;
int * inputSet; // pole vstupnich dat
set<int> S;

struct Node
{
    set<int> inputSet; // zbyvajici vstupni mnozina
    vector<int> * sets;
    vector<int> placement;
    int * sums;
    int price;

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
        price = o.price;
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
        price = 0;
        placement.assign(a, 0);
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
        price += number;
        return true;
    }

    int pop()
    {
        set<int>::iterator it = inputSet.begin();
        int ret = *it;
        inputSet.erase(it);

        return ret;
    }

    set<int>::iterator beginSet()
    {
      return inputSet.begin();
    }

    set<int>::iterator endSet() const
    {
      return inputSet.end();
    }

    void setErase(const int it)
    {
      inputSet.erase(it);
    }

    int setSize() const
    {
      return inputSet.size();
    }

    bool empty()
    {
        return inputSet.empty();
    }

    string toString() const
    {
        stringstream ret;
        ret << "Price " << price << endl;
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
        return(price > o->price);
    }

    inline int maxPrice() const
    {
    	return upperBound;
    }

    bool zeroPrice() const
    {
        return(price == 0);
    }

    bool hasMaxPrice() const
    {
    	return price == maxPrice();
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

        if(node->hasMaxPrice())
        {
            cout << "Maximum price solution" << endl;
            delete solution;
            solution = node;
            break;
        }

        if(node->isBetterThan(solution))
        {
            cout << "Better solution:" << endl;
            cout << node->toString() << endl;

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
        //cout << node->toString();
#endif
        if(node != solution)
            delete node;
    }

    cout << "----------" << endl;

    if(solution->zeroPrice())
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

    cout << "c (max price per subset): " << c << endl << "a (number of subsets): " << a << endl;

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

    upperBound = a * (c - 1);
    cout << "Upper bound is: " << upperBound << endl;

    return true;
}

int main(int argc, char ** argv)
{
	double start, stop;
    if(argc != 2)
    {
        cout << "Usage: sop <file>" << endl;
        return 0;
    }

    if(!loadSet(argv[1]))
    {
        return 1;
    }

    MPI_Init(&argc, &argv);                     /* start up MPI */
    start = MPI_Wtime();
    doSolve();
    stop = MPI_Wtime();
    delete [] inputSet;

    cout << "Solve time: " << stop - start << endl;

    return 0;

}

