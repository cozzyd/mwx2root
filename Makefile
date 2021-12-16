

CXXFLAGS=`root-config --cflags` -g -O3
LDFLAGS=`root-config --glibs --ldflags` -lXMLParser

all: mwx2root 

clean: 
	rm -f mwx2root
