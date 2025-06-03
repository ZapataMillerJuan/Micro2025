#include "qform1.h"
#include "ui_qform1.h"

QForm1::QForm1(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QForm1)
{
    ui->setupUi(this);
    QSerialPort1=new QSerialPort(this);
    ui->comboBox->installEventFilter(this);

    ui->comboBox_2->addItem("ALIVE", 0xF0);
    ui->comboBox_2->addItem("FIRMWARE", 0xF1);
    ui->comboBox_2->addItem("LEDS", 0x10);
    ui->comboBox_2->addItem("PULSADORES", 0x12);
    ui->comboBox_2->addItem("SERVOANGLE", 0xA2);
    ui->comboBox_2->addItem("DISTANCIA", 0xA3);
    ui->comboBox_2->addItem("ANALOGSENSORS", 0xA0);
    header=0;
    QTimer1 = new QTimer(this);
    QTimer2 = new QTimer(this);
    QTimer3 = new QTimer(this);
    connect(QSerialPort1,&QSerialPort::readyRead, this,&QForm1::OnRxChar);
    connect(QTimer1, &QTimer::timeout, this, &QForm1::OnQTimer1);
    connect(QTimer2, &QTimer::timeout, this, &QForm1::OnQTimer2);
    connect(QTimer3, &QTimer::timeout, this, &QForm1::OnQTimer3);
    QPaintBox1 = new QPaintBox(0, 0 ,ui->widget);
    angle=0;
    aux=0;
}


QForm1::~QForm1()
{
    delete ui;
}

bool QForm1::eventFilter(QObject *watched, QEvent *event){
    if(watched == ui->comboBox) {
        if (event->type() == QEvent::MouseButtonPress) {
            ui->comboBox->clear();
            QSerialPortInfo SerialPortInfo1;

            for(int i=0;i<SerialPortInfo1.availablePorts().count();i++)
                ui->comboBox->addItem(SerialPortInfo1.availablePorts().at(i).portName());

            return QMainWindow::eventFilter(watched, event);
        }
        else {
            return false;
        }
    }
    else{
         return QMainWindow::eventFilter(watched, event);
    }
}

void QForm1::OnRxChar(){
    int count;
    uint8_t *buf;
    QString strHex;

    count = QSerialPort1->bytesAvailable();
    if(count <= 0)
        return;

    buf = new uint8_t[count];
    QSerialPort1->read((char *)buf, count);

    strHex = "<-- 0x";
    for (int a=0; a<count; a++) {
        strHex = strHex + QString("%1").arg(buf[a], 2, 16, QChar('0')).toUpper();
    }
     ui->plainTextEdit->appendPlainText(strHex);
    for (int i=0; i<count; i++) {
        strHex = strHex + QString("%1").arg(buf[i], 2, 16, QChar('0')).toUpper();

        switch(header){
        case 0:
            if(buf[i] == 'U'){
                header = 1;
                tmoRX = 5;
            }
            break;
        case 1:
            if(buf[i] == 'N')
                header = 2;
            else{
                header = 0;
                i--;
            }
            break;
        case 2:
            if(buf[i] == 'E')
                header = 3;
            else{
                header = 0;
                i--;
            }
            break;
        case 3:
            if(buf[i] == 'R')
                header = 4;
            else{
                header = 0;
                i--;
            }
            break;
        case 4:
            nBytes = buf[i];
            header = 5;
            break;
        case 5:
            if(buf[i] == ':'){
                header = 6;
                index = 0;
                cks = 'U' ^ 'N' ^ 'E' ^ 'R' ^ ':' ^ nBytes;
            }
            else{
                header = 0;
                i--;
            }
            break;
        case 6:
            nBytes--;
            if(nBytes > 0){
                rxBuf[index++] = buf[i];
                cks ^= buf[i];
            }
            else{
                header = 0;
                if(cks == buf[i])
                    DecodeCmd(rxBuf);
                else
                    ui->plainTextEdit->appendPlainText("ERROR CHECKSUM");
            }
            break;
        }
    }


    delete [] buf;

}



void QForm1::SendCMD(uint8_t *buf, uint8_t length){
    uint8_t tx[24];
    uint8_t cks, i;
    QString strHex;
    _work w;

    if(!QSerialPort1->isOpen())
        return;

    w.i32 = -1000;

    tx[7] = w.u8[0];
    tx[8] = w.u8[1];
    tx[9] = w.u8[2];
    tx[10] = w.u8[3];


    tx[0] = 'U';
    tx[1] = 'N';
    tx[2] = 'E';
    tx[3] = 'R';
    tx[4] = length + 1;
    tx[5] = ':';

    memcpy(&tx[6], buf, length);

    cks = 0;
    for (i=0; i<(length+6); i++) {
        cks ^= tx[i];
    }

    tx[i] = cks;

    strHex = "--> 0x";
    for (int i=0; i<length+7; i++) {
        strHex = strHex + QString("%1").arg(tx[i], 2, 16, QChar('0')).toUpper();
    }

    ui->plainTextEdit->appendPlainText(strHex);

    QSerialPort1->write((char *)tx, length+7);
}



