# Author info
**Haoran Wang LOGIN: wang2226**
## Features that work
- Will create url.txt and word.txt with correct format
- Works with *http://* and *https://* format urls
- Can run up to **407** links for a **single** given initial url input (-u 407 <http://...>)
- You may run more links after -u if you give more initial urls
- Here is gdb log why it crashes for -u 450 http://www.purdue.edu 

	error involved with libcurl but it works fine on my ubuntu
	(gdb) bt
	+bt
	#0  0x00007ffff70376ea in strlen () from /lib64/libc.so.6
	#1  0x00007ffff703740e in strdup () from /lib64/libc.so.6
	#2  0x00007ffff7b90e12 in ?? () from /usr/lib64/libcurl.so.4
	#3  0x00007ffff7b93b25 in ?? () from /usr/lib64/libcurl.so.4
	#4  0x00007ffff7ba1993 in curl_easy_setopt () from /usr/lib64/libcurl.so.4
	#5  0x0000000000402d70 in fetchHTML (
		    url=0x6874206574697362 <error: Cannot access memory at address 0x6874206574697362>,
			    n=0x7fffffffe1e4) at openhttp.cpp:54
	#6  0x00000000004014f9 in WebCrawler::crawl (this=0x7fffffffe290) at webcrawler.cpp:44
	#7  0x0000000000402660 in main (argc=4, argv=0x7fffffffe5b8) at webcrawler.cpp:387

- Descriptions are meaningful characters
- Links are in right order
- No duplicate urls in url.txt
- No duplicate words in word.txt
### Features that DO NOT work for now
- Can NOT run over 407 links for a single given initail url input
#### Extra Features
- None
