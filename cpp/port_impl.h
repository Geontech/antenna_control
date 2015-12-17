#ifndef PORT_H
#define PORT_H

#include "ossie/Port_impl.h"
#include <queue>
#include <list>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>

class antenna_control_base;
class antenna_control_i;

#define CORBA_MAX_TRANSFER_BYTES omniORB::giopMaxMsgSize()

#include <ossie/MessageInterface.h>
#include "struct_props.h"

// ----------------------------------------------------------------------------------------
// ExtendedEvent_MessageEvent_Out_i declaration
// ----------------------------------------------------------------------------------------
class ExtendedEvent_MessageEvent_Out_i : public MessageSupplierPort
{
    public:
        ExtendedEvent_MessageEvent_Out_i(std::string port_name) : MessageSupplierPort(port_name) {
        };

        void sendMessage(switch_pattern_struct message) {
            CF::Properties outProps;
            CORBA::Any data;
            outProps.length(1);
            outProps[0].id = CORBA::string_dup(message.getId().c_str());
            outProps[0].value <<= message;
            data <<= outProps;
            push(data);
        }

        void sendMessages(std::vector<switch_pattern_struct> messages) {
            CF::Properties outProps;
            CORBA::Any data;
            outProps.length(messages.size());
            for (unsigned int i=0; i<messages.size(); i++) {
                outProps[i].id = CORBA::string_dup(messages[i].getId().c_str());
                outProps[i].value <<= messages[i];
            }
            data <<= outProps;
            push(data);
        }
};

#endif
