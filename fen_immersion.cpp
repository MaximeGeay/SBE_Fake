#include "fen_immersion.h"
#include "ui_fen_immersion.h"
#include <QDebug>

fen_Immersion::fen_Immersion(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::fen_Immersion)
{
    ui->setupUi(this);
}

fen_Immersion::~fen_Immersion()
{
    delete ui;
}

void fen_Immersion::setValue(double dImmersion)
{
    if(qRound(dImmersion)>9&&qRound(dImmersion)<=99)
        ui->lcd_Immersion->setDigitCount(5);
    else if (qRound(dImmersion)>99) {
        ui->lcd_Immersion->setDigitCount(6);
    }
    else {
        ui->lcd_Immersion->setDigitCount(4);
    }

    ui->lcd_Immersion->display(QString::number(dImmersion,'f',2));
}
