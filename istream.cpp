#include <iostream>
#include <fstream>
#include <iterator>
#include <exception>

#include "istream.h"

#include <boost/timer.hpp> 
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/alert.hpp>
#include <libtorrent/alert_types.hpp>

#include <vector>

using namespace std;
using namespace libtorrent;
//using namespace boost::posix_time;

int filename, hash_name, piece_id;

//buffer calculations

void displayStatus(torrent_handle th) {
  torrent_status ts = th.status();
  if (ts.state == 3) 
    cout << "Downloading..." << endl;
  else 
    cout << ts.state << endl;
}

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


void setPriority(torrent_handle th, int numPieces, int numBuffer) {
  int i, priority;
  i = 0;
  for (priority = 7; priority >= 1; priority--) {
    for (; i < numBuffer; i++) {
      th.piece_priority(i, priority);
    }
    i++;
    numBuffer += numBuffer;
  }
}

//void fillGaps(torrent_handle th, torrent_status ts, session s)  {
//  for (int i = 10; i < ts.pieces.size(); i++) {
//    bool comp = true;
//    if (!ts.pieces[i]) {
//      for (int j = i; i > (i - 10); j--) 
//        if (!ts.pieces[j]) comp = false;
//      if (comp) {
//        th.read_piece(i - 1);
//        alert* a = s.wait_for_alert(10);
//        cout << a->what() << endl;
//      }
//    }
//  }
//}

void printPriority(torrent_handle th) {
  std::vector<int> prio = th.piece_priorities();
  for(int i =0 ; i < prio.size(); i++)
    cout << "piece: " << i << " prio: " << prio[i] << endl;
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

  if (argc < 2) {
    cout << "Please specify a torrent to start" << endl;
  }

	session s;

	s.listen_on(std::make_pair(6881, 6889));
	add_torrent_params p;
	p.save_path = "./";
  p.override_resume_data = true;
	error_code ec;
	p.ti = new torrent_info(argv[1], ec);

	if (ec) {
		std::cout << ec.message() << std::endl;
		return 1;
	}

  s.set_alert_mask(alert::storage_notification);



	torrent_handle th = s.add_torrent(p, ec);

  //sets sequential download
//  th.set_sequential_download(true);

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

  torrent_info ti = th.get_torrent_info();
  std::vector<int> availability;

  int numPieces = ti.num_pieces();
  int numBuffer = numPiecesToBuffer(ti, MOVIE, 15); // buffer 15 secs

  cout << "Buffering: " << numBuffer << " pieces" << endl;

  setPriority(th, numPieces, numBuffer);

//  printPriority(th);

  while(1) {
    if (t.elapsed() > 5) {
      torrent_status ts = th.status();
      download_rate = ts.download_rate / 1000;
      upload_rate = ts.upload_rate / 1000;
      num_seeds = ts.num_seeds;
      bitfield pieces = ts.pieces;

      int dlpiece = -1;


      if (dlpiece == -1) {
        for(int i = 0; i < 15; i++) {
          if (ts.pieces[i]) {
            dlpiece = i;
            break;
          }
        }
      }

      if (dlpiece != -1) {
        cout << "CALLED READ" << endl;
        bool found = false;

        time_duration td = seconds(5);

        th.read_piece(dlpiece);

        const alert * a = s.wait_for_alert(td);
        std::auto_ptr<alert> holder = s.pop_alert();
        if (alert_cast<read_piece_alert>(a)) {
          const read_piece_alert* rpa = alert_cast<read_piece_alert>(a);
          cout << "Piece found: " << rpa->piece;
        }
      }


      for (int i = 0; i < ts.pieces.size(); i++)
        cout <<  ts.pieces[i] << flush;
      cout << endl;

//      fillGaps(th, ts, s);

//        for (int i = 10; i < pieces.size(); i++) {
//          bool comp = true;
//          if (!ts.pieces[i]) {
//            for (int j = (i- 1); j > (i - 5); j--) 
//              if (!ts.pieces[j]) comp = false;
//            if (comp) {
//              cout << "Calling READ: " << endl;
//              th.read_piece(i - 1);
//              time_duration td = seconds(5);
//              bool findAlert = false;
//              while (!findAlert) {
//                const alert * a = s.wait_for_alert(td);
//                std::auto_ptr<alert> holder = s.pop_alert();
//                if (strcmp(a->what(), "read_piece_alert"))
//                  findAlert = true;
//              cout << a->what() << endl;
//              }
//            }
//          }
//        }

          //      displayStatus(th);

          cout << "\r Download Rate: " << download_rate << " kb/s Upload Rate: " 
            << upload_rate << " kb/s Number of Connected Seeds: " << num_seeds << " Status: " << ts.state << flush;
          t.restart();
      }
  }

	return 0;
}
