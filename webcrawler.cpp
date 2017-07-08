#include "SimpleHTMLParser.h"
#include "webcrawler.h"
#include "openhttp.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Add your implementation here

// constructor
WebCrawler::WebCrawler(int maxUrls, int nurlRoots, const char ** urlRoots){
	//initialize elements
	_urlArray = new URLRecord[maxUrls]; 
	_nInitialURLs = nurlRoots;
	_maxUrls = maxUrls;
	_headURL = 0;
	_tailURL = nurlRoots;
	_urlToUrlRecord = new HashTableTemplate<int>();
	_wordToURLRecordList = new HashTableTemplate<URLRecordList *>();

	//insert the initial URLs to urlArray
	for(int i = 0; i < nurlRoots; i++){
		int size = strlen(urlRoots[i]) + 1;

		_urlArray[i]._url = (char *)malloc(size * sizeof(char)); 
		strcpy(_urlArray[i]._url, urlRoots[i]);

		_urlArray[i]._description = (char *)malloc(500 * sizeof(char));
		strcpy(_urlArray[i]._description, "");
		//add to urlToUrlRecord
		_urlToUrlRecord->insertItem(urlRoots[i], i);
	}
}

void
WebCrawler::crawl(){
	//loop urlArray
	while(_headURL < _tailURL){
		//Fetch the next URL in _headURL
		char * url = _urlArray[_headURL]._url;
		int n = 0;
		char * buffer = fetchHTML(url, &n);

		//call function parse
		if(buffer != NULL){
			parse(buffer, n);
			free(buffer);
		}
		
		//finish description
		strcat(_urlArray[_headURL]._description, "\0");

		//increment _headURL
		_headURL++;
	}

	wordToHashTable();
}

//break into words
char * word = (char *)malloc(100 * sizeof(char));

char * getWord(char * &buffer){
printf("buffer=%s\n", buffer);
	int i = 0;
	while(*buffer != '\0'){
		//invalid char
		if((int)(*buffer) < 97 || (int)(*buffer) > 122){
			if(i == 0){
				buffer++;
				continue;
			} else{ //end of the word
				word[i] = '\0';
				return word;
			}
		}
		//add char to word[]
		word[i] = *buffer;
		i++;
		buffer++;
	}
	//handle the very last word in buffer
	if(i > 0){
		word[i] = '\0';
		return word;
	}

	return NULL;	
}

//add words to hashtable
void
WebCrawler::wordToHashTable(){
	for(int i = 0; i < _tailURL; i++){
		//first check
		if(_urlArray[i]._description != NULL){
			//get the description of urlarray[i]
			char * oneDescrip = _urlArray[i]._description;
			char * oneWord;

			URLRecordList * list = NULL;
			//break the description down to words
			while((oneWord = getWord(oneDescrip)) != NULL){
				//use word as key to find in hashtable, if not, add it in front of the llist
				if(_wordToURLRecordList->find(oneWord, &list) == false){
					URLRecordList * data = new URLRecordList();
					data->_urlRecordIndex = i;
					data->_next = list;
					_wordToURLRecordList->insertItem(oneWord,data);
				} else { //loop the llist and check if word duplicate 
					URLRecordList * temp = list;	
					int found = 0;
					while(temp != NULL){
						if(temp->_urlRecordIndex == i){
							found = 1;
							break;
						}
						temp = temp->_next;
					}
					//add
					if(found == 0){
						URLRecordList * data = new URLRecordList();
						data->_urlRecordIndex = i;
						data->_next = temp;
						_wordToURLRecordList->insertItem(oneWord,data);
					}
				}
			}

		}
	}
}

char * descrip = (char *)malloc(500*sizeof(char));
char * wordBuff = descrip;

//override onCoutentFound
void
WebCrawler::onContentFound(char c){
	//found start of description
	if(c == (char)16){
		//prepare
		*wordBuff = '\0';
		wordBuff = descrip; 

		//description empty ready to add
		if(_urlArray[_headURL]._description == NULL)
			//check if description len below 499 then add 
			if(strlen(wordBuff) <= 499){
				strcpy(_urlArray[_headURL]._description, wordBuff);
			} else {
				//only copy first 499 to description
				strncpy(_urlArray[_headURL]._description, wordBuff, 499);
			}
		else { //add after the original description
			//check size
			if((strlen(_urlArray[_headURL]._description) + strlen(wordBuff)+1) <= 499){
				strcat(_urlArray[_headURL]._description, " ");
				strcat(_urlArray[_headURL]._description, wordBuff);
			} else {
				//add only up to 500
				char b[500];
				sprintf(b, "%s%s", " ", wordBuff);
				int size = strlen(_urlArray[_headURL]._description) + 1;
				strncpy(_urlArray[_headURL]._description, b, 499-size);
			}
		}
	} else if (c == (char)18){ //sign of description end
		memset(descrip, 0, sizeof(char)*strlen(descrip));
		wordBuff = descrip;
	} else if (c == '"'){
		//
	} else {
		*wordBuff = tolower(c);
		wordBuff++;
	}
}



