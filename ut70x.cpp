#include "ut70x.h"

#include <QThread>

UT70X::UT70X(QObject* parent)
    : QSerialPort(parent)

{
}

//int UT70X::eeprom()
//{
//    int i;
//    QByteArray buf(4, '\0');
//    int len;

//    for (i = 0; i < 1; i++) {
//        buf[0] = 0xd0;

//        buf[1] = ((i / 16) & 0x0f) | 0x40;
//        buf[2] = (i & 0x0f) | 0x40;
//        buf[3] = 0x0a;

//        //debug
//        buf[1] = 0x40;
//        buf[2] = 0x40;
//        write(buf, 4);
//        thread()->msleep(100);
//        buf.resize(15);
//        if ((len = read(buf.data(), 15)) > 0)
//            dumpdata(reinterpret_cast<unsigned char*>(buf.data()), len);
//        else
//            qCritical("No response\n");
//        thread()->msleep(100);
//    }
//    return 0;
//}

int UT70X::dumpdata(unsigned char* d, int len)
{
    int /*i,*/ ret = 0;
    //    unsigned char xor_;

    qDebug("dump: %d\n", len);
    if (len == 1)
        return 1;
    if (len < 3) {
        qCritical("DEBUG: paket too short (%d), ignoring\n", len);
        ret++;
    }
    if (checksum(d, len)) {
        qCritical("DEBUG: bad checksum, ignoring paket\n");
        ret++;
    }

    //    qDebug("[");
    //    for (xor_ = i = 0; i < len; i++) {
    //        if (d[i] != 0x0a)
    //            qDebug("%02X ", d[i]);
    //        else {
    //            qDebug("0A");
    //            break;
    //        }
    //    }
    //    qDebug("]\n");
    return ret;
}

int UT70X::checksum(unsigned char* d, int len)
{
    int i;
    unsigned char xor_;
    if (len < 3)
        return 1;
    for (xor_ = i = 0; i < len - 2; i++)
        xor_ = xor_ ^ d[i];
    xor_ = xor_ ^ ((xor_ & 0xc0) / 4);
    xor_ &= 0x3f;
    xor_ = xor_ - d[len - 2] + 0x22;
    return xor_;
}

void UT70X::parserawdata(unsigned char* d, int len)
{
    int val;
    if (len < 7)
        return;
    printmode(d, len);
    printsvalue(d, len);
    switch (d[0]) {
        if (len != 11) {
            qCritical("DEBUG: Wrong  paket len=%d 11 wanted\n", len);
            return;
        }
    case (128):
    case (129):
    case (130):
    case (131):
    case (132):
    case (133):
    case (134):
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
    case (135):
    case (136):
        if (len != 15) {
            qCritical("DEBUG: Wrong  paket len=%d 15 wanted\n", len);
            return;
        }

        qDebug(" {%02X%02X%02X%02X%02X%02X%02X%02X}", d[5], d[6], d[7], d[8],
            d[9], d[10], d[11], d[12]);
        break;
        //bargraph
    case (138):
        if (len != 8) {
            qCritical("DEBUG: Wrong  paket len=%d 8 wanted\n", len);
            return;
        }

        qDebug("%d", d[5] - 0x80);
        break;
        //standard read, ADC value in 4/5 positions
    case (137):
    case (139):
    case (140):
    case (141):
    case (142):
    case (143):
    case (144):
    case (145):
    case (146):
    case (147):
    case (148):
    case (150):
        if (len != 12) {
            qCritical("DEBUG: Wrong  paket len=%d 12 wanted\n", len);
            return;
        }
        parseValue();
        break;
        //read only settings
    case (149):
        if (len != 7) {
            qCritical("DEBUG: Wrong  paket len=%d 7 wanted\n", len);
            return;
        }
        break;
    }
    qDebug("\n");
}

