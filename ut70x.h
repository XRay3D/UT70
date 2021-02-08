#pragma once

#include <QtSerialPort>

#pragma pack(push, 1)

struct Paket {
    Paket() { }
    uint8_t echo; // byt 0
    uint8_t mode; // byt 1
    struct /*Range*/ {
        uint8_t unit : 3;
        uint8_t subRange : 3;
        uint8_t manualNAuto : 1;
        uint8_t _alwaysOne8_2 : 1; //?
    }; // byt 2
    struct /*Flags*/ {
        uint8_t _alwaysZero1_3 : 1; //?
        uint8_t _alwaysZero2_3 : 1; //?
        uint8_t peak1ms : 1;
        uint8_t minMax : 2;
        uint8_t _alwaysZero6_3 : 1; //?
        uint8_t rec : 1;
        uint8_t dummy : 1;
    }; // byt 3
    struct /*Flags*/ {
        uint8_t hold : 1;
        uint8_t _alwaysZero2_4 : 1; //?
        uint8_t freq : 1;
        uint8_t overflow : 1;
        uint8_t nSign : 1;
        uint8_t lowbat : 1;
        uint8_t _alwaysZero7_4 : 1; //?
        uint8_t _alwaysOne8_4 : 1; //?
    }; // byt 4
    uint8_t valData[]; // byt 5
    //uint8_t checksum();
    //uint8_t endOfPaket(); //0xA0
};
#pragma pack(pop)

class UT70X : public QSerialPort {
    Q_OBJECT
    QByteArray m_data;
    unsigned char* data() { return reinterpret_cast<unsigned char*>(m_data.data()); }
    Paket* paket() { return reinterpret_cast<Paket*>(m_data.data()); }

public:
    UT70X(QObject* parent = nullptr);

    enum Mode : uint8_t {
        FAIL_range,
        Hz_range,
        PR_range,
        A_DC = 0xA8, //   A8 1010 1000 A=
        A_AC = 0xA9, //   A9 1010 1001 A~
        mA_DC = 0xB0, //  B0 1011 0000 mA=
        mA_AC = 0xB1, //  B1 1011 0001 mA~
        Dio = 0xD8, //    D8 1101 1000 Dio
        R_C = 0xE0, //    E0 1110 0000 R/C
        F = 0xE1, //      E1 1110 0001 F
        mV = 0xE8, //     E8 1110 1000 mV
        V_DC = 0xF0, //   F0 1111 0000 V=
        V_AC = 0xF8, //   F8 1111 1000 V~
    };
    Q_ENUM(Mode)

    enum SampleType : uint8_t {
        Norm = 0,
        Max = 1,
        Min = 2,
        AVG = 3,
        PeakMin = 0,
        PeakMax = 1,
    };
    Q_ENUM(SampleType)

    enum MeasType : uint8_t {
        Capacity = 0,
        //MAX_ = 1,
        Main = 2,
        //AVG_ = 3,
        Frequency = 4,
        Percent = 5,
    };
    Q_ENUM(MeasType)

    struct Range {
        uint8_t data[8];
        uint8_t operator[](int i) const { return data[i]; }
        QString description;
    };
    static inline const std::map<Mode, Range> parMap {
        { A_AC, { { 1, 2, 0, 0, 0, 0, 0, 0 }, "AC A " } },
        { A_DC, { { 1, 2, 0, 0, 0, 0, 0, 0 }, "DC A " } },
        { Dio, { { 1, 0, 0, 0, 0, 0, 0, 0 }, "D " } },
        { F, { { 1, 2, 3, 1, 2, 3, 0, 0 }, "C " } },
        { FAIL_range, { { 0, 0, 0, 0, 0, 0, 0, 0 }, "?" } },
        { Hz_range, { { 2, 3, 1, 2, 3, 1, 2, 3 }, "Hz" } },
        { PR_range, { { 0, 2, 0, 0, 0, 0, 0, 0 }, "%" } },
        { R_C, { { 3, 1, 2, 3, 1, 2, 2, 0 }, "R " } }, //last active range is nS on 5 digit display (800nS)
        { V_AC, { { 3, 1, 2, 3, 4, 0, 0, 0 }, "AC V " } },
        { V_DC, { { 1, 2, 3, 4, 0, 0, 0, 0 }, "DC V " } },
        { mA_AC, { { 2, 3, 0, 0, 0, 0, 0, 0 }, "DC mA " } },
        { mA_DC, { { 2, 3, 0, 0, 0, 0, 0, 0 }, "AC mA " } },
        { mV, { { 2, 3, 0, 0, 0, 0, 0, 0 }, "DC mV " } },
    };
    Mode mode() { return static_cast<Mode>(reinterpret_cast<uint8_t*>(m_data.data())[1]); };
    const uint8_t* range(Mode mode) const
    {
        if (parMap.find(mode) != parMap.end())
            return parMap.at(mode).data; /*[(data[2] >> 3) & 0x7]*/
        return nullptr;
    }
    const uint8_t* baseRange; // value * 10^base_range

    int checkSum(uint8_t* d, int len);
    bool checkData(unsigned char* d, int len);
    //  int eeprom();
    bool initSerial(const QString& n);
    void measure();
    void parseRawData(unsigned char* d, int len);
    void printMode(unsigned char* buf, int len);
    void printsValue(unsigned char* d, int len);
    void parseValue();

    double value() { return m_value; }

    void startMs(int msec)
    {
        if (m_timerId)
            killTimer(m_timerId);
        m_timerId = startTimer(msec);
    }
    void stop()
    {
        if (m_timerId)
            killTimer(m_timerId);
        m_timerId = 0;
    }

private:
    double m_value = 0.0;
    double m_unit = 0.0;
    int m_timerId = 0;
    char m_pol = '\0';

signals:
    void valueChanged(double);
    void unit(const QString&);
    void sign(const QString&);

    // QObject interface
protected:
    void timerEvent(QTimerEvent* event) override;
};
