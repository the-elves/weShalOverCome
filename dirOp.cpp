#include <iostream>
#include <pthread.h>
#include <algorithm>
#include <dirent.h>
#include "sha256.h"
#include <string>
#include <vector>
using namespace std;

enum filetype  {F,D};
struct simpleDirent{
	string name;
	filetype type;
};

vector<struct simpleDirent> traverseDirectoryLevel(string dir){
	DIR *ptr_dir;
	cout<<dir.c_str()<<endl;
	ptr_dir = opendir(dir.c_str());
	struct dirent *holder;
	struct simpleDirent temp;
	vector<struct simpleDirent> fileList;
	//if(ptr_dir == NULL){
	//	cout<<"DIR PTR NULL"<<endl;
	//	return fileList;
	//}
	while((holder = readdir(ptr_dir)) != NULL &&( ptr_dir != NULL)){
		if (holder == NULL)
		{
			cout<<"NULL"<<endl;
			break;
		}
		if(holder->d_type == DT_DIR)	
			temp.type = D;
		else
			temp.type = F;

		temp.name=holder->d_name;
		fileList.push_back(temp);
		temp.name = "";
	}
	return fileList;
}
bool comparisonFunction(struct simpleDirent ent1, struct simpleDirent ent2){
	if(ent1.name.compare(ent2.name)<0)
		return true;
	else
		return false;

}
int main(int argc, char *argv[]){
	vector<struct simpleDirent> a = traverseDirectoryLevel("/home/hduser");
	sort(a.begin(),a.end(), comparisonFunction);
	for(simpleDirent i : a){
		cout<<(i).name<<endl;
	}
	return 0;
}


