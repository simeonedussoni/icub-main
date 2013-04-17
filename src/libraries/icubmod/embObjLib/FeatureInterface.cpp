/*
 * Copyright (C) 2012 iCub Facility, Istituto Italiano di Tecnologia
 * Authors: Alberto Cardellino
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */


#include "FeatureInterface.h"
#include "FeatureInterface_hid.h"
#include "IRobotInterface.h"
#include <ethManager.h>
#include "eOcfg_EPs_board.h"
#include "embObjMotionControl.h"
#include "embObjAnalogSensor.h"
#include "embObjSkin.h"

#ifdef _SETPOINT_TEST_
#include <time.h>
#include <sys/time.h>
#include "EOYtheSystem.h"
#endif

static TheEthManager *_interface2ethManager = NULL;

void initCallback(void *p)
{
    if(_interface2ethManager == NULL )
        printf("\n\nInitting _interface2ethManager\n\n");
    _interface2ethManager = (TheEthManager*) p;
}

fakestdbool_t addEncoderTimeStamp(FEAT_ID *id, int jointNum)
{
    embObjMotionControl *tmp = (embObjMotionControl *)(_interface2ethManager->getHandleFromEP(id->ep));

    if(tmp != NULL)
    {
        tmp->refreshEncoderTimeStamp(jointNum);
        return fakestdbool_true;
    }

    return fakestdbool_false;
}

fakestdbool_t findAndFill(FEAT_ID *id, void *sk_array)
{
    // new with table, data stored in eoSkin;
    // specie di view grezza, usare dynamic cast?
    static int error = 0;
    static int notYetInitted = 0;
    IiCubFeature *skin;
    EmbObjSkin *tmp = (EmbObjSkin *)(_interface2ethManager->getHandleFromEP(id->ep));

    if(NULL == tmp)
    {
        if(0 == (error%1000) )
            yError() << "Got a SKIN message from EMS with EP " << id->ep << "board number " << id->boardNum << "but no class was instatiated for it";

        error++;
        return fakestdbool_false;
    }
    else
    {
        skin = dynamic_cast<IiCubFeature *>(tmp);
        if(NULL != skin)
        {
            skin->fillData((void *)sk_array);
        }
        else
        {
            if(0 == (notYetInitted%1000))
                yWarning() << "Got a SKIN message with EP "<< id->ep << "board number " << id->boardNum << " not yet initialized";

            notYetInitted++;
            return fakestdbool_false;
        }
    }
    return fakestdbool_true;
}

void *get_MChandler_fromEP(eOnvEP_t ep)
{
    void *h = NULL;
    h = _interface2ethManager->getHandleFromEP(ep);
//     FEAT_ID id = _interface2ethManager->getFeatInfoFromEP(ep);

    return h;
}

fakestdbool_t handle_AS_data(FEAT_ID *id, void *as_array)
{
    IiCubFeature *iAnalog;

    eOsnsr_arrayofupto12bytes_t *debug_tmp = (eOsnsr_arrayofupto12bytes_t *) as_array;
    // specie di view grezza, usare dynamic cast?
    embObjAnalogSensor *tmp = (embObjAnalogSensor *)(_interface2ethManager->getHandleFromEP(id->ep));

    if(NULL == tmp)
    {
//        printf( "/************************************\\\n"
//                "            Parte non trovata!!!\n       "
//                "\\***********************************/\n ");
        return fakestdbool_false;
    }
    else
    {
        iAnalog = dynamic_cast<IiCubFeature *>(tmp);
        iAnalog->fillData(as_array);
    }

    return fakestdbool_true;
}

fakestdbool_t MCmutex_post(void *p, uint16_t epindex, uint16_t nvindex)
{
    //epindex in realtà non serve.
    eoThreadEntry *th = NULL;
    embObjMotionControl *handler = (embObjMotionControl *) p;
    int threadId;
    eoThreadFifo *fuffy = handler->requestQueue->getFifo(nvindex);

    if( (threadId=fuffy->pop()) < 0)
    {
        yError() << "Received an answer message nobody is waiting for (MCmutex_post)";
        return fakestdbool_false;
    }
    else
    {
        th = handler->requestQueue->threadPool->getThreadTable(threadId);
        if(NULL == th)
            yError() << "MCmutex_post error at line " << __LINE__;
        th->push();
        return fakestdbool_true;
    }

    return fakestdbool_false;
}

