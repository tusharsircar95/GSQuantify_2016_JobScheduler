#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include<bits/stdc++.h>
#include <algorithm>
#include<string>
#define ll unsigned long long
using namespace std;

/*
USER DEFINED STRUCTURES
(a) Job
Stores all information about a job, that is the timestamp,processID,originSystem,instruction,importance and duration
along with a few introduced variables:
 - ID   : SNo. of the job based on the order that it comes in. Uniquely identifies a job.
 - qExit: Time when job leaves the queue i.e. when a CPU is allotted to it

(b) MaxMinNode
Used to store the maximum and minimum timestamp across jobs waiting in the queue at a particular timestamp.
Used to reduce search space while querying (Explained later)

(c) CPUNode
Node for a height-balanced BST tree (AVL Tree). This may contain the same value multiple times and this is tracked
using the frequency variable.

STL CONTAINER USED:
(a) Vector

ALGORITHM OUTLINE:
Part 1: 'job' and 'assign'
Objective here is to be able to store incoming jobs in such a way that the max priority job can be fetched
efficiently during 'assign'. Moreover, we need to keep track of CPU's to which jobs can be assigned at any given time.

The former is achieved by using a max-heap based priority queue (PQ). Whenever a job comes in we simply insert into the heap and at the time of assignment we simply extract the max priority job.

Now to find the number of CPU's that are available at a given time T, it is enough to find the number of CPU's
that get free at any time t ( <= T) (assuming that a new job can be scheduled as soon as the CPU gets free)
To achieve this, we have used a height balanced BST  (AVL Tree). 

Let's call the time after which a CPU gets free as freeAfter.
Inititally all CPU's have freeAfter set to 0 (because timestamp >= 0).

To find number of CPU's availble at time T we query the number of values in BST that are less than or equal to T. This is done in logarithimic time. Then, for every job to be assigned, we delete a node with minimum freeAfter value and insert
a new node with freeAfter value set to (T + Duration Of Job)

Also while assigning a job to the CPU, we update it's qExti value to the current timestamp.
Note that each job is inserted into a job list so that all information of a job can be directly accessed once we have
it's ID.

Complexity:
'job'   : O(logN)
'assign': O(logM)
N: Number of jobs present in queue at the moment
M: Number of CPU's

Part 2: 'query'
Here we need to print in decreasing order of priority all jobs that were waiting in queue at a particular time T
in history.
Firstly note that if we have got 'job' and 'assign' commands at times T(1), T(2) T(3), ... etc. then querying for time T
where T(i) <= T < T(i+1) is equivalent to querying for T(i).
The naive way to do this is to store the state of the job heap at every timestamp and while querying simply extract
the required number of jobs and print. But obviously, this will blow up the memory.

The other way is to go through all the jobs in history and check for jobs that have timestamp <= T and T < qExit as
these are jobs that were residing in the queue at time T. Once we get these jobs we build a heap on top of them and then extract the desired number of highest priority jobs.


The main components that consume time here are:
 - Searching all jobs in history
 - Building a heap for each query
 
Note that we have all the jobs stored in increasing order of timestamp. So at each timestamp instead of storing the entire queue we store the maximum and minimum value of timestamp for jobs in the queue at that moment.
This way when we make a query we only have to search for jobs whose timestamps lie in this range and this can be found directly with a binary search routine. 
This is helpful as for a large timestamp query it is likely that we might not need to look at jobs with a very small timestamp.

The next bottle-neck is the construct of heap for every query. To avoid this we store the jobs in an ordering based on priority so that we can scan through them in decreasing order of priority.

For this, we use the fact that there are only 100 distinct importance levels. We first bucket all jobs based on this. Also within this bucket all the jobs are in increasing order of timestamps. The only ambiguity is for jobs with the same timestamp and bucket but different durations. To handle this we keep a seperate priority queue for each importance bucket so that we insert jobs with same timestamps in increasing order of duration.
This way once we enter into a bucket, all jobs here are in decreasing order of priority.

So finally, given a query for time T we start visiting these buckets in decreasing order of importance and scan through them linearly till we get the required number of jobs. No heap construction is requried now.
 
COMPLEXITY:
Worst Case: O(N)
But since jobs will be distributed across prioritites and we are significantly reducing our search space using the
min/max values we get good average running times


Note that a partition based on 'origin' was also tried, but that gave us higher running time on the given test cases as so that approach was dropped.


OPTIMIZATIONS AND DESGIN CHOICES:
(a) Firstly since the number of distinct importance levels was 100, we partitioned our jobHeaps based on this so that the heaps that we perform operations on are 100 times smaller at times of 'assign' and 'delete' (assuming a uniform distribution of jobs across all importance levels)

(b) Instead of making a heap where each node corresponds to a Job instance we have used the jobID as the heap variable. This is more efficient as we avoid copying all the satellite data between nodes duing heapify and extract operations.
Also, accessing the job information using ID is O(1).

(c) To store the freeAfter times of CPU's we have used a BST, but a min-heap would also have done the job. But a BST is more efficient because here we are storing the frequency of a value as well in a node, this makes the size of tree smaller when there are multiple CPU's with the same freeAfter time. This improves both time and memory.
eg. Initially when all M CPU's have freeAfter 0. Instread of creating a heap with M entries all equal to 0, our BST stores a single node with value 0 and frequency M.

(d) While deleting a value from the BST, we can delete a CPUNode with freeAfter less than the given T. We could have deleted the root if it's value was less but we avoided doing so as it would involve finding the in-order sucessor in the right subtree and calling delete on that which would involve more recursions and would be less efficient.

(e) A standard binary heap was chosen over a k-ary heap as k-ary heaps improve insertion / deletion times but increase the time to extract max values. Since this problem heavily uses the extractMax operation, k-ary heap was not a suitable choice.

MEMORY
jobList stores information for all jobs once. O(N)
qJobList simply stores the ID's of all jobs once. 
Similarly our priority queues store JobID rather than the complete information of jobs which makes it efficient in terms of memory.

SOME THINGS WE MISSED
The process of finding free CPU's can be optimzed further. Once we reach time T, all CPU's with freeAfter <= T are equivalent for us and so the tree can be pruned by merging all these into a single node, reducing the tree size.
This however will not leave the tree balanced and a custom balancing routine needs to be thought for it.

Also to maintain our duration heaps we are flushing it into qJobList whenever we see a timestamp greater than a previously seen timestamp. While doing this, to avoid scanning through 100 buckets every time, we have maintained a totalSize variable so that when all the bucket heaps are empty we don't unnecessarily scan through them.
*/



