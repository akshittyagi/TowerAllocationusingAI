//
//  main.cpp
//  COL333-A1
//

#include <iostream>
#include <string>
#include <fstream>
#include <string.h>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <time.h>
#define max 10000
using namespace std;

struct Bid
{
    int companyId;
    double price;
    int numRegionsInBid;  // no. of regions in one bid
    int regions[max];
};

float tim;
int numRegions;
int numBids;
int numCompanies;
struct Bid allBids[max]; // all input bids

struct State
{
    //Array for marking selected items in ith State
    bool bidsSelected[max];
    bool companiesSelected[max];
    bool regionsSelected[max];

    State(){
        for (int i=0; i<numBids; i++)
            bidsSelected[i] = false;
        for (int i=0; i<numRegions; i++)
            regionsSelected[i] = false;
        for (int i=0; i<numCompanies; i++)
            companiesSelected[i] = false;
    }

    State (State &s){
        for (int i=0; i<numBids; i++)
            bidsSelected[i] = s.bidsSelected[i];
        for (int i=0; i<numRegions; i++)
            regionsSelected[i] = s.regionsSelected[i];
        for (int i=0; i<numCompanies; i++)
            companiesSelected[i] = s.companiesSelected[i];
    }

    bool checkValidState(){
        for (int i=0; i<numRegions; i++)
            regionsSelected[i] = false;
        for (int i=0; i<numCompanies; i++)
            companiesSelected[i] = false;
        for (int i=0; i<numBids; i++){
            if (bidsSelected[i]){
                if (companiesSelected[allBids[i].companyId])
                    return false;
                else
                    companiesSelected[allBids[i].companyId] = true;
                for (int j=0; j<allBids[i].numRegionsInBid; j++){
                    if (regionsSelected[allBids[i].regions[j]])
                        return false;
                    else
                        regionsSelected[allBids[i].regions[j]] = true;
                }
            }
        }
        return true;
    }

    State* getNeighbours (int &numNeighbours){
        State* neighbours = new State[max];
        numNeighbours = 0;

        for(int i=0; i<numBids; i++){
            State neighbour(*this);
            if (bidsSelected[i]){
                neighbour.bidsSelected[i] = false;
                if (neighbour.checkValidState())
                    neighbours[numNeighbours++] = neighbour;

                for (int j=0; j<numBids; j++){
                    if (j!=i and !neighbour.bidsSelected[j]){
                        neighbour.bidsSelected[j]=true;
                        if (neighbour.checkValidState())
                            neighbours[numNeighbours++] = neighbour;
                        neighbour.bidsSelected[j]=false;
                    }
                }
            } else {
                neighbour.bidsSelected[i] = true;
                if (neighbour.checkValidState())
                    neighbours[numNeighbours++] = neighbour;
            }
        }

        return neighbours;
    }

    double getValue(){
        double value = 0;
        for (int i=0; i<numBids; i++)
            if (bidsSelected[i])
                value += allBids[i].price;
        return value;
    }

    void randomizeState(){

        for (int i=0; i<numBids; i++)
            bidsSelected[i] = false;
        for (int i=0; i<numRegions; i++)
            regionsSelected[i] = false;
        for (int i=0; i<numCompanies; i++)
            companiesSelected[i] = false;

        srand(time(NULL));
        int num = rand() % numBids;
        addBidToState(num);

        for(int i = (num+1)%numBids; i!=num; i = (i+1)%numBids){
            if(companiesSelected[allBids[i].companyId] || checkRegionClashWithBid(i))
                continue;
            addBidToState(i);
        }
    }

    bool checkRegionClashWithBid (int bidNum){
        for(int i=0; i<allBids[bidNum].numRegionsInBid; i++)
            if(regionsSelected[allBids[bidNum].regions[i]])
                return true;
        return false;
    }

