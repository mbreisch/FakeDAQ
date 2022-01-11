#include "Factory.h"

Tool* Factory(std::string tool) {
Tool* ret=0;

// if (tool=="Type") tool=new Type;
if (tool=="DAQ_AccConfig") ret=new DAQ_AccConfig;
if (tool=="DAQ_AccDataReceive") ret=new DAQ_AccDataReceive;
if (tool=="DAQ_CreateAnnieEvent") ret=new DAQ_CreateAnnieEvent;
if (tool=="DummyTool") ret=new DummyTool;

return ret;
}