// Class to store job information
class Job
{
    public:
    ll processID;
    ll timestamp;
    string originSystem;
    string instruction;
    ll importance;
    ll duration;
    ll qExit;
    ll ID;
    Job(){}
    Job(ll ID,ll processID,ll timestamp,string originSystem,string instruction,ll importance,ll duration)
    {
        this->processID = processID;
        this->timestamp = timestamp;
        this->originSystem = originSystem;
        this->instruction = instruction;
        this->importance = importance;
        this->duration = duration;
        this->qExit = 0;
        this->ID = ID;
    }
    void printDescription()
    {
        cout<<"job "<<timestamp<<" "<<processID<<" "<<originSystem<<" "<<instruction<<" "<<importance<<" "<<duration<<endl;
    }
    
    
};

// Stores Max/Min Timestamp of a job residing in queue at
// a particular timestamp
struct MaxMinNode
{
    ll Max,Min;
    ll timestamp;
    MaxMinNode(){}
    MaxMinNode(ll Min,ll Max,ll timestamp)
    {
        this->Max = Max;
        this->Min = Min;
        this->timestamp = timestamp;
    }
    void updateValues(ll value)
    {
        Max = max(Max,value);
        Min = min(Min,value);
    }
};

// BST Node to store time after which CPU is free
class CPUNode
{
    public:
    ll freeAfter;
    ll subtreeSize;
    ll freq;
    int height;
    CPUNode *left;
    CPUNode *right;
    CPUNode() {}
    CPUNode(ll freeAfter,ll freq)
    {
        this->freeAfter = freeAfter;
        this->left = this->right = NULL;
        this->freq = freq;
        this->height = 1;
    }
};


