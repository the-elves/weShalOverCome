#include "dirOp.h"
#include <pthread.h>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <map>
typedef simpleDirent Dir;
void  printCurrentLevel(vector<Dir> currentLevel){
	for(Dir i : currentLevel){
		cout<<i.name<<"  ";
	}
	cout<<endl;
}

//-------------------------------------------------
//class comparator
//------------------------------------------------
//

vector<bool> vThreadGo;
vector<bool> vThreadResponse;
vector<pthread_cond_t *> vWaitTForCommand;
pthread_cond_t waitParentForResponse = PTHREAD_COND_INITIALIZER;
vector<pthread_mutex_t *> vTMutex;
pthread_mutex_t respMutex=PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t vthreadLevelMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vMutexMutex= PTHREAD_MUTEX_INITIALIZER;
vector< vector< vector<Dir> > > vfileSystem;
vector<vector<string> > vcurrentTPath;
vector<vector<Dir> > vthreadLevel;


vector<string> vRoot;
vector<string> vThreadCommand;

class Comparator{
	vector<pthread_t *> vTh;
	vector<bool> vHandleResponseFrom;
	vector<string> majorityChildren;
	vector<vector <Dir> > commonSubtree;
	bool commonSubtreeExists, allEqual;
	public :
		int k;
		static void *thread1Function(void *);
		static void *thread2Function(void *);
		Comparator();
		string formCommandWord(string s, vector<vector<Dir> > fileSystem,vector<string> &currentPath,string root);
		void start();	
		void analyseAction();
		void fireThread1(int);
		vector<Dir>::iterator find(Dir ,vector<Dir>,vector<Dir>::iterator);
		void prepareTrees();
		bool checkBreakCondition();
};

void Comparator::prepareTrees(){
	vector <vector<vector <Dir> > >::iterator it;
	vector<Dir> temp;
	vector<Dir>::iterator fIt;
	int i = 0;
	vector<vector <Dir> > currentFS;
	for(it = vfileSystem.begin(); it!= vfileSystem.end(); it++){
		currentFS = *it;
		temp = *(currentFS.end()-1);
		for(string majName : majorityChildren){
			cout<<"majname = "<<majName;
			for(fIt = temp.begin(); fIt!= temp.end(); fIt++){
				if((*fIt).name == majName){
					cout<<"found";
					iter_swap(temp.begin()+i, fIt);
					i++;
					break;
				}
			}
		}
		*(currentFS.end()-1) = temp;
		*it = currentFS;

	}
	

}

/* gcvector<bool> vThreadGo; */
/* vector<bool> vThreadResponse; */
/* vector<pthread_cond_t> vWaitTForCommand; */
/* pthread_cond_t waitParentForResponse = PTHREAD_COND_INITIALIZER; */
/* vector<pthread_mutex_t> vTMutex; */
/* pthread_mutex_t respMutex=PTHREAD_MUTEX_INITIALIZER; */ 
Comparator::Comparator(){
	k = 3;
	int i;
	for(i=0; i < k; i++){
		vThreadGo.push_back(false);
		vThreadResponse.push_back(false);
		pthread_cond_t *c = new pthread_cond_t();
		pthread_cond_init(c,NULL) ;
		pthread_mutex_t *m = new pthread_mutex_t();
		pthread_mutex_init(m, NULL);
		pthread_t *t = new pthread_t();
		vHandleResponseFrom.push_back(false);
		vWaitTForCommand.push_back(c);
		vTMutex.push_back(m);
		vTh.push_back(t);
		vector<Dir> thempt;
		vthreadLevel.resize(k);
		vfileSystem.resize(k);
		vThreadCommand.resize(k);
		vcurrentTPath.resize(k);
	}
	vRoot.push_back("/home/illuminati/root1");
	vRoot.push_back("/home/illuminati/root2");
	vRoot.push_back("/home/illuminati/root3");
	allEqual = 1;
	commonSubtreeExists = 0;
	start();
	for(i=0;i<k;i++)
		pthread_join(*(vTh[i]),NULL);
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
	map<string,int> occurences;
	for(int i=0; i<k;i++){
		if(vThreadCommand[i][0] == 't' && vHandleResponseFrom[i]){
			/* cout<<"thread one returned"<<endl; */
			cout<<"thread"<<i<<":";
			printCurrentLevel(vthreadLevel[i]);
			if(vthreadLevel[i].size()==0){
				/* cout<<"thread1levelsize = 0\n"; */
				while((*(vfileSystem[i].end()-1)).size() == 1 && vfileSystem[i].size()!= 0){
					vfileSystem[i].erase(vfileSystem[i].end()-1);
				}
				if((*(vfileSystem[i].end()-1)).size() > 1 && vfileSystem[i].size()!=0 ){
					vector<Dir> lastVec = *(vfileSystem[i].end()-1);
					lastVec.erase(lastVec.begin());
					*(vfileSystem[i].end()-1) = lastVec;
				}
			}
			else
				vfileSystem[i].push_back(vthreadLevel[i]);
		}
		for(Dir a : vthreadLevel[i]){
			if(occurences.find(a.name) != occurences.end()){
				occurences[a.name]=occurences[a.name]+1;
			}else{
				occurences[a.name]=1;
			}
		}
	}
	majorityChildren.clear();
	for(map<string,int>::iterator it= occurences.begin(); it!=occurences.end(); it++){
		if(it->second > k/2){
			majorityChildren.push_back(it->first);
		}
		if(it->second != k)
			allEqual = false;
	}
	if(majorityChildren.size()>0)
		commonSubtreeExists=true;

}

