#include "dirOp.h"
#include <pthread.h>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
typedef simpleDirent Dir;
void  printCurrentLevel(vector<Dir> currentLevel){
	for(Dir i : currentLevel){
		cout<<i.name<<endl;
	}
}

//-------------------------------------------------
//class comparator
//------------------------------------------------
//

bool thread1Go = 0, thread2Go = 0,thread1Response = 0, thread2Response = 0;
pthread_cond_t waitT1ForCommand= PTHREAD_COND_INITIALIZER;
pthread_cond_t waitT2ForCommand= PTHREAD_COND_INITIALIZER;
pthread_cond_t waitParentForResponse = PTHREAD_COND_INITIALIZER;
pthread_mutex_t t1Mutex=PTHREAD_MUTEX_INITIALIZER, t2Mutex=PTHREAD_MUTEX_INITIALIZER, respMutex=PTHREAD_MUTEX_INITIALIZER; 
vector< vector<Dir> > fileSystem1, fileSystem2;
vector< vector<Dir> >::iterator fs1It, fs2It;
vector<string> currentPath;
class Comparator{
	vector<string> changes;
	pthread_t th1,th2;
	public :
		static void *thread1Function(void *);
		static void *thread2Function(void *);
		Comparator();
		void start();	
};

string root1 = "/home/illuminati/root1";
string root2 = "/home/illuminati/root2";
string threadOneCommand;
string threadTwoCommand;
vector<string> currentPathT1, currentPathT2;
Comparator::Comparator(){ 
	start();
	pthread_join(th1,NULL);

}
//---------------------------------------------------
//functions
//t = traverse
//h = hash
//
//



void Comparator::start(){
	threadOneCommand = "t " + root1;
	threadTwoCommand = "t " + root2;
	pthread_create(&th1,NULL,thread1Function, NULL);
	pthread_create(&th2,NULL, thread2Function, NULL);
	thread1Response = thread2Response = 0;
	printCurrentLevel(traverseDirectoryLevel(root1));
	printCurrentLevel(traverseDirectoryLevel(root2));
	fileSystem1.push_back(traverseDirectoryLevel(root1));
	fileSystem2.push_back(traverseDirectoryLevel(root2));
	do{
		threadOneCommand = "t " + root1 + "/" +(*(( fileSystem1[0]).end()-1)).name;
		pthread_mutex_lock(&t1Mutex);
		thread1Go = 1;
		pthread_cond_signal(&waitT1ForCommand);
		pthread_mutex_unlock(&t1Mutex);

		threadTwoCommand = "t" + root2 + "/" +(*(( fileSystem2[0]).end()-1)).name;
		pthread_mutex_lock(&t2Mutex);
		thread2Go = 1;
		pthread_cond_signal(&waitT2ForCommand);
		pthread_mutex_unlock(&t2Mutex);

		pthread_mutex_lock(&respMutex);
		pthread_cond_wait(&waitParentForResponse, &respMutex);
		pthread_mutex_unlock(&respMutex);
		
		cout<< "Threads returned"<<endl;
		printCurrentLevel(*(fileSystem1.end()-1));
		printCurrentLevel(*(fileSystem2.end()-1));

	}while(!fileSystem1.empty() && !fileSystem2.empty());
}

void* Comparator::thread1Function(void *){
	while(1){
		cout<<"Thread1 waiting for command"<<endl;
		pthread_mutex_lock(&t1Mutex);
		while(thread1Go == 0)
			pthread_cond_wait(&waitT1ForCommand, &t1Mutex);
		thread1Go = 0;
		pthread_mutex_unlock(&t1Mutex);
		cout<<"thread 1 command received"<<threadOneCommand<<endl;		
		if(threadOneCommand[0] == 't')
			vector<Dir> temp = traverseDirectoryLevel(threadOneCommand.substr(2));
		if(temp.size>0)
		fileSystem1.push_back(temp);
	
		cout<<"thread 1 sending response"<<endl;
		pthread_mutex_lock(&respMutex);
		thread1Response = 1;
		pthread_cond_signal(&waitParentForResponse);
		pthread_mutex_unlock(&respMutex);

	}
	return NULL;
}
void* Comparator::thread2Function(void *){
	while(1){
		cout<<"Thread2 waiting for command"<<endl;
		pthread_mutex_lock(&t2Mutex);
		while(thread2Go == 0)
			pthread_cond_wait(&waitT2ForCommand, &t2Mutex);
		thread2Go = 0;
		pthread_mutex_unlock(&t2Mutex);
		cout<<"thread 2 command received"<<threadTwoCommand<<endl;

		fileSystem2.push_back( traverseDirectoryLevel(threadTwoCommand.substr(2)));
	
	
		cout<<"thread 2 sending response"<<endl;
	
	

	
	
	
		pthread_mutex_lock(&respMutex);
		thread2Response = 1;
		pthread_cond_signal(&waitParentForResponse);
		pthread_mutex_unlock(&respMutex);
	}
}
int main(){
	Comparator c;
	return 0;
}
/* class Traverser{ */

/* 	vector<Dir> currentLevel; */
/* 	vector<string> files; */
/* 	string root; */
/* 	public: */ 
/* 		Traverser(); */
/* 		Traverser(string); */
/* 		void printCurrentLevel(); */
/* 		void traverse(string); */
/* 		void selectFiles(); */
/* }; */

/* void Traverser::selectFiles(){ */
/* 	files.clear(); */
/* 	for(Dir i : currentLevel) */
/* 		if (i.type == F) */
/* 			files.push_back(i.name); */
/* } */

/* void Traverser::traverse(string root){ */
/* 	currentLevel = traverseDirectoryLevel(root); */
/* 	selectFiles(); */
/* } */

/* Traverser::Traverser(){ */
/* 	cout<<"Enter The root:"; */
/* 	cin>>root; */
/* } */

/* Traverser::Traverser(string root){ */
/* 	this->root = root; */	
/* } */

		/* if(thread1Response == 1){ */

		/* 	pthread_mutex_lock(&respMutex); */
		/* 		thread1Response=0; */	
		/* 	pthread_mutex_unlock(&respMutex); */
		/* 	pthread_mutex_lock(&t1Mutex); */
		/* 	thread1Go = 1; */
		/* 	pthread_cond_signal(&waitT1ForCommand); */
		/* 	pthread_mutex_unlock(&t1Mutex); */
		/* } */

		/* if(thread2Response == 1) { */
		/* 	pthread_mutex_lock(&respMutex); */
		/* 	thread2Response=0; */	
		/* 	pthread_mutex_unlock(&respMutex); */
		/* 	pthread_mutex_lock(&t2Mutex); */
		/* 	thread2Go = 1; */
		/* 	pthread_cond_signal(&waitT2ForCommand); */
		/* 	pthread_mutex_unlock(&t2Mutex); */
		/* } */
		/* cout<<"main waiting for response"<<endl; */
		/* pthread_mutex_lock(&respMutex); */
		/* while(thread1Response == 0 && thread2Response == 0) */
		/* 	 pthread_cond_wait(&waitParentForResponse,&respMutex); */
		/*  pthread_mutex_unlock(&respMutex); */
		/* cout<<"THNO:"<<endl; */
		/* cin >> thread1Response>> thread2Response; */
