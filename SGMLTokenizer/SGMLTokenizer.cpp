#include <fstream>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

static void loadFile(const string &fileName, vector<string> &lines)
{
	lines.clear();
	ifstream ifstr(fileName);
	string line;
	while (std::getline(ifstr, line)) {  // the default delimiter is the CRLF or LF character
		lines.push_back(line);             // and it will NOT be included
	}
	ifstr.close();
}

void refine(char str[], int &NERmark, vector<int>& status, vector<string>& tokens, vector<int>& NERnum)
{
	string line(str);
	if ("<TEXT>" == line) { return; }
	if ("<p>" == line) { return; }
	if (NULL == str) { return; }
	size_t inp;
	vector<string> openstring = {"<ENAMEX","<TIMEX","<NUMEX","STATUS=\"OPT\""};
	vector<string> closestring = { "</ENAMEX>","</TIMEX>","</NUMEX>" };
	vector<string> typestring = { "TYPE=\"ORGANIZATION\"","TYPE=\"PERSON\"",
								  "TYPE=\"LOCATION\"","TYPE=\"DATE\"","TYPE=\"TIME\"",
								  "TYPE=\"MONEY\"","TYPE=\"PERCENT\"",
								  "TYPE=\"ORGANIZATION|LOCATION\"","TYPE=\"DATE|TIME\""};
	for (string s:openstring)
	{
		inp = line.find(s);
		if (inp != string::npos)
		{
			line.erase(inp, s.length());
		}
	}

	for (int i = 0; i < typestring.size(); i++)
	{
		inp = line.find(typestring[i]);
		if (inp != string::npos)
		{
			line.erase(inp, typestring[i].length());
			status.push_back(i + 1);
			//cout << "push in " << i+1;
		}
	}

	for (string s : closestring)
	{
		inp = line.find(s);
		if (inp != string::npos)
		{
			if (inp != 0){
				if (status.size() != 0){
					NERmark = status[status.size() - 1];
				}
				//cout << line.substr(0,inp) << "\t\t"<<NERmark<<"\n";
				tokens.push_back(line.substr(0,inp));
				NERnum.push_back(NERmark);
				line.erase(0,inp+s.length());
			}
			else{
				line.erase(inp, s.length());
			}
			if (status.size() != 0){
				//cout << "pop out " << status[status.size()-1];
				status.pop_back();
			}
		}
	}
	for (string s : closestring)
	{
		inp = line.find(s);
		if (inp != string::npos)
		{
			//line.erase(inp, s.length());
			if (inp != 0){
				if (status.size() != 0){
					NERmark = status[status.size() - 1];
				}
				//cout << line.substr(0,inp) << "\t\t"<<NERmark<<"\n";
				tokens.push_back(line.substr(0, inp));
				NERnum.push_back(NERmark);
				line.erase(0,inp+s.length());
			}
			else{
				line.erase(inp, s.length());
			}
			if (status.size() != 0){
				//cout << "pop out " << status[status.size()-1];
				status.pop_back();
			}
		}
	}

	if ("" != line){
		if (status.size() != 0){
			NERmark = status[status.size() - 1];
		}
		else
		{
			NERmark = 0;
		}
		//cout << line << "\t\t"<<NERmark<<"\n";
		tokens.push_back(line);
		NERnum.push_back(NERmark);
	}

}

void tokenize(char str[], int &NERmark, vector<int> &status, vector<string> &tokens, vector<int> &NERnum)
{
	string line(str);
	if ("<TEXT>" == line) { return; }
	if (NULL == str) { return;  }
	char* pch = strtok(str, " ");
	int i = 0;
	while (pch != NULL)
	{
		//cout << refine(pch, NERmark) << "\n";
		refine(pch, NERmark, status, tokens, NERnum);
		pch = strtok(NULL, " ");
	}
	return;
}

int tokentype(int x)
{
	if ((x>47)&&(x<58)) { return 1; }  //number
	if ((x>64)&&(x<91)) { return 2; }  //big letter
	if ((x>96)&&(x<123)) { return 3; } //small letter
	return 0; //mark
}

