
all: git-commit gethttp webcrawler

gethttp: gethttp.cpp openhttp.cpp SimpleHTMLParser.cpp
	g++ -o gethttp -g gethttp.cpp openhttp.cpp SimpleHTMLParser.cpp -lcurl

git-commit:
	git add Makefile *.h *.cpp  .local.git.out >> .local.git.out
	git commit -a -m "Commit web-crawler" >> .local.git.out

webcrawler: webcrawler.cpp openhttp.cpp SimpleHTMLParser.cpp
	g++ -o webcrawler -g webcrawler.cpp openhttp.cpp SimpleHTMLParser.cpp -lcurl

clean:
	rm -f *.o gethttp webcrawler core
