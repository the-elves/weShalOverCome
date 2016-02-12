#include <iostream>
#include <pthread.h>
#include <algorithm>
#include <dirent.h>
#include "sha256.h"
#include <string>
#include <vector>
#include <fstream>
using namespace std;

enum filetype  {F,D};

struct simpleDirent{
	string name,parent;
	filetype type;
	long long int size;
};
typedef simpleDirent fsEntity;
pthread_mutex_t fileMutex;
bool comparisonFunction(struct simpleDirent ent1, struct simpleDirent ent2){
	if(ent1.name.compare(ent2.name)<0)
		return true;
	else
		return false;

}
vector<struct simpleDirent> traverseDirectoryLevel(string dir){
	DIR *ptr_dir;
	ptr_dir = opendir(dir.c_str());
	struct dirent *holder;
	struct simpleDirent temp;
	vector<struct simpleDirent> fileList;
	if(ptr_dir == NULL){
		cout<<"DIR PTR NULL"<<endl;
		return fileList;
	}
	while((holder = readdir(ptr_dir)) != NULL &&( ptr_dir != NULL)){
		if (holder == NULL)
		{
			cout<<"NULL"<<endl;
			break;
		}
		if(holder->d_type == DT_DIR){	
			temp.type = D;
			temp.size = 0;
		}
		else{
			temp.type = F;
			fstream a;
			pthread_mutex_lock(&fileMutex);
			a.open(dir + "/" +holder->d_name, ios::ate|ios::binary);
			temp.size = a.tellg();
			a.close();
			pthread_mutex_unlock(&fileMutex);
		}
		temp.name=holder->d_name;
		temp.parent = dir;
		if(temp.name!="." && temp.name != "..")
			fileList.push_back(temp);
		temp.name = "";
	}
	sort(fileList.begin(),fileList.end(),comparisonFunction);
	return fileList;
}


