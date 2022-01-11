#include "DAQ_AccDataReceive.h"

DAQ_AccDataReceive::DAQ_AccDataReceive():Tool(){}


bool DAQ_AccDataReceive::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  std::string ip="";
  std::string port="0";

  if(!m_variables.Get("IP",ip)) ip="0.0.0.0";
  if(!m_variables.Get("Port",port)) port="8888";

  DataReceive = new zmq::socket_t(*m_data->context, ZMQ_DEALER);

  std::string connection="tcp://"+ip+":"+port;
  DataReceive->bind(connection.c_str());
 
  items[0].socket = *DataReceive;
  items[0].fd = 0;
  items[0].events = ZMQ_POLLIN;
  items[0].revents =0;

  return true;
}


bool DAQ_AccDataReceive::Execute(){

  int rec_counter = 0;
  m_data->TCS.receiveFlag = 0;

  zmq::poll(&items[0], 1, 100);
  
  if ((items [0].revents & ZMQ_POLLIN)) 
  {
  	m_data->PData.Receive(DataReceive);
  	m_data->PData.Print();
  
  	if((m_data->PData.RawWaveform.size()%7795 == 0 || m_data->PData.RawWaveform.size()%16 == 0 ) && m_data->PData.AccInfoFrame.size() == 32)
  	{
  		m_data->TCS.receiveFlag = 1;
  	}
    std::cout<<"Received LAPPD Data"<<std::endl;
  }


  return true;
}


bool DAQ_AccDataReceive::Finalise(){

  delete DataReceive;
  DataReceive=0;

  return true;
}
