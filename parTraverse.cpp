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
vector<string> currentT1Path, currentT2Path;
vector<Dir> thread1Level,thread2Level;
class Comparator{
	vector<string> changes;
	pthread_t th1,th2;
	bool handleResponseFrom1,handleResponseFrom2;
	public :
		static void *thread1Function(void *);
		static void *thread2Function(void *);
		Comparator();
		string formCommandWord(string s, vector<vector<Dir> > fileSystem,vector<string> &currentPath,string root);
		void start();	
		void analyseAction();
		void fireThread1();
		void fireThread2();
		vector<Dir>::iterator find(Dir ,vector<Dir>,vector<Dir>::iterator);
};

string root1 = "/home/illuminati/root1";
string root2 = "/home/illuminati/root2";
string threadOneCommand;
string threadTwoCommand;
Comparator::Comparator(){ 
	handleResponseFrom1=handleResponseFrom2=false;
	start();
	pthread_join(th1,NULL);
	pthread_join(th2,NULL);
}
//---------------------------------------------------
//functions
//t = traverse
//h = hash
//
//

string Comparator::formCommandWord(string s, vector<vector<Dir> > fileSystem,vector<string> &currentPath,string root){
	vector<vector<Dir> >::iterator it = fileSystem.begin();
	currentPath.clear();
	string path = "";
	while(it!=fileSystem.end() ){
		currentPath.push_back((*it)[0].name);
		if((*it)[0].name[0] == '/')
			path=path+(*it)[0].name;
		else
		path=path+"/"+(*it)[0].name;
		it++;
	}
	if(s=="t")
		return "t " + path;

}

vector<Dir>::iterator Comparator::find(Dir a,vector<Dir> v,vector<Dir>::iterator rit){
	vector<Dir>::iterator it=v.begin();
	while(it!=v.end()){
		/* cout<<"find:"<<endl<<(*it).name<<":"<<a.name<<endl; */
		if((*it).name == a.name)
			return it;
		it++;
	}
	return rit;
}


void Comparator::analyseAction(){
	if(threadOneCommand[0] == 't' && handleResponseFrom1){
		cout<<"thread one returned"<<endl;
		printCurrentLevel(thread1Level);
		if(thread1Level.size()==0){
			cout<<"thread1levelsize = 0\n";
			while((*(fileSystem1.end()-1)).size() == 1 && fileSystem1.size()!= 0){
				fileSystem1.erase(fileSystem1.end()-1);
			}
			if((*(fileSystem1.end()-1)).size() > 1 && fileSystem1.size()!=0 ){
				vector<Dir> lastVec = *(fileSystem1.end()-1);
				lastVec.erase(lastVec.begin());
				*(fileSystem1.end()-1) = lastVec;
			}
		}
		else
			fileSystem1.push_back(thread1Level);
	}
	if(threadTwoCommand[0] == 't' && handleResponseFrom2){
		cout<<"thread two returned"<<endl;
		printCurrentLevel(thread2Level);
		if(thread2Level.size()==0){
			while((*(fileSystem2.end()-1)).size() == 1 && fileSystem2.size()!= 0){
				fileSystem2.erase(fileSystem2.end()-1);
			}
			if((*(fileSystem2.end()-1)).size() > 1 && fileSystem2.size()!=0 ){
				vector<Dir> lastVec = *(fileSystem2.end()-1);
				lastVec.erase(lastVec.begin());
				*(fileSystem2.end()-1) = lastVec;
			}
		}else{
			
			fileSystem2.push_back(thread2Level);
		}
	}
	vector<string>::iterator it1,it2;
	it1=currentT1Path.begin()+1;
	it2=currentT2Path.begin()+1;
	string path1 = "";
	string path2 = "";
	/* bool currentPathNotEqual = false; */
	while(it1!=currentT1Path.end() ){
		path1 = path1+"/" + (*it1);
		it1++;
	}
	while(it2!=currentT2Path.end() ){
		path2 = path2+"/" + (*it2);
		it2++;
	}
	if(path1 != path2){
		for(Dir a : thread1Level)
			changes.push_back(path1 + "/" + a.name + " deleted");
		for(Dir a : thread2Level)
			changes.push_back(path2 + "/" + a.name + " added");
	}
	else{
		for(Dir a : thread1Level){
			if( find(a, thread2Level,thread2Level.end()) == thread2Level.end() ){
				changes.push_back(path1 + "/" + a.name + " deleted");
			}

		}

		for(Dir a : thread2Level){
			
			if( find(a, thread1Level,thread1Level.end()) == thread1Level.end() ){
				changes.push_back(path2 + "/" + a.name + " added");
			}
		}
	}
	/* vector<Dir>::iterator th1it= thread1Level.begin(); */
	/* vector<Dir>::iterator th2it= thread2Level.begin(); */
	/* while(th1it!=thread1Level.end() &&  th2it!=thread2Level.end()){ */
	/* 	string tempname1 = path1+ "/" + (*th1it).name; */
	/* 	string tempname2 = path2+ "/" + (*th2it).name; */ 
	/* 	if(tempname1 != tempname2){ */
	/* 		changes.push_back(tempname1 + " deleted"); */
	/* 		changes.push_back(tempname2 + " added"); */
	/* 	} */
	/* 	th1it++; */
	/* 	th2it++; */
	/* } */
	/* while(th1it!=thread1Level.end() ){ */
	/* 	string tempname1 = path1 + "/" + (*th1it).name; */
	/* 	changes.push_back(tempname1 + " added"); */
	/* 	th1it++; */
	/* } */

	/* while(th2it!=thread2Level.end() ){ */
	/* 	string tempname2 = path2 + "/" + (*th2it).name; */
	/* 	changes.push_back(tempname2 + " added"); */
	/* 	th2it++; */
	/* } */


}

