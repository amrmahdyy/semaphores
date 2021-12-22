#include <iostream>


#include <semaphore.h>


#include <ctime>


#include <unistd.h>


#include <queue>


#include <thread>

#include<vector>


using namespace std;


int counter;


queue<int>buffer;


int numOfThreads;

sem_t semaphore;

sem_t full;

sem_t sEmpty;

sem_t sBuffer;


// returns random Number of type int, takes one input as the maximum value

int generateRandomTime(int maxTime){

    srand(time(NULL));

    int randomTime=(rand()%maxTime)+1;

    return randomTime;

}

void newMessage(int tid){

    while(true){

        cout<<"Thread ID: "<<tid<<" Recieved a message"<<endl;

        cout<<""<<endl;

        // check if it is locked or not, if not, it will return -1

        if(sem_trywait(&semaphore)!=0){

            cout<<"Thread ID: "<<tid<<" Waiting to write"<<endl;

        }

        else

            sem_wait(&semaphore);

        counter++; // increment message

        cout<<"Thread ID: "<<tid<<" now adding to counter,current counter value "<<counter<<endl;

        cout<<""<<endl;

        sem_post(&semaphore);

        sleep(generateRandomTime(10)); // set maxTime to 10 seconds

    }




}

void produce(){

    while(true){

        sleep(generateRandomTime(10));

        // if counter is busy

        if(sem_trywait(&semaphore)!=0){

            cout<<"Waiting to read the Counter"<<endl;

            sem_wait(&semaphore);

        }

        int tempCounter=counter;

        // reset the counter to 0

        counter=0;

        sem_post(&semaphore);

        // wait if queue is full, which means empty is 0

        if(sem_trywait(&sEmpty)!=0){

            cout<<"Buffer is Full"<<endl;

            sem_wait(&sEmpty);

        }

        // wait if buffer is busy, used by producer or consumer

        if(sem_trywait(&sBuffer)!=0){

            cout<<"Buffer is Busy"<<endl;

            sem_wait(&sBuffer);

        }

        cout<<"Adding value: "<<tempCounter<<" to the Buffer"<<endl;

        buffer.push(tempCounter);

        // releasing the semaphor buffer

        sem_post(&sBuffer);

        // increment the atomic semaphore full

        sem_post(&full);


    }

}

void consume(){

    while(true){

        sleep(generateRandomTime(12));

        // check if buffer is empty, if empty wait

        if(sem_trywait(&full)!=0){

            cout<<"Buffer is Empty"<<endl;

            sem_wait(&full);

        }

        // if Buffer is busy

        if(sem_trywait(&sBuffer)!=0){

            cout<<"Buffer is busy"<<endl;

            sem_wait(&sBuffer);

        }

        cout<<"mCollector value is: "<<buffer.front()<<endl;

        buffer.pop();

        sem_post(&sBuffer);

        sem_post(&sEmpty);
    }
}
void intializeSemaphores(int bufferSize){
    sem_init(&semaphore,0,1);
    sem_init(&full,0,0);
    sem_init(&sEmpty,0,bufferSize);
    sem_init(&sBuffer,0,1);

}

void createThreads(int n){

    // intialize mmonitor thread

    thread mmonitor(produce);
    // intialize mcollector
    thread mcollector(consume);
    vector<thread>threads;
    for(int i=1;i<=n;i++){
        threads.push_back( thread(newMessage,i));
    }
    mmonitor.join();
    mcollector.join();
    for(auto &thr:threads){
        thr.join();
    }
}

void userInputs(){
    int n,bufferSize;
    cout<<"Enter Number Of threads: ";
    cin>>n;
    cout<<""<<endl;
    cout<<"Enter the size of the buffer: ";
    cin>>bufferSize;
    cout<<""<<endl;
    intializeSemaphores(bufferSize);
    createThreads(n);
}
int main(){
    userInputs();
}