// Maps Job ID to Job Class
vector<Job> jobList;

// Partition of jobs seen so far based on importance
vector<ll> qJobList[101];
vector<ll> durationHeaps[101];  // Heaps to store durations for jobs with same timestamp and importance
ll prevTimestamps[101]; // Last seen timestamp by a duration heap
ll durationHeapSize[101] = {0}; // Sizes of duration heaps
ll totalSize = 0; //Total jobs pending to be inserted


// Vector of MaxMinNodes, one for each timestamp seen so far
vector<MaxMinNode> MaxMins;


ll getLeft(ll pos)
{
    return (2*pos) + 1;
}
ll getRight(ll pos)
{
    return (2*pos) + 2;
}
ll getParent(ll pos)
{
    return ((pos-1)/2);
}


bool hasHigherPriority(ll jID1,ll jID2)
{
    Job j1 = jobList[jID1];
    Job j2 = jobList[jID2];
    if(j1.importance > j2.importance)
        return true;
    if(j1.importance < j2.importance)
        return false;
    if(j1.timestamp < j2.timestamp)
        return true;
    if(j1.timestamp > j2.timestamp)
        return false;
    if(j1.duration < j2.duration)
         return true;
    return false; // j2 has higher duration
}

bool containsAlphabet(string s)
{
    for(ll i=0,l=s.length(); i<l; i++)
        if(isalpha(s[i]))
            return true;
    return false;
}

ll convertStringToLL(string s)
{
    ll l = s.length();
    ll result = 0;
    ll k;
    for(ll i=0; i<l; ++i)
    {
        k = s[i] - '0';
        result = (result * 10) + k;
    }
    return result;
}


void maxHeapify(vector<ll> &jobHeap,ll pos,ll jobHeapSize)
{
    ll maxPos = pos;
    ll left = getLeft(pos);
    ll right = getRight(pos);
    
    if(left < jobHeapSize && hasHigherPriority(jobHeap[left],jobHeap[maxPos]))
        maxPos = left;
    if(right < jobHeapSize && hasHigherPriority(jobHeap[right],jobHeap[maxPos]))
        maxPos = right;
    
    if(maxPos != pos)
    {
        ll tempJobID = jobHeap[maxPos];
        jobHeap[maxPos] = jobHeap[pos];
        jobHeap[pos] = tempJobID;
        maxHeapify(jobHeap,maxPos,jobHeapSize);
    }
    
}

void maxHeapify_Duration(vector<ll> &jobHeap,ll pos,ll jobHeapSize)
{
    ll maxPos = pos;
    ll left = getLeft(pos);
    ll right = getRight(pos);
    
    if(left < jobHeapSize && jobList[jobHeap[left]].duration < jobList[jobHeap[maxPos]].duration)
        maxPos = left;
    if(right < jobHeapSize && jobList[jobHeap[right]].duration < jobList[jobHeap[maxPos]].duration)
        maxPos = right;
    
    if(maxPos != pos)
    {
        ll tempJobID = jobHeap[maxPos];
        jobHeap[maxPos] = jobHeap[pos];
        jobHeap[pos] = tempJobID;
        maxHeapify_Duration(jobHeap,maxPos,jobHeapSize);
    }
    
}