void QForm1::on_pushButton_clicked()
{
    if(QSerialPort1->isOpen()){
        QSerialPort1->close();
        ui->pushButton->setText("OPEN");
    }
    else{
        if(ui->comboBox->currentText() == "")
            return;

        QSerialPort1->setPortName(ui->comboBox->currentText());
        QSerialPort1->setBaudRate(115200);
        QSerialPort1->setParity(QSerialPort::NoParity);
        QSerialPort1->setDataBits(QSerialPort::Data8);
        QSerialPort1->setStopBits(QSerialPort::OneStop);
        QSerialPort1->setFlowControl(QSerialPort::NoFlowControl);

        if(QSerialPort1->open(QSerialPort::ReadWrite)){
            ui->pushButton->setText("CLOSE");
        }
        else
            QMessageBox::information(this, "Serial PORT", "ERROR. Opening PORT");
    }
}

void QForm1::OnQTimer1(){
    /*uint8_t cmd, buf[24];

    int n;
    cmd = DISTANCE;
    n = 1;
    buf[0] = cmd;
    SendCMD(buf,n);*/
    uint8_t cmd, buf[24];

    int n;
    cmd = DISTANCE;
    n = 1;
    buf[0] = cmd;
    SendCMD(buf,n);
}

void QForm1::OnQTimer2(){
    uint8_t cmd, buf[24];
    int n;
    cmd = RADAR;
    n = 2;
    buf[0] = cmd;
    buf[1] = angle;
    SendCMD(buf,n);
    if(angle == 0){
        angle++;
        aux=0;
    }
    else if(angle==180){
        aux=1;
        angle--;
    }else if(angle<180 && aux==0){
        angle++;
    }else if(angle<180 && aux==1){
        angle--;
    }
}

void QForm1::OnQTimer3(){
    QPainter paint(QPaintBox1->getCanvas());

    QPen pen;

    QBrush brush;

    QPoint center,extreme;

    center.setX(ui->widget->width()/2);

    center.setY(ui->widget->height());

    //extreme.setX(0);

    //extreme.setY(ui->widget->height());

    brush.setColor(Qt::black);

    brush.setStyle(Qt::SolidPattern);

    paint.setBrush(brush);

    //paint.drawRect(0,0,ui->widget->width(),ui->widget->height());

    pen.setWidth(3);

    pen.setColor(Qt::darkGreen);

    paint.setPen(pen);

    //brush.setColor(Qt::darkCyan);

    //brush.setStyle(Qt::SolidPattern);

    paint.setBrush(Qt::NoBrush);



    paint.save();


    paint.translate(ui->widget->width()/2,ui->widget->height());

    paint.rotate(-angle);

    paint.setOpacity(0.1);
    paint.opacity();
    pen.setColor(Qt::red);
    paint.setPen(pen);
    paint.drawLine(0,0,400,0);
    paint.setOpacity(0.2);
    paint.opacity();
    pen.setColor(Qt::darkGreen);
    paint.setPen(pen);

    paint.drawLine(0,0,distance*10,0);


    paint.restore();


    paint.setOpacity(1);
    paint.opacity();
    paint.drawEllipse(center,100,100);

    paint.drawEllipse(center,200,200);

    paint.drawEllipse(center,300,300);

    paint.drawEllipse(center,400,400);
    QPaintBox1->update();
}

void QForm1::on_pushButton_3_clicked()
{
    ui->plainTextEdit->clear();
    ui->widget->clearMask();
}


void QForm1::on_pushButton_2_clicked()
{
    uint8_t cmd, buf[24];
//    _work w;
    int n;
//    bool ok;
    QString strHex;


    if(ui->comboBox_2->currentText() == "")
        return;

    cmd = ui->comboBox_2->currentData().toInt();
    ui->plainTextEdit->appendPlainText("0x" + (QString("%1").arg(cmd, 2, 16, QChar('0'))).toUpper());

    n = 0;
    switch (cmd) {
        case ALIVE://ALIVE   PC=>MBED 0xF0 ;  MBED=>PC 0xF0 0x0D
            n = 1;
            break;
        case FIRMWARE://FIRMWARE   PC=>MBED 0xF1 ;  MBED=>PC 0xF1 FIRMWARE
            n = 1;
            break;
        case LEDS://LEDS   PC=>MBED 0x10 LEDS_A_MODIFICAR VALOR_LEDS ;  MBED=>PC 0x10 ESTADO_LEDS
            n = 3;
            break;
        case PULSADORES://PULSADORES   PC=>MBED 0x12 ;  MBED=>PC 0x12 VALOR PULSADORES
            n = 1;
            break;
        case SERVOANGLE:
            n = 2;
            break;
        case DISTANCE://DISTANCE       PC=>MBED 0xA3;; MBED=>PC 0xA3
            n=1;
            break;
        case ANALOGSENSORS:
            n=1;
            break;
        case RADAR:
            n=2;
            break;
        default:
        ;
    }

    if(n==1){
        buf[0] = cmd;
        SendCMD(buf, n);
    }
    if(n==2){
        buf[0] = cmd;
        buf[1] = angle;
        SendCMD(buf, n);
    }



}


