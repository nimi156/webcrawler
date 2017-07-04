#include "SimpleHTMLParser.h"
#include "webcrawler.h"
#include "openhttp.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stlib.h>

// Add your implementation here
// constructor
WebCrawler::WebCrawler(int maxUrls, int nurlRoots, const char ** urlRoots){
	//initialize the elements in the web crawler object
	_urlArray = new URLRecod[maxUrls];
	_nurlRoots = nurlRoots;
	_maxUrls = maxUrls;
	_headURL = 0;
	_tailURL = nurlRoots;
	_urlToUrlRecord = new HashTableTemplate<int>();
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

//implement crawl()
void
Webcrawler::crawl(){
	while(_headURL <= _tailURL){
		//Fetch the next URL in _headURL
		char * url = _urlArray[_headURL]._url;
		char * urlFetched = fetchHTML(url, n);
		

		//increment _headURL
		_headURL++;
	}
}
