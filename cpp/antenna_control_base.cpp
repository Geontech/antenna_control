#include "antenna_control_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the device class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

antenna_control_base::antenna_control_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    Device_impl(devMgr_ior, id, lbl, sftwrPrfl),
    serviceThread(0)
{
    construct();
}

antenna_control_base::antenna_control_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    Device_impl(devMgr_ior, id, lbl, sftwrPrfl, compDev),
    serviceThread(0)
{
    construct();
}

antenna_control_base::antenna_control_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    Device_impl(devMgr_ior, id, lbl, sftwrPrfl, capacities),
    serviceThread(0)
{
    construct();
}

antenna_control_base::antenna_control_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    Device_impl(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev),
    serviceThread(0)
{
    construct();
}

void antenna_control_base::construct()
{
    Resource_impl::_started = false;
    loadProperties();
    serviceThread = 0;
    
    PortableServer::ObjectId_var oid;
    message_out = new ExtendedEvent_MessageEvent_Out_i("message_out");
    oid = ossie::corba::RootPOA()->activate_object(message_out);

    registerOutPort(message_out, message_out->_this());
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void antenna_control_base::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
}

void antenna_control_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread == 0) {
        serviceThread = new ProcessThread<antenna_control_base>(this, 0.1);
        serviceThread->start();
    }
    
    if (!Resource_impl::started()) {
    	Resource_impl::start();
    }
}

void antenna_control_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    // release the child thread (if it exists)
    if (serviceThread != 0) {
        if (!serviceThread->release(2)) {
            throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
        }
        serviceThread = 0;
    }
    
    if (Resource_impl::started()) {
    	Resource_impl::stop();
    }
}

CORBA::Object_ptr antenna_control_base::getPort(const char* _id) throw (CORBA::SystemException, CF::PortSupplier::UnknownPort)
{

    std::map<std::string, Port_Provides_base_impl *>::iterator p_in = inPorts.find(std::string(_id));
    if (p_in != inPorts.end()) {
    }

    std::map<std::string, CF::Port_var>::iterator p_out = outPorts_var.find(std::string(_id));
    if (p_out != outPorts_var.end()) {
        return CF::Port::_duplicate(p_out->second);
    }

    throw (CF::PortSupplier::UnknownPort());
}

void antenna_control_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the device running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    // deactivate ports
    releaseInPorts();
    releaseOutPorts();

    delete(message_out);

    Device_impl::releaseObject();
}

void antenna_control_base::loadProperties()
{
    addProperty(device_kind,
                "Pi GPIO Control",
                "DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d",
                "device_kind",
                "readonly",
                "",
                "eq",
                "allocation,configure");

    addProperty(device_model,
                "raspberry pi",
                "DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb",
                "device_model",
                "readonly",
                "",
                "eq",
                "allocation,configure");

    addProperty(DF_mode,
                false,
                "DF_mode",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(switch_pattern,
                switch_pattern_struct(),
                "switch_pattern",
                "",
                "readwrite",
                "",
                "external",
                "message");

}