void Comparator::fireThread1(){
	pthread_mutex_lock(&t1Mutex);
	thread1Go = 1;
	pthread_cond_signal(&waitT1ForCommand);
	pthread_mutex_unlock(&t1Mutex);
}


void Comparator::fireThread2(){
	pthread_mutex_lock(&t2Mutex);
	thread2Go = 1;
	pthread_cond_signal(&waitT2ForCommand);
	pthread_mutex_unlock(&t2Mutex);
}
void Comparator::start(){
	pthread_create(&th1,NULL,thread1Function, NULL);
	pthread_create(&th2,NULL, thread2Function, NULL);
	thread1Response = thread2Response = 0;
	/* fileSystem1.push_back(traverseDirectoryLevel(root1)); */
	/* fileSystem2.push_back(traverseDirectoryLevel(root2)); */
	/* printCurrentLevel(traverseDirectoryLevel(root1)); */
	/* printCurrentLevel(traverseDirectoryLevel(root2)); */
	vector<Dir> rootVec;
	Dir rootDir;
	rootDir.name = root1;
	rootDir.type = D;
	rootDir.parent = "";
	rootVec.push_back(rootDir);
	fileSystem1.push_back(rootVec);
	
	rootVec.clear();
	rootDir.name = root2;
	rootVec.push_back(rootDir);
	fileSystem2.push_back(rootVec);
	do{
		//string Comparator::formCommandWord(string s, vector<vector<Dir> > fileSystem,vector<string> &currentPath,string root){
		if(fileSystem1.size()==fileSystem2.size()){
			threadOneCommand=formCommandWord("t",fileSystem1,currentT1Path,root1);
			cout<<"T1Command:"<<threadOneCommand<<endl;
			handleResponseFrom1=handleResponseFrom2=true;
			fireThread1();

			
			threadTwoCommand=formCommandWord("t",fileSystem2,currentT2Path,root2);
			cout<<"T2Command:"<<threadTwoCommand<<endl;
			fireThread2();

			pthread_mutex_lock(&respMutex);							//wait
			while(thread1Response !=1 || thread2Response != 1)				//analyse
				pthread_cond_wait(&waitParentForResponse, &respMutex);			//repeat
			thread1Response = 0;
			thread2Response = 0;
			pthread_mutex_unlock(&respMutex);
		}

		else if(fileSystem2.size()>fileSystem1.size()){

			handleResponseFrom2=true;
			handleResponseFrom1=false;
			threadTwoCommand=formCommandWord("t",fileSystem2,currentT2Path,root2);
			cout<<"T2Command:"<<threadTwoCommand<<endl;
			fireThread2();

			pthread_mutex_lock(&respMutex);							//wait
			while(thread2Response !=1 )				//analyse
				pthread_cond_wait(&waitParentForResponse, &respMutex);			//repeat
			thread2Response = 0;
			pthread_mutex_unlock(&respMutex);
		}
													//Issue a command
		
		else if(fileSystem1.size()>fileSystem2.size()){
			handleResponseFrom1=true;
			handleResponseFrom2=false;

			threadOneCommand=formCommandWord("t",fileSystem1,currentT1Path,root1);
			cout<<"T1Command:"<<threadOneCommand<<endl;
			fireThread1();

			pthread_mutex_lock(&respMutex);							//wait
			while( thread1Response != 1)				//analyse
				pthread_cond_wait(&waitParentForResponse, &respMutex);			//repeat
			thread1Response = 0;
			pthread_mutex_unlock(&respMutex);

		}	
		analyseAction();
		/* int i; */
		/* cin>>i; */
	}while(!fileSystem1.empty() && !fileSystem2.empty());
	threadOneCommand = "e";
	threadTwoCommand = "e";
	
		pthread_mutex_lock(&t1Mutex);
		thread1Go = 1;
		pthread_cond_signal(&waitT1ForCommand);
		pthread_mutex_unlock(&t1Mutex);
		
		pthread_mutex_lock(&t2Mutex);
		thread2Go = 1;
		pthread_cond_signal(&waitT2ForCommand);
		pthread_mutex_unlock(&t2Mutex);

		cout<<"ALL CHANGES-----------------------------"<<endl;
		for(string i : changes)
			cout<<i<<endl;
}