string NER(int x)
{
	if (x==0) { return "O"; }
	if (x==1) { return "ORG"; }
	if (x==2) { return "PER"; }
	if (x==3) { return "LOC"; }
	if (x==4) { return "DAT"; }
	if (x==5) { return "TIM"; }
	if (x==6) { return "MON"; }
	if (x==7) { return "PCT"; }
	if (x==8) { return "ORG|LOC"; }
	if (x==9) { return "DAT|TIM"; }
	return "";
}

void endofsentence(vector<string>& tokens, vector<int>& NERnum, vector<string>& NERtag, vector<string>& finaltokens,int i,string endtoken)
{
	int endsentence = 0;
	if (endtoken.size()==1)
	{
		if ((endtoken!=".")&&(endtoken!="!")&&(endtoken!="?")){
			finaltokens.push_back(endtoken);
			NERtag.push_back(NER(NERnum[i]));
			//cout << endtoken << "\t\t\t" << NER(NERnum[i]) << "\n";
		}
		else{
			finaltokens.push_back(endtoken);
			NERtag.push_back(NER(NERnum[i]));
			//cout << endtoken << "\t\t\t" << NER(NERnum[i]) << "\n";
			if (i==tokens.size()-1){
				endsentence = 1;
			}
			else{
				string nexttoken = tokens[i+1];
				if ((NERnum[i]!=NERnum[i+1])&&(tokentype((int)nexttoken[0])==2)){
					endsentence = 1;
				}
				if (NERnum[i]==0)
				{
					endsentence = 1;
				}
			}
		}
	}
	else
	{
		if (((int)endtoken[0]!=46)&&((int)endtoken[0]!=33)&&((int)endtoken[0]!=63)){
			if ((int)endtoken[0]==44)
			{
				finaltokens.push_back(endtoken.substr(0,1));
				NERtag.push_back(NER(NERnum[i]));
				//cout << endtoken.substr(0,1) << "\t\t\t" << NER(NERnum[i]) << "\n";
				endtoken.erase(0,1);
				finaltokens.push_back(endtoken);
				NERtag.push_back(NER(NERnum[i]));
				//cout << endtoken << "\t\t\t" << NER(NERnum[i]) << "\n";
			}
			else {
				finaltokens.push_back(endtoken);
				NERtag.push_back(NER(NERnum[i]));
				//cout << endtoken << "\t\t\t" << NER(NERnum[i]) << "\n";
			}
		}
		else{
			finaltokens.push_back(endtoken.substr(0,1));
			NERtag.push_back(NER(NERnum[i]));
			//cout << endtoken.substr(0,1) << "\t\t\t" << NER(NERnum[i]) << "\n";
			endtoken.erase(0,1);
			finaltokens.push_back(endtoken);
			NERtag.push_back(NER(NERnum[i]));
			//cout << endtoken << "\t\t\t" << NER(NERnum[i]) << "\n";
			endsentence = 1;
		}
	}
	if (endsentence)
	{
		finaltokens.push_back(" ");
		NERtag.push_back(" ");
		//cout << " " << "\t\t\t" << " " << "\n";
	}
}

