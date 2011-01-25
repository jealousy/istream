#include <iostream>
#include <fstream>
#include <iterator>
#include <exception>

#include "istream.h"

#include <boost/timer.hpp> 
#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"

using namespace std;
using namespace libtorrent;

int filename, hash_name, piece_id;

//buffer calculations

float numPiecesToBuffer(torrent_info ti, int mode, int timeToBuffer) {
  if (mode == MOVIE) {
    int numPieces = ti.num_pieces();
    float length = 100 * 60; // assume that we are streaming a movie which is 100min
    float secsPerPiece = (float)numPieces / length;
    return timeToBuffer / secsPerPiece;
  } else {
    return 100.0; //default bullshit value
  }
}


void setPriority(torrent_handle th, int numPieces) {
  for (int i = 0; i < numPieces; i++) {

  }

}


void printAvailability(torrent_handle th) {
  std::vector<int> availability;
  th.piece_availability(availability);
  for (int i=0; i < availability.size(); i++) {
    cout << i << ": " << availability[i] << endl;
  }
}

void test() {
  cout << "cat" << endl;
}

int main(int argc, char* argv[]) {

#if BOOST_VERSION < 103400
	namespace fs = boost::filesystem;
	fs::path::default_name_check(fs::no_check);
#endif


  if (argc < 2) {
    cout << "Please specify a torrent to start" << endl;
  }

	session s;
	s.listen_on(std::make_pair(6881, 6889));
	add_torrent_params p;
	p.save_path = "./";
	error_code ec;
	p.ti = new torrent_info(argv[1], ec);

	if (ec) {
		std::cout << ec.message() << std::endl;
		return 1;
	}

	torrent_handle th = s.add_torrent(p, ec);

  //sets sequential download
  th.set_sequential_download(true);

  th.set_max_connections(40);

	if (ec) {
		std::cerr << ec.message() << std::endl;
		return 1;
	}

  boost::timer t;
  boost::timer t2;

  bool opened = true;
  int download_rate = 0;
  int upload_rate = 0;
  int num_seeds = 0;

  torrent_status ts = th.status();
  torrent_info ti = th.get_torrent_info();
  std::vector<int> availability;

  int numBuffer = numPiecesToBuffer(ti, MOVIE, 15); // buffer 15 secs


  cout << "Buffering: " << numBuffer << " pieces" << endl;

  while(1) {
    if (t.elapsed() > 2) {
      download_rate = ts.download_rate / 1000;
      upload_rate = ts.upload_rate / 1000;
      num_seeds = ts.num_seeds;

      cout << "\r Download Rate: " << download_rate << " kb/s Upload Rate: " 
           << upload_rate << " kb/s Number of Connected Seeds: " << num_seeds << flush;
      t.restart();
    }
  }

	return 0;
}
