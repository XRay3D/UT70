#include "ut70x.h"

// Peter Popovec <popovec@fei.tuke.sk>

#include <QThread>

UT70X::UT70X(QObject* parent)
    : QSerialPort(parent)
{
}

bool UT70X::checkData(unsigned char* d, int len)
{
    qDebug("dump: %d\n", len);
    do {
        if (len == 1)
            break;
        if (len < 3) {
            qCritical("DEBUG: paket too short (%d), ignoring\n", len);
            break;
        }
        if (checkSum(d, len)) {
            qCritical("DEBUG: bad checksum, ignoring paket\n");
            break;
        }
        return true;
    } while (0);
    return {};
}

int UT70X::checkSum(uint8_t* d, int len)
{
    uint8_t xor_ {};
    if (len < 3)
        return 1;
    for (int i = 0; i < len - 2; i++)
        xor_ = xor_ ^ d[i];
    xor_ = xor_ ^ ((xor_ & 0xc0) / 4);
    xor_ &= 0x3f;
    xor_ = xor_ - d[len - 2] + 0x22;
    return xor_;
}

void UT70X::parseRawData(unsigned char* d, int len)
{
    int val;
    if (len < 7)
        return;
    printMode(d, len);
    printsValue(d, len);
    switch (d[0]) {
        if (len != 11) {
            qCritical("DEBUG: Wrong  paket len=%d 11 wanted\n", len);
            return;
        }
    case 128:
    case 129:
    case 130:
    case 131:
    case 132:
    case 133:
    case 134:
        val = d[5] & 0x3f;
        val *= 64;
        val |= d[6] & 0x3f;
        val *= 64;
        val |= d[7] & 0x3f;
        val *= 64;
        val |= d[8] & 0x3f;
        //      if (d[5] & 20)
        //      val = -((~val) & 0xffffff);
        qDebug("%d <%d>", val, val / 4096);
        break;
        //unknown data
    case 135:
    case 136:
        if (len != 15) {
            qCritical("DEBUG: Wrong  paket len=%d 15 wanted\n", len);
            return;
        }
        qDebug(" {%02X%02X%02X%02X%02X%02X%02X%02X}", d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12]);
        break;
        //bargraph
    case 138:
        if (len != 8) {
            qCritical("DEBUG: Wrong  paket len=%d 8 wanted\n", len);
            return;
        }
        qDebug("%d", d[5] - 0x80);
        break;
        //standard read, ADC value in 4/5 positions
    case 137:
    case 139:
    case 140:
    case 141:
    case 142:
    case 143:
    case 144:
    case 145:
    case 146:
    case 147:
    case 148:
    case 150:
        if (len != 12) {
            qCritical("DEBUG: Wrong  paket len=%d 12 wanted\n", len);
            return;
        }
        parseValue();
        break;
        //read only settings
    case 149:
        if (len != 7) {
            qCritical("DEBUG: Wrong  paket len=%d 7 wanted\n", len);
            return;
        }
        break;
    }
    qDebug("\n");
}

void UT70X::printsValue(unsigned char* d, int len)
{
    //byte2
    if (!paket()->_alwaysOne8_2) {
        qCritical("DEBUG: bit 7 in byte[2] not 1: ");
        checkData(d, len);
    }

    qDebug() << "0" << (paket()->manualNAuto ? "Manual" : "AUTO");
    qDebug() << "0 Range: " << paket()->subRange;
    qDebug() << "0 Unit: " << paket()->unit;

    if (paket()->unit == Frequency)
        baseRange = range(Hz_range);
    if (paket()->unit == Percent)
        baseRange = range(PR_range);

    //byte 3  bits 5,0,1 unknmown, always 0 ?
    //    if ((d[3] & 0x23)) {
    //        qCritical("DEBUG: bits (1,5,6) not zero in byte[3]: ");
    //        checkData(d, len);
    //    }
    //    if (!(d[3] & 0x80)) {
    //        qCritical("DEBUG: bit 7 in byte[3] not 1: ");
    //        checkData(d, len);
    //    }

    qDebug() << "0" << (paket()->peak1ms ? "Peak 1ms" : "_");
    qDebug() << "1" << (paket()->rec ? "REC" : "_");
    qDebug() << "2" << SampleType(paket()->minMax);

    //byte 4
    //    if ((d[4] & 0x42)) {
    //        qCritical("DEBUG: bits (1,5,6) not zero in byte[4]: ");
    //        checkData(d, len);
    //    }
    //    if (!(d[4] & 0x80)) {
    //        qCritical("DEBUG: bit 7 in byte[4] not 1: ");
    //        checkData(d, len);
    //    }
    qDebug() << m_data.mid(4, 1).toHex();
    qDebug() << "3" << (paket()->freq ? "Hz" : "_");
    qDebug() << "4" << (paket()->hold ? "HOLD" : "SAMPLE");
    qDebug() << "5" << (paket()->lowbat ? "LOWBAT" : "_");
    qDebug() << "6" << (paket()->nSign ? "-" : "+");
    qDebug() << "7" << (paket()->overflow ? "OVERFLOW" : "_");
}

void UT70X::parseValue()
{
    int range = baseRange[paket()->subRange];
    auto d = paket()->valData;
    QString str;
    str.reserve(10);
    for (int i = 6; --i; ++d) {
        switch (*d) {
        case 0X3F:
            str.append(' ');
            continue;
        case 0X3E:
            str.append('L');
            continue;
        case 0x30:
        case 0X31:
        case 0X32:
        case 0X33:
        case 0X34:
        case 0X35:
        case 0X36:
        case 0X37:
        case 0X38:
        case 0X39:
            if (!range--)
                str.append('.');
            str.append(*d);
            continue;
        default:
            qCritical("DEBUG, unknown character in ascci adc value %02X: ", *d);
            checkData(data(), this->m_data.length());
        }
    }
    bool fl;
    if (auto value = str.trimmed().toDouble(&fl); fl) {
        m_value = paket()->nSign ? -value : +value;
        emit valueChanged(m_value);
    } else {
        qDebug() << "Err" << str;
    }
}

void UT70X::printMode(unsigned char* buf, int len)
{
    baseRange = range(Mode(buf[1]));
    if (!baseRange) {
        qDebug("?");
        baseRange = parMap.at(FAIL_range).data;
        qCritical("DEBUG, unknown mode %02X: ", buf[1]);
        checkData(buf, len);
    } else {
        qDebug() << parMap.at(Mode(buf[1])).description;
    }
}

bool UT70X::initSerial(const QString& name)
{
    stop();
    close();

    setPortName(name);

    if (open(ReadWrite)) {
        setBaudRate(Baud9600);
        setDataBits(Data8);
        setFlowControl(NoFlowControl);
        setDataTerminalReady(true);
        setRequestToSend(false);
        startMs(500);
        return true;
    }
    qDebug() << errorString();
    return false;
}

void UT70X::measure()
{
    unsigned char cmd = 137;
    write(reinterpret_cast<char*>(&cmd), 1);
    waitForReadyRead(100);
    m_data = readAll();
    qDebug() << m_data.toHex('|').toUpper();
    if (checkData(reinterpret_cast<unsigned char*>(m_data.data()), m_data.size()))
        parseRawData(reinterpret_cast<unsigned char*>(m_data.data()), m_data.size());
}

void UT70X::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_timerId)
        measure();
}
