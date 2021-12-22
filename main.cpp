#include <iostream>


#include <semaphore.h>


#include <ctime>


#include <unistd.h>


#include <queue>


#include <thread>

#include<vector>


using namespace std;


int counter;


int bufferSize;


int numOfThreads;

sem_t semaphore;


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

int main(){

    int n=5;

    sem_init(&semaphore,0,1);

    vector<thread>threads;

    for(int i=1;i<=n;i++){

        threads.push_back( thread(newMessage,i));

    }

    for(auto &thr:threads){

        thr.join();




    }

}