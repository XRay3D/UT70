/* ut70d multimeter communication software

Copyright (c) 2010 Peter Popovec <popovec@fei.tuke.sk>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
USA.

*/

/*
DEBUG switches
-r      raw mode, send command (default  command=137) to device and print raw output
-c<num> send command <num> to device, implies raw. If no argument, command "0" is send
-i      if set, parse paket and print internals, imples raw
*/
#pragma once

#include <QtSerialPort>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <sys/file.h>
//#include <sys/ioctl.h>
//#include <termios.h>
//#include <unistd.h>

#pragma pack(push, 1)

struct Paket {
    Paket() { }
    uint8_t echo; // byt 1
    uint8_t mode; // byt 2
    uint8_t range; // byt 3
    //uint8_t mode; // byt 4
    uint8_t data[]; // byt 5
    uint8_t checksum();
    uint8_t endOfPaket(); //0xA0

    /*
byt 1  MODE
-----
F8 1111 1000  V~
F0 1111 0000  V=
E8 1110 1000  mV
E0 1110 0000  R/C
E1 1110 0001  F
D8 1101 1000  Dio
A8 1010 1000  A=
A9 1010 1001  A~
B0 1011 0000  mA=
B1 1011 0001  mA~

byt 2  RANGE and UNITS
-----
bit 7  always 1 ?
bit 6  manual/auto
bit 5,4,3 range ..
bit 2,1,0
    0 = F-capacitance, 1=? 2= V,A,OHM 3 =?
        4 = Hz frequency, 5 = %  6= ? 7 = ?


byt 3
-----
0 always 0 ?
1 always 0 ?
2 1=beeper on
4,3	00 - none 01 MAX 10 MIN  11 AVG
5 always 0 ?
6 "REC"
7 always 1 ?


byt4  binary maped:
----
0   1 = hold 0 = sample
1   always 0 ?
2   1 = Hz   0 = no Hz
3   1 = overflow (0L on display)
4   1 = +    0 = -
5   1 = low bat
6   always 0 ?
7   always 1 ?

*/
};
#pragma pack(pop)

class UT70X : public QSerialPort {
    Q_OBJECT
public:
    UT70X(QObject* parent = nullptr);

    enum Mode : uint8_t {
        V_AC = 0xF8, //  F8 1111 1000  V~
        V_DC = 0xF0, //  F0 1111 0000  V=
        mV = 0xE8, //    E8 1110 1000  mV
        R_C = 0xE0, //   E0 1110 0000  R/C
        F = 0xE1, //     E1 1110 0001  F
        Dio = 0xD8, //   D8 1101 1000  Dio
        A_DC = 0xA8, //  A8 1010 1000  A=
        A_AC = 0xA9, //  A9 1010 1001  A~
        mA_DC = 0xB0, // B0 1011 0000  mA=
        mA_AC = 0xB1, // B1 1011 0001  mA~
        FAIL_range,
        Hz_range,
        PR_range,
    };
    Q_ENUM(Mode)

    struct Range {
        uint8_t data[8];
        uint8_t& operator[](int i) { return data[i]; }
        uint8_t operator[](int i) const { return data[i]; }
    };
    static inline const std::map<Mode, Range> parMap {
        { V_AC, { 3, 1, 2, 3, 4, 0, 0, 0 } },
        { V_DC, { 1, 2, 3, 4, 0, 0, 0, 0 } },
        { mV, { 2, 3, 0, 0, 0, 0, 0, 0 } },
        { R_C, { 3, 1, 2, 3, 1, 2, 2, 0 } }, //last active range is nS on 5 digit display (800nS)
        { F, { 1, 2, 3, 1, 2, 3, 0, 0 } },
        { Dio, { 1, 0, 0, 0, 0, 0, 0, 0 } },
        { A_DC, { 1, 2, 0, 0, 0, 0, 0, 0 } },
        { A_AC, { 1, 2, 0, 0, 0, 0, 0, 0 } },
        { mA_DC, { 2, 3, 0, 0, 0, 0, 0, 0 } },
        { mA_AC, { 2, 3, 0, 0, 0, 0, 0, 0 } },
        { FAIL_range, { 0, 0, 0, 0, 0, 0, 0, 0 } },
        { Hz_range, { 2, 3, 1, 2, 3, 1, 2, 3 } },
        { PR_range, { 0, 2, 0, 0, 0, 0, 0, 0 } },
    };

    // static constexpr int F8_range[] = { 3, 1, 2, 3, 4, 0, 0, 0 };
    // static constexpr int F0_range[] = { 1, 2, 3, 4, 0, 0, 0, 0 };
    // static constexpr int E8_range[] = { 2, 3, 0, 0, 0, 0, 0, 0 };
    // static constexpr int E0_range[] = { 3, 1, 2, 3, 1, 2, 2, 0 }; //last active range is nS on 5 digit display (800nS)
    // static constexpr int E1_range[] = { 1, 2, 3, 1, 2, 3, 0, 0 };
    // static constexpr int D8_range[] = { 1, 0, 0, 0, 0, 0, 0, 0 };
    // static constexpr int A8_range[] = { 1, 2, 0, 0, 0, 0, 0, 0 };
    // static constexpr int A9_range[] = { 1, 2, 0, 0, 0, 0, 0, 0 };
    // static constexpr int B0_range[] = { 2, 3, 0, 0, 0, 0, 0, 0 };
    // static constexpr int B1_range[] = { 2, 3, 0, 0, 0, 0, 0, 0 };
    // static constexpr int FAIL_range[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    // static constexpr int Hz_range[] = { 2, 3, 1, 2, 3, 1, 2, 3 };
    // static constexpr int PR_range[] = { 0, 2, 0, 0, 0, 0, 0, 0 };

    int* base_range; // value * 10^base_range
    int sub_range; //

    int checksum(unsigned char* d, int len);
    int dumpdata(unsigned char* d, int len, FILE* fd);
    int eeprom();
    bool initserial(const QString& n);
    int mmm();
    void bits(int b);
    void parserawdata(unsigned char* d, int len);
    void printmode(unsigned char* buf, int len);
    void printsvalue(unsigned char* d, int len);
    void printxvalue(unsigned char* d, int len, int dot);
    void restoreserial();
};