void* Comparator::thread1Function(void *){
	while(1){
		/* cout<<"Thread1 waiting for command"<<endl; */
		pthread_mutex_lock(&t1Mutex);
		while(thread1Go == 0)
			pthread_cond_wait(&waitT1ForCommand, &t1Mutex);
		thread1Go = 0;
		pthread_mutex_unlock(&t1Mutex);
		/* cout<<"thread 1 command received"<<threadOneCommand<<endl; */		
		if(threadOneCommand[0] == 't'){
			thread1Level = traverseDirectoryLevel(threadOneCommand.substr(2));
		}
		
		else if(threadOneCommand == "e")
			pthread_exit(NULL);

		/* cout<<"thread 1 sending response"<<endl; */
		pthread_mutex_lock(&respMutex);
		thread1Response = 1;
		pthread_cond_signal(&waitParentForResponse);
		pthread_mutex_unlock(&respMutex);
		
	}
	return NULL;
}
void* Comparator::thread2Function(void *){
	while(1){
		/* cout<<"Thread2 waiting for command"<<endl; */
		pthread_mutex_lock(&t2Mutex);
		while(thread2Go == 0)
			pthread_cond_wait(&waitT2ForCommand, &t2Mutex);
		thread2Go = 0;
		pthread_mutex_unlock(&t2Mutex);
		/* cout<<"thread 2 command received"<<threadTwoCommand<<endl; */
		if(threadTwoCommand[0] == 't')
			thread2Level = traverseDirectoryLevel(threadTwoCommand.substr(2));
		
			
		else if(threadTwoCommand == "e")
			pthread_exit(NULL);
		
		/* cout<<"thread 2 sending response"<<endl; */
	
	
	
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
			



			/* if((*(fileSystem2.end()-1)).size() == 1){ */
			/* 	fileSystem2.erase(fileSystem2.end()-1); */
			/* 	vector<Dir> lastVec = *(fileSystem2.end()-1); */
			/* 	lastVec.erase(lastVec.begin()); */
			/* 	*(fileSystem2.end()-1) = lastVec; */
			
			/* }else{ */
			/* 	vector<Dir> lastVec = *(fileSystem2.end()-1); */
			/* 	lastVec.erase(lastVec.begin()); */
				/* *(fileSystem2.end()-1) = lastVec; */

			/* } */
