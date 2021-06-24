#include "DAQ_LoadGeometry.h"

DAQ_LoadGeometry::DAQ_LoadGeometry():Tool(){}


bool DAQ_LoadGeometry::Initialise(std::string configfile, DataModel &data){

  /////////////////// Usefull header ///////////////////////
  if(configfile!="")  m_variables.Initialise(configfile); //loading config file
  //m_variables.Print();

  m_data= &data; //assigning transient data pointer
  /////////////////////////////////////////////////////////////////

  // Make the ANNIEEvent Store if it doesn't exist
  int recoeventexists = m_data->Stores.count("LAPPD");
  if(recoeventexists==0)
  {
    m_data->Stores["LAPPD"] = new BoostStore(false,2);
  }

  return true;
}


bool DAQ_LoadGeometry::Execute(){
  
  // Make the ANNIEEvent Store if it doesn't exist
  int recoeventexists = m_data->Stores.count("LAPPD");
  if(recoeventexists==0)
  {
    m_data->Stores["LAPPD"] = new BoostStore(false,2);
  }
  
  return true;
}


bool DAQ_LoadGeometry::Finalise(){
  return true;
}


