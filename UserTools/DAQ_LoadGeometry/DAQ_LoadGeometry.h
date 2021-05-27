#ifndef DAQ_LoadGeometry_H
#define DAQ_LoadGeometry_H

#include <string>
#include <iostream>

#include "Tool.h"
#include <boost/algorithm/string.hpp>

class DAQ_LoadGeometry: public Tool {


 public:

  DAQ_LoadGeometry();
  bool Initialise(std::string configfile,DataModel &data);
  bool Execute();
  bool Finalise();
 
 private:

  

};


#endif
