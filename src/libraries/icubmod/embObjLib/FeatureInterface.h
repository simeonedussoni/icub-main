/*
 * Copyright (C) 2012 iCub Facility, Istituto Italiano di Tecnologia
 * Authors: Alberto Cardellino
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

// - include guard ----------------------------------------------------------------------------------------------------

#ifndef _FEATUREINTERFACE_H_
#define _FEATUREINTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

// -- this file can be included also by C modules. it provides interface to C++ classes by means of its functions.

// - external dependencies --------------------------------------------------------------------------------------------

#include "EoCommon.h"
#include "EoProtocol.h"
#include "EOconstvector.h"
#include "EOnvSet.h"


// - public #define  --------------------------------------------------------------------------------------------------
// empty-section


// - declaration of public user-defined types -------------------------------------------------------------------------
// empty-section

// - declaration of extern public functions ---------------------------------------------------------------------------


void feat_Initialise(void *handleOfTheEthManager);

void feat_DeInitialise();

eObool_t feat_manage_motioncontrol_data(eOipv4addr_t ipv4, eOprotID32_t id32, void* rxdata);

eObool_t feat_manage_motioncontrol_addinfo_multienc(eOipv4addr_t ipv4, eOprotID32_t id32, void* rxdata);

eObool_t feat_manage_skin_data(eOipv4addr_t ipv4, eOprotID32_t id32, void *arrayofcandata);

eObool_t feat_manage_analogsensors_data(eOipv4addr_t ipv4, eOprotID32_t id32, void *data);

void * feat_MC_handler_get(eOipv4addr_t ipv4, eOprotID32_t id32);

eObool_t feat_MC_mutex_post(void * mchandler, uint32_t prognum);

double feat_yarp_time_now(void);

eObool_t feat_signal_network_reply(eOipv4addr_t ipv4, eOprotID32_t id32, uint32_t signature);

eObool_t feat_CANprint(eOipv4addr_t ipv4, eOmn_info_basic_t* infobasic);

const char * feat_GetBoardName(eOipv4addr_t ipv4);

void feat_PrintTrace(char *string);

void feat_PrintDebug(char *string);

void feat_PrintInfo(char *string);

void feat_PrintWarning(char *string);

void feat_PrintError(char *string);

void feat_PrintFatal(char *string);


void* ace_mutex_new(void);

// returns 0 on success to take mutex, -3 on failure upon timeout, -2 on failure upon null pointer. m is pointer obtained w/ ace_mutex_new(), tout_usec is in microsec (no timeout is 0xffffffff).
int8_t ace_mutex_take(void* m, uint32_t tout_usec);

// returns 0 on success to take mutex, -1 on genric failure of releasing mutex, -2 on failure upon null pointer. m is pointer obtained w/ ace_mutex_new(),
int8_t ace_mutex_release(void* m);

void ace_mutex_delete(void* m);



#ifdef __cplusplus
}       // closing brace for extern "C"
#endif

#endif  // include-guard


// - end-of-file (leave a blank line after)----------------------------------------------------------------------------





