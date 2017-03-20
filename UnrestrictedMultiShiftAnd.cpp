//License MIT 2017 Ahmad Retha

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include "UnrestrictedMultiShiftAnd.hpp"

using namespace std;

/**
 * @constructor An alphabet must be supplied of the letters that will be used in the patterns
 *
 * @param alphabet A list of characters used in the text/patterns e.g. ACGT
 */
UnrestrictedMultiShiftAnd::UnrestrictedMultiShiftAnd(const string & alphabet)
{
    this->alphabet = alphabet;
    this->N = 0;
    this->M = 0;
    this->L = 1;
    this->Sv.push_back(0ul);
    this->Ev.push_back(0ul);
    this->D.push_back(0ul);
    vector<WORD> v;
    unsigned int i;
    for (i = 0; i < this->alphabet.length(); i++)
    {
        this->Sigma[(int)alphabet[i]] = i + 1;
        v.push_back(0ul);
    }
    this->Bv.push_back(v);
}

/**
 * Add a pattern
 *
 * @param pattern
 */
void UnrestrictedMultiShiftAnd::addPattern(const std::string & pattern)
{
    unsigned int i, j, m = pattern.length();

    //if we need more memory to hold the new pattern then create and initialize new words
    unsigned int l = (unsigned int) ceil((double)(this->M + m) / (double)BITSINWORD);
    unsigned int numWordsDiff = l - this->L;
    if (numWordsDiff > 0)
    {
        for (i = 0; i < numWordsDiff; i++)
        {
            this->Sv.push_back(0ul);
            this->Ev.push_back(0ul);
            this->D.push_back(0ul);
        }
        for (i = 0; i < numWordsDiff; i++)
        {
            vector<WORD> v;
            for (j = 0; j < this->alphabet.length(); j++)
            {
                v.push_back(0ul);
            }
            this->Bv.push_back(v);
        }
    }

    //process the pattern for Bitvector Bv
    int charIdx;
    unsigned int currWordIdx = this->L - 1;
    unsigned int currBitIdx = this->M % BITSINWORD;
    for (i = 0; i < m; i++)
    {
        charIdx = (int) this->Sigma[(int)pattern[i]];
        if (charIdx > 0) {
            this->Bv[currWordIdx][charIdx - 1] = this->Bv[currWordIdx][charIdx - 1] | (1ul << currBitIdx);
        }
        currBitIdx = ++currBitIdx % BITSINWORD;
        if (currBitIdx == 0) {
            currWordIdx++;
        }
    }

    //process the pattern to set Start bit in Sv
    currWordIdx = this->L - 1;
    currBitIdx = this->M % BITSINWORD;
    this->Sv[currWordIdx] = this->Sv[currWordIdx] | (1ul << currBitIdx);

    //process the pattern to set End bit in Ev and update the class variables
    this->L = this->L + numWordsDiff;
    this->M = this->M + m;
    this->N = this->N + 1;
    this->Ev[this->L - 1] = this->Ev[this->L - 1] | (1ul << ((this->M % BITSINWORD) - 1));

    //keep a record of position the pattern is stored and pattern id - for search results
    this->positions.insert(pair<int,int>(this->M - 1, this->N - 1));
}

/**
 * Search a text
 * @param text
 * @return True if one or matches found, otherwise False
 */
bool UnrestrictedMultiShiftAnd::search(const string & text)
{
    //initialize vector D to have a fresh state for the new search
    this->D.assign(this->L, 0ul);
    return this->search(text, this->D);
}

/**
 * Search a text but supply an initial search state - this is useful for searching
 * text provided intermittently (online algorithm). For simple searches just use
 * UnrestrictedMultiShiftAnd::search()
 *
 * @param text
 * @param startingSearchState A std::vector<WORD> with L elements
 * @return True if one or matches found, otherwise False
 */
bool UnrestrictedMultiShiftAnd::search(const string & text, vector<WORD> & startingSearchState)
{
    unsigned int i, j, k, n = text.length();
    bool matchFound = false;

    if (this->M == 0) {
        cerr << "No patterns added to search." << endl;
        return false;
    }

    //Make sure vector D has sufficient memory for the search
    this->D = startingSearchState;
    if (this->D.size() < this->L) {
        j = this->L - this->D.size();
        for (i = 0; i < j; i++) {
            this->D.push_back(0ul);
        }
    }

    //init tracking vars
    int charIdx;
    WORD temp, carry, checking, one = 1ul, byteMask = 255ul;
    WORD carryMask = 1ul << (BITSINWORD - 1);

    //loop through the text
    for (i = 0; i < n; i++)
    {
        carry = 0;
        charIdx = (int) this->Sigma[(int)text[i]] - 1;

        //loop through the words
        for (j = 0; j < this->L; j++)
        {
            temp = this->D[j];

            this->D[j] = (((this->D[j] << 1) | carry) | this->Sv[j]) & this->Bv[j][charIdx];

            //check if any matches found
            if (this->D[j] & this->Ev[j])
            {
                matchFound = true;

                //find out position of the match
                checking = this->D[j] & this->Ev[j];
                k = 0;
                while (checking != 0)
                {
                    if (checking & byteMask)
                    {
                        if (checking & one) {
                            this->matches.insert(pair<int,int>((int)i, this->positions.find(j * BITSINWORD + k)->second));
                        }
                        checking = checking >> 1;
                        k++;
                    }
                    else
                    {
                        checking = checking >> 8;
                        k += 8;
                    }
                }
            }

            carry = (WORD) ((carryMask & temp) != 0);
        }
    }

    return matchFound;
}

/**
 * After doing a search, get the last state of the search
 */
std::vector<WORD> UnrestrictedMultiShiftAnd::getLastSearchState() const
{
    return this->D;
}

/**
 * Clear matches
 */
void UnrestrictedMultiShiftAnd::clearMatches()
{
    this->matches.clear();
}

/**
 * Get the number of patterns already added
 */
unsigned int UnrestrictedMultiShiftAnd::getNumberOfPatterns() const
{
    return this->N;
}

/**
 * Get the total length of the patterns added
 */
unsigned int UnrestrictedMultiShiftAnd::getTotalPatternLength() const
{
    return this->M;
}

/**
 * Get a map of all the matches found - <index_in_t, pattern_id>
 */
std::map<int,int> UnrestrictedMultiShiftAnd::getMatches() const
{
    return this->matches;
}