void refine2(vector<string>& tokens, vector<int>& NERnum, vector<string>& NERtag, vector<string>& finaltokens)
{
	int del = 0;
	for (int i = 0; i < tokens.size(); i++)
	{
		string token = tokens[i];
		if (del){
			size_t inp = token.find(">");
			if (inp != string::npos){
				token.erase(0, inp);
				del = 0;
			}
			else
			{
				token.erase(0, token.size());
			}
		}
		else
		{
			size_t inp = token.find("MIN=");
			if (inp != string::npos){
				size_t ketinp = token.find(">");
				if (ketinp != string::npos){
					token.erase(0,ketinp);
				}
				else
				{
					token.erase(0, token.size());
					del = 1;
				}
			}
		}
		size_t inp = token.find(">");
		if (inp != string::npos){
			token.erase(inp, 1);
		}
		//cout << token << "\t\t\t\t" << NERnum[i] << endl;
		if (token!=""){
			if (tokentype((int)token[0])==0)
			{
				int x=0;
				while (x<token.size())
				{
					if (tokentype((int)token[x]) == 0) { x++; }
					else { break; }
				}
				//finaltokens.push_back(token.substr(0,x));
				//NERtag.push_back(NER(NERnum[i]));
				string check = token.substr(0,x);
				if ((check==".")||(check=="!")||(check=="?"))
				{
					finaltokens.push_back(token.substr(0,x));
					NERtag.push_back(NER(NERnum[i]));
					finaltokens.push_back(" ");
					NERtag.push_back(" ");
					/*cout << token.substr(0,x) << "\t\t\t" << NER(NERnum[i]) << "\n";
					cout << " " << "\t\t\t" << " " << "\n";*/
				}
				else if ((check==".''")||(check=="!''")||(check=="?''")||(check==",''"))
				{
					finaltokens.push_back(token.substr(0,1));
					NERtag.push_back(NER(NERnum[i]));
					finaltokens.push_back(token.substr(1,x-1));
					NERtag.push_back(NER(NERnum[i]));
					/*cout << token.substr(0, 1) << "\t\t\t" << NER(NERnum[i]) << "\n";
					cout << token.substr(1, x-1) << "\t\t\t" << NER(NERnum[i]) << "\n";*/
				}
				else if (token=="'s")
				{
					finaltokens.push_back("'s");
					NERtag.push_back("O");
					/*cout << "'s" << "\t\t\t" << "O" << "\n";*/
					x=2;
				}
				else
				{
					finaltokens.push_back(token.substr(0,x));
					NERtag.push_back(NER(NERnum[i]));
					//cout << token.substr(0, x) << "\t\t\t" << NER(NERnum[i]) << "\n";
				}
				token.erase(0,x);
			}
		}
		string endtoken;
		string endtag;
		if (token!=""){	
			if ((tokentype((int)token[token.size()-1])==0)&&(NERnum[i]==0))
			{
				int x=token.size()-1;
				/*while (tokentype((int)token[x])==0)
				{
					x--;
				}*/
				while (x>-1)
				{
					if (tokentype((int)token[x]) == 0) { x--; }
					else { break; }
				}
				endtoken = token.substr(x+1,token.size()-x-1);
				endtag = NER(NERnum[i]);
				token.erase(x+1,token.size()-x-1);
			}
		}
		if (token!=""){
			finaltokens.push_back(token);
			NERtag.push_back(NER(NERnum[i]));
		}
		if (endtoken!=""){
			endofsentence(tokens,NERnum,NERtag,finaltokens,i,endtoken);
		}
	}
	return;
}

int main(const int argc, const char *argv[], const char *env[])
{
	if (argc != 2){
		cout << "Wrong input.\n";
		return -1;
	}
	const string inFileName = argv[1];
	vector<string> lines;
	vector<string> tokens;
	vector<int> NERnum;
	vector<string> NERtag;
	loadFile(inFileName, lines);
	/*for (string line : lines){
		cout << line << "\n" <<"\n";
	}*/
	bool tokenizerswitch = 0;
	int NERmark = 0; //0-o 1-org 2-per 3-loc 4-dat 5-tim 6-mon 7-pec
	vector<int> status; //0-waiting 1-# of tag 2-tag1 3-tag2 4-tag3
	for (string line : lines){
		if ("<TEXT>" == line) { tokenizerswitch = 1; }
		if ((tokenizerswitch)&&("</TEXT>"==line)&&("<TEXT>"!=line)){
			tokenizerswitch = 0;
		}
		if (tokenizerswitch){
			char* buf = strdup(line.c_str());
			tokenize(buf, NERmark, status, tokens, NERnum);
			delete buf;
		}
	}
	vector<string> finaltokens;
	refine2(tokens, NERnum, NERtag, finaltokens);
	for (int i=0; i<finaltokens.size()-1;i++)
	{
		cout << finaltokens[i]<<"\t\t\t"<<NERtag[i]<<"\n";
	}
	return 0;
}