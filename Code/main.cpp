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
#include <queue>
#include <ctime>
#include <unordered_set>
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
vector<vector<int> > bidsOfCompany;

struct State
{
    vector<bool> bidsSelected;
    vector<bool> companiesSelected;
    vector<bool> regionsSelected;
    
    State(){
        bidsSelected.resize(numBids);
        regionsSelected.resize(numRegions);
        companiesSelected.resize(numCompanies);
        for (int i=0; i<numBids; i++)
            bidsSelected[i] = false;
        for (int i=0; i<numRegions; i++)
            regionsSelected[i] = false;
        for (int i=0; i<numCompanies; i++)
            companiesSelected[i] = false;
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
    
    vector<State> getNeighbours (){
        vector<State> neighbours;
        
        for(int i=0; i<numBids; i++){
            State neighbour = *this;
            if (bidsSelected[i]){
                neighbour.removeBidFromState(i);
                neighbours.push_back(neighbour);
                
                for (int c=0; c<numCompanies; c++)
                    if (!neighbour.companiesSelected[c])
                        for (int j=0; j<bidsOfCompany[c].size(); j++)
                            if (bidsOfCompany[c][j]!=i)
                                if (!neighbour.checkRegionClashWithBid(bidsOfCompany[c][j])){
                                    neighbour.addBidToState(bidsOfCompany[c][j]);
                                    neighbours.push_back(neighbour);
                                    neighbour.removeBidFromState(bidsOfCompany[c][j]);
                                }

            } else {
                if (neighbour.canAddBidToState(i)){
                    neighbour.addBidToState(i);
                    neighbours.push_back(neighbour);
                }
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

    bool canAddBidToState (int bidNum){
        if (companiesSelected[allBids[bidNum].companyId])
            return false;
        return (!checkRegionClashWithBid(bidNum));
    }
    
    void removeBidFromState (int bidNum){
        bidsSelected[bidNum] = false;
        companiesSelected[allBids[bidNum].companyId] = false;
        for (int i=0; i<allBids[bidNum].numRegionsInBid; i++)
            regionsSelected[allBids[bidNum].regions[i]] = false;
    }
};

void readFile()
{
    // Give file to STDIN in with "./a.out < inputfile.txt" or enter full file path below
//    freopen("/Users/Shantanu/Documents/College/SemVII/AI/Assign1/Code/TestCases/3.txt", "r", stdin);
    
    string g;
    scanf("%f\n\n",&tim);
    scanf("%d\n\n",&numRegions);
    scanf("%d\n\n",&numBids);
    scanf("%d\n\n",&numCompanies);
    
    bidsOfCompany.resize(numCompanies);
    for (int i=0; i<numCompanies; i++){
        vector<int> A;
        bidsOfCompany[i] = A;
    }
    
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
            {   x++;}
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
    
    for(int i=0;i<numBids;i++){
        bidsOfCompany[allBids[i].companyId].push_back(i);
    }
}

long HillClimbing()
{
    State currentState;
    currentState.randomizeState();
    //    cout << "State Value: " << (long)currentState.getValue() <<  endl;
    
    while(true){
        vector<State> nextStates = currentState.getNeighbours();
        
        int maxValueState = 0;
        double maxValue = 0;
        for (int i=0; i<nextStates.size(); i++)
            if (nextStates[i].getValue() > maxValue){
                maxValueState = i;
                maxValue = nextStates[i].getValue();
            }
        
        if (maxValue >= currentState.getValue())
            currentState = nextStates[maxValueState];
        else
            break;
        cout << "State Value: " << (long)currentState.getValue() <<  endl;
    }
    
    return (long)currentState.getValue();
    /*
     for(int i=0; i<numBids; i++)
     if(currentState.bidsSelected[i])
     cout << i << " ";
     cout << "#" << endl;
     cout << "State Value: " << (long)currentState.getValue() <<  endl;
     */
}

struct StateValueComparator{
    const bool operator()(State A, State B) const{
        return (A.getValue() <= B.getValue());
    }
};

void BeamSearch (int k){
    State *beam = new State[k];
    for (int i=0; i<k; i++)
        beam[i].randomizeState();
    cout << "State Value: " << (long)beam[0].getValue() <<  endl;
    
    while(true){
        priority_queue<State, vector<State>, StateValueComparator> stateHeap;
        
        for (int i=0; i<k; i++){
            vector<State> nextStates = beam[i].getNeighbours();
            for (int j=0; j<nextStates.size(); j++)
                stateHeap.push(nextStates[j]);
        }
        
        State* nextBeam = new State[k];
        int i;
        for (i=0; i<k && !stateHeap.empty(); i++){
            nextBeam[i] = stateHeap.top();
            stateHeap.pop();
        }
        k = i;
        
        if (nextBeam[0].getValue() >= beam[0].getValue()){
            delete [] beam;
            beam = nextBeam;
        }
        else
            break;
        cout << "State Value: " << (long)beam[0].getValue() <<  endl;
    }
}

long HillClimbingWithTabu()
{
    State currentState;
    currentState.randomizeState();
    //    cout << "State Value: " << (long)currentState.getValue() <<  endl;

    unordered_set<vector<bool> > TabuList;

    int TabuStepCount = 0;
    double BestTillNow = 0;

    int steps = 0;
    while(true){
        vector<State> nextStates = currentState.getNeighbours();
        steps++;
        
        int maxValueState = 0;
        double maxValue = 0;
        for (int i=0; i<nextStates.size(); i++)
            if (nextStates[i].getValue() > maxValue && (TabuList.find(nextStates[i].bidsSelected)==TabuList.end())){
                maxValueState = i;
                maxValue = nextStates[i].getValue();
            }

        if (maxValue <= currentState.getValue()){
            TabuStepCount++;
            TabuList.insert(currentState.bidsSelected);
            if (TabuStepCount > 1000)
                break;
        }
        currentState = nextStates[maxValueState];
        if (BestTillNow < currentState.getValue())
            BestTillNow = currentState.getValue();

//        cout << "State Value: " << (long)currentState.getValue() << " Best: " << (long)BestTillNow <<  " Steps: " << steps << endl;
    }
    
    return (long)currentState.getValue();
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
    long maxValue = -1;
    for (int i = 0; i<maxLimit; i++){
        long currValue = HillClimbingWithTabu();
        if (currValue > maxValue)
            maxValue = currValue;
        cout << "Current maxvalue is: " << maxValue << endl;
    }
    cout << "Maxvalue after " << maxLimit << " random restarts: " << maxValue << endl;
}

int main()
{
    srand((unsigned int)time(NULL));
//    srand(0);
    readFile();
    HillClimbingWithRandomRestarts(100);
//    BeamSearch(50);
    
    return 0;
}
