istream: istream.cpp
	g++ -I /usr/local/boost_1_43_0 -L/opt/local/lib -L/usr/local/lib/ -lboost_filesystem -lboost_system -ltorrent-rasterbar istream.cpp -o istream
clean:
	rm -f istream
