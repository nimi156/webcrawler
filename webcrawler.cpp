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

		strcat(_urlArray[_headURL]._descpriton, "\0");

		
		_headURL++;
	}
}

//override onCoutentFound
void
WebCrawler::onContentFound(char c, int status){

}

//override onAnchorFound
void
WebCrawler::onAnchorFound(char * url){

}

//write array of URLs and descriptions to file
void
WebCrawler::writeURLFile(const char * urlFileName){

}

//write list of words with their urls to file
void
WebCrawler::writeWordFile(const char * wordFileName){

}

//main
int main(int argc, char ** argv){

}
