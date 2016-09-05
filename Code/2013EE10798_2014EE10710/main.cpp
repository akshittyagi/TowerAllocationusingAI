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
#include <pthread.h>
using namespace std;

struct Bid
{
    int bidId;
    int companyId;
    double price;
    int numRegionsInBid;  // no. of regions in one bid
    vector<int> regions;
};

struct BidPriceComparator{
    const bool operator()(Bid A, Bid B) const{
        return (A.price > B.price);
    }
};

double START_TIME;
float tim;
int numRegions;
int numBids;
int numCompanies;
vector<Bid> allBids;    // all input bids
vector<vector<int> > bidsOfCompany;
vector<vector<int> > bidsForRegion;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
char* OutputFileName;

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
    
    vector<State> getBetterNeighboursFaster (){
        vector<State> neighbours;
        
        for(int i=0; i<numBids; i++){
            State neighbour = *this;
            if (bidsSelected[i]){
                neighbour.removeBidFromState(i);
                neighbours.push_back(neighbour);
                
                unordered_set<int> candidateBids;
                for (int j=0; j<allBids[i].numRegionsInBid; j++)
                    for (int k=0; k<bidsForRegion[allBids[i].regions[j]].size(); k++)
                        candidateBids.insert(bidsForRegion[allBids[i].regions[j]][k]);
                for (int j=0; j<bidsOfCompany[allBids[i].companyId].size(); j++)
                    candidateBids.insert(bidsOfCompany[allBids[i].companyId][j]);
                candidateBids.erase(i);

                for (unordered_set<int>::iterator it=candidateBids.begin(); it!=candidateBids.end(); it++){
                    if (neighbour.canAddBidToState(*it)){
                        neighbour.addBidToState(*it);
                        neighbours.push_back(neighbour);
                        
                        State neighbour2 = neighbour;
                        for (unordered_set<int>::iterator it2=it; it2!=candidateBids.end(); it2++){
                            if (neighbour2.canAddBidToState(*it2)){
                                neighbour2.addBidToState(*it2);
                                neighbours.push_back(neighbour2);
                            }
                        }
                        neighbour.removeBidFromState(*it);
                    }
                }
                
                for (unordered_set<int>::iterator it=candidateBids.begin(); it!=candidateBids.end(); it++){
                    if (neighbour.canAddBidToState(*it)){
                        neighbour.addBidToState(*it);
                        for (unordered_set<int>::iterator it2=it; it2!=candidateBids.end(); it2++){
                            if (neighbour.canAddBidToState(*it2)){
                                neighbour.addBidToState(*it2);
                                neighbours.push_back(neighbour);
                            }
                        }
                        neighbour.removeBidFromState(*it);
                    }
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
        return;
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
        return;
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
        return;
    }
    
    void print(){
        ofstream fil(OutputFileName);
        for(int i=0; i<numBids; i++)
            if(bidsSelected[i])
                fil << allBids[i].bidId << " ";
        fil << "#" << endl;
        fil.close();
        return;
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
    bidsForRegion.resize(numRegions);
    for (int i=0; i<numRegions; i++){
        vector<int> A;
        bidsForRegion[i] = A;
    }
    
    for(int i=0;i<numBids;i++)
    {
        Bid inputBid;
        inputBid.bidId = i;
        cout<<flush;
        string ch;
        getline(cin,ch);
        int t=0;int j=0;
        char ch1[100];
        while(ch[t]!=' ')
        {
            ch1[j]=ch[t];
            j++;t++;
        }
        
        ch1[j]='\0';
        inputBid.companyId=atoi(ch1);
        
        ch1[0]='\0';j=0;t++;
        while(ch[t]!=' ')
        {
            ch1[j]=ch[t];
            j++;t++;
        }
        ch1[j]='\0';
        inputBid.price=strtod (ch1, NULL);
        t++;
        
        int x=0;
        int w=t;
        while(ch[t]!='#')
        {
            if(ch[t]==' ')
            {   x++;}
            t++;
        }
        inputBid.numRegionsInBid=x;
        t=w;
        inputBid.regions.resize(x);
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
            inputBid.regions[qq]=atoi(ch1);
        }
        allBids.push_back(inputBid);
        getline(cin,g);
    }
    
    sort(allBids.begin(), allBids.end(), BidPriceComparator());
    for(int i=0;i<numBids;i++){
        bidsOfCompany[allBids[i].companyId].push_back(i);
        for (int j=0; j<allBids[i].numRegionsInBid; j++)
            bidsForRegion[allBids[i].regions[j]].push_back(i);
    }
    return;
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

long BeamSearch (int k){
    vector<State> beam;
    State randState;
    for (int i=0; i<k; i++){
        randState.randomizeState();
        beam.push_back(randState);
    }
    cout << "State Value: " << (long)beam[0].getValue() <<  endl;
    
    while(true){
        priority_queue<State, vector<State>, StateValueComparator> stateHeap;
        
        unordered_set<vector<bool> > statesInHeap;
        for (int i=0; i<beam.size(); i++)
            statesInHeap.insert(beam[i].bidsSelected);
        for (int i=0; i<beam.size(); i++){
            vector<State> nextStates = beam[i].getNeighbours();
            for (int j=0; j<nextStates.size(); j++)
                if (statesInHeap.find(nextStates[j].bidsSelected)==statesInHeap.end()){
                    stateHeap.push(nextStates[j]);
                    statesInHeap.insert(nextStates[j].bidsSelected);
                }
        }
        
        vector<State> nextBeam;
        for (int i=0; i<k && !stateHeap.empty(); i++){
            nextBeam.push_back(stateHeap.top());
            stateHeap.pop();
        }
        
        if (nextBeam[0].getValue() > beam[0].getValue())
            beam = nextBeam;
        else
            break;
        cout << "State Value: " << (long)beam[0].getValue() <<  endl;
    }
    return (long)beam[0].getValue();
}

long HillClimbingWithTabu()
{
    State currentState;
    currentState.randomizeState();
    //    cout << "State Value: " << (long)currentState.getValue() <<  endl;

    unordered_set<vector<bool> > TabuList;

    int TabuStepCount = 0;
    State BestStateTillNow;
    double BestStateTillNowValue = 0;

    while(true){
        vector<State> nextStates = currentState.getNeighbours();
        
        int maxValueState = 0;
        double maxValue = 0;
//        int start_i = rand() % (nextStates.size() - 5);
        for (int i=0; i<nextStates.size(); i++)
            if (nextStates[i].getValue() > maxValue && (TabuList.find(nextStates[i].bidsSelected)==TabuList.end())){
                maxValueState = i;
                maxValue = nextStates[i].getValue();
            }

        if (maxValue <= currentState.getValue()){
            TabuStepCount++;
            if (TabuStepCount > 100)
                break;
        }
        TabuList.insert(currentState.bidsSelected);
        currentState = nextStates[maxValueState];
        if (BestStateTillNowValue < currentState.getValue()){
            BestStateTillNowValue = currentState.getValue();
            BestStateTillNow = currentState;
        }

//        cout << "State Value: " << (long)currentState.getValue() << " Best: " << (long)BestTillNow << endl;
    }
    
    return (long)BestStateTillNow.getValue();
}

State BeamSearchWithTabu (int k, unordered_set<vector<bool> > TabuList){
    vector<State> beam;
    State randState;
    for (int i=0; i<k; i++){
        randState.randomizeState();
        beam.push_back(randState);
    }
//    cout << "State Value: " << (long)beam[0].getValue() <<  endl;
    
    int TabuStepCount = 0;
    State BestStateTillNow;
    double BestStateTillNowValue = 0;

    while(true){
        priority_queue<State, vector<State>, StateValueComparator> stateHeap;
        
        for (int i=0; i<beam.size(); i++)
            TabuList.insert(beam[i].bidsSelected);
        for (int i=0; i<beam.size(); i++){
            vector<State> nextStates = beam[i].getBetterNeighboursFaster();
            for (int j=0; j<nextStates.size(); j++)
                if (TabuList.find(nextStates[j].bidsSelected)==TabuList.end()){
                    stateHeap.push(nextStates[j]);
                    TabuList.insert(nextStates[j].bidsSelected);
                }
        }
        
        if (BestStateTillNowValue < ((State)stateHeap.top()).getValue()){
            BestStateTillNowValue = ((State)stateHeap.top()).getValue();
            BestStateTillNow = stateHeap.top();
        }
        
        vector<State> nextBeam;
        double maxValue = 1.5 * ((State)stateHeap.top()).getValue();
        for (int i=0; nextBeam.size()<k && !stateHeap.empty(); i++){
            if (rand()%100 < ((State)stateHeap.top()).getValue()*100/maxValue)
                nextBeam.push_back(stateHeap.top());
            stateHeap.pop();
        }

        if (nextBeam[0].getValue() <= beam[0].getValue()){
            TabuStepCount++;
            if (TabuStepCount > 10)
                break;
        }
        beam = nextBeam;
        
//        cout << "State Value: " << (long)beam[0].getValue() << " Tabu Steps: " << TabuStepCount <<  endl;
    }
    return BestStateTillNow;
}

void* BeamSearchWithRandomRestarts (void* ptr)
{
    int maxLimit = 100;
    long* maxValue = (long*)ptr;
    unordered_set<vector<bool> > TabuList;
    int i;
    for (i = 0; i<maxLimit && ((time(NULL) - START_TIME) < (tim * 60)); i++){
        State bestState = BeamSearchWithTabu(20, TabuList);
        long currValue = bestState.getValue();
        if (currValue > *maxValue){
            pthread_mutex_lock( &mutex1 );
            if (currValue > *maxValue){
                *maxValue = currValue;
//                cout << "Current maxvalue is: " << *maxValue << " at " << time(NULL) - START_TIME << " seconds and " << i << " restarts" << endl;
                bestState.print();
            }
            pthread_mutex_unlock( &mutex1 );
        }
    }
//    cout << "Maxvalue after " << i << " random restarts: " << *maxValue << endl;
    return NULL;
}

int main (int argc, char *argv[])
{
    START_TIME = time(NULL);
    srand((unsigned int)time(NULL));
    OutputFileName = argv[1];
    readFile();

    pthread_t thread1, thread2, thread3;
    int  iret1, iret2, iret3;
    long maxValue = -1;

    iret1 = pthread_create( &thread1, NULL, BeamSearchWithRandomRestarts, &maxValue);
    if(iret1)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
        exit(EXIT_FAILURE);
    }
    
    iret2 = pthread_create( &thread2, NULL, BeamSearchWithRandomRestarts, &maxValue);
    if(iret2)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret2);
        exit(EXIT_FAILURE);
    }

    iret2 = pthread_create( &thread2, NULL, BeamSearchWithRandomRestarts, &maxValue);
    if(iret3)
    {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",iret3);
        exit(EXIT_FAILURE);
    }

//    printf("pthread_create() for thread 1 returns: %d\n",iret1);
//    printf("pthread_create() for thread 2 returns: %d\n",iret2);
//    printf("pthread_create() for thread 3 returns: %d\n",iret3);
    
    pthread_join( thread1, NULL);
    pthread_join( thread2, NULL);
    pthread_join( thread3, NULL);
    
    exit(EXIT_SUCCESS);
    
    return 0;
}

