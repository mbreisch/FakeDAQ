#include "DAQ_ParseData.h"

DAQ_ParseData::DAQ_ParseData():Tool(){}


bool DAQ_ParseData::Initialise(std::string configfile, DataModel &data){

  if(configfile!="")  m_variables.Initialise(configfile);
  //m_variables.Print();

  m_data= &data;
  m_log= m_data->Log;

  if(!m_variables.Get("verbose",m_verbose)) m_verbose=1;

  first=boost::posix_time::microsec_clock::local_time();
	
  return true;
}


bool DAQ_ParseData::Execute(){

  if(m_data->TCS.receiveFlag == 1)
  {
	  int boardindex, retval;
	  std::vector<unsigned short> temp_Waveform;

	  for(int i=0; i<m_data->TCS.PsecClassStore.size(); i++)
	  {
		  
		  	string rawfn = "./Raw_b" + to_string(m_data->TCS.PsecClassStore[i].BoardIndex) + ".txt";
			ofstream d(rawfn.c_str(), ios::app); 
		        d << m_data->TCS.PsecClassStore[i].FailedReadCounter  << " ";
			for(unsigned short k: m_data->TCS.PsecClassStore[i].RawWaveform)
			{
				d <<  hex <<  k << " ";
			}
			d << endl;
			d.close();
		  
			//fill ParsedStream with vectors from data
		  	
			retval = getParsedData(m_data->TCS.PsecClassStore[i].RawWaveform);
			if(retval!=0)
			{
				std::cout << "Parsing went wrong! " << retval << std::endl;
			}
			retval = getParsedMeta(m_data->TCS.PsecClassStore[i].RawWaveform,i);
			if(retval!=0)
			{
				std::cout << "Meta parsing went wrong! " << retval << std::endl;
			}
			m_data->TCS.ParsedDataStream.insert(data.begin(),data.end());
			m_data->TCS.ParsedMetaStream.insert(m_data->TCS.ParsedMetaStream.end(),meta.begin(),meta.end());
			
	  }
  }
	
  boost::posix_time::ptime current(boost::posix_time::microsec_clock::local_time());
  boost::posix_time::time_duration dt = current-first;
  m_data->TCS.deltaT.push_back(boost::posix_time::to_simple_string(dt));
  return true;
}


bool DAQ_ParseData::Finalise(){

	ofstream of("timevector_DAQ.txt");
	for(string k: m_data->TCS.deltaT)
	{
		of << k << std::endl;
	}
	of.close();
	return true;
}

int DAQ_ParseData::getParsedMeta(std::vector<unsigned short> buffer, int classindex)
{
	//Catch empty buffers
	if(buffer.size() == 0)
	{
		std::cout << "You tried to parse ACDC data without pulling/setting an ACDC buffer" << std::endl;
		return -1;
	}

 	//Prepare the Metadata vector 
	meta.clear();

	//Helpers
	int DistanceFromZero;
	int chip_count = 0;

	//Indicator words for the start/end of the metadata
	const unsigned short startword = 0xBA11; 
	unsigned short endword = 0xFACE; 
    	unsigned short endoffile = 0x4321;

	//Empty metadata map for each Psec chip <PSEC #, vector with information>
	map<int, vector<unsigned short>> PsecInfo;

	//Empty trigger metadata map for each Psec chip <PSEC #, vector with trigger>
	map<int, vector<unsigned short>> PsecTriggerInfo;
	unsigned short CombinedTriggerRateCount;

	//Empty vector with positions of aboves startword
	vector<int> start_indices; 

	//Find the startwords and write them to the vector
	vector<unsigned short>::iterator bit;
	for(bit = buffer.begin(); bit != buffer.end(); ++bit)
	{
        if(*bit == startword)
        {
        	DistanceFromZero= std::distance(buffer.begin(), bit);
        	start_indices.push_back(DistanceFromZero);
        }
	}

	//Filter in cases where one of the start words is found in the metadata 
	    if(start_indices.size()>NUM_PSEC)
	    {
		for(int k=0; k<(int)start_indices.size()-1; k++)
		{
		    if(start_indices[k+1]-start_indices[k]>6*256+14)
		    {
			//nothing
		    }else
		    {
			start_indices.erase(start_indices.begin()+(k+1));
			k--;
		    }
		}
	    }
	
	//Last case emergency stop if metadata is still not quite right
	if(start_indices.size() != NUM_PSEC)
	{
        string fnnn = "meta-corrupt-psec-buffer.txt";
        cout << "Printing to file : " << fnnn << endl;
        ofstream cb(fnnn);
        for(unsigned short k: buffer)
        {
            cb << hex << k << endl;
        }
        return -2;
	}

	//Fill the psec info map
	for(int i: start_indices)
	{
		//Write the first word after the startword
		bit = buffer.begin() + (i+1);

		//As long as the endword isn't reached copy metadata words into a vector and add to map
		vector<unsigned short> InfoWord;
		while(*bit != endword && *bit != endoffile && InfoWord.size() < 14)
		{
			InfoWord.push_back(*bit);
			++bit;
		}
		PsecInfo.insert(pair<int, vector<unsigned short>>(chip_count, InfoWord));
		chip_count++;
	}

	//Fill the psec trigger info map
	for(int chip=0; chip<NUM_PSEC; chip++)
	{
	    for(int ch=0; ch<NUM_CH; ch++)
	    {
	    	//Find the trigger data at begin + last_metadata_start + 13_info_words + 1_end_word + 1 
	        bit = buffer.begin() + start_indices[4] + 13 + 1 + 1 + ch + (chip*NUM_CH-1);
	        PsecTriggerInfo[chip].push_back(*bit);
	    }
	}

	//Fill the combined trigger
	CombinedTriggerRateCount = buffer[7792];

	//----------------------------------------------------------
	//Start the metadata parsing 

	meta.push_back(m_data->TCS.PsecClassStore[classindex].BoardIndex);
	for(int CHIP=0; CHIP<NUM_PSEC; CHIP++)
	{
		meta.push_back((0xCA00 | CHIP));
		for(int INFOWORD=0; INFOWORD<13; INFOWORD++)
		{
			if(CHIP==4 && INFOWORD==7)
			{
				meta.push_back(((PsecInfo[CHIP][INFOWORD] & 0xf000)>>12));
				meta.push_back(((PsecInfo[CHIP][INFOWORD] & 0x800)>>11));
				meta.push_back(((PsecInfo[CHIP][INFOWORD] & 0x400)>>10));
				meta.push_back((PsecInfo[CHIP][INFOWORD] & 0x3ff));
			}else
			{
				meta.push_back(PsecInfo[CHIP][INFOWORD]);
			}
			
		}
		for(int TRIGGERWORD=0; TRIGGERWORD<6; TRIGGERWORD++)
		{
			meta.push_back(PsecTriggerInfo[CHIP][TRIGGERWORD]);
		}
	}

	meta.push_back(CombinedTriggerRateCount);

	return 0;
}