fakestdbool_t EP_NV_2_index(eOnvEP_t ep, eOnvID_t nvid, uint16_t *epindex, uint16_t *nvindex)
{
    // prendo la struttura che mi descrive la classe eoXXX n base all'EndPoint.
    FEAT_ID _fId = _interface2ethManager->getFeatInfoFromEP(ep);
    // prendo la hash function salvata in precedenza dentro la struttura.
    eOuint16_fp_uint16_t p = _fId.EPhash_function;
    // Ricavo l'ep index dalla funzione di hash
    *epindex = p(ep);
    // Prendo la descrizione dell'EndPoint in base all'indice (relativo alla singola EMS)
    eOnvscfg_EP_t  *EPcfg = eo_cfg_nvsEP_board_EPs_cfg_get(_fId.EPvector, *epindex);
    // Finalmente prendo l'indice della nv all'interno diu questo EndPoint
    *nvindex = eo_cfg_nvsEP_board_NVs_endpoint_Nvindex_get(EPcfg, nvid);
    // Ora ho gli indici che mi servono per accedere alla tabella dei thread
    return fakestdbool_true; // acemor added
}

void transceiver_wait(eOnvEP_t ep)
{
    embObjMotionControl *handler = (embObjMotionControl *) get_MChandler_fromEP(ep);
    handler->res->transMutex.wait();
}

void transceiver_post(eOnvEP_t ep)
{
    embObjMotionControl *handler = (embObjMotionControl *) get_MChandler_fromEP(ep);
    handler->res->transMutex.post();
}

#ifdef _SETPOINT_TEST_
//static void s_print_test_data(int jointNum, setpoint_test_data_t *test_data_ptr, uint64_t diff_ms)
//{
//  printf("\n\n ******* IN CBK J_STATUS n=%d, diff=(ms)%llu, rec_pkt=%u , process_pkt=%u, exit_rx_phase_cond=%d\n\n",jointNum, diff_ms, test_data_ptr->numofrecpkt, test_data_ptr->numofprocesspkt, (int)test_data_ptr->exit_rx_phase_cond);
//}

