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

//implement crawl()
void
WebCrawler::crawl(){
	//loop urlArray
	while(_headURL <= _tailURL){
		//Fetch the next URL in _headURL
		char * url = _urlArray[_headURL]._url;
		int n = 0;
		char * buffer = fetchHTML(url, &n);
		//call function parse
		if(buffer != NULL){
			parse(buffer, n);
		}
		free(buffer);

		strcat(_urlArray[_headURL]._description, "\0");

		//increment _headURL
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
		
		if(isalpha(c) == 0)
			return false;
	}
	return true;
}

char * word;
char * description;
URLRecordList * holder = new URLRecordList();
//override onCoutentFound
void
WebCrawler::onContentFound(char c){
	//initialize word
	word = new char[1];
	word[0] = '\0';

	//initialize description
	description = new char[1];
	description[0] = '\0';

	if(('a' <= c <= 'z') || ('A' <= c <= 'Z') || ('0' <= c <= '9')){
		char * next = new char[2];
		next[0] = 'c';
		next[1] = '\0';
		strcat(word, next);	
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
			if(strcmp(url, _urlArray[i]._url) == 0)
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
				fprintf(fp, "%c", _urlArray[i]._description[j++]);
		}
	}
	fclose(fp);
}

//write list of words with their urls to file
void
WebCrawler::writeWordFile(const char * wordFileName){

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