void QForm1::DecodeCmd(uint8_t *rxBuf){
    QString str;
    _work w;
    switch (rxBuf[0]) {
    case LEDS:

        break;
     case PULSADORES:
            str = "SW3: ";
            if(rxBuf[1] & 0x08)
                str = str + "HIGH";
            else
                str = str + "LOW";
            str = str + " - SW2: ";
            if(rxBuf[1] & 0x04)
                str = str + "HIGH";
            else
                str = str + "LOW";
            str = str + " - SW1: ";
            if(rxBuf[1] & 0x02)
                str = str + "HIGH";
            else
                str = str + "LOW";
            str = str + " - SW0: ";
            if(rxBuf[1] & 0x01)
                str = str + "HIGH";
            else
                str = str + "LOW";
            ui->plainTextEdit->appendPlainText(str);
        break;
    case ALIVE:
        if(rxBuf[1] == ACKNOWLEDGE)
            ui->plainTextEdit->appendPlainText("I'M ALIVE");
        break;
    case SERVOANGLE:
        ui->plainTextEdit->appendPlainText("ANGULO:");
        ui->plainTextEdit->appendPlainText(QString("%1").arg(rxBuf[1], 4 , 10, QChar('0')));
        break;
    case DISTANCE:
        w.u8[0]    = rxBuf[1];
        w.u8[1]    = rxBuf[2];
        w.u8[2]    = rxBuf[3];
        w.u8[3]    = rxBuf[4];
        distance = w.u32;
        distance = distance/58;
        //if(distance>50 || distance<4){
        //    distance = 0;
        //}
        ui->plainTextEdit->appendPlainText("DISTANCIA EN CM:");
        ui->plainTextEdit->appendPlainText(QString("%1").arg(distance, 4 , 10, QChar('0')));
        break;
    case RADAR:
        w.u8[0]    = rxBuf[1];
        w.u8[1]    = rxBuf[2];
        w.u8[2]    = rxBuf[3];
        w.u8[3]    = rxBuf[4];
        distance = w.u32;
        distance = distance/58;
        ui->plainTextEdit->appendPlainText("DISTANCIA EN CM:");
        ui->plainTextEdit->appendPlainText(QString("%1").arg(distance, 4 , 10, QChar('0')));
        ui->plainTextEdit->appendPlainText("ANGULO:");
        ui->plainTextEdit->appendPlainText(QString("%1").arg(rxBuf[5], 4 , 10, QChar('0')));
        break;
    case ANALOGSENSORS:
        w.u32 = 0;
        w.u8[0]    = rxBuf[1];
        w.u8[1]    = rxBuf[2];
        irSensor[0] = w.u16[0];
        w.u32 = 0;
        w.u8[0]    = rxBuf[3];
        w.u8[1]    = rxBuf[4];
        irSensor[1] = w.u16[0];
        w.u32 = 0;
        w.u8[0]    = rxBuf[5];
        w.u8[1]    = rxBuf[6];
        irSensor[2] = w.u16[0];
        ui->plainTextEdit->appendPlainText("IR LEFT:");
        ui->plainTextEdit->appendPlainText(QString("%1").arg(irSensor[0], 4 , 10, QChar('0')));
        ui->plainTextEdit->appendPlainText("IR CENTER:");
        ui->plainTextEdit->appendPlainText(QString("%1").arg(irSensor[1], 4 , 10, QChar('0')));
        ui->plainTextEdit->appendPlainText("IR RIGHT:");
        ui->plainTextEdit->appendPlainText(QString("%1").arg(irSensor[2], 4 , 10, QChar('0')));
        break;
    case UNKNOWNCOMANND:
        ui->plainTextEdit->appendPlainText("NO CMD");
        break;
    }
}

void QForm1::on_pushButton_4_clicked()
{
    if(QTimer1->isActive()){
        QTimer1->stop();
    }else{
        QTimer1->start(60);
    }
}


void QForm1::on_horizontalSlider_sliderReleased()
{

}


void QForm1::on_pushButton_5_clicked()
{
    uint8_t cmd, buf[24];

    int n;
    cmd = ANALOGSENSORS;
    n = 1;
    buf[0] = cmd;
    SendCMD(buf,n);
}


void QForm1::on_horizontalSlider_valueChanged(int value)
{
    angle = ui->horizontalSlider->value();
}


void QForm1::on_pushButton_6_clicked()
{
    if(QTimer3->isActive()){
        QTimer3->stop();
    }else{
        QTimer3->start(10);
    }
    /*if(QTimer1->isActive()){
        QTimer1->stop();
    }else{
        QTimer1->start(40);
    }*/
    if(QTimer2->isActive()){
        QTimer2->stop();
    }else{
        QTimer2->start(200);
    }

}

