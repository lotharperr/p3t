/***************************************************************************
*  Copyright 2015 Lothar Perr
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*  http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
***************************************************************************/

#pragma once

enum ProtocolType {TCP, UDP};
#define DEFAULTPORT 14057
#define DEFAULTPACKETSPERBLOCK 1
#define DEFAULTBLOCKDELAY 0
#define DEFAULTREFRESHRATE 10
#define DEFAULTSMOOTHCOUNT 4
#define strTCP _T("TCP")
#define strUDP _T("UDP")
#define REFRESHRATE 2
#define SMOOTHTIME 250
#define rxmaxDescription_default _T("RX/s max: 0")
#define rxmaxDescription _T("RX/s max: ")

