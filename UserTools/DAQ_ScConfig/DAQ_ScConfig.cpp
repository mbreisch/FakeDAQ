#include "DAQ_ScConfig.h"

DAQ_ScConfig::DAQ_ScConfig():Tool(){}


bool DAQ_ScConfig::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  std::string ip="";
  std::string port="0";

  if(!m_variables.Get("IP",ip)) ip="127.0.0.1";
  if(!m_variables.Get("Port",port)) port="4444";

  SCConfigSend= new zmq::socket_t(*m_data->context, ZMQ_DEALER);

  std::string connection="tcp://"+ip+":"+port;
  SCConfigSend->connect(connection.c_str());
 
  items[0].socket = *SCConfigSend;
  items[0].fd = 0;
  items[0].events = ZMQ_POLLOUT;
  items[0].revents =0;

  // fill config variables for start of run 
  LoadSettings();
  
  zmq::poll(&items[0], 1, -1);
  
  if ((items [0].revents & ZMQ_POLLOUT)) {
    
    m_data->SCMonitorData.Send_Config(SCConfigSend);
    std::cout<<"Sending SlowControl start config variables"<<std::endl;
    m_data->SCMonitorData.Print();  
  }


  return true;
}


bool DAQ_ScConfig::Execute(){

  return true;
}


bool DAQ_ScConfig::Finalise(){

  delete SCConfigSend;
  SCConfigSend=0;

  return true;
}


bool DAQ_ScConfig::LoadSettings(){
  
  	bool temp_HVS;
		m_variables.Get("HV_state_set",temp_HVS);
		m_data->SCMonitorData.HV_state_set = temp_HVS;

		bool temp_LVS;
		m_variables.Get("LV_state_set",temp_LVS);
		m_data->SCMonitorData.LV_state_set = temp_LVS;

		float temp_HVV;
		m_variables.Get("HV_volts",temp_HVV);
		m_data->SCMonitorData.HV_volts = temp_HVV;

		float temp_Tr1;
		m_variables.Get("Trig1_threshold",temp_Tr1);
		m_data->SCMonitorData.Trig1_threshold = temp_Tr1;

		float temp_Tr0;
		m_variables.Get("Trig0_threshold",temp_Tr0);
		m_data->SCMonitorData.Trig0_threshold = temp_Tr0;

		float temp_VREF;
		m_variables.Get("TrigVref",temp_VREF);
		m_data->SCMonitorData.TrigVref = temp_VREF;

		bool temp_r1, temp_r2, temp_r3;
		m_variables.Get("relayCh1",temp_r1);
		m_data->SCMonitorData.relayCh1 = temp_r1;
		m_variables.Get("relayCh2",temp_r2);
		m_data->SCMonitorData.relayCh2 = temp_r2;
		m_variables.Get("relayCh3",temp_r3);
		m_data->SCMonitorData.relayCh3 = temp_r3;
  
		m_data->SCMonitorData.recieveFlag = 1;
  
  return true; 
}