ll extractNextJob(vector<ll> &jobHeap,ll &jobHeapSize)
{
    ll jobID = jobHeap[0];
    
    if(jobHeapSize == 1)
    {
        jobHeapSize = 0;
        return jobID;
    }
    jobHeapSize--;
    jobHeap[0] = jobHeap[jobHeapSize];
    maxHeapify(jobHeap,0,jobHeapSize);
    return jobID;
}

ll extractNextID(vector<ll> &jobHeap,ll &jobHeapSize)
{
    totalSize--;
    ll jobID = jobHeap[0];
    
    if(jobHeapSize == 1)
    {
        jobHeapSize = 0;
        return jobID;
    }
    jobHeapSize--;
    jobHeap[0] = jobHeap[jobHeapSize];
    maxHeapify_Duration(jobHeap,0,jobHeapSize);
    return jobID;
}

void insertJobIntoHeap(ll jobID,vector<ll> &jobHeap,ll &jobHeapSize)
{
    if(jobHeapSize + 1 > jobHeap.size())
        jobHeap.push_back(jobID);
    else jobHeap[jobHeapSize] = jobID;
    
    ll pos = jobHeapSize;
    jobHeapSize++;
    
    while(1)
    {
        if(pos == 0)
            break;
        ll parent = getParent(pos);
        if(hasHigherPriority(jobHeap[pos],jobHeap[parent]))
        {
            jobHeap[pos] = jobHeap[parent];
            jobHeap[parent] = jobID;
            pos = parent;
        }
        else break;
    }   
}

void insertIntoDurationHeap(ll jobID,vector<ll> &jobHeap,ll &jobHeapSize)
{
    totalSize++;
    if(jobHeapSize + 1 > jobHeap.size())
        jobHeap.push_back(jobID);
    else jobHeap[jobHeapSize] = jobID;
    
    ll pos = jobHeapSize;
    jobHeapSize++;
    
    while(1)
    {
        if(pos == 0)
            break;
        ll parent = getParent(pos);
        if(jobList[jobHeap[pos]].duration < jobList[jobHeap[parent]].duration)
        {
            jobHeap[pos] = jobHeap[parent];
            jobHeap[parent] = jobID;
            pos = parent;
        }
        else break;
    }   
}


void emptyOutDurationHeap(ll importance)
{
    if(durationHeapSize[importance] == 0)
        return;
    while(durationHeapSize[importance])
    {
        ll popID = extractNextID(durationHeaps[importance],durationHeapSize[importance]);
        qJobList[importance].push_back(popID);
    }
}

void emptyOutAllDurationHeaps()
{
    for(ll i=1; i<=100; i++)
        emptyOutDurationHeap(i);
}

ll getSubtreeSize(CPUNode *cpuNode)
{
    if(cpuNode == NULL)
        return 0;
    return cpuNode->subtreeSize;
}

int getHeight(CPUNode *cpuNode)
{
    if(cpuNode == NULL)
        return 0;
    return cpuNode->height;
}

int getBalance(CPUNode *cpuNode)
{
    if(cpuNode == NULL)
        return 0;
    return getHeight(cpuNode->left) - getHeight(cpuNode->right);
}

CPUNode *rightRotate(CPUNode *y)
{
    CPUNode *x = y->left;
    CPUNode *T2 = x->right;
 
    // Perform rotation
    x->right = y;
    y->left = T2;
 
    // Update heights
    y->height = max(getHeight(y->left), getHeight(y->right))+1;
    x->height = max(getHeight(x->left), getHeight(x->right))+1;
 
    y->subtreeSize = getSubtreeSize(y->left) + y->freq + getSubtreeSize(y->right);
    x->subtreeSize = getSubtreeSize(x->left) + x->freq + getSubtreeSize(x->right);
    // Return new root
    return x;
}
 
