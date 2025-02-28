/*
 * fsdata.h
 *
 *  Created on: 30-Jan-2025
 *      Author: 14169
 */
//webpage files

#ifndef FSDATA_H_
#define FSDATA_H_

#include "fs/AllParameters.h"
#include "fs/DataLog.h"
#include "fs/EmailConfig.h"
#include "fs/EthernetSetting.h"
#include "fs/FlagSetting.h"
#include "fs/MainPage.h"
#include "fs/RTCSetting.h"
#include "fs/ScheduleSetting.h"
#include "fs/SerialSetting.h"

#include "fs/CSS/Main.h"
#include "fs/CSS/Tab.h"

#include "fs/images/blue.h"

#include "fs/Script/AllParameter.h"
#include "fs/Script/DataLog.h"
#include "fs/Script/Email.h"
#include "fs/Script/EthernetSetting.h"
#include "fs/Script/FlagSetting.h"
#include "fs/Script/MainPage.h"
#include "fs/Script/RTCSetting.h"
#include "fs/Script/ScheduleSetting.h"
#include "fs/Script/SerialSetting.h"
#include "fs/Script/WebSocket.h"

#include <fats.h>

// files in fs
struct file fs_files[] = {
    {.filename = (unsigned char*)"AllParameters.shtml", .filesize = AllParameters_SIZE, .filedata = AllParameters},
    {.filename = (unsigned char*)"DataLog.shtml", .filesize = DataLog_SIZE, .filedata = DataLog},
    {.filename = (unsigned char*)"EmailConfig.shtml", .filesize = EmailConfig_SIZE, .filedata = EmailConfig},
    {.filename = (unsigned char*)"EthernetSetting.shtml", .filesize = EthernetSetting_SIZE, .filedata = EthernetSetting},
    {.filename = (unsigned char*)"FlagSetting.shtml", .filesize = FlagSetting_SIZE, .filedata = FlagSetting},
    {.filename = (unsigned char*)"MainPage.shtml", .filesize = MainPage_SIZE, .filedata = MainPage},
    {.filename = (unsigned char*)"RTCSetting.shtml", .filesize = RTCSetting_SIZE, .filedata = RTCSetting},
    {.filename = (unsigned char*)"ScheduleSetting.shtml", .filesize = ScheduleSetting_SIZE, .filedata = ScheduleSetting},
    {.filename = (unsigned char*)"SerialSetting.shtml", .filesize = SerialSetting_SIZE, .filedata = SerialSetting}
};

struct file css_files[] = {
    {.filename = (unsigned char*)"Main.css", .filesize = Main_SIZE, .filedata = Main},
    {.filename = (unsigned char*)"Tab.css", .filesize = Tab_SIZE, .filedata = Tab}
};

struct file image_files[] = {
    {.filename = (unsigned char*)"blue.jpg", .filesize = blue_SIZE, .filedata = blue}
};

struct file script_files[] = {
    {.filename = (unsigned char*)"AllParameter.js", .filesize = AllParameter_SIZE, .filedata = AllParameter},
    {.filename = (unsigned char*)"DataLog.js", .filesize = DataLog_script_SIZE, .filedata = DataLog_script},
    {.filename = (unsigned char*)"Email.js", .filesize = Email_SIZE, .filedata = Email},
    {.filename = (unsigned char*)"EthernetSetting.js", .filesize = EthernetSetting_script_SIZE, .filedata = EthernetSetting_script},
    {.filename = (unsigned char*)"FlagSetting.js", .filesize = FlagSetting_script_SIZE, .filedata = FlagSetting_script},
    {.filename = (unsigned char*)"MainPage.js", .filesize = MainPage_script_SIZE, .filedata = MainPage_script},
    {.filename = (unsigned char*)"RTCSetting.js", .filesize = RTCSetting_SIZE, .filedata = RTCSetting},
    {.filename = (unsigned char*)"ScheduleSetting.js", .filesize = ScheduleSetting_script_SIZE, .filedata = ScheduleSetting_script},
    {.filename = (unsigned char*)"SerialSetting.js", .filesize = SerialSetting_script_SIZE, .filedata = SerialSetting_script},
    {.filename = (unsigned char*)"WebSocket.js", .filesize = WebSocket_SIZE, .filedata = WebSocket}
};
// Define subfolders
struct folder subfolders[] = {
    {
        .folder_name = (unsigned char*)"CSS",
        .file_count = 2,
        .files = css_files,
        .subfolder_count = 0,
        .subfolders = NULL
    },
    {
        .folder_name = (unsigned char*)"images",
        .file_count = 1,
        .files = image_files,
        .subfolder_count = 0,
        .subfolders = NULL
    },
    {
        .folder_name = (unsigned char*)"Script",
        .file_count = 10,
        .files = script_files,
        .subfolder_count = 0,
        .subfolders = NULL
    }
};

// Define the root folder
struct folder root_folder = {
    .folder_name = (unsigned char*)"fs",
    .file_count = 9,
    .files = fs_files,
    .subfolder_count = 3,
    .subfolders = subfolders
};

#endif


