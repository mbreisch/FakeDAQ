#include "DAQ_CreateAnnieEvent.h"

DAQ_CreateAnnieEvent::DAQ_CreateAnnieEvent():Tool(){}


bool DAQ_CreateAnnieEvent::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  m_variables.Get("path",path);
  //path+= getTime();
  m_variables.Get("WaveformLabel",WaveformLabel);
  m_variables.Get("AccLabel",AccLabel);
  m_variables.Get("MetaLabel",MetaLabel);
  m_variables.Get("PPSLabel",PPSLabel);

  return true;
}


bool DAQ_CreateAnnieEvent::Execute(){

	std::cout << "RAW: " << m_data->TCS.ParsedDataStream.size() << " with " << m_data->TCS.ParsedDataStream[0].size() << std::endl;
	std::cout << "PPS: " << m_data->TCS.ParsedPpsStream.size() << std::endl;
	std::cout << "ACC: " << m_data->TCS.ParsedAccStream.size() <<  std::endl;
	std::cout << "Meta: " << m_data->TCS.ParsedMetaStream.size() << std::endl;

	
	if(m_data->TCS.receiveFlag == 1)
	{
		//Prepare temporary vectors
		std::map<unsigned long, vector<Waveform<double>>> LAPPDWaveforms;
		Waveform<double> tmpWave;
		vector<Waveform<double>> VecTmpWave;
		
		if(m_data->TCS.ParsedPpsStream.size() == 0)
		{	
			//Loop over data stream
			for(std::map<int, vector<unsigned short>>::iterator it=m_data->TCS.ParsedDataStream.begin(); it!=m_data->TCS.ParsedDataStream.end(); ++it)
			{
				for(unsigned short k: it->second)
				{
					tmpWave.PushSample((double)k);
				}
				VecTmpWave.push_back(tmpWave);
				LAPPDWaveforms.insert(LAPPDWaveforms.end(),std::pair<unsigned long, vector<Waveform<double>>>((unsigned long)it->first,VecTmpWave));
			}	
		}
		
		std::cout << "WAVE: " << LAPPDWaveforms.size() << " with " << LAPPDWaveforms[0].size() << std::endl;
		
		//m_data->Stores["LAPPDStore"]->Set(WaveformLabel,LAPPDWaveforms);
		m_data->Stores["LAPPDStore"]->Set(AccLabel,m_data->TCS.ParsedAccStream);
		//m_data->Stores["LAPPDStore"]->Set(MetaLabel,m_data->TCS.ParsedMetaStream);
		//m_data->Stores["LAPPDStore"]->Set(PPSLabel,m_data->TCS.ParsedPpsStream);
		m_data->Stores["LAPPDStore"]->Save(path.c_str()); std::cout << "SAVED" << std::endl;	
		m_data->Stores["LAPPDStore"]->Delete();
			
		

	}else
	{
		std::cout << "Nothing received!" << std::endl;	
	}
	
	//Cleanup	
	m_data->TCS.ParsedDataStream.clear();
	m_data->TCS.ParsedMetaStream.clear();
	m_data->TCS.ParsedPpsStream.clear();
	m_data->TCS.ParsedAccStream.clear();
	m_data->TCS.PsecClassStore.clear();
	return true;
}


bool DAQ_CreateAnnieEvent::Finalise(){
	m_data->Stores["LAPPDStore"]->Close();
	delete m_data->Stores["LAPPDStore"];
	m_data->Stores["LAPPDStore"] = 0;
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
