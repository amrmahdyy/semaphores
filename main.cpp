#include <iostream>
#include <semaphore.h>
#include <ctime>
#include <unistd.h>
#include <queue>
#include <thread>
#include<vector>
using namespace std;

#define RESET   "\033[0m"
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define CYAN    "\033[36m"      /* Cyan */

int counter;

queue<int>buffer;

int numOfThreads;
sem_t semaphore;
sem_t full;
sem_t sEmpty;
sem_t sBuffer;

// returns  random Number of type int, takes one input as the maximum value
int generateRandomTime(int maxTime){
    srand(time(NULL));
    int randomTime=(rand()%maxTime)+1;
    return randomTime;
}
void newMessage(int tid,int counterMaxTime){
    while(true){
        cout<<"Counter Thread ID: "<<tid<<" Recieved a message"<<endl;
        cout<<""<<endl;
        // check if it is locked or not, if not, it will return -1
        if(sem_trywait(&semaphore)!=0){
            cout<<"Counter Thread ID: "<<tid<<" Waiting to write"<<endl;
            sem_wait(&semaphore);
        }
        counter++; // increment message
        cout<<"Counter Thread ID: "<<tid<<" now adding to counter,current counter value "<<counter<<endl;
        cout<<""<<endl;
        sem_post(&semaphore);
        sleep(generateRandomTime(counterMaxTime));
    }

}
void produce(int monitorMaxSleepingTime){
    while(true){
        sleep(generateRandomTime(monitorMaxSleepingTime));
        // if counter is busy
        if(sem_trywait(&semaphore)!=0){
            cout<<"Monitor Thread: Waiting to read  the Counter"<<endl;
            cout<<""<<endl;
            sem_wait(&semaphore);
        }
        int tempCounter=counter;
        // reset the counter to 0
        counter=0;
        sem_post(&semaphore);
        // wait if queue is full, which means empty is 0
        if(sem_trywait(&sEmpty)!=0){
            cout<<"Monitor Thread: Buffer is Full"<<endl;
            cout<<""<<endl;
            sem_wait(&sEmpty);
        }
        //  wait if buffer is busy, used by producer or consumer
        if(sem_trywait(&sBuffer)!=0){
            cout<<"Monitor Thread: Buffer is Busy"<<endl;
            cout<<""<<endl;
            sem_wait(&sBuffer);
        }
        cout<<"Monitor Thread: Adding value: "<<tempCounter<<" to the Buffer"<<endl;
        cout<<""<<endl;
        buffer.push(tempCounter);
        // releasing the semaphor buffer
        sem_post(&sBuffer);
        // increment the atomic semaphore full
        sem_post(&full);

    }
}
void consume(int collectorMaxSleepTime){
    while(true){
        sleep(generateRandomTime(collectorMaxSleepTime));
        // check if buffer is  empty, if empty wait
        if(sem_trywait(&full)!=0){
            cout<<"Collector Thread: Buffer is Empty"<<endl;
            cout<<""<<endl;
            sem_wait(&full);
        }
        // if Buffer is busy
        if(sem_trywait(&sBuffer)!=0){
            cout<<"Collector Thread: Buffer is busy"<<endl;
            cout<<""<<endl;
            sem_wait(&sBuffer);
        }
        cout<<"Collector Thread: Reading counter value: "<<buffer.front()<<endl;
        cout<<""<<endl;
        buffer.pop();
        sem_post(&sBuffer);
        sem_post(&sEmpty);

    }

}
void intializeSemaphores(int bufferSize=20){
    sem_init(&semaphore,0,1);
    sem_init(&full,0,0);
    sem_init(&sEmpty,0,bufferSize);
    sem_init(&sBuffer,0,1);
}
// intializing threads
void createThreads(int n=5,int counterMaxTime=10,int monitorMaxTime=20,int collectorMaxTime=20){
    // intialize mmonitor thread
    thread mmonitor(produce,monitorMaxTime);
    // intialize mcollector
    thread mcollector(consume,collectorMaxTime);
    vector<thread>threads;
    for(int i=1;i<=n;i++){
        threads.push_back( thread(newMessage,i,counterMaxTime));
    }

    mmonitor.join();
    mcollector.join();
    for(auto &thr:threads){
        thr.join();
    }
}
void automaticMode(){
    intializeSemaphores();
    createThreads();
}
void manualMode(){
    int n,bufferSize,counterMaxTime,monitorMaxTIme,collectorMaxTime;
    cout<<BLUE<<"Enter Number Of threads: "<<GREEN;
    cin>>n;
    cout<<""<<endl;
    cout<<BLUE<<"Enter the size of the buffer: "<<GREEN;
    cin>>bufferSize;
    cout<<""<<RESET<<endl;
    cout<<BLUE<<"Enter maximum sleeping time for Counter thread in seconds: "<<GREEN;
    cin>>counterMaxTime;
    cout<<""<<RESET<<endl;
    cout<<BLUE<<"Enter maximum sleeping time for Monitor thread in seconds: "<<GREEN;
    cin>>monitorMaxTIme;
    cout<<""<<RESET<<endl;
    cout<<BLUE<<"Enter maximum sleeping time for Collector thread in seconds: "<<GREEN;
    cin>>collectorMaxTime;
    cout<<""<<RESET<<endl;
    intializeSemaphores(bufferSize);
    createThreads(n,counterMaxTime,monitorMaxTIme,collectorMaxTime);
}
int menu(){
    int choice;
    cout<<GREEN"Modes:"<<endl;
    cout<<CYAN<<"  1-Automatic mode"<<endl;
    cout<<YELLOW<<"   (Num of threads:5, buffer size:20,counter max time:10, monitor and collector max time:20 )"<<endl;
    cout<<CYAN<<"  2-Manual mode"<<endl;
    cout<<BLUE<<"Enter choice: "<<GREEN;
    cin>>choice;
    cout<<RESET;
    return choice;
}
int main(){
    int userChoice=menu();
    if(userChoice<1 || userChoice>2){
        cout<<"Incorrect choice";
        return 0;
    }
    if(userChoice==1)automaticMode();
    else if(userChoice==2)manualMode();
}