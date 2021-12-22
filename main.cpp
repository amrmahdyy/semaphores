
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

        cout<<"Produces"<<endl;

        // wait if queue is full, which means empty is 0

        sem_wait(&sEmpty);

        sem_wait(&semaphore);

        buffer.push(counter);

        // reset the counter to 0

        counter=0;

        sem_post(&semaphore);

        // increment the atomic semaphore full

        sem_post(&full);

        sleep(generateRandomTime(10));

    }

}

void consume(){

    while(true){

        // check if buffer is not empty, if empty wait

        sem_wait(&full);

        sem_wait(&semaphore);

        cout<<"mCollector value is: "<<buffer.front()<<endl;

        buffer.pop();

        sem_post(&semaphore);

        sem_post(&sEmpty);

        sleep(generateRandomTime(12));

    }


}

int main(){

    // number of threads

    int n=5;

    // buffer size

    int bufferSize=10;

    sem_init(&semaphore,0,1);

    sem_init(&full,0,0);

    sem_init(&sEmpty,0,bufferSize);

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




    // intialize new semaphore


}