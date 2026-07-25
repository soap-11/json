/// JSON Parser === David Morozan 2026
/// This file is released into the public domain. See the LICENSE file for details.

/// Instructions:
/// Call json::parse(string), then read the data from json::tree.
/// json::debug() shows the whole json tree. Use this function as a reference.
/// Trailing commas is allowed, "\uxxxx" is not supported, std::stod is used for numbers.

/// The following errors can be returned:
/// Error  1: incomplete json
/// Error  2: invalid value
/// Error  3: ']' instead of '}' or vice versa
/// Error 1x: required character missing
/// Error 23: number error
/// Error 24: string error

#include <vector>
#include <string>
#include <stack>
#include <map>
namespace json{
using namespace std;

struct value{
	int type;
	union{
		double d;
		int i;
	};
};
vector<string> strings;
vector<vector<value>> vectors;
vector<map<string, value>> maps;

map<string, value> tree;

void element(value x){
	if(x.type==0) cout<<"null\n";
	if(x.type==1) cout<<"true\n";
	if(x.type==2) cout<<"false\n";
	if(x.type==3) cout<<x.d<<'\n';
	if(x.type==4) cout<<'"'<<strings[x.i]<<"\"\n";
	if(x.type==5) cout<<"[]\n";
	if(x.type==6) cout<<"{}\n";
}


//------------------------------------------------------------------------------


void indent(int x){for(int i=0; i<x; i++) cout<<"  ";}

void debmap(map<string, value>, int);

void debvec(vector<value> vec, int ind){
	for(unsigned int i=0; i<vec.size(); i++){
		indent(ind);
		cout<<i<<':';
		if(vec[i].type<=4)
			element(vec[i]);
		if(vec[i].type==5){
			cout<<"[\n";
			debvec(vectors[vec[i].i], ind+1);
			indent(ind);
			cout<<"]\n";
		}
		if(vec[i].type==6){
			cout<<"{\n";
			debmap(maps[vec[i].i], ind+1);
			indent(ind);
			cout<<"}\n";
		}
	}
}

void debmap(map<string, value> tree, int ind){
	for(auto x:tree){
		indent(ind);
		cout<<x.first<<':';
		if(x.second.type<=4)
			element(x.second);
		if(x.second.type==5){
			cout<<"[\n";
			debvec(vectors[x.second.i], ind+1);
			indent(ind);
			cout<<"]\n";
		}
		if(x.second.type==6){
			cout<<"{\n";
			debmap(maps[x.second.i], ind+1);
			indent(ind);
			cout<<"}\n";
		}
	}
}

void debug(){debmap(tree,0);}


//------------------------------------------------------------------------------


__attribute__ ((warn_unused_result)) int getstr(string s, int &i, string &str){
	str="";
	for(i++; s[i]!='"'; i++){
		if(!s[i] || iscntrl(s[i])) return 1;
		if(s[i]!='\\'){
			str+=s[i];
			continue;
		}
		i++;
		if(!s[i] || iscntrl(s[i])) return 1;
		switch(s[i]){
		case 'b':
			str+='\b';
			break;
		case 'f':
			str+='\f';
			break;
		case 'n':
			str+='\n';
			break;
		case 'r':
			str+='\r';
			break;
		case 't':
			str+='\t';
			break;
		case 'u':
		default:
			str+=s[i];
		}
	}
	return 0;
}

void tree_insert(int node, string name, value val){
	if(node==0)
		tree[name]=val;
	if(node>0)
		maps[node][name]=val;
	if(node<0)
		vectors[-node].push_back(val);
}



__attribute__ ((warn_unused_result)) int parse(std::string s){
	//Modes: 9=init,  0='"',  1=':',  2=val/obj(->0),  3=','/'}'
	int i, mode=9, err;
	string name;
	stack<int> st;
	
	strings.clear();
	vectors.clear();
	vectors.emplace_back();
	maps.clear();
	maps.emplace_back();
	
	for(i=0; s[i]; i++){
		if(s[i]=='\x09' || s[i]=='\x0a' || s[i]=='\x0d' || s[i]=='\x20')
			continue;
		//cout<<mode<<' '<<s[i]<<'\n';
		if((mode==0 || mode==3) && (s[i]==']' || s[i]=='}')){
			if(st.top()<0 && s[i]=='}')
				return 3;
			if(st.top()>=0 && s[i]==']')
				return 3;
			st.pop();
			if(st.empty())
				return 0;
			mode=3;
			continue;
		}
		
		switch(mode){
		case 9:
			if(s[i]!='{')
				return 19;
			st.push(0);
			mode=0;
			break;
		
		case 0:
			if(s[i]!='"')
				return 10;
			err=getstr(s,i,name);
			if(err) return 24;
			//cout<<name<<'\n';
			mode=1;
			break;
		
		case 1:
			if(s[i]!=':')
				return 11;
			mode=2;
			break;
		case 3:
			if(s[i]!=',')
				return 13;
			if(st.top()>=0)
				mode=0;
			else mode=2;
			break;
		
		case 2:
			value val;
			mode=3;
			if(s.substr(i,4)=="null"){
				val.type=0;
				tree[name]=val;
				tree_insert(st.top(), name, val);
				i+=3;
				break;
			}
			if(s.substr(i,4)=="true"){
				val.type=1;
				tree_insert(st.top(), name, val);
				i+=3;
				break;
			}
			if(s.substr(i,5)=="false"){
				val.type=2;
				tree_insert(st.top(), name, val);
				i+=4;
				break;
			}
			
			if(isdigit(s[i]) || s[i]=='+' || s[i]=='-'){
				size_t pos;
				val.type=3;
				try{val.d=stod(s.data()+i, &pos);
				}catch(...){return 23;}
				tree_insert(st.top(), name, val);
				i+=pos-1;
				break;
			}
			if(s[i]=='"'){
				val.type=4;
				val.i=strings.size();
				strings.emplace_back();
				err=getstr(s,i,strings.back());
				if(err) return 24;
				tree_insert(st.top(), name, val);
				break;
			}
			
			if(s[i]=='['){
				val.type=5;
				val.i=vectors.size();
				tree_insert(st.top(), name, val);
				st.push(-vectors.size());
				vectors.emplace_back();
				mode=2;
				break;
			}
			if(s[i]=='{'){
				val.type=6;
				val.i=maps.size();
				tree_insert(st.top(), name, val);
				st.push(maps.size());
				maps.emplace_back();
				mode=0;
				break;
			}
			return 2;
		}
	}
	return 1;
}
}