CPUNode *leftRotate(CPUNode *x)
{
    CPUNode *y = x->right;
    CPUNode *T2 = y->left;
 
    // Perform rotation
    y->left = x;
    x->right = T2;
 
    //  Update heights
    x->height = max(getHeight(x->left), getHeight(x->right))+1;
    y->height = max(getHeight(y->left), getHeight(y->right))+1;
 
    x->subtreeSize = getSubtreeSize(x->left) + x->freq + getSubtreeSize(x->right);
    y->subtreeSize = getSubtreeSize(y->left) + y->freq + getSubtreeSize(y->right);
    
    // Return new root
    return y;
}

ll getMinimum(CPUNode *startTimeRoot)
{
    CPUNode *root = startTimeRoot;
    while(root->left)
        root = root->left;
    return root->freeAfter;
}
ll getMaximum(CPUNode *startTimeRoot)
{
    CPUNode *root = startTimeRoot;
    while(root->right)
        root = root->right;
    return root->freeAfter;
}

CPUNode* createBST(ll noOfCPU)
{
    CPUNode* root = NULL;
    if(noOfCPU <= 0)
        return root;
    root = new CPUNode(0,noOfCPU);
    root->subtreeSize = noOfCPU;
    return root;
}

ll getValuesLessThanEqualTo(CPUNode *root,ll value)
{
    if(root == NULL)
        return 0;
    if(root->freeAfter <= value)
            return getSubtreeSize(root->left) + root->freq + getValuesLessThanEqualTo(root->right,value);
    return getValuesLessThanEqualTo(root->left,value);
}

CPUNode* deleteCPUNode(CPUNode *root,ll value)
{
    if(root == NULL)
        return root;
    if(root->freeAfter > value) //recurse on left
        root->left = deleteCPUNode(root->left,value);
    else if(root->left != NULL) //recurse on left
        root->left = deleteCPUNode(root->left,value);
    else if(root->freq > 1) //no recursion base case
    {
        root->subtreeSize = root->subtreeSize - 1;
        root->freq = root->freq - 1;
        return root;
    }
    else //delete current node
    {
        if(root->right == NULL)
            return NULL;
        *root = *(root->right);
    }
        
    root->subtreeSize = getSubtreeSize(root->left) + root->freq + getSubtreeSize(root->right);
    root->height = max(getHeight(root->left),getHeight(root->right)) + 1;
    
    int balance = getBalance(root);
    
    if(abs(balance) <= 1)
        return root;
    
    if (balance > 1 && getBalance(root->left) >= 0)
        return rightRotate(root);
 
    // Left Right Case
    if (balance > 1 && getBalance(root->left) < 0)
    {
        root->left =  leftRotate(root->left);
        return rightRotate(root);
    }
 
    // Right Right Case
    if (balance < -1 && getBalance(root->right) <= 0)
        return leftRotate(root);
 
    // Right Left Case
    if (balance < -1 && getBalance(root->right) > 0)
    {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }   
    return root;
}

CPUNode* deleteCustom(CPUNode *root,ll value)
{
    if(root == NULL)
        return root;
    if(root->freeAfter > value) //recurse on left
        root->left = deleteCustom(root->left,value);
    else if(root->freeAfter < value) //recurse on left
        root->right = deleteCustom(root->right,value);
    else if(root->freq > 1) //no recursion base case
    {
        root->subtreeSize = root->subtreeSize - 1;
        root->freq = root->freq - 1;
        return root;
    }
    else //delete current node
    {
        if( (root->left == NULL) || (root->right == NULL) )
        {
            CPUNode *temp = root->left ? root->left : root->right;
            // No child case
            if(temp == NULL)
            {
                temp = root;
                root = NULL;
            }
            else // One child case
             *root = *temp;
            free(temp);
        }
        else
        {
            ll temp = getMinimum(root->right);
            root->freeAfter = temp;
            root->right = deleteCustom(root->right, temp);
        }
    }
        
    if(root == NULL)
        return NULL;
    
    root->subtreeSize = getSubtreeSize(root->left) + root->freq + getSubtreeSize(root->right);
    root->height = max(getHeight(root->left),getHeight(root->right)) + 1;
    
    int balance = getBalance(root);
    
    if(abs(balance) <= 1)
        return root;
    
    if (balance > 1 && getBalance(root->left) >= 0)
        return rightRotate(root);
 
    // Left Right Case
    if (balance > 1 && getBalance(root->left) < 0)
    {
        root->left =  leftRotate(root->left);
        return rightRotate(root);
    }
 
    // Right Right Case
    if (balance < -1 && getBalance(root->right) <= 0)
        return leftRotate(root);
 
    // Right Left Case
    if (balance < -1 && getBalance(root->right) > 0)
    {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }   
    return root;
}


