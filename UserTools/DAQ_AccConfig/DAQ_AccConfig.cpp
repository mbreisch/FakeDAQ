#include "DAQ_AccConfig.h"

DAQ_AccConfig::DAQ_AccConfig():Tool(){}


bool DAQ_AccConfig::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  std::string ip="";
  std::string port="0";

  if(!m_variables.Get("IP",ip)) ip="127.0.0.1";
  if(!m_variables.Get("Port",port)) port="6666";

  ConfigSend= new zmq::socket_t(*m_data->context, ZMQ_DEALER);

  std::string connection="tcp://"+ip+":"+port;
  ConfigSend->connect(connection.c_str());
 
  items[0].socket = *ConfigSend;
  items[0].fd = 0;
  items[0].events = ZMQ_POLLOUT;
  items[0].revents =0;

  // fill config variables for start of run 
  LoadSettings();

  zmq::poll(&items[0], 1, -1);
  
  if ((items [0].revents & ZMQ_POLLOUT)) {
    m_data->PConf.Send(ConfigSend);
    std::cout<<"Sending LAPPD start config variables"<<std::endl;
    m_data->PConf.Print();
  }

  return true;
}


bool DAQ_AccConfig::Execute(){

  return true;
}


bool DAQ_AccConfig::Finalise(){

  delete ConfigSend;
  ConfigSend=0;

  return true;
}


bool DAQ_AccConfig::LoadSettings(){
	string temp;

	m_variables.Get("Triggermode",m_data->PConf.triggermode);	
	
	m_variables.Get("ResetSwitchACC",m_data->PConf.ResetSwitchACC);
	m_variables.Get("ResetSwitchACDC",m_data->PConf.ResetSwitchACDC);

	m_variables.Get("ACC_Sign",m_data->PConf.ACC_Sign);
	m_variables.Get("ACDC_Sign",m_data->PConf.ACDC_Sign);
	m_variables.Get("SELF_Sign",m_data->PConf.SELF_Sign);

	m_variables.Get("SELF_Enable_Coincidence",m_data->PConf.SELF_Enable_Coincidence);
	m_variables.Get("SELF_Coincidence_Number",m_data->PConf.SELF_Coincidence_Number);
	m_variables.Get("SELF_threshold",m_data->PConf.SELF_threshold);

	m_variables.Get("PSEC_Chip_Mask_0",m_data->PConf.PSEC_Chip_Mask_0);
	m_variables.Get("PSEC_Chip_Mask_1",m_data->PConf.PSEC_Chip_Mask_1);
	m_variables.Get("PSEC_Chip_Mask_2",m_data->PConf.PSEC_Chip_Mask_2);
	m_variables.Get("PSEC_Chip_Mask_3",m_data->PConf.PSEC_Chip_Mask_3);
	m_variables.Get("PSEC_Chip_Mask_4",m_data->PConf.PSEC_Chip_Mask_4);

	
	m_variables.Get("PSEC_Channel_Mask_0",temp);
	m_data->PConf.PSEC_Channel_Mask_0 = std::stoul(temp,nullptr,16);
	m_variables.Get("PSEC_Channel_Mask_1",temp);
	m_data->PConf.PSEC_Channel_Mask_1 = std::stoul(temp,nullptr,16);
	m_variables.Get("PSEC_Channel_Mask_2",temp);
	m_data->PConf.PSEC_Channel_Mask_2 = std::stoul(temp,nullptr,16);
	m_variables.Get("PSEC_Channel_Mask_3",temp);
	m_data->PConf.PSEC_Channel_Mask_3 = std::stoul(temp,nullptr,16);
	m_variables.Get("PSEC_Channel_Mask_4",temp);
	m_data->PConf.PSEC_Channel_Mask_4 = std::stoul(temp,nullptr,16);	

	m_variables.Get("Validation_Start",m_data->PConf.Validation_Start);
	m_variables.Get("Validation_Window",m_data->PConf.Validation_Window);

	m_variables.Get("Pedestal_channel",m_data->PConf.Pedestal_channel);
	m_variables.Get("Pedestal_channel_mask",temp);
	m_data->PConf.Pedestal_channel_mask = std::stoul(temp,nullptr,16);

	m_variables.Get("ACDC_mask",temp);
	m_data->PConf.ACDC_mask = std::stoul(temp,nullptr,16);		

	m_variables.Get("Calibration_Mode",m_data->PConf.Calibration_Mode);
	m_variables.Get("Raw_Mode",m_data->PConf.Raw_Mode);	

	m_variables.Get("PPS_Ratio",temp);
	m_data->PConf.PPSRatio = std::stoul(temp,nullptr,16);
	m_variables.Get("PPS_Mux",m_data->PConf.PPSBeamMultiplexer);
	
	m_variables.Get("receiveFlag",m_data->PConf.receiveFlag);

  return true;
}
