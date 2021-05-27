#include "DAQ_LoadGeometry.h"

DAQ_LoadGeometry::DAQ_LoadGeometry():Tool(),adet(nullptr),AnnieGeometry(nullptr),LAPPD_channel_count(0){}


bool DAQ_LoadGeometry::Initialise(std::string configfile, DataModel &data){

  /////////////////// Usefull header ///////////////////////
  if(configfile!="")  m_variables.Initialise(configfile); //loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////

  // Make the ANNIEEvent Store if it doesn't exist
  int recoeventexists = m_data->Stores.count("ANNIEEvent");
  if(recoeventexists==0) m_data->Stores["ANNIEEvent"] = new BoostStore(false,2);

  //Load LAPPD Geometry Information
  this->LoadLAPPDs();

  return true;
}


bool DAQ_LoadGeometry::Execute(){
  return true;
}


bool DAQ_LoadGeometry::Finalise(){
  std::cout << "DAQ_LoadGeometry tool exitting" << std::endl;
  return true;
}