void Comparator::fireThread1(int i){
	pthread_mutex_lock(vTMutex[i]);
	vThreadGo[i] = true;
	pthread_cond_signal(vWaitTForCommand[i]);
	pthread_mutex_unlock(vTMutex[i]);
}

bool Comparator::checkBreakCondition(){
	int a = true;
	for(int i=0; i<k;i++){
		a = a && vfileSystem[i].empty();
	}
	return !a;
}

/* void Comparator::fireThread2(){ */
/* 	pthread_mutex_lock(&t2Mutex); */
/* 	thread2Go = 1; */
/* 	pthread_cond_signal(&waitT2ForCommand); */
/* 	pthread_mutex_unlock(&t2Mutex); */
/* } */
void Comparator::start(){
	for(int i = 0; i<k; i++){

		pthread_create(vTh[i],NULL,thread1Function,(void*) &i);
		vThreadResponse[i]=(false);
		usleep(20000);
		/* cout<<"OK!!"<<endl; */
		vector<Dir> rootVec;
		Dir rootDir;
		rootDir.name = vRoot[i];
		rootDir.type = D;
		rootDir.parent = "";
		rootVec.push_back(rootDir);
		vfileSystem[i].push_back(rootVec);
	}
	do{
		for(vector<vector <Dir> > s : vfileSystem){
			for(vector<Dir> a : s)
				printCurrentLevel(a);
			cout<<endl;
		}
		for(int i =0;i<k;i++){
			vHandleResponseFrom[i]=true;
			cout<<"T1command"<<formCommandWord("t",vfileSystem[i],vcurrentTPath[i],vRoot[i])<<endl;
			vThreadCommand[i]=(formCommandWord("t",vfileSystem[i],vcurrentTPath[i],vRoot[i]));
			fireThread1(i);
		}
		bool respAnd = false;
		pthread_mutex_lock(&respMutex);
		do{
			pthread_cond_wait(&waitParentForResponse,&respMutex);
			respAnd = vThreadResponse[0];
			for(bool r : vThreadResponse){
				respAnd = r && respAnd;
			}
			//cout<<endl;
			/* cout<<"r/esp and"<<respAnd<<endl; */
		}while(!respAnd);
		for(int l=0;l<k;l++)
			vThreadResponse[l]=0;
		pthread_mutex_unlock(&respMutex);
		/* cout<<"parwnt active"<<endl; */
		analyseAction();
		/* int i; */
		/* cin>>i; */
	}while(checkBreakCondition());
	for(int i=0;i<k;i++){
		vThreadCommand[i] = "e";
		fireThread1(i);
	}
	if(allEqual)
		cout<<"all the fs are equal"<<endl;
	else if(commonSubtreeExists)
		cout<<"Common subtree exists"<<endl;
	else
		cout<<"Completly Divergent"<<endl;
}

void* Comparator::thread1Function(void *i){
	int id = *((int *)i);
	while(1){
		
		/* cout<<"Thread"<<id<<"lock acquired"<<endl; */
		pthread_mutex_lock(vTMutex[id]);
		while(vThreadGo[id] == 0)
			pthread_cond_wait(vWaitTForCommand[id], vTMutex[id]);
		vThreadGo[id] = 0;
		pthread_mutex_unlock(vTMutex[id]);
		cout<<"thread "<<id<<" command received:"<<vThreadCommand[id]<<endl;		
		if(vThreadCommand[id][0] == 't'){
			pthread_mutex_lock(&vthreadLevelMutex);
			vthreadLevel[id].clear();
			vthreadLevel[id] = traverseDirectoryLevel(vThreadCommand[id].substr(2));
			cout<<"inside thread:";
			printCurrentLevel(vthreadLevel[id]);
			pthread_mutex_unlock(&vthreadLevelMutex);
		}
		
		else if(vThreadCommand[id] == "e"){
			cout<<id<<":Exiting"<<endl;
			pthread_exit(NULL);
			
		}

		/* cout<<"thread "<<id<<" sending response"<<endl; */
		pthread_mutex_lock(&respMutex);
		vThreadResponse[id] = 1;
		pthread_cond_signal(&waitParentForResponse);
		pthread_mutex_unlock(&respMutex);
		/* cout<<"thread "<<id<<" response sent"<<endl; */		
	}
	return NULL;
}
/* void* Comparator::thread2Function(void *){ */
/* 	while(1){ */
/* 		/1* cout<<"Thread2 waiting for command"<<endl; *1/ */
/* 		pthread_mutex_lock(&t2Mutex); */
/* 		while(thread2Go == 0) */
/* 			pthread_cond_wait(&waitT2ForCommand, &t2Mutex); */
/* 		thread2Go = 0; */
		/* pthread_mutex_unlock(&t2Mutex); */
		/* /1* cout<<"thread 2 command received"<<threadTwoCommand<<endl; *1/ */
		/* if(threadTwoCommand[0] == 't') */
		/* 	thread2Level = traverseDirectoryLevel(threadTwoCommand.substr(2)); */
		
			
		/* else if(threadTwoCommand == "e") */
		/* 	pthread_exit(NULL); */
		
		/* /1* cout<<"thread 2 sending response"<<endl; *1/ */
	
	
	
		/* pthread_mutex_lock(&respMutex); */
		/* thread2Response = 1; */
		/* pthread_cond_signal(&waitParentForResponse); */
		/* pthread_mutex_unlock(&respMutex); */
	/* } */
