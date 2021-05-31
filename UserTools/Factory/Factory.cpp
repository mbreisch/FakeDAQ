#include "Factory.h"

Tool* Factory(std::string tool) {
Tool* ret=0;

// if (tool=="Type") tool=new Type;
if (tool=="DAQ_AccConfig") ret=new DAQ_AccConfig;
if (tool=="DAQ_AccDataReceive") ret=new DAQ_AccDataReceive;
if (tool=="DAQ_CreateAnnieEvent") ret=new DAQ_CreateAnnieEvent;
if (tool=="DAQ_ParseData") ret=new DAQ_ParseData;
if (tool=="DAQ_ScConfig") ret=new DAQ_ScConfig;
if (tool=="DAQ_SCMonitorReceive") ret=new DAQ_SCMonitorReceive;
if (tool=="DummyTool") ret=new DummyTool;
if (tool=="DAQ_LoadGeometry") ret=new DAQ_LoadGeometry;
  if (tool=="PrintParsedData") ret=new PrintParsedData;
return ret;
}