//override onAnchorFound
void
WebCrawler::onAnchorFound(char * url){
	if(url == NULL)
		return;

	//check "http://www." format
	char * http= new char [12];
	strcpy(http, "http://www.");
	size_t size = 11;
	bool httpFormat = (strncasecmp(http, url, size) == 0);

	//check if it contains invalid symbols
	bool goodURL = !strstr(url, "?") && !strstr(url, "#") && !strstr(url, "&")
				&& !strstr(url, ",");

	if(httpFormat && goodURL && _tailURL < _maxUrls){
		int n = 0;
		char * htmlBuffer = fetchHTML(url, &n);

		//check if the HTML fetched is null
		bool goodHTML = false;
		if(htmlBuffer != NULL){
			goodHTML = true;
		}
		free(htmlBuffer);

		//check if the url already exist in the urlArray
		bool exist = false;
		for(int i = 0; i < _tailURL; i++){
			if(_urlArray[i]._url == NULL)
				i++;
			else if(strcmp(url, _urlArray[i]._url) == 0)
				exist = true;
		}
		
		//add url to urlArray and urlToUrlRecord
		if(goodHTML && !exist){
			_urlArray[_tailURL]._url = strdup(url);
			_urlArray[_tailURL]._description = (char *)malloc(500 * sizeof(char));
			strcpy(_urlArray[_tailURL]._description, "");
			_tailURL++;
		
			_urlToUrlRecord->insertItem(url, _tailURL-1);
		}
	}
	delete [] http;
	delete [] https;
}

//write array of URLs and descriptions to file
void
WebCrawler::writeURLFile(const char * urlFileName){
	FILE * fp;
	fp = fopen(urlFileName, "w");

	if(fp == NULL)
		return;

	//loop urlArray
	for(int i = 0; i < _tailURL; i++){
		//write its url
		fprintf(fp, "%d %s\n", i, _urlArray[i]._url);
		//write its description
		int j = 0;

		while(_urlArray[i]._description[j] != '\0'){
			char c;
			c = _urlArray[i]._description[j];
			//allow only char,number,space in description
			if((int)c == 32 || (int)c >=44 && (int) c<= 122)
				fprintf(fp, "%c", _urlArray[i]._description[j++]);
			else
				j++;
		}
		fprintf(fp,"\n\n");
	}
	fclose(fp);
}

//write list of words with their urls to file
void
WebCrawler::writeWordFile(const char * wordFileName){
	FILE * fp;	
	fp = fopen(wordFileName, "w");

	if(fp == NULL)
		return;

	//create iterator
	HashTableTemplateIterator<URLRecordList *> iterator(_wordToURLRecordList);
	int index = -1;

	const char * key;
	URLRecordList * data;

	//loop the hashtable
	while(iterator.next(key,data)){
		//add word
		fprintf(fp, "%s ", key);
		URLRecordList * curr = data;

		while(curr != NULL){
			//no duplicate index
			if(curr->_urlRecordIndex != index){
				//add index of that word
				fprintf(fp, "%d ", curr->_urlRecordIndex);
			}
			index = curr->_urlRecordIndex; //update index
			curr = curr->_next;
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

void
printUsage(){
	fprintf(stderr, "Usage: webcrawl [-u <maxurls>] url-list\n");
}

//main
int main(int argc, char ** argv){
	int nArgv = 0;
	while(argv[nArgv] != NULL){
		nArgv++;
	}

	//invalid command line input
	if(nArgv < 2 || nArgv == 3){
		printUsage();
		return 1;
	} else if (nArgv == 2) { //webcrawl+url
		const char ** urlRoots = new const char * [1];
		urlRoots[0] = argv[1];
		
		//check "http://"
		char * http = new char[12];
		strcpy(http, "http://www.");
		size_t size = 11;
		bool httpFormat = strncasecmp(http, urlRoots[0], size) == 0;

		//start crawling
		if(httpFormat){
			printf("Initialize web crawler...\n");
			WebCrawler wc(1000,1,urlRoots);
			printf("Processing...\n");
			wc.crawl();
			printf("Making url.txt...\n");
			wc.writeURLFile("url.txt");
			printf("Making word.txt...\n");
			wc.writeWordFile("word.txt");
		} else {
			printUsage();
			return 1;
		}
		printf("Done!\n");
		delete [] urlRoots;
		delete [] http;
		return 0;

	} else if (nArgv >= 4){ //webcrawl + -u + maxurls + url-list
		//get maxUrls
		int maxUrls = atoi(argv[2]);

		//check
		int nurlRoots = 0;
		const char ** urlRoots = new const char * [nArgv-3];

		char * http = new char [12];
		strcpy(http, "http://www.");
		size_t size = 11;
		
		for(int i = 0; i < nArgv-3; i++){
			char * temp = argv[i+3];
			bool httpFormat = strncasecmp(http, temp, size) == 0;

			if(httpFormat){
				urlRoots[nurlRoots++] = temp;
			}
		}

		//Start crawling
		printf("Initialize web crawler...\n");
		WebCrawler wc(maxUrls,nurlRoots,urlRoots);
		printf("Processing...\n");
		wc.crawl();
		printf("Making url.txt...\n");
		wc.writeURLFile("url.txt");
		printf("Making word.txt...\n");
		wc.writeWordFile("word.txt");
		printf("Done!\n");
		delete [] urlRoots;
		delete [] http;
		return 0;
	}
}
