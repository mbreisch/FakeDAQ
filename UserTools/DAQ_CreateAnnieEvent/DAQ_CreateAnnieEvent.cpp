#include "DAQ_CreateAnnieEvent.h"

DAQ_CreateAnnieEvent::DAQ_CreateAnnieEvent():Tool(){}


bool DAQ_CreateAnnieEvent::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  m_variables.Get("path",path);
  path+= getTime();
  m_variables.Get("OutputWavLabel",OutputWavLabel);

  return true;
}


bool DAQ_CreateAnnieEvent::Execute(){

  if(m_data->TCS.receiveFlag == 1)
  {
	std::map<unsigned long, vector<Waveform<double>>> LAPPDWaveforms;
	Waveform<double> tmpWave;
	vector<Waveform<double>> VecTmpWave;
	for(std::map<int, vector<unsigned short>>::iterator it=m_data->TCS.ParsedDataStream.begin(); it!=m_data->TCS.ParsedDataStream.end(); ++it)
	{
		for(unsigned short k: it->second)
		{
			tmpWave.PushSample((double)k);
		}
		VecTmpWave.push_back(tmpWave);
		LAPPDWaveforms.insert(LAPPDWaveforms.end(),std::pair<unsigned long, vector<Waveform<double>>>((unsigned long)it->first,VecTmpWave));
	}	
	  
	m_data->Stores["ANNIEEvent"]->Set(OutputWavLabel,LAPPDWaveforms);
	m_data->Stores["ANNIEEvent"]->Set("ACDCmetadata",m_data->TCS.ParsedMetaStream);
	m_data->Stores["ANNIEEvent"]->Save(path.c_str());
	std::cout << "SAVED" << std::endl;	
	m_data->Stores["ANNIEEvent"]->Delete();
	m_data->TCS.ParsedDataStream.clear();
	m_data->TCS.ParsedMetaStream.clear();
  }
	return true;
}


bool DAQ_CreateAnnieEvent::Finalise(){
  
  return true;
}
