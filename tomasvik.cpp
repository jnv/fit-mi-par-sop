#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <math.h>

using namespace std;

    ifstream infile;
    int n=0;
    int c=0;
    int a=0;
    int output;
    int * inputArray;
    int actualSet=0;
    long totalIterator=0;
    int possibleCombinations;
    string *combinations;
    int *sumOfMask;
    long maskAmount;
    bool hasSolution=true;
    int sum;
    int allElementsSum;



template <typename Iter>
bool next(Iter begin, Iter end)
{
    while (begin != end)       // we're not done yet
    {
        --end;
        if ((*end & 1) == 0)   // even number is treated as zero
        {
            ++*end;            // increase to one
            return true;       // still more numbers to come
        }
        else                   // odd number is treated as one
        {
            --*end;            // decrease to zero and loop
        }
    }
    return false;              // that was the last number
}

/* Metoda zobrazi vstupnu mnozinu */



void showInput(){
    for(int i=0;i<actualSet;i++)
            {
                cout << inputArray[i] << " ";
            }
            cout << endl;
}

/* Metoda vyhlada najmensi prvok pola */

int findMin()
        {
            int minimum=inputArray[0];

            for(int i=0;i<actualSet;i++)
            {
                if(inputArray[i]<minimum) minimum=inputArray[i];
            }
            return minimum;
        }

void applyMask(string mask){

        sum=0;
            for(int i=0;i<actualSet;i++){
                if(mask.at(i)=='1'){
                    sum=sum+inputArray[i];
                    if(sum>c) return;
                    sumOfMask[totalIterator]=sum;
                }

            }
            totalIterator++;
}

bool bitAND(string a, string b){
    for(int k=0;k<actualSet;k++) {if(a.at(k)=='1' && b.at(k)=='1') return false;}
    return true;
}

string sumUpMasks(string a,string b){
    string tempMask="";
    for(int k=0;k<actualSet;k++) {
        if(a.at(k)=='1' || b.at(k)=='1') tempMask=tempMask+"1";
        else tempMask=tempMask+"0";
        }
    return tempMask;

}

void writeMaskedSet(string mask){
    for(int i=0;i<actualSet;i++)if(mask[i]=='1') cout << inputArray[i]<<" ";
    cout << endl;
}
int main(int argc, char ** argv)
{

//Otvorenie suboru

 infile.open(argv[1]);
 if( !infile ){
      cout << "Vstupny subor nebol najdeny.\n";
      return 0;
 }



//nacitanie dat zo suboru, validacia vstupnych parametrov

    if (infile.is_open()) {

    infile >> n;

if( n < 20 ) {
cout << "Mohutnost mnoziny musi byt vacsia ako 19.";
infile.close();
return 0;
}

    infile >> c;



    infile >> a;

if( a < 2 || a > n/10 ) {
cout << "Pocet podmnozin musi byt vacsi nez 1 a mensi nez n/10.";
infile.close();
return 0;
}


    inputArray=new int[n];
    for(int i=0;i<n;i++){
        inputArray[i]=0;
        }

cout << "Nacitam prvky zo suboru: ";

    while (!infile.eof()) {


    infile >> output;
    totalIterator++;
    cout << output << " ";
    if(output<=c)
    {
        inputArray[actualSet]=output;
    actualSet++;

    for(int i=0;i<actualSet-1;i++){
    if(inputArray[i]==output){
        cout << "\nDuplicitny prvok. Neplatna mnozina.\n";
        delete [] inputArray;
        return 0;}
    }

    }
 }
 /*if(totalIterator!=n)
 {
 cout << "\nPocet prvkov nezodpoveda mohutnosti mnoziny.\n";
 delete [] inputArray;
 return 0;
 }*/

 cout << "\n\nPrvky prevysujuce horni mez boli odstranene.\n" << endl;

}

infile.close();

if(c < findMin()) {
    cout << "Zadane c je mensie nez najmensi prvok mnoziny.\n";
    delete [] inputArray;
    return 0;
    }

cout << "Mohutnost mnoziny: " << n << endl << "Horni mez: " << c << endl << "Pocet podmnozin: " << a << endl << "Mnozina: ";


showInput();

/*
    Vypocet:

    inputArray - obsahuje len prvky,ktore sa budu ucastnit vypoctu
    actualSet - pocet prvkov ucastniacich sa vypoctu
*/
allElementsSum=0;
totalIterator=0;
string mask("");
// vytvorenie povodnej masky

for(int i=0;i<actualSet;i++){
mask.append("0");
allElementsSum=allElementsSum+inputArray[i];
}

//generovanie vsetkych moznych masiek

//maskAmount=pow(2,actualSet);
maskAmount=100000;
combinations=new string[maskAmount];
sumOfMask=new int[maskAmount];

    while (next(mask.begin(), mask.end()))
    {

        combinations[totalIterator]=mask;

        /*
        kazda vytvorena maska je vhodena do metody applyMask
        kde sa kontroluje ci maska na aktualnej mnozine nepresiahne
        sumou hornu mez, v pripade ze ano nezvysuje sa iterator
         a teda sa aktualna maska prepise nasledujucou
        */

        applyMask(combinations[totalIterator]);                 //



    }


/*
combinations - vsetky masky, ktore aplikovane na actualSet tvoria mnozinu so sumou mensou nez c
totalIterator - pocet tychto masiek
*/


string result="";
string temp;
int finalSum;
int totalSum=0;
int iteration;

mask="";
for(int i=0;i<totalIterator;i++)
mask.append("0");




while (next(mask.begin(), mask.end()))
    {
        temp="";
        for(int i=0;i<actualSet;i++)
        temp.append("0");
        finalSum=0;
        iteration=0;


        for(int j=0;j<totalIterator;j++){

            if(mask[j]=='1') {
                if(bitAND(temp,combinations[j])){

                iteration++;
                if(iteration>a) break;
                temp=sumUpMasks(temp,combinations[j]);

                finalSum=finalSum+sumOfMask[j];

                }
                else break;
            }






        }

                if(finalSum>totalSum) {
                totalSum=finalSum;
                result=mask;
                cout << "\nZatial najdeny najvacsi sucet disjunktnych podmnozin je: " << totalSum<<endl;
                for(int k=0;k<totalIterator;k++)
                if(result[k]=='1')writeMaskedSet(combinations[k]);

                if(totalSum==allElementsSum){
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
