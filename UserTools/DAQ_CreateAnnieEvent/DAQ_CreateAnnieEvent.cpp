#include "DAQ_CreateAnnieEvent.h"

DAQ_CreateAnnieEvent::DAQ_CreateAnnieEvent():Tool(){}


bool DAQ_CreateAnnieEvent::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  system("mkdir -p Results");
  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;
	
  // Make the ANNIEEvent Store if it doesn't exist
  int recoeventexists = m_data->Stores.count("ANNIEEvent");
  if(recoeventexists==0)
  {
    m_data->Stores["ANNIEEvent"] = new BoostStore(false,0);
  }
  // Make the LAPPD Store if it doesn't exist	
  int recoeventexists = m_data->Stores.count("LAPPDStore");
  if(recoeventexists==0)
  {
    m_data->Stores["LAPPDStore"] = new BoostStore(false,2);
  }

  m_variables.Get("path",path);
  path+= getTime();
  m_variables.Get("WaveformLabel",WaveformLabel);
  m_variables.Get("AccLabel",AccLabel);
  m_variables.Get("BoardsLabel",BoardsLabel);
  m_variables.Get("ErrorLabel",ErrorLabel);
  m_variables.Get("FailLabel",FailLabel);
  
  return true;
}


bool DAQ_CreateAnnieEvent::Execute(){

	std::cout << "RAW: " << m_data->PData.RawWaveform.size() << std::endl;
	std::cout << "ACC: " << m_data->PData.AccInfoFrame.size() << std::endl;
	std::cout << "Number of Boards: " << m_data->PData.BoardIndex.size() << std::endl;
	std::cout << "Errorcodes: " << m_data->PData.errorcodes.size() << " (1 is ok if it's 0x0) " << std::endl;
	if(m_data->PData.errorcodes.size()==1)
	{
		printf("Error is 0x%08x\n", m_data->PData.errorcodes[0]);
	}
	std::cout << "Number of failed reads: " << m_data->PData.FailedReadCounter << std::endl;
	
	if(m_data->TCS.receiveFlag == 1)
	{
		std::cout << "Count LAPPD Stores: " << m_data->Stores.count("LAPPDStore") << std::endl;
		m_data->Stores["LAPPDStore"]->Set(WaveformLabel,m_data->PData.RawWaveform);
		m_data->Stores["LAPPDStore"]->Set(AccLabel,m_data->PData.AccInfoFrame);
		m_data->Stores["LAPPDStore"]->Set(BoardsLabel,m_data->PData.BoardIndex);
		m_data->Stores["LAPPDStore"]->Set(ErrorLabel,m_data->PData.errorcodes);
		m_data->Stores["LAPPDStore"]->Set(FailLabel,m_data->PData.FailedReadCounter);
		m_data->Stores["ANNIEEvent"]->Set("LAPPDStore",m_data->Stores["LAPPDStore"]);
		m_data->Stores["ANNIEEvent"]->Save(path.c_str()); 
		std::cout << "SAVED" << std::endl;	
		m_data->Stores["LAPPDStore"]->Delete(); 
		m_data->Stores["ANNIEEvent"]->Delete();
	}else
	{
		std::cout << "Nothing received!" << std::endl;	
	}
	
	//Cleanup	
	m_data->PData.RawWaveform.clear();
	m_data->PData.AccInfoFrame.clear();
	m_data->PData.BoardIndex.clear();
	m_data->PData.errorcodes.clear();
	return true;
}


bool DAQ_CreateAnnieEvent::Finalise(){
	m_data->Stores["LAPPDStore"]->Close();
	delete m_data->Stores["LAPPDStore"];
	m_data->Stores["LAPPDStore"] = 0;
	m_data->Stores["ANNIEEvent"]->Close();
	delete m_data->Stores["ANNIEEvent"];
	m_data->Stores["ANNIEEvent"] = 0;
	usleep(1000000);
	std::string datapath = path;
	BoostStore *indata=new BoostStore(false,2); //this leaks but its jsut for testing
	indata->Initialise(datapath);
	std::cout <<"Print indata:"<<std::endl;
	indata->Print(false);
	long entries;
	indata->Header->Get("TotalEntries",entries);
	std::cout <<"entries: "<<entries<<std::endl;
	
	return true;
}
