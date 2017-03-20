//License MIT 2017 Ahmad Retha

#include <iostream>
#include <cstdlib>
#include <string>
#include <map>
#include "UnrestrictedMultiShiftAnd.hpp"

using namespace std;

int main(int argc, char ** argv)
{
    if (argc < 3) {
        cerr << "Too few Arguments!" << endl;
        cerr << "Usage: ./umsa DNA_SEQUENCE DNA_PATTERN1 [DNA_PATTERN2 ...]" << endl;
        return 1;
    }

    //setting it to search only DNA patterns in a text
    UnrestrictedMultiShiftAnd umsa("ACGT");

    for (int i = 2; i < argc; i++)
    {
        string p(argv[i]);
        umsa.addPattern(p);
    }

    string text(argv[1]);

    bool matchesFound = umsa.search(text);

    if (matchesFound)
    {
        for (const pair<int,int> & p : umsa.getMatches()) {
            cout << "Pattern " << p.second << " found at position " << p.first << endl;
        }
    }
    else
    {
        cout << "No matches found!" << endl;
    }

    return 0;
}
