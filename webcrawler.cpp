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

		int n = 0;
		char * buffer = fetchHTML(_urlArray[i]._url, &n);
		if(buffer != NULL && n != 0){
			_urlArray[i].html = (char *)malloc((strlen(buffer)+1) * sizeof(char));
			strcpy(_urlArray[i].html, buffer);
			_urlArray[i].size = n;

			free(buffer);
		}
	}
}

void
WebCrawler::crawl(){
	//loop urlArray
	while(_headURL < _tailURL){
		//Fetch the next URL in _headURL
		char * url = _urlArray[_headURL]._url;
		int n = 0;
		//char * buffer = fetchHTML(url, &n);
		char * buffer = _urlArray[_headURL].html;
		n = _urlArray[_headURL].size;
printf("head=%d, tail=%d, url=%s, n=%d\n", _headURL, _tailURL, url, n);
fflush(stdout);
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
	int i = 0;
	while(*buffer != '\0'){
		//invalid char
		if(((int)(*buffer) < 97 || (int)(*buffer) > 122) && (*buffer != '\'')){
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

URLRecordList * list = new URLRecordList();
//add words to hashtable
void
WebCrawler::wordToHashTable(){
	for(int i = 0; i < _tailURL; i++){
		//first check
		if(_urlArray[i]._description != NULL){
			//get the description of urlarray[i]
			char * oneDescrip = _urlArray[i]._description;
			char * oneWord;
			//break the description down to words
			while((oneWord = getWord(oneDescrip)) != NULL){
			//use word as key to find in hashtable, if not, add it in front of the llist
				if(_wordToURLRecordList->find(oneWord, &list) == false){
					URLRecordList * data = new URLRecordList();
					data->_urlRecordIndex = i;
					data->_next = NULL;
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
						data->_next = list;
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
	//check "http(s)://www." format
	char * http = new char [12];
	strcpy(http, "http://www.");
	char * https = new char [13];
	strcpy(https, "https://www.");
	size_t size11 = 11;
	size_t size12 = 12;

	//bool httpFormat = (strncasecmp(http, url, size) == 0);
	bool goodFormat = false;
	if(strncasecmp(http,url,size11) == 0) { 
		goodFormat = true; 
	} else if (strncasecmp(https,url,size12) == 0){ 
		goodFormat = true; 
	}

	//check if it contains invalid symbols
	bool goodURL = !strstr(url, "?") && !strstr(url, "#") && !strstr(url, "&") 
                       && !strstr(url, ",") && !strstr(url, "pdf");

	if(goodFormat && goodURL && (_tailURL < _maxUrls)){
		int n = 0;
		char * htmlBuffer = fetchHTML(url, &n);

		//check if the HTML fetched is null
		bool goodHTML = false;
		if(htmlBuffer != NULL && n !=0){
			goodHTML = true;
		//	free(htmlBuffer);
		}

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
		
			_urlArray[_tailURL].html = (char *)malloc((strlen(htmlBuffer)+1) * sizeof(char));
            strcpy(_urlArray[_tailURL].html, htmlBuffer);
			_urlArray[_tailURL].size = n;
			_tailURL++;

			free(htmlBuffer);

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
	fp = fopen(wordFileName,"w");
	
	for(int i = 0; i < 2039; i++){ 
		HashTableTemplateEntry<URLRecordList*>* temp = _wordToURLRecordList->_buckets[i];
		while(temp != NULL){
			const char * key = temp->_key; 
			fprintf(fp,"%s",key);
			URLRecordList * data =  temp->_data;
			int recordIndex[1000];
			int index = 0;
			while(data != NULL){
				recordIndex[index++] =  data->_urlRecordIndex;
				data = data->_next;
			}
			for(int j = index - 1; j >= 0;j--){
				fprintf(fp," %d", recordIndex[j]);
			}

			fprintf(fp,"\n");
			temp = temp -> _next;
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

	//invalid command line input
	if(nArgv < 2 || nArgv == 3){
		printUsage();
		return 1;
	} else if (nArgv == 2) { //webcrawl+url
		const char ** urlRoots = new const char * [1];
		urlRoots[0] = argv[1];
		
		//check "http(s)://"
		char * http = new char[12];
		char * https = new char[13];
		strcpy(http, "http://www.");
		strcpy(https,"https://www.");
		size_t size11 = 11;
		size_t size12 = 12;
		//bool httpFormat = strncasecmp(http, urlRoots[0], size) == 0;
		bool goodFormat = false;
		if(strncasecmp(http,urlRoots[0],size11) == 0){
			goodFormat = true;
		} else if (strncasecmp(https,urlRoots[0],size12) == 0){
			goodFormat = true;
		}

		//start crawling
		if(goodFormat){
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
		delete [] https;
		return 0;

	} else if (nArgv >= 4){ //webcrawl + -u + maxurls + url-list
		//get maxUrls
		int maxUrls = atoi(argv[2]);

		//check
		int nurlRoots = 0;
		const char ** urlRoots = new const char * [nArgv-3];

		char * http = new char [12];
		strcpy(http, "http://www.");
		size_t size11 = 11;
		char * https = new char [13];
		strcpy(https, "https://www.");
		size_t size12 = 12;
		
		for(int i = 0; i < nArgv-3; i++){
			char * temp = argv[i+3];
			//bool httpFormat = strncasecmp(http, temp, size) == 0;
			bool goodFormat = false;
			if(strncasecmp(http,temp,size11) == 0){
				goodFormat = true;
			} else if (strncasecmp(https,temp,size12) == 0){
				goodFormat = true;
			}

			if(goodFormat){
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
		delete [] https;
		return 0;
	}

}