void check_received_debug_data(FEAT_ID *id, int jointNum, setpoint_test_data_t *test_data_ptr)
{
    static int old_diff_packets[10][8] = {0}; // scheda/joint
    struct timespec curr_time;
    static int count = 0;
    static bool ep_printed_6 = false;
    static bool ep_printed_8 = false;
    uint64_t curr_time_us;
    uint64_t diff_ms = 0;
    embObjMotionControl *tmp = (embObjMotionControl *)(_interface2ethManager->getHandleFromEP(id->ep));
#warning "check_received_debug_data"

    if(!((tmp->getFeat_id().boardNum == 6) || (tmp->getFeat_id().boardNum == 8)))
        return;


    if(tmp == 0)
    {
        printf("error in heck_received_debug_data\n");
    }

//    //prendo il numero di endpoint per le schede 6 e 7
//
//    if( (!ep_printed_6) && (tmp->getFeat_id().boardNum == 6))
//     {
//      yError() << "++++++++++++++++++++++++ board num "<<  tmp->getFeat_id().boardNum << "ep = " << tmp->getFeat_id().ep;
//      ep_printed_6 = true;
//     }
//
//    if( (!ep_printed_8) && (tmp->getFeat_id().boardNum == 8))
//      {
//          yError() << "++++++++++++++++++++++++ board num "<<  tmp->getFeat_id().boardNum << "ep = " << tmp->getFeat_id().ep;
//          ep_printed_8 = true;
//      }



    //only for joint num 0 verify loop time
    if((jointNum == 0) && ((tmp->getFeat_id().boardNum == 6) || (tmp->getFeat_id().boardNum == 8)))
    {
//      if(test_data_ptr->looptime >10)
//      {
//          printf("--------- LOOP TIME----- %s, %u\n", tmp->getFeat_id().name, test_data_ptr->looptime);
//      }
        if(test_data_ptr->diff_packets != old_diff_packets[tmp->getFeat_id().boardNum][jointNum])
        {
            yError() << "board" <<  tmp->getFeat_id().name << "joint" << jointNum << "--------- diff_packets ----- " <<  test_data_ptr->diff_packets;
            old_diff_packets[tmp->getFeat_id().boardNum][jointNum] = test_data_ptr->diff_packets;
        }
    }

    //check if received values are default
    if((test_data_ptr->time == 0xABABABABABABABAB) &&
            (test_data_ptr->setpoint == 0xCCCCCCCC) &&
            (test_data_ptr->numofrecpkt == 0xFF) &&
            (test_data_ptr->numofprocesspkt == 0xEE)
//        (test_data_ptr->exit_rx_phase_cond == 0xDD)
      )
    {
//      tmp->j_debug_data[jointNum].last_time = test_data_ptr->time;
        return; //no setpoint are received!!
    }

//    if(test_data_ptr->numofsepoint > 1)
//      yError() << "*** for EMS" << tmp->getFeat_id().boardNum << "joint " << jointNum << "setpoint num =" << test_data_ptr->numofsepoint;

    if(jointNum == 0)
    {
        if((test_data_ptr->numofrecpkt != test_data_ptr->numofprocesspkt) || (test_data_ptr->numofrecpkt > 1))
        {
            struct  timeval     err_time;
            gettimeofday(&err_time, NULL);
            yError() << "[" << err_time.tv_sec <<"." <<err_time.tv_usec << "] QUEUEUEUEUE" << tmp->getFeat_id().boardNum << "joint " << jointNum << "recv " << test_data_ptr->numofrecpkt << "processed " << test_data_ptr->numofprocesspkt;
        }
    }

    if(jointNum == 0)
    {
        if((test_data_ptr->exit_cond != 0) && (test_data_ptr->exit_cond != -5))
        {
            struct  timeval     err_time;
            gettimeofday(&err_time, NULL);
            yError() << "[" << err_time.tv_sec <<"." <<err_time.tv_usec << "] board " << tmp->getFeat_id().boardNum << "exit_cond" << test_data_ptr->exit_cond;
        }
    }

    tmp->j_debug_data[jointNum].mutex.wait();

    if(test_data_ptr->setpoint == tmp->j_debug_data[jointNum].pos)
    {
        if(tmp->j_debug_data[jointNum].gotIt)
        {
            tmp->j_debug_data[jointNum].mutex.post();
            return;  // siamo felici
        }

        else
        {
            tmp->j_debug_data[jointNum].gotIt = true;
            curr_time_us = eoy_sys_abstime_get(eoy_sys_GetHandle());
            diff_ms = (curr_time_us - test_data_ptr->time) /1000;
            struct  timeval     err_time;
            gettimeofday(&err_time, NULL);

            yWarning() << "Got Ack for EMS" << tmp->getFeat_id().boardNum << "joint " << jointNum << "with time " << diff_ms << "and count_old " << tmp->j_debug_data[jointNum].count_old << "[" << err_time.tv_sec <<"." <<err_time.tv_usec << "]";
//          tmp->j_debug_data[jointNum].count_old = 0;
        }
    }
    else if(test_data_ptr->setpoint == tmp->j_debug_data[jointNum].last_pos)
    {
        tmp->j_debug_data[jointNum].count_old++;

        if(tmp->j_debug_data[jointNum].count_old == 10)
            yWarning() << "*** for EMS" << tmp->getFeat_id().boardNum << "joint " << jointNum << "count_old =" << tmp->j_debug_data[jointNum].count_old;
    }
    else
    {
        if(!tmp->j_debug_data[jointNum].wtf)
        {
            struct  timeval     err_time;
            gettimeofday(&err_time, NULL);

            yError() << "[" << err_time.tv_sec <<"." <<err_time.tv_usec << "] LOST PACKET for EMS" << tmp->getFeat_id().boardNum << "joint " << jointNum << "and count_old " << tmp->j_debug_data[jointNum].count_old;
        }

        tmp->j_debug_data[jointNum].wtf = true;
    }

    tmp->j_debug_data[jointNum].mutex.post();

//    if(test_data_ptr->time == tmp->j_debug_data[jointNum].last_time)
//    {
//      //verifico se ho perso un setpoint in direzione pc104-->ems
////        if( (tmp->j_debug_data[jointNum].pos != test_data_ptr->setpoint)  && (tmp->getFeat_id().boardNum == 6))
////        {
////            printf("******* LOST_PKT: IN CBK J_STATUS %s, n=%d received pos %d instead of %d\n", tmp->getFeat_id().name, jointNum, test_data_ptr->setpoint, tmp->j_debug_data[jointNum].pos);
////        }
//
//      return; //la scheda non ha più riceviuto set point
//    }
//    else
//    {
//      tmp->j_debug_data[jointNum].last_time = test_data_ptr->time;
//    }
//
//    curr_time_us = eoy_sys_abstime_get(eoy_sys_GetHandle());
//    diff_ms = (curr_time_us - test_data_ptr->time) /1000;
//
//
//    if( (diff_ms >3) || (test_data_ptr->numofrecpkt != test_data_ptr->numofprocesspkt) || (test_data_ptr->exit_rx_phase_cond != proccessed_all_rec_pkt) )
//    {
//      if( (tmp->getFeat_id().boardNum == 6) || (tmp->getFeat_id().boardNum == 8) )
//          s_print_test_data(jointNum, test_data_ptr, diff_ms);
//    }
//    else
//    {
//      printf("| ");
//      fflush(stdout);
//    }
//
//
//    if( (tmp->j_debug_data[jointNum].pos != test_data_ptr->setpoint)  && ((tmp->getFeat_id().boardNum == 6) || (tmp->getFeat_id().boardNum == 8)) )
//    {
//      printf("******* IN CBK J_STATUS %s, n=%d received pos %d instead of %d\n", tmp->getFeat_id().name, jointNum, test_data_ptr->setpoint, tmp->j_debug_data[jointNum].pos);
//    }

}
#endif

// eof


