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
		  	/*
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
			*/
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

int DAQ_ParseData::getParsedMeta(std::vector<unsigned short> buffer, int i)
{
	//make sure an acdc buffer has been
	//filled. if not, there is nothing to be done.
	if(buffer.size() == 0)
	{
		string err_msg = "You tried to parse ACDC data without pulling/setting an ACDC buffer";
		return -1;
	}

	meta.clear();
	int dist;

	//byte that indicates the metadata of
	//each psec chip is about to follow. 
	const unsigned short startword = 0xBA11; 

	//to hold data temporarily. local to this function. 
	//this is used to match to my old convention
	//from the old software (which took a while just)
	//to type out properly. 
	//ac_info[chip][index in buffer]
	map<int, vector<unsigned short>> ac_info;

	//this is the header to the ACDC buffer that
	//the ACC appends at the beginning, found in
	//packetUSB.vhd. 
	vector<unsigned short> cc_header_info;

	
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
        	if(start_indices.size()!=0 && abs(dist-start_indices[start_indices.size()])<6*256)
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
        string fnnn = "meta-corrupt-psec-buffer.txt";
        cout << "Printing to file : " << fnnn << endl;
        ofstream cb(fnnn);
        for(unsigned short k: buffer)
        {
            cb << hex << k << endl;
        }
        return -2;
	}

	
	

	//loop through each startword index and store metadata. 
	int chip_count = 0;
	unsigned short endword = 0xFACE; //end of info buffer. 
    unsigned short endoffile = 0x4321;
	for(int i: start_indices)
	{
		//re-use buffer iterator from above
		//to set starting point. 
		bit = buffer.begin() + i + 1; //the 1 is to start one element after the startword
		//while we are not at endword, 
		//append elements to ac_info
		vector<unsigned short> infobytes;
		while(*bit != endword && *bit != endoffile && infobytes.size() < 20)
		{
			infobytes.push_back(*bit);
			++bit;
		}
		ac_info.insert(pair<int, vector<unsigned short>>(chip_count, infobytes));
		chip_count++;
	}

    map<int, unsigned short> trigger_info;
    for(int ch=0; ch<NUM_CH; ch++)
    {
        bit = buffer.begin() + ch + start_indices[4]+15;
        trigger_info[ch] = *bit;
    }
    //trigger_info.insert(pair<int, unsigned short>(ch, bit));
    unsigned short combined_trigger = buffer[7792];

    meta.push_back(m_data->TCS.PsecClassStore[i].BoardIndex);
    for(int i = 0; i < NUM_PSEC; i++)
    {
    	meta.push_back(ac_info[i][12]);//Info 13
    }
    for(int i = 0; i < NUM_PSEC; i++)
    {
    	meta.push_back(ac_info[i][4]);//Info 5
    }
    for(int i = 0; i < NUM_PSEC; i++)
    {
    	meta.push_back(ac_info[i][11]);//Info 12, later bit(31-16)
    }
    for(int i = 0; i < NUM_PSEC; i++)
    {
    	meta.push_back(ac_info[i][10]);//Info 11, later bit(15-0)
    }
    for(int i = 0; i < NUM_PSEC; i++)
    {
    	meta.push_back(ac_info[i][2]);//Info 3
    }
	meta.push_back(ac_info[0][8] & 0x7); // Info 9 PSEC0 bit(2-0)
	meta.push_back(combined_trigger);
	meta.push_back(ac_info[1][9]); //Info 10 PSEC1 later bit(31-16)
    meta.push_back(ac_info[0][9]); //Info 10 PSEC0 later bit(15-0)
    for(int i = 0; i < NUM_PSEC; i++)
    {
    	meta.push_back(ac_info[i][0]);//Info 1
    }
    for(int i = 0; i < NUM_PSEC; i++)
    {
    	meta.push_back(ac_info[i][1]);//Info 2
    }
    for(int ch=0; ch<NUM_CH; ch++)
    {
        meta.push_back(trigger_info[ch]);
    }
    for(int i = 0; i < NUM_PSEC; i++)
    {
        meta.push_back(ac_info[i][3]); //Info 4
    }

    meta.push_back(ac_info[0][8]); // Info 9 PSEC0 later bit(15-0)
	meta.push_back(ac_info[1][8]); // Info 9 PSEC1 later bit(31-16)
	meta.push_back(ac_info[2][8]); // Info 9 PSEC2 later bit(47-32)
	meta.push_back(ac_info[3][8]); // Info 9 PSEC3 later bit(63-48)
	meta.push_back((ac_info[3][5] & 0x4) >> 2); // Info 6, PSEC3 bit(2)
	meta.push_back((ac_info[3][5] & 0x8) >> 3); // Info 6, PSEC3 bit(3)
	meta.push_back(ac_info[1][5] & 0xF); //Info 6, PSEC1 bit(3-0)
	meta.push_back((ac_info[3][5] & 0x7C0) >> 6); // Info 6, PSEC3 bit(10-6)
	meta.push_back((ac_info[3][5] & 0x1) >> 4); // Info 6, PSEC3 bit(4)
	meta.push_back((ac_info[3][5] & 0x20) >> 5); //Info 6, PSEC3 bit(5)
	meta.push_back(ac_info[0][7] & 0xFFF); //Info 8 PSEC0 bit(11-0)
	meta.push_back(ac_info[1][7] & 0xFFF); //Info 8 PSEC1 bit(11-0)
	meta.push_back(ac_info[2][7] & 0xFFF); //Info 8 PSEC2 bit(11-0)
	meta.push_back(ac_info[3][7] & 0xFFF); //Info 8 PSEC3 bit(11-0)
	meta.push_back(ac_info[4][7] & 0xFFF); //Info 8 PSEC4 bit(11-0)

	meta.push_back(ac_info[0][6]); //Info 7 PSEC0
	meta.push_back(ac_info[1][6]); //Info 7 PSEC1
	meta.push_back(ac_info[2][6]); //Info 7 PSEC2
	meta.push_back(ac_info[3][6]); //Info 7 PSEC3
	meta.push_back(ac_info[4][6]); //Info 7 PSEC4
	meta.push_back(ac_info[3][5] & 0x1); // Info 6, PSEC3 bit(0)   
	meta.push_back((ac_info[3][5] & 0x2) >> 1); // Info 6, PSEC3 bit(1)
	meta.push_back((ac_info[1][5] & 0xFFF0) >> 4); //Info 6, PSEC1 bit(15-4)
	meta.push_back(ac_info[2][5] & 0xFFF); //info 6, PSEC2 bit(11-0)

    if(ac_info[0][5] != 0xEEEE)
    {
        std::cout << "PSEC frame data, trigger_info (0,0) at psec info 6 is not right" << std::endl;
        return -3;
    }
    if(ac_info[4][5] != 0xEEEE)
    {
        std::cout << "PSEC frame data, trigger_info (0,4) at psec info 6 is not right" << std::endl;
        return -4;
    }

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