int DAQ_ParseData::getParsedData(std::vector<unsigned short> buffer)
{

	//make sure an acdc buffer has been
	//filled. if not, there is nothing to be done.
	if(buffer.size() == 0)
	{
		string err_msg = "You tried to parse ACDC data without pulling/setting an ACDC buffer";
		return -1;
	}

	//clear the data map prior.
	data.clear();

	int dist;

	//byte that indicates the metadata of
	//each psec chip is about to follow. 
	const unsigned short startword = 0xF005; 
	unsigned short endword = 0xBA11;
	unsigned short endoffile = 0x4321;

	//indices of elements in the acdcBuffer
	//that correspond to the byte ba11
	vector<int> start_indices; 
	vector<unsigned short>::iterator bit;

	//loop through the data and find locations of startwords. 
    //this can be made more efficient if you are having efficiency problems.
	for(bit = buffer.begin(); bit != buffer.end(); ++bit)
	{
		//the iterator is at an element with startword value. 
		//push the index (integer, from std::distance) to a vector. 
        if(*bit == startword)
        {
        	dist= std::distance(buffer.begin(), bit);
        	if(start_indices.size()!=0 && abs(dist-start_indices[start_indices.size()])<(6*256+15))
        	{
            	continue;        
        	}
        	start_indices.push_back(dist);
        }
	}

	if(start_indices.size()>NUM_PSEC)
	{
		for(int k=0; k<(int)start_indices.size()-1; k++)
		{
			if(start_indices[k+1]-start_indices[k]>6*256+14)
			{
				//nothing
			}else
			{
				start_indices.erase(start_indices.begin()+(k+1));
				k--;
			}
		}
	}

	if(start_indices.size() != NUM_PSEC)
	{
        string fnnn = "acdc-corrupt-psec-buffer.txt";
        cout << "Printing to file : " << fnnn << endl;
        ofstream cb(fnnn);
        for(unsigned short k: buffer)
        {
            cb << hex << k << endl;
        }
        return -2;
	}

	for(int i: start_indices)
	{
		//re-use buffer iterator from above
		//to set starting point. 
		bit = buffer.begin() + i + 1; //the 1 is to start one element after the startword
		//while we are not at endword, 
		//append elements to ac_info
		vector<unsigned short> infobytes;
		while(*bit != endword && *bit != endoffile)
		{
			infobytes.push_back((unsigned short)*bit);
			if(infobytes.size()==NUM_SAMP)
			{
				data[channel_count] = infobytes;
				infobytes.clear();
				channel_count++;
			}
			++bit;
		}	
	}


	return 0;
}