CPUNode* insertCPUNode(CPUNode *root,ll value)
{
    if(root == NULL)
    {
        root = new CPUNode(value,1);
        root->subtreeSize = 1;
        return root;
    }
    if(root->freeAfter == value)
    {
        root->subtreeSize = root->subtreeSize + 1;
        root->freq = root->freq + 1;
        return root;
    }
    
    if(root->freeAfter < value)
        root->right = insertCPUNode(root->right,value);
    else root->left = insertCPUNode(root->left,value);
    
    root->subtreeSize = getSubtreeSize(root->left) + root->freq + getSubtreeSize(root->right);
    root->height = max(getHeight(root->left),getHeight(root->right)) + 1;
    
    int balance = getBalance(root);
    if(abs(balance) <= 1)
        return root;
    // Left Left Case
    if (balance > 1 && value < root->left->freeAfter)
        return rightRotate(root);
 
    // Right Right Case
    if (balance < -1 && value > root->right->freeAfter)
        return leftRotate(root);
 
    // Left Right Case
    if (balance > 1 && value > root->left->freeAfter)
    {
        root->left =  leftRotate(root->left);
        return rightRotate(root);
    }
 
    // Right Left Case
    if (balance < -1 && value < root->right->freeAfter)
    {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }
    
    return root;
}

ll doBinarySearchMaxMins(ll timestamp,ll left,ll right)
{
    if(MaxMins[right].timestamp <= timestamp)
        return right;
    ll mid = (left + right)/2;
    if(MaxMins[mid].timestamp <= timestamp)
    {
        if(MaxMins[mid+1].timestamp <= timestamp)
                return doBinarySearchMaxMins(timestamp,mid+1,right);
        return mid;
    }
    return doBinarySearchMaxMins(timestamp,left,mid-1);
}

ll getStartPoint(ll value,vector<ll> &jobIDVector,ll left,ll right)
{
    if(left == right)
        return left;
    ll mid = (left + right)/2;
    if(jobList[jobIDVector[mid]].timestamp == value)
        return getStartPoint(value,jobIDVector,left,mid);
    if(jobList[jobIDVector[mid]].timestamp < value)
        return getStartPoint(value,jobIDVector,mid+1,right);
    if(mid != left)
        return getStartPoint(value,jobIDVector,left,mid-1);
    return left;
}

ll getEndPoint(ll value,vector<ll> &jobIDVector,ll left,ll right)
{
    if(left == right)
        return left;
    ll mid = (left + right)/2;
    if(jobList[jobIDVector[mid]].timestamp == value)
    {
        if(jobList[jobIDVector[mid+1]].timestamp == value)
             return getEndPoint(value,jobIDVector,mid+1,right);
        else return mid;
    }
    if(jobList[jobIDVector[mid]].timestamp < value)
    {
        if(jobList[jobIDVector[mid+1]].timestamp > value)
            return mid;
        return getEndPoint(value,jobIDVector,mid+1,right);
    }
    return getEndPoint(value,jobIDVector,left,mid-1);
}



