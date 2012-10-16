#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cmath>
#include <cstdlib>

using namespace std;

ifstream infile;
int n = 0;
int c = 0;
int a = 0;
int output;
int * inputArray;
int actualSet = 0;
long totalIterator = 0;
int possibleCombinations;
string *combinations;
int *sumOfMask;
long maskAmount;
bool hasSolution = true;
int sum;
int allElementsSum;

template <typename Iter>
bool next(Iter begin, Iter end)
{
    while(begin != end) // we're not done yet
    {
        --end;
        if((*end & 1) == 0) // even number is treated as zero
        {
            ++*end; // increase to one
            return true; // still more numbers to come
        }
        else // odd number is treated as one
        {
            --*end; // decrease to zero and loop
        }
    }
    return false; // that was the last number
}

/* Zobrazi vstupnu mnozinu */
void showInput()
{
    for(int i = 0; i < actualSet; i++)
    {
        cout << inputArray[i] << " ";
    }
    cout << endl;
}

/* Vyhlada najmensi prvok pola */
int findMin()
{
    int minimum = inputArray[0];

    for(int i = 0; i < actualSet; i++)
    {
        if(inputArray[i] < minimum)
            minimum = inputArray[i];
    }
    return minimum;
}

void applyMask(string mask)
{
    sum = 0;
    for(int i = 0; i < actualSet; i++)
    {
        if(mask.at(i) == '1')
        {
            sum = sum + inputArray[i];
            if(sum > c) return;
            sumOfMask[totalIterator] = sum;
        }

    }
    totalIterator++;
}

bool bitAND(string a, string b)
{
    for(int k = 0; k < actualSet; k++)
    {
        if(a.at(k) == '1' && b.at(k) == '1') return false;
    }
    return true;
}

string sumUpMasks(string a, string b)
{
    string tempMask = "";
    for(int k = 0; k < actualSet; k++)
    {
        if(a.at(k) == '1' || b.at(k) == '1') tempMask = tempMask + "1";
        else tempMask = tempMask + "0";
    }
    return tempMask;

}

void writeMaskedSet(string mask)
{
    for(int i = 0; i < actualSet; i++)if(mask[i] == '1') cout << inputArray[i] << " ";
    cout << endl;
}

void printUsage()
{
    cout << "Usage:" << endl << endl;
    cout << "sop <n> <c> <a> << file" << endl;
    cout << "n - size of the given set" << endl;
    cout << "c - max sum of the subsets" << endl;
    cout << "a - number of generated sets";
    cout << "file - (or input stream) with numbers" << endl;
}

bool hasElement(int elem, int n)
{
    for(int i = 0; i < n - 1; i++)
    {
        if(inputArray[i] == elem)
        {
            return true;
        }
    }
    return false;
}

bool loadSet()
{
    int input;
    inputArray = new int[n];
    for(int i = 0; i < n; i++)
    {
        inputArray[i] = 0;
    }

    while(cin >> input)
    {
        if(input > c)
        {
            cout << input << " je vetsi nez c" << endl;
            continue;
        }
        cout << input << endl;
        inputArray[actualSet] = input;
        actualSet++;

        if(hasElement(input, actualSet))
        {
            cout << "\nDuplicitny prvok. Neplatna mnozina.\n";
            delete [] inputArray;
            return false;
        }

        if(actualSet > n)
        {
            break;
        }

    }
    cout << "\n\nPrvky prevysujuce horni mez boli odstranene.\n" << endl;
    return true;
}

int main(int argc, char **argv)
{

    if(argc != 4)
    {
        printUsage();
        return 0;
    }

    n = atoi(argv[1]);
    if(n < 20)
    {
        cout << "Mohutnost mnoziny musi byt vacsia ako 19.";
        return 1;
    }
    c = atoi(argv[2]);
    a = atoi(argv[3]);

    if(a < 2 || a > n / 10)
    {
        cout << "Pocet podmnozin musi byt vacsi nez 1 a mensi nez n/10.";
        return 2;
    }

    cout << "Nacitam prvky zo suboru: " << endl;
    if(!loadSet())
        return 3;

    if(c < findMin())
    {
        cout << "Zadane c je mensie nez najmensi prvok mnoziny.\n";
        delete [] inputArray;
        return 4;
    }

    cout << "Mohutnost mnoziny: " << n << endl << "Horni mez: " << c << endl << "Pocet podmnozin: " << a << endl << "Mnozina: ";
    showInput();


    /*
        Vypocet:

        inputArray - obsahuje len prvky,ktore sa budu ucastnit vypoctu
        actualSet - pocet prvkov ucastniacich sa vypoctu
     */
    allElementsSum = 0;
    totalIterator = 0;
    string mask("");
    // vytvorenie povodnej masky

    for(int i = 0; i < actualSet; i++)
    {
        mask.append("0");
        allElementsSum = allElementsSum + inputArray[i];
    }

    //generovanie vsetkych moznych masiek

    //maskAmount=pow(2,actualSet);
    maskAmount = 100000;
    combinations = new string[maskAmount];
    sumOfMask = new int[maskAmount];

    while(next(mask.begin(), mask.end()))
    {
        combinations[totalIterator] = mask;

        /*
        kazda vytvorena maska je vhodena do metody applyMask
        kde sa kontroluje ci maska na aktualnej mnozine nepresiahne
        sumou hornu mez, v pripade ze ano nezvysuje sa iterator
         a teda sa aktualna maska prepise nasledujucou
         */
        applyMask(combinations[totalIterator]); //
    }

    /*
    combinations - vsetky masky, ktore aplikovane na actualSet tvoria mnozinu so sumou mensou nez c
    totalIterator - pocet tychto masiek
     */
    string result = "";
    string temp;
    int finalSum;
    int totalSum = 0;
    int iteration;

    mask = "";
    for(int i = 0; i < totalIterator; i++)
        mask.append("0");

    while(next(mask.begin(), mask.end()))
    {
        temp = "";
        for(int i = 0; i < actualSet; i++)
            temp.append("0");
        finalSum = 0;
        iteration = 0;


        for(int j = 0; j < totalIterator; j++)
        {
            if(mask[j] == '1')
            {
                if(bitAND(temp, combinations[j]))
                {

                    iteration++;
                    if(iteration > a) break;
                    temp = sumUpMasks(temp, combinations[j]);

                    finalSum = finalSum + sumOfMask[j];

                }
                else break;
            }
        }

        if(finalSum > totalSum)
        {
            totalSum = finalSum;
            result = mask;
            cout << "\nZatial najdeny najvacsi sucet disjunktnych podmnozin je: " << totalSum << endl;
            for(int k = 0; k < totalIterator; k++)
                if(result[k] == '1')writeMaskedSet(combinations[k]);

            if(totalSum == allElementsSum)
            {
                delete [] inputArray;
                delete [] combinations;
                return 0;

            }

        }
    }



    delete [] inputArray;
    delete [] combinations;
    return 0;
}
