#ifndef TempClassStore_H
#define TempClassStore_H

#include <iostream>
#include <vector>
#include <map>

#include <PsecData.h> 

using namespace std;

class TempClassStore{


 public:

  TempClassStore();

  vector<string> deltaT;
 
  int receiveFlag=0;

  PsecData *pdata = new PsecData;

  std::map<int, PsecData> PsecClassStore;

  std::map<int, vector<unsigned short>> ParsedDataStream; // channelnumber|data
  std::vector<unsigned short> ParsedMetaStream={}; //meta
  std::vector<unsigned short> ParsedPpsStream={}; //pps
  std::vector<unsigned short> ParsedAccStream={}; //Acc info frame

 private:

 
};

#endif
