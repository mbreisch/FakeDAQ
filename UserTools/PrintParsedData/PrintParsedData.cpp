#include "PrintParsedData.h"

PrintParsedData::PrintParsedData():Tool(){}


bool PrintParsedData::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;


  return true;
}


bool PrintParsedData::Execute(){

	if(m_data->TCS.receiveFlag == 1)
	{ 
		ofstream ofMETA("META.txt");
		ofstream ofDATA("DATA.txt");
		ofstream ofACC("ACC.txt",ios::app);
		for(int smp=0; smp<256; smp++)
		{
			for(int ch=0; ch<30; ch++)
			{
				ofDATA << std::hex <<m_data->TCS.ParsedDataStream[ch][smp] << " ";
			}
			ofDATA << std::endl;
		}

		for(unsigned short k: m_data->TCS.ParsedMetaStream)	
		{
			ofMETA << std::hex << k << std::endl;
		}
		
		for(unsigned short l: m_data->TCS.ParsedAccStream)	
		{
			ofACC << std::hex << l << " ";
		}
		ofACC << std::endl
			
		ofMETA.close();
		ofDATA.close();
		ofACC.close();
	}

  	return true;
}


bool PrintParsedData::Finalise(){

  return true;
}
