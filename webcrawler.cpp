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
	//initialize URL array
	_urlArray = new URLRecord[maxUrls]; 
	//initialize the number of given urls
	_nInitialURLs = nurlRoots;
	//initialize maxUrls
	_maxUrls = maxUrls;
	//initialize headURL
	_headURL = 0;
	//initialize tailURL
	_tailURL = nurlRoots;
	//initialize urlToUrlRecord
	_urlToUrlRecord = new HashTableTemplate<int>();
	//initailize wordToURLRecordList
	_wordToURLRecordList = new HashTableTemplate<URLRecordList *>();

	//insert the initial URLs
	for(int i = 0; i < nurlRoots; i++){
		int size = strlen(urlRoots[i]) + 1;
		//initialize its _url
		_urlArray[i]._url = (char *)malloc(size * sizeof(char)); 
		strcpy(_urlArray[i]._url, urlRoots[i]);
		//initialize its _description
		_urlArray[i]._description = (char *)malloc(500 * sizeof(char));
		strcpy(_urlArray[i]._description, "");
		//insert to the URLArray
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
isWord(char * word){
	char c;
	int size = strlen(word);

	if(size <= 1)
		return false;

	for(int i = 0; i < size; i++){
		c = word[i];

		if(!isalpha(c))
			return false;
	}
	return true;
}

//override onCoutentFound
void
WebCrawler::onContentFound(char c, int status){

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
		
		//add url to urlArray
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

}