void printHistory_TOPK(ll timestamp,ll K)
{
    ll selected = 0;
    int importance = 100;
    ll tsNearbyIndex = doBinarySearchMaxMins(timestamp,0,MaxMins.size()-1);
    ll Min = MaxMins[tsNearbyIndex].Min;
    ll Max = MaxMins[tsNearbyIndex].Max;
    if(Max < Min)
        return;
    
    while(selected < K && importance != 0)
    {
        vector<ll> jobIDVector = qJobList[importance--];
        if(jobIDVector.size() == 0)
                continue;
        
        if(Max < jobList[jobIDVector[0]].timestamp)
            continue;
        if(Min > jobList[jobIDVector[jobIDVector.size()-1]].timestamp)
            continue;
        
         ll left = getStartPoint(Min,jobIDVector,0,jobIDVector.size()-1);
         ll right = getEndPoint(Max,jobIDVector,0,jobIDVector.size()-1);

        for(ll j=left,l=right; j<=l; ++j)
        {
            ll jobID = jobIDVector[j];
            Job job = jobList[jobID];
            if(job.timestamp <= timestamp && (timestamp < job.qExit || job.qExit == 0))
            {
                job.printDescription();
                selected++;
            }
            if(job.timestamp > timestamp || selected == K)
                break;
        }
    }
}

void printHistory_Origin(ll timestamp,string origin)
{
    ll selected = 0;
    int importance = 100;
    ll tsNearbyIndex = doBinarySearchMaxMins(timestamp,0,MaxMins.size()-1);
    ll Min = MaxMins[tsNearbyIndex].Min;
    ll Max = MaxMins[tsNearbyIndex].Max;
    
    if(Max < Min)
        return;
    
    while(importance != 0)
    {
        vector<ll> jobIDVector = qJobList[importance--];
        if(jobIDVector.size() == 0)
                continue;
        
        if(Max < jobList[jobIDVector[0]].timestamp)
            continue;
        if(Min > jobList[jobIDVector[jobIDVector.size()-1]].timestamp)
            continue;
        
        ll left = getStartPoint(Min,jobIDVector,0,jobIDVector.size()-1);
        ll right = getEndPoint(Max,jobIDVector,0,jobIDVector.size()-1);
    
        for(ll j=left,l=right; j<=l; ++j)
        {
            ll jobID = jobIDVector[j];
            Job job = jobList[jobID];
            if(job.timestamp <= timestamp && (timestamp < job.qExit || job.qExit == 0) && job.originSystem == origin)
            {
                job.printDescription();
                selected++;
            }
            if(job.timestamp > timestamp)
                break;
        }
    }
}