/* } */
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



	/* if(fileSystem1.size()==fileSystem2.size()){ */
			
		/* 	if(fileSystem1.size()!=1) */
		/* 		prepareTrees(); */
			
		/* 	threadOneCommand=formCommandWord("t",fileSystem1,currentT1Path,root1); */
		/* 	/1* cout<<"T1Command:"<<threadOneCommand<<endl; *1/ */
		/* 	handleResponseFrom1=handleResponseFrom2=true; */
		/* 	fireThread1(); */

			
		/* 	threadTwoCommand=formCommandWord("t",fileSystem2,currentT2Path,root2); */
		/* 	/1* cout<<"T2Command:"<<threadTwoCommand<<endl; *1/ */
		/* 	fireThread2(); */

		/* 	pthread_mutex_lock(&respMutex);							//wait */
		/* 	while(thread1Response !=1 || thread2Response != 1)				//analyse */
		/* 		pthread_cond_wait(&waitParentForResponse, &respMutex);			//repeat */
		/* 	thread1Response = 0; */
		/* 	thread2Response = 0; */
		/* 	pthread_mutex_unlock(&respMutex); */
		/* } */

		/* else if(fileSystem2.size()>fileSystem1.size()){ */

		/* 	handleResponseFrom2=true; */
		/* 	handleResponseFrom1=false; */
		/* 	threadTwoCommand=formCommandWord("t",fileSystem2,currentT2Path,root2); */
		/* 	/1* cout<<"T2Command:"<<threadTwoCommand<<endl; *1/ */
		/* 	fireThread2(); */

		/* 	pthread_mutex_lock(&respMutex);							//wait */
		/* 	while(thread2Response !=1 )				//analyse */
		/* 		pthread_cond_wait(&waitParentForResponse, &respMutex);			//repeat */
		/* 	thread2Response = 0; */
		/* 	pthread_mutex_unlock(&respMutex); */
		/* } */
		/* 											//Issue a command */
		
		/* else if(fileSystem1.size()>fileSystem2.size()){ */
		/* 	handleResponseFrom1=true; */
		/* 	handleResponseFrom2=false; */

		/* 	threadOneCommand=formCommandWord("t",fileSystem1,currentT1Path,root1); */
		/* 	/1* cout<<"T1Command:"<<threadOneCommand<<endl; *1/ */
		/* 	fireThread1(); */

		/* 	pthread_mutex_lock(&respMutex);							//wait */
		/* 	while( thread1Response != 1)				//analyse */
		/* 		pthread_cond_wait(&waitParentForResponse, &respMutex);			//repeat */
		/* 	thread1Response = 0; */
		/* 	pthread_mutex_unlock(&respMutex); */

		/* } */	

	/* vector<string>::iterator it1,it2; */
	/* it1=currentT1Path.begin()+1; */
	/* it2=currentT2Path.begin()+1; */
	/* string path1 = ""; */
	/* string path2 = ""; */
	/* /1* bool currentPathNotEqual = false; *1/ */
	/* while(it1!=currentT1Path.end() ){ */
	/* 	path1 = path1+"/" + (*it1); */
	/* 	it1++; */
	/* } */
	/* while(it2!=currentT2Path.end() ){ */
	/* 	path2 = path2+"/" + (*it2); */
	/* 	it2++; */
	/* } */
	/* if(path1 != path2){ */
	/* 	for(Dir a : thread1Level) */
	/* 		changes.push_back(path1 + "/" + a.name + " deleted"); */
	/* 	for(Dir a : thread2Level) */
	/* 		changes.push_back(path2 + "/" + a.name + " added"); */
	/* } */
	/* else{ */
	/* 	for(Dir a : thread1Level){ */
	/* 		if( find(a, thread2Level,thread2Level.end()) == thread2Level.end() ){ */
	/* 			changes.push_back(path1 + "/" + a.name + " deleted"); */
	/* 		} */

	/* 	} */

	/* 	for(Dir a : thread2Level){ */
			
	/* 		if( find(a, thread1Level,thread1Level.end()) == thread1Level.end() ){ */
	/* 			changes.push_back(path2 + "/" + a.name + " added"); */
	/* 		} */
	/* 	} */
	/* } */
