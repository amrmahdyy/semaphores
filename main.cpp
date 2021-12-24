#include <iostream>
#include <semaphore.h>
#include <ctime>
#include <unistd.h>
#include <queue>
#include <thread>
#include<vector>
#include <fstream>
#include <chrono>
#include <ctime>
#include<sstream>
using namespace std;

#define RESET   "\033[0m"
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define CYAN    "\033[36m"      /* Cyan */
#define RED     "\033[31m"      /* Red */

int counter;

queue<int>buffer;
queue<string>loggingBuffer;

int numOfThreads;
sem_t semaphore;
sem_t full;
sem_t sEmpty;
sem_t sBuffer;
// logging buffer
sem_t logFull;
//sem_t logEmpty;
sem_t sLogBuffer;

void writeToFile(string message){
    ofstream logFile("loggingMessages",std::ios_base::app);
    logFile<<message<<"\n";
    logFile.close();
}
void logConsumer(){
    while(true){
        // sleeping for short amount of time
        sleep(1.5);

        if(sem_trywait(&sLogBuffer)!=0){
            cout<<YELLOW<<"LOGGING BUFFER IS BUSY"<<RESET<<endl;
            sem_wait(&sLogBuffer);
        }
// check if logging queue is empty
        if(sem_trywait(&logFull)!=0){
            cout<<YELLOW<<"LOGGING BUFFER IS EMPTY"<<RESET<<endl;
            sem_wait(&logFull);
        }
        string message=loggingBuffer.front();
// appending to logFile

        cout<<YELLOW<<"APPENDING TO LOGGING FILE"<<RESET<<endl;
        writeToFile(message);
// removing the last inserted message from loggingBuffer
        loggingBuffer.pop();
        sem_post(&sLogBuffer);
// increase the number of empty slots in logging buffer
//sem_post(&logEmpty);

    }
}
// returns  random Number of type int, takes one input as the maximum value
int generateRandomTime(int maxTime){
    srand(time(NULL));
    int randomTime=(rand()%maxTime)+1;
    return randomTime;
}
void logProducer(string message){
    if(sem_trywait(&sLogBuffer)!=0){
        cout<<YELLOW<<"LOGGING BUFFER IS BUSY"<<RESET<<endl;
        sem_wait(&sLogBuffer);
    }
    cout<<YELLOW<<"ADDING TO LOGGING BUFFER"<<RESET<<endl;
    loggingBuffer.push(message);
    sem_post(&sLogBuffer);
    sem_post(&logFull);
}
void newMessage(int tid,int counterMaxTime){
    while(true){
        string message="Counter Thread ID: "+to_string(tid)+" Recieved a message";
        cout<<message<<endl;
        logProducer(message);
        cout<<""<<endl;
        // check if it is locked or not, if not, it will return -1
        if(sem_trywait(&semaphore)!=0){
            message="Counter Thread ID: "+to_string(tid)+" Waiting to write";
            cout<<message<<endl;
            logProducer(message);
            sem_wait(&semaphore);
        }
        counter++; // increment message
        message="Counter Thread ID: "+to_string(tid)+" now adding to counter,current counter value "+to_string(counter);
        cout<<message<<endl;
        logProducer(message);
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
            string message="Monitor Thread: Waiting to read  the Counter";
            cout<<message<<endl;
            logProducer(message);
            cout<<""<<endl;
            sem_wait(&semaphore);
        }
        int tempCounter=counter;
        // reset the counter to 0
        counter=0;
        sem_post(&semaphore);
        // wait if queue is full, which means empty is 0
        if(sem_trywait(&sEmpty)!=0){
            string message="Monitor Thread: Buffer is Full";
            cout<<message<<endl;
            logProducer(message);
            cout<<""<<endl;
            sem_wait(&sEmpty);
        }
        //  wait if buffer is busy, used by producer or consumer
        if(sem_trywait(&sBuffer)!=0){
            string message="Monitor Thread: Buffer is Busy";
            cout<<message<<endl;
            logProducer(message);
            cout<<""<<endl;
            sem_wait(&sBuffer);
        }
        string message="Monitor Thread: Adding value: "+to_string(tempCounter)+" to the Buffer";
        cout<<message<<endl;
        logProducer(message);
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
            string message="Collector Thread: Buffer is Empty";
            cout<<message<<endl;
            logProducer(message);
            cout<<""<<endl;
            sem_wait(&full);
        }
        // if Buffer is busy
        if(sem_trywait(&sBuffer)!=0){
            string message="Collector Thread: Buffer is busy";
            cout<<message<<endl;
            logProducer(message);
            cout<<""<<endl;
            sem_wait(&sBuffer);
        }
        string message="Collector Thread: Reading counter value: "+to_string(buffer.front());
        cout<<message<<endl;
        logProducer(message);
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
    // intialize logging semaphore for buffer to binary semaphore
    sem_init(&logFull,0,0);
    // setting logging queue size to very large number
    //sem_init(&logEmpty,0,1000);
    sem_init(&sLogBuffer,0,1);
}
// intializing threads
void createThreads(int n=5,int counterMaxTime=10,int monitorMaxTime=20,int collectorMaxTime=20){
    // intialize mmonitor thread
    thread mmonitor(produce,monitorMaxTime);
    // intialize mcollector
    thread mcollector(consume,collectorMaxTime);
    //intialize thread for loggingBuffer
    thread logging(logConsumer);
    vector<thread>threads;
    for(int i=1;i<=n;i++){
        threads.push_back( thread(newMessage,i,counterMaxTime));
    }

    mmonitor.join();
    mcollector.join();
    logging.join();
    for(auto &thr:threads){
        thr.join();
    }
}
void programCatalog(int n,int bufferSize,int counterMaxTime,int monitorMaxTime,int collectorMaxTime){
    string message="Number of thread(s): "+to_string(n);
    writeToFile(message);
    message="Size the of the buffer: "+to_string(bufferSize);
    writeToFile(message);
    message="maximum sleeping time for Counter thread in seconds: "+to_string(counterMaxTime);
    writeToFile(message);
    message="maximum sleeping time for Monitor thread in seconds: "+to_string(monitorMaxTime);
    writeToFile(message);
    message="maximum sleeping time for Collector thread in seconds: "+to_string(collectorMaxTime);
    writeToFile(message);
    writeToFile("");
    writeToFile("");

}
void automaticMode(){
    programCatalog(5,20,10,20,20);
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
    programCatalog(n,bufferSize,counterMaxTime,monitorMaxTIme,counterMaxTime);
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
// return current date in string type
string getCurrentDate(){
    auto start = std::chrono::system_clock::now();
    std::time_t startTime = std::chrono::system_clock::to_time_t(start);
    std::stringstream dateStr;
    dateStr<<std::ctime(&startTime);
    return dateStr.str();
}
int main(){
    int userChoice=menu();
    if(userChoice<1 || userChoice>2){
        cout<<"Incorrect choice";
        return 0;
    }
    writeToFile("**************");
    writeToFile("USER MODE: "+to_string(userChoice));
    writeToFile("CURRENT DATE: "+getCurrentDate());

    if(userChoice==1)automaticMode();
    else if(userChoice==2)manualMode();
}