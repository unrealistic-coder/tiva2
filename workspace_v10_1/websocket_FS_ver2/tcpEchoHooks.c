
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#define TCPPORT 8765
#define WSPORT 8765
#define TCPHANDLERSTACK 2048 //  mostly i does not exceed 1024 stack usage
#define WEBHANDLERSTACK 2048

/* Prototypes */
Void tcpHandler(UArg arg0, UArg arg1);
//Void webHandler(UArg arg0, UArg arg1);
/*
 *  ======== netOpenHook ========
 *  User defined hook function to call after the stack is ready to run
 */
void netOpenHook()
{
    Task_Handle taskHandletcp;
    Task_Params taskParamstcp;
    Error_Block eb1;
    Error_init(&eb1);

    /* Make sure Error_Block is initialized */

//    Task_Handle taskHandleweb;
//    Task_Params taskParamsweb;
//    Error_Block eb2;
//    Error_init(&eb2);

    /*
     *  Create the Task that farms out incoming TCP connections.
     *  arg0 will be the port that this task listens to.
     */
    Task_Params_init(&taskParamstcp);
    taskParamstcp.stackSize = TCPHANDLERSTACK;
    taskParamstcp.priority = 1;
    taskParamstcp.arg0 = TCPPORT;
    taskHandletcp = Task_create((Task_FuncPtr)tcpHandler, &taskParamstcp, &eb1);

    if (taskHandletcp == NULL) {
        System_printf("netOpenHook: Failed to create tcpHandler Task\n");
    }

//    Task_Params_init(&taskParamsweb);
//    taskParamsweb.stackSize = WEBHANDLERSTACK;
//    taskParamsweb.priority = 2;
//    taskParamsweb.arg0 = WSPORT;
//    taskHandleweb = Task_create((Task_FuncPtr)webHandler, &taskParamsweb, &eb2);
//
//    if (taskHandleweb == NULL) {
//        System_printf("netOpenHook: Failed to create webHandler Task\n");
//    }

    System_flush();
}
