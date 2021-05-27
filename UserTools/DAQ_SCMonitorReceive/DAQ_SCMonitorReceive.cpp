#include "DAQ_SCMonitorReceive.h"

DAQ_SCMonitorReceive::DAQ_SCMonitorReceive():Tool(){}


bool DAQ_SCMonitorReceive::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  m_util=new Utilities(m_data->context);

  MonitorReceiver= new zmq::socket_t(*m_data->context, ZMQ_SUB);
  MonitorReceiver->setsockopt(ZMQ_SUBSCRIBE, "", 0);

  items[0].socket = *MonitorReceiver;
  items[0].fd = 0;
  items[0].events = ZMQ_POLLIN;
  items[0].revents =0;

  m_util->UpdateConnections("MonitorData", MonitorReceiver, connections);

  last= boost::posix_time::ptime(boost::posix_time::second_clock::local_time());
  period =boost::posix_time::time_duration(0,0,1,0);

  return true;
}


bool DAQ_SCMonitorReceive::Execute(){

  boost::posix_time::ptime current(boost::posix_time::second_clock::local_time());
  boost::posix_time::time_duration duration(current - last);
  if(duration>period){
    last=current;
    m_util->UpdateConnections("MonitorData", MonitorReceiver, connections);
  }

  if(connections.size()>0){

    zmq::poll(&items[0], 1, 0);
    
    if ((items [0].revents & ZMQ_POLLIN)) {
      zmq::message_t command;
      MonitorReceiver->recv(&command);
      
      std::istringstream tmp(static_cast<char*>(command.data()));
      std::cout<<"received monitoring data of type: "<<tmp.str()<<std::endl;

      if(tmp.str()=="LAPPDMonData"){
	      //receive slowcontrol mon data
	      m_data->SCMonitorData.Receive_Mon(MonitorReceiver);
	      m_data->SCMonitorData.Print();
      }else if(tmp.str()=="LAPPDDaqMonData"){
      	// receive LAPPD monitoring data
      	//m_data->LAPPD_mon_data_class.Receive(MonitorReceive);
        //m_data->LAPPD_mon_data_class.Print();
      }
    }
  }

  return true;

}


bool DAQ_SCMonitorReceive::Finalise(){

  delete MonitorReceiver;
  MonitorReceiver=0;

  delete m_util;
  m_util=0;

  connections.clear();

  return true;

}