void UT70X::printsvalue(unsigned char* d, int len)
{
    //byte2
    if (!(d[2] & 0x80)) {
        qCritical("DEBUG: bit 7 in byte[2] not 1: ");
        dumpdata(d, len);
    }
    const auto manualNAuto_ = d[2] & 0x40;
    const auto range_ = (d[2] / 8) & 7;
    const auto unit_ = d[2] & 7;

    qDebug("%s ", manualNAuto_ ? "Manual" : "AUTO");
    qDebug("range:%d Unit:%d ", range_, unit_);

    sub_range = (d[2] >> 3) & 0x07;
    if ((d[2] & 7) == 4)
        base_range = range(Hz_range_);
    if ((d[2] & 7) == 5)
        base_range = range(PR_range_);

    //byte 3  bits 5,0,1 unknmown, always 0 ?
    if ((d[3] & 0x23)) {
        qCritical("DEBUG: bits (1,5,6) not zero in byte[3]: ");
        dumpdata(d, len);
    }
    if (!(d[3] & 0x80)) {
        qCritical("DEBUG: bit 7 in byte[3] not 1: ");
        dumpdata(d, len);
    }
    m_beep = d[3] & 0x04;
    m_rec = d[3] & 0x40;
    qDebug("%s %s ",
        m_beep ? "BEEP" : "_",
        m_rec ? "REC" : "_");
    qDebug() << MeasType(d[3] & 0x18);

    //byte 4
    if ((d[4] & 0x42)) {
        qCritical("DEBUG: bits (1,5,6) not zero in byte[4]: ");
        dumpdata(d, len);
    }
    if (!(d[4] & 0x80)) {
        qCritical("DEBUG: bit 7 in byte[4] not 1: ");
        dumpdata(d, len);
    }
    m_freq = d[4] & 0x04;
    m_hold = d[4] & 0x01;
    m_lowbat = d[4] & 0x20;
    m_nSign = d[4] & 0x10;
    m_overflow = d[4] & 0x08;
    qDebug() << int(d[4]);
    qDebug(" %s %s %s %s %s",
        m_freq ? "Hz" : "_",
        m_hold ? "HOLD" : "SAMPLE",
        m_lowbat ? "LOWBAT" : "_",
        m_nSign ? "-" : "+",
        m_overflow ? "OVERFLOW" : "_");
}

void UT70X::parseValue()
{
    int range = base_range[sub_range];
    auto d = paket()->data;
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
            dumpdata(data(), this->m_data.length());
        }
        //        if (*d == 0X3F) {
        //            str.append(' ');
        //            continue;
        //        } else if (*d == 0X3E) {
        //            str.append('L');
        //            continue;
        //        } else if (*d >= 0x30 && *d <= 0X39) {
        //            if (range-- == 1)
        //                str.append('.');
        //            str.append(*d);
        //            continue;
        //        }
        //        qCritical("DEBUG, unknown character in ascci adc value %02X: ", *d);
        //        dumpdata(data(), this->m_data.length());
    }
    bool fl;
    if (auto value = str.trimmed().toDouble(&fl); fl) {
        m_value = m_nSign ? -value : +value;
        emit valueChanged(m_value);
    } else {
        qDebug() << "Err" << str;
    }
}

void UT70X::printmode(unsigned char* buf, int len)
{
    base_range = range(Mode(buf[1]));
    if (!base_range) {
        qDebug("?");
        base_range = parMap.at(FAIL_range_).data;
        qCritical("DEBUG, unknown mode %02X: ", buf[1]);
        dumpdata(buf, len);
    } else {
        qDebug() << parMap.at(Mode(buf[1])).description;
    }
}

bool UT70X::initserial(const QString& name)
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
    qDebug() << __FUNCTION__ << errorString();
    return false;
}

int UT70X::mmm()
{
    //    //    int fd, opt;
    //    unsigned char buf[15];

    //    int raw = 0;
    //    unsigned char cmd = 137;

    //    //while ((opt = getopt(argc, argv, "irc:")) != -1) {
    //    switch ('i') {
    //    case 'i':
    //        raw |= 3;
    //        break;
    //    case 'r':
    //        raw |= 1;
    //        break;
    //    case 'c':
    //        raw |= 1;
    //        //        cmd = (strtol(optarg, (char**)NULL, 10)) & 255;
    //        break;
    //    }
    //    //}
    //    //    if (argc < 2) {
    //    //        qCritical( "Usage: %s <device>\n", argv[0]);
    //    //        exit(1);
    //    //    }
    //    //    fd = initserial(argv[argc - 1]);
    //    //    usleep(10000);

    //    if (raw) {
    //        int len;
    //        if ((cmd & 0xfe) == 0xd0) {
    //            return eeprom();
    //        }
    //        buf[0] = cmd;
    //        write(reinterpret_cast<char*>(buf), 1);
    //        thread()->msleep(90);
    //        if ((len = read(reinterpret_cast<char*>(buf), 15)) > 0) {
    //            dumpdata(buf, len, stdout);
    //            if (raw & 2)
    //                parserawdata(buf, len);
    //        } else
    //            qCritical( "No response\n");

    //        restoreserial();
    //        return (len <= 0);
    //    }
    //    return 0;
    unsigned char cmd = 137;
    write(reinterpret_cast<char*>(&cmd), 1);
    waitForReadyRead(100);
    m_data = readAll();
    qDebug() << m_data.toHex('|').toUpper();
    //   qDebug() << "mode\t" << mode();
    //    qDebug() << "range\t" << range(mode());
    dumpdata(reinterpret_cast<unsigned char*>(m_data.data()), m_data.size());
    parserawdata(reinterpret_cast<unsigned char*>(m_data.data()), m_data.size());

    return {};
}

void UT70X::restoreserial()
{
    //    if (flock(fd, LOCK_UN) == -1)
    //        exit(3);
    //    if (close(fd) != 0)
    //        exit(2);
}

void UT70X::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_timerId) {
        mmm();
    }
}
