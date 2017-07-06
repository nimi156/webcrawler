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

	//insert the initial URLs
	for(int i = 0; i < nurlRoots; i++){
		int size = strlen(urlRoots[i]) + 1;

		_urlArray[i]._url = (char *)malloc(size * sizeof(char)); 
		strcpy(_urlArray[i]._url, urlRoots[i]);

		_urlArray[i]._description = (char *)malloc(500 * sizeof(char));
		strcpy(_urlArray[i]._description, "");

		_urlToUrlRecord->insertItem(urlRoots[i], i);
	}
}

//size of current description
int descriptionSize = 0;

//implement crawl()
void
WebCrawler::crawl(){
	while(_headURL <= _tailURL){
		//Fetch the next URL in _headURL
		char * url = _urlArray[_headURL]._url;
		int n = 0;
		char * buffer = fetchHTML(url, &n);

		if(buffer != NULL){
			parse(buffer, n);
		}
		free(buffer);

		strcat(_urlArray[_headURL]._description, "\0");

		
		_headURL++;
	}
}

//check if it is a valid word
bool
isWord(char * wordBuffer){
	char c;
	int size = strlen(wordBuffer);

	if(size <= 1)
		return false;

	for(int i = 0; i < size; i++){
		c = wordBuffer[i];

		if(!isalpha(c))
			return false;
	}
	return true;
}


char * wordBuffer = '\0';
URLRecordList * holder = new URLRecordList();
//override onCoutentFound
void
WebCrawler::onContentFound(char c, int status){
	if(wordBuffer == '\0' && c != '\0'){
		free(wordBuffer);
		wordBuffer = (char *)malloc(2 * sizeof(char));
		wordBuffer[0] = c;
		wordBuffer[1] = '\0';
	} else {
		if(wordBuffer == '\0')
			return;
	}

	int wordlen = strlen(wordBuffer);
	if(c != ' ' && c != ','){
		char * temp = new char [wordlen+2];
		strcpy(temp, wordBuffer);
		free(wordBuffer);
		temp[wordlen] = c;
		temp[wordlen+1] = '\0';
		int size = wordlen + 2;
		wordBuffer = (char *)malloc(size * sizeof(char));
		strcpy(wordBuffer, temp);
		delete [] temp;
	} else {
		if(wordBuffer == NULL){
			return;
		}
		if(strcmp(wordBuffer, " ") == 0){
			free(wordBuffer);
			wordBuffer = '\0';
			return;
		}

		for(int i = 0; i < strlen(wordBuffer); i++){
			wordBuffer[i] = tolower(wordBuffer[i]);
		}

		bool validWord = isWord(wordBuffer);
		bool wordExist = _wordToURLRecordList->find(wordBuffer, &holder);

		if(validWord && !wordExist){
			URLRecordList * list = new URLRecordList();
			list->_urlRecordIndex = _headURL;
			list->_next = NULL;
			_wordToURLRecordList->insertItem(wordBuffer,list);
		} else {
			bool match = true;
			int prev = holder->_urlRecordIndex;
			if(prev != _headURL)
				match = false;

			if(!match && validWord){
				URLRecordList * list = new URLRecordList();
				list->_urlRecordIndex = _headURL;
				list->_next = holder;
				_wordToURLRecordList->insertItem(wordBuffer, list);
			}
		}
		free(wordBuffer);
		wordBuffer = '\0';
	}

}

//override onAnchorFound
void
WebCrawler::onAnchorFound(char * url){
	if(url == NULL)
		return;

	//check format
	char * httpFormat = new char [12];
	strcpy(httpFormat, "http://www.");
	size_t size = 11;
	bool isFormat = (strncasecmp(httpFormat, url, size) == 0);

	if(isFormat & _tailURL < _maxUrls){
		int n = 0;
		char * htmlBuffer = fetchHTML(url, &n);

		//check if the HTML fetched is null
		bool goodHTML = false;
		if(htmlBuffer != NULL){
			goodHTML = true;
		}
		free(htmlBuffer);

		//check if the url already exist in the urlArray
		bool isExist = false;
		for(int i = 0; i < _tailURL; i++){
			if(strcmp(url, _urlArray[i]._url) == 0)
				isExist = true;
		}
		
		//add url to urlArray and urlToUrlRecord
		if(goodHTML && !isExist){
			_urlArray[_tailURL]._url = strdup(url);
			_urlArray[_tailURL]._description = (char *)malloc(500 * sizeof(char));
			strcpy(_urlArray[_tailURL]._description, "");
			_tailURL++;
		
			_urlToUrlRecord->insertItem(url, _tailURL-1);
		}
	}
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
			if((int)c == 32 || isalpha(c) || isdigit(c))
				fprintf(fp, "%c", _urlArray[i]._description[j]);
			j++;
		}
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

	for(int i = 0; i < 2039; i++){
		HashTableTemplateEntry<URLRecordList *> * temp = _wordToURLRecordList->_buckets[i];
		while(temp != NULL){
			const char * key = temp->_key;
			fprintf(fp, "%s", key);
			URLRecordList * e = temp->_data;
			int recordIndex[1000];
			int index = 0;
			while(e != NULL){
				recordIndex[index++] = e->_urlRecordIndex;
				e = e->_next;
			}
			for(int j = index - 1; j >= 0; j--){
				fprintf(fp, "%d", recordIndex[j]);
			}
			temp = temp->_next;
		}
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

	//invalid command lien input
	if(nArgv < 2 || nArgv == 3){
		printUsage();
		return 1;
	} else if (nArgv == 2) { //webcrawl+url
		const char ** urlRoots = new const char * [1];
		urlRoots[0] = argv[1];

		char * http = new char[12];
		strcpy(http, "http://www.");
		size_t size = 11;
		bool httpFormat = strncasecmp(http, urlRoots[0], size) == 0;
		
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
		printf("Done...\n");
		delete [] urlRoots;
		delete [] http;
	} else if (nArgv >= 4){ //webcrawl + -u + maxurls + url-list
		//invalid option
		if(strcmp(argv[1], "-u") != 0){
			printUsage();
			return 1;
		}
		//get maxUrls
		int maxUrls = atoi(argv[2]);

		//check format of urlRoots
		int nurlRoots = 0;
		const char ** urlRoots = new const char * [nArgv - 3];
		char * http = new char [12];
		strcpy(http, "http://www.");
		size_t size = 11;
		
		for(int i = 0; i < nArgv; i++){
			char * temp = argv[i+3];
			bool httpFormat = strncasecmp(http, temp, size) == 0;

			if(httpFormat){
				urlRoots[nurlRoots++] = temp;
			}
		}

		//Start webcrawling
		printf("Initialize web crawler...\n");
		WebCrawler wc(maxUrls,nurlRoots,urlRoots);
		printf("Processing...\n");
		wc.crawl();
		printf("Making url.txt...\n");
		wc.writeURLFile("url.txt");
		printf("Making word.txt...\n");
		wc.writeWordFile("word.txt");
		printf("Done...\n");
		delete [] urlRoots;
		delete [] http;
	}

}
