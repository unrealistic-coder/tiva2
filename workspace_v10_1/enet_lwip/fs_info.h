/*
 * fs_info.h
 *
 *  Created on: 06-Feb-2025
 *      Author: 14169
 */

#ifndef FS_INFO_H_
#define FS_INFO_H_

#include "fs_gzip_array/AllParameters_shtml.h"
#include "fs_gzip_array/DataLog_shtml.h"
#include "fs_gzip_array/EmailConfig_shtml.h"
#include "fs_gzip_array/EthernetSetting_shtml.h"
#include "fs_gzip_array/FlagSetting_shtml.h"
#include "fs_gzip_array/MainPage_shtml.h"
#include "fs_gzip_array/RTCSetting_shtml.h"
#include "fs_gzip_array/ScheduleSetting_shtml.h"
#include "fs_gzip_array/SerialSetting_shtml.h"

#include "fs_gzip_array/CSS/Main_css.h"
#include "fs_gzip_array/CSS/Tab_css.h"

#include "fs_gzip_array/images/blue_jpg.h"

#include "fs_gzip_array/Script/AllParameter_js.h"
#include "fs_gzip_array/Script/DataLog_js.h"
#include "fs_gzip_array/Script/Email_js.h"
#include "fs_gzip_array/Script/EthernetSetting_js.h"
#include "fs_gzip_array/Script/FlagSetting_js.h"
#include "fs_gzip_array/Script/MainPage_js.h"
#include "fs_gzip_array/Script/RTCSetting_js.h"
#include "fs_gzip_array/Script/ScheduleSetting_js.h"
#include "fs_gzip_array/Script/SerialSetting_js.h"
#include "fs_gzip_array/Script/WebSocket_js.h"

struct file {
    unsigned char* filename;
    int filesize;
    unsigned char* filedata;
};

struct folder {
    unsigned char* folder_name;
    int file_count;
    int subfolder_count;
    struct folder* subfolders;
    struct file* files;
};

struct file fs_files[] = {
    {.filename = (unsigned char*)"AllParameters.shtml", .filesize = AllParameters_shtml_SIZE, .filedata = AllParameters_shtml},
    {.filename = (unsigned char*)"DataLog.shtml", .filesize = DataLog_shtml_SIZE, .filedata = DataLog_shtml},
    {.filename = (unsigned char*)"EmailConfig.shtml", .filesize = EmailConfig_shtml_SIZE, .filedata = EmailConfig_shtml},
    {.filename = (unsigned char*)"EthernetSetting.shtml", .filesize = EthernetSetting_shtml_SIZE, .filedata = EthernetSetting_shtml},
    {.filename = (unsigned char*)"FlagSetting.shtml", .filesize = FlagSetting_shtml_SIZE, .filedata = FlagSetting_shtml},
    {.filename = (unsigned char*)"MainPage.shtml", .filesize = MainPage_shtml_SIZE, .filedata = MainPage_shtml},
    {.filename = (unsigned char*)"RTCSetting.shtml", .filesize = RTCSetting_shtml_SIZE, .filedata = RTCSetting_shtml},
    {.filename = (unsigned char*)"ScheduleSetting.shtml", .filesize = ScheduleSetting_shtml_SIZE, .filedata = ScheduleSetting_shtml},
    {.filename = (unsigned char*)"SerialSetting.shtml", .filesize = SerialSetting_shtml_SIZE, .filedata = SerialSetting_shtml}
};

struct file css_files[] = {
    {.filename = (unsigned char*)"Main.css", .filesize = Main_css_SIZE, .filedata = Main_css},
    {.filename = (unsigned char*)"Tab.css", .filesize = Tab_css_SIZE, .filedata = Tab_css}
};

struct file image_files[] = {
    {.filename = (unsigned char*)"blue.jpg", .filesize = blue_jpg_SIZE, .filedata = blue_jpg}
};

struct file script_files[] = {
    {.filename = (unsigned char*)"AllParameter.js", .filesize = AllParameter_js_SIZE, .filedata = AllParameter_js},
    {.filename = (unsigned char*)"DataLog.js", .filesize = DataLog_js_SIZE, .filedata = DataLog_js},
    {.filename = (unsigned char*)"Email.js", .filesize = Email_js_SIZE, .filedata = Email_js},
    {.filename = (unsigned char*)"EthernetSetting.js", .filesize = EthernetSetting_js_SIZE, .filedata = EthernetSetting_js},
    {.filename = (unsigned char*)"FlagSetting.js", .filesize = FlagSetting_js_SIZE, .filedata = FlagSetting_js},
    {.filename = (unsigned char*)"MainPage.js", .filesize = MainPage_js_SIZE, .filedata = MainPage_js},
    {.filename = (unsigned char*)"RTCSetting.js", .filesize = RTCsetting_js_SIZE, .filedata = RTCsetting_js},
    {.filename = (unsigned char*)"ScheduleSetting.js", .filesize = ScheduleSetting_js_SIZE, .filedata = ScheduleSetting_js},
    {.filename = (unsigned char*)"SerialSetting.js", .filesize = SerialSetting_js_SIZE, .filedata = SerialSetting_js},
    {.filename = (unsigned char*)"WebSocket.js", .filesize = WebSocket_js_SIZE, .filedata = WebSocket_js}
};



#endif /* FS_INFO_H_ */
