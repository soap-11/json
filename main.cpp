#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include "json.cpp"

int main(){
	using namespace std;
	ifstream file("file.json");
	string s;
	int err;
	char chr;
	
	while(file.get(chr))
		s+=chr;
	err=json::parse(s);
	if(err){
		cout<<"JSON PARSE ERROR "<<err<<'\n';
		return 1;
	}
	cout<<"\nSUCCESS. DEBUG RESULT BELOW:\n\n";
	json::debug();
	return 0;
}