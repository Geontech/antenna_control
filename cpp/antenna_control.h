#ifndef ANTENNA_CONTROL_IMPL_H
#define ANTENNA_CONTROL_IMPL_H

#include "antenna_control_base.h"

class antenna_control_i;

class antenna_control_i : public antenna_control_base
{
    ENABLE_LOGGING
    public:
        antenna_control_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        antenna_control_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        antenna_control_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        antenna_control_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~antenna_control_i();
        int serviceFunction();


    private:
        void _construct();
        void _set_df_mode(const std::string& id);
        bool _update_switch_pattern();
        long _current_pattern;
};

#endif
