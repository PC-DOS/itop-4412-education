/*
 * NETWORKING CONTROL INTERFACE
 *
 * This file is the interface of networking interface.
 * The interface is divided into 2 parts: TCP Client and TCP Server.
 * Normally, the Client side sends data to the remote, and the Server side receives data from the remote.
 *
 * This file is a part of DataSourceProvider, but was separated for easier maintainance.
 * For DataFrames' definitions and stream operators, please refer to DataSourceProvider.
 *
 */

#ifndef NETWORKINGCONTROLINTERFACE_H
#define NETWORKINGCONTROLINTERFACE_H

#include "NetworkingControlInterface.Client.h"
#include "NetworkingControlInterface.Server.h"

/* AT Commands */
#define AT_CMD_SET_SYSGAIN "SETSYSGAIN"
#define AT_CMD_SET_REFGAIN "SETREFGAIN"
#define AT_CMD_SET_DISTANCE "SETDISTANCE"
#define AT_CMD_SET_DELAY "SETDELAY"
#define AT_CMD_SET_OFFSET "SETOFFSET"
#define AT_CMD_SET_VELOCITY "SETVELOCITY"
#define AT_CMD_SET_ANGLE "SETANGLE"

#endif // NETWORKINGCONTROLINTERFACE_H