    void addBidToState (int bidNum){
        bidsSelected[bidNum] = true;
        companiesSelected[allBids[bidNum].companyId] = true;
        for (int i=0; i<allBids[bidNum].numRegionsInBid; i++)
            regionsSelected[allBids[bidNum].regions[i]] = true;
    }
};

void readFile()
{
    // Give file to STDIN in with "./a.out < inputfile.txt" or enter full file path below
//    freopen("/Users/Shantanu/Documents/College/SemVII/AI/Assign1/A1/A1/input.txt", "r", stdin);

    string g;
    scanf("%f\n\n",&tim);
    scanf("%d\n\n",&numRegions);
    scanf("%d\n\n",&numBids);
    scanf("%d\n\n",&numCompanies);
    for(int i=0;i<numBids;i++)
    {
        cout<<flush;
        string ch;
        getline(cin,ch);
        int t=0;int j=0;
        char ch1[max];
        while(ch[t]!=' ')
        {
            ch1[j]=ch[t];
            j++;t++;
        }

        ch1[j]='\0';
        allBids[i].companyId=atoi(ch1);

        ch1[0]='\0';j=0;t++;
        while(ch[t]!=' ')
        {
            ch1[j]=ch[t];
            j++;t++;
        }
        ch1[j]='\0';
        allBids[i].price=strtod (ch1, NULL);
        t++;

        int x=0;
        int w=t;
        while(ch[t]!='#')
        {
            if(ch[t]==' ')
            {	x++;}
            t++;
        }
        allBids[i].numRegionsInBid=x;
        t=w;
        for(int qq=0;qq<x;qq++)
        {
            ch1[0]='\0';j=0;
            while(ch[t]!=' ')
            {
                ch1[j]=ch[t];
                j++;t++;
            }
            t++;
            ch1[j]='\0';
            allBids[i].regions[qq]=atoi(ch1);
        }
        getline(cin,g);
    }
}

void HillClimbing()
{
    State currentState;
    currentState.randomizeState();
    cout << "State Value: " << (long)currentState.getValue() <<  endl;

    while(true){
        int numNeighbours = 0;
        State* nextStates = currentState.getNeighbours(numNeighbours);

        int maxValueState = 0;
        double maxValue = 0;
        for (int i=0; i<numNeighbours; i++)
            if (nextStates[i].getValue() > maxValue){
                maxValueState = i;
                maxValue = nextStates[i].getValue();
            }

        if (maxValue > currentState.getValue())
            currentState = nextStates[maxValueState];
        else
            break;
        cout << "State Value: " << (long)currentState.getValue() <<  endl;
    }


/*
    for(int i=0; i<numBids; i++)
        if(currentState.bidsSelected[i])
            cout << i << " ";
    cout << "#" << endl;
    cout << "State Value: " << (long)currentState.getValue() <<  endl;
*/
}

void HillClimbingWithRandomRestarts(int maxLimit)
{

    long maxValue=-1;
    for(int counter = 0;counter<maxLimit; counter++)
    {
        State currentState;
        currentState.randomizeState();
        cout<<"Starting"<<endl;
        cout << "State Value: " << (long)currentState.getValue() <<  endl;

    while(true){
        int numNeighbours = 0;
        State* nextStates = currentState.getNeighbours(numNeighbours);

        int maxValueState = 0;
        double maxValue = 0;
        for (int i=0; i<numNeighbours; i++)
            if (nextStates[i].getValue() > maxValue){
                maxValueState = i;
                maxValue = nextStates[i].getValue();
            }

        if (maxValue > currentState.getValue())
            currentState = nextStates[maxValueState];
        else
            break;
        cout << "State Value: " << (long)currentState.getValue() <<  endl;
    }

    if(maxValue<currentState.getValue())
    {
        maxValue = currentState.getValue();
    }
    cout<<"Ending"<<endl;
    }

    cout<<"Maxvalue is: "<<maxValue<<endl;
}


int main()
{
    readFile();
   // HillClimbing();
    HillClimbingWithRandomRestarts(2);
    return 0;
}
