#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_spectrum.h"

class spectrum : public QMainWindow
{
    Q_OBJECT

public:
    spectrum(QWidget *parent = nullptr);
    ~spectrum();

private:
    Ui::spectrumClass ui;
};