int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */   
    char lineInput[5000];
    string operation;
    
    ll noOfCPU;
    ll processID,timestamp,duration,importance;
    string originSystem,instruction;
    ll K;
    
    // Priority Queue Of Jobs Waiting (partitioned on importance value)
    vector<ll> jobHeap[101];
    ll jobHeapSize[101] = {0};
    
    // Create BST To Store CPU Free Times
    CPUNode *cpuTreeRoot;
    
    // Stores timestamps of jobs waiting in queue as a Priority Queue
    CPUNode *startTimeRoot = NULL;
    
    ll ID = 0; //Dummmy Variable acting as ID
    ll prevTimestamp; // last seen timestamp
    
    while(gets(lineInput))
    {
        stringstream ss(lineInput);
        ss>>operation;
        if(operation == "cpus")
        {
            ss>>noOfCPU;
            cpuTreeRoot = createBST(noOfCPU); // Create root node of CPU BST Tree
        }   
        else if(operation == "job")
        {
            ss>>timestamp;
            ss>>processID;
            ss>>originSystem;
            ss>>instruction;
            ss>>importance;
            ss>>duration;
            
            if(timestamp > prevTimestamp && totalSize > 0)
                emptyOutAllDurationHeaps();
            
            prevTimestamp = timestamp;
            
            Job job = Job(ID,processID,timestamp,originSystem,instruction,importance,duration);
            jobList.push_back(job);
            insertJobIntoHeap(ID,jobHeap[importance],jobHeapSize[importance]);
            
            // Duration heap is empty
            if(durationHeapSize[importance] == 0)
            {
                insertIntoDurationHeap(ID,durationHeaps[importance],durationHeapSize[importance]);
                prevTimestamps[importance] = timestamp;
            }
            // Alread some entries are present
            else
            {
                if(prevTimestamps[importance] == timestamp)
                    insertIntoDurationHeap(ID,durationHeaps[importance],durationHeapSize[importance]);
                else
                {
                    prevTimestamps[importance] = timestamp;
                    emptyOutDurationHeap(importance);
                    insertIntoDurationHeap(ID,durationHeaps[importance],durationHeapSize[importance]);
                }
            }
            
            ID++;
            
            startTimeRoot = insertCPUNode(startTimeRoot,timestamp);
            
            if(MaxMins.size() == 0)
                MaxMins.push_back(MaxMinNode(timestamp,timestamp,timestamp));
            else if(MaxMins[MaxMins.size()-1].timestamp == timestamp)
                MaxMins[MaxMins.size()-1].updateValues(timestamp);
             else
            {
                ll Min = getMinimum(startTimeRoot);
                ll Max = getMaximum(startTimeRoot);
                MaxMins.push_back(MaxMinNode(Min,Max,timestamp));
             }
        }
        else if(operation == "assign")
        {
            ss>>timestamp;
            ss>>K;
            
            // No jobs with multiple timestamps less than this will be seen so flush the durationHeaps
            if(totalSize > 0)
                emptyOutAllDurationHeaps();
            
            // Get number of jobs to be assigned
            ll freeCPU = getValuesLessThanEqualTo(cpuTreeRoot,timestamp);
            
            prevTimestamp = timestamp;
            
            
            // Scan from 100 - 1 importance buckets to get the jobs to be assigned
            K = min(freeCPU,K);
            ll selected = 0;
            importance = 100;
            // Keep assigning till desired number is met
            while(selected < K && importance != 0)
            {
                ll choose = K - selected;
                ll newJobsToSchedule = min(choose,jobHeapSize[importance]);
                if(newJobsToSchedule == 0)
                {
                    importance--;
                    continue;
                }
                
                for(ll i=0; i<newJobsToSchedule; ++i)
                {
                    ll nextJobID = extractNextJob(jobHeap[importance],jobHeapSize[importance]);
                    jobList[nextJobID].printDescription();
                    cpuTreeRoot = deleteCPUNode(cpuTreeRoot,timestamp);
                    cpuTreeRoot = insertCPUNode(cpuTreeRoot,(timestamp + jobList[nextJobID].duration));
                    jobList[ nextJobID ].qExit = timestamp;
                    startTimeRoot = deleteCustom(startTimeRoot,jobList[nextJobID].timestamp);
            
                }
                selected += newJobsToSchedule;
                importance--;
            }
            
            
            // Updating the max-min values
            ll Max,Min;
            if(startTimeRoot == NULL)
            {
                Max = 0;
                Min = 1;
            }
            else
            {
                Max = getMaximum(startTimeRoot);        
                Min = getMinimum(startTimeRoot);
            }

            if(MaxMins[MaxMins.size()-1].timestamp == timestamp)
                MaxMins[MaxMins.size()-1] = MaxMinNode(Min,Max,timestamp);
            else MaxMins.push_back(MaxMinNode(Min,Max,timestamp));
            
        }
        else if(operation == "query")
        {
            ss>>timestamp;
            ss>>operation;
            
            // If timestamp is greater than earlier seen timestamp then flush out the durationHeaps
            if(timestamp >= prevTimestamp && totalSize > 0)
                emptyOutAllDurationHeaps();
            
            // Checks if string contains alphabet to distinguish the two query types
            if(!containsAlphabet(operation))
                printHistory_TOPK(timestamp,convertStringToLL(operation));
            else
                printHistory_Origin(timestamp,operation);
        }
    }
    
    return 0;
}
