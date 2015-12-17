/**************************************************************************

    This is the device code. This file contains the child class where
    custom functionality can be added to the device. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "antenna_control.h"
#include <wiringPi.h>

PREPARE_LOGGING(antenna_control_i)


antenna_control_i::antenna_control_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    antenna_control_base(devMgr_ior, id, lbl, sftwrPrfl)
{
	this->_construct();
}

antenna_control_i::antenna_control_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    antenna_control_base(devMgr_ior, id, lbl, sftwrPrfl, compDev)
{
	this->_construct();
}

antenna_control_i::antenna_control_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    antenna_control_base(devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
	this->_construct();
}

antenna_control_i::antenna_control_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    antenna_control_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
{
	this->_construct();
}

void antenna_control_i::_construct() {
	// For GPIO control
	setPropertyChangeListener("DF_mode", this, &antenna_control_i::_set_df_mode);
	_current_pattern = 0;
	wiringPiSetupSys();
}

antenna_control_i::~antenna_control_i()
{
}

/*
 * Set GPIO pin high if corresponding bit in antenna_mode is SET.
 */
void antenna_control_i::_set_df_mode(const std::string& id) {
	digitalWrite(17, (DF_mode) ? HIGH : LOW);

	if (not started()) start();
}

/* Uses the DF_mode property and GPIO [23, 22, 27] to
 * calculate what antenna switching pattern is in use
 * by the attached arduino:
 * DF_mode = false -> output 0
 * DF_mode = true:
 * 		result = [23, 22, 27]
 * 		         [001] -> "1" -> Antenna 1-2
 * 		         [010] -> "2" -> Antenna 2-3
 * 		         [011] -> "3" -> Antenna 3-4
 * 		         [111] -> "7" -> Antenna 4-1*
 * Note the value for Antenna 4-1: it's 7, not 4.
 *
 * These pins must be exported as inputs:
 *    gpio export 27 input
 *    (etc.)
 *
 * Returns true if there was a change.
 * */
bool antenna_control_i::_update_switch_pattern() {
	long now = 0;

	now = (digitalRead(21) & 0x1);
	now |= ((digitalRead(22) & 0x1) << 1);
	now |= ((digitalRead(27) & 0x1) << 2);

	bool result = (now != _current_pattern);
	_current_pattern = now;
	return result;
}

/***********************************************************************************************

    Basic functionality:

        The service function is called by the serviceThread object (of type ProcessThread).
        This call happens immediately after the previous call if the return value for
        the previous call was NORMAL.
        If the return value for the previous call was NOOP, then the serviceThread waits
        an amount of time defined in the serviceThread's constructor.
        
    SRI:
        To create a StreamSRI object, use the following code:
                std::string stream_id = "testStream";
                BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);

	Time:
	    To create a PrecisionUTCTime object, use the following code:
                BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();

        
    Ports:

        Data is passed to the serviceFunction through the getPacket call (BULKIO only).
        The dataTransfer class is a port-specific class, so each port implementing the
        BULKIO interface will have its own type-specific dataTransfer.

        The argument to the getPacket function is a floating point number that specifies
        the time to wait in seconds. A zero value is non-blocking. A negative value
        is blocking.  Constants have been defined for these values, bulkio::Const::BLOCKING and
        bulkio::Const::NON_BLOCKING.

        Each received dataTransfer is owned by serviceFunction and *MUST* be
        explicitly deallocated.

        To send data using a BULKIO interface, a convenience interface has been added 
        that takes a std::vector as the data input

        NOTE: If you have a BULKIO dataSDDS port, you must manually call 
              "port->updateStats()" to update the port statistics when appropriate.

        Example:
            // this example assumes that the device has two ports:
            //  A provides (input) port of type bulkio::InShortPort called short_in
            //  A uses (output) port of type bulkio::OutFloatPort called float_out
            // The mapping between the port and the class is found
            // in the device base class header file

            bulkio::InShortPort::dataTransfer *tmp = short_in->getPacket(bulkio::Const::BLOCKING);
            if (not tmp) { // No data is available
                return NOOP;
            }

            std::vector<float> outputData;
            outputData.resize(tmp->dataBuffer.size());
            for (unsigned int i=0; i<tmp->dataBuffer.size(); i++) {
                outputData[i] = (float)tmp->dataBuffer[i];
            }

            // NOTE: You must make at least one valid pushSRI call
            if (tmp->sriChanged) {
                float_out->pushSRI(tmp->SRI);
            }
            float_out->pushPacket(outputData, tmp->T, tmp->EOS, tmp->streamID);

            delete tmp; // IMPORTANT: MUST RELEASE THE RECEIVED DATA BLOCK
            return NORMAL;

        If working with complex data (i.e., the "mode" on the SRI is set to
        true), the std::vector passed from/to BulkIO can be typecast to/from
        std::vector< std::complex<dataType> >.  For example, for short data:

            bulkio::InShortPort::dataTransfer *tmp = myInput->getPacket(bulkio::Const::BLOCKING);
            std::vector<std::complex<short> >* intermediate = (std::vector<std::complex<short> >*) &(tmp->dataBuffer);
            // do work here
            std::vector<short>* output = (std::vector<short>*) intermediate;
            myOutput->pushPacket(*output, tmp->T, tmp->EOS, tmp->streamID);

        Interactions with non-BULKIO ports are left up to the device developer's discretion

    Properties:
        
        Properties are accessed directly as member variables. For example, if the
        property name is "baudRate", it may be accessed within member functions as
        "baudRate". Unnamed properties are given a generated name of the form
        "prop_n", where "n" is the ordinal number of the property in the PRF file.
        Property types are mapped to the nearest C++ type, (e.g. "string" becomes
        "std::string"). All generated properties are declared in the base class
        (antenna_control_base).
    
        Simple sequence properties are mapped to "std::vector" of the simple type.
        Struct properties, if used, are mapped to C++ structs defined in the
        generated file "struct_props.h". Field names are taken from the name in
        the properties file; if no name is given, a generated name of the form
        "field_n" is used, where "n" is the ordinal number of the field.
        
        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A boolean called scaleInput
              
            if (scaleInput) {
                dataOut[i] = dataIn[i] * scaleValue;
            } else {
                dataOut[i] = dataIn[i];
            }
            
        A callback method can be associated with a property so that the method is
        called each time the property value changes.  This is done by calling 
        setPropertyChangeListener(<property name>, this, &antenna_control_i::<callback method>)
        in the constructor.
            
        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            
        //Add to antenna_control.cpp
        antenna_control_i::antenna_control_i(const char *uuid, const char *label) :
            antenna_control_base(uuid, label)
        {
            setPropertyChangeListener("scaleValue", this, &antenna_control_i::scaleChanged);
        }

        void antenna_control_i::scaleChanged(const std::string& id){
            std::cout << "scaleChanged scaleValue " << scaleValue << std::endl;
        }
            
        //Add to antenna_control.h
        void scaleChanged(const std::string&);
        
        
************************************************************************************************/
int antenna_control_i::serviceFunction()
{
	if (_update_switch_pattern()) {
		switch_pattern_struct msg;
		msg.value = _current_pattern;
		message_out->sendMessage(msg);
		return NORMAL;
	}
    return NOOP;
}
