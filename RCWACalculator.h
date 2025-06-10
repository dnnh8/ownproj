#pragma once
#include "DatabaseManager.h"
#include <QObject>
#include <QVector>
#include <complex>

class RCWACalculator : public QObject {
    Q_OBJECT
public:
    explicit RCWACalculator(DatabaseManager& db, QObject* parent = nullptr);

public slots:
    void calculateSpectrum(const QString& structureName,
        const QVector<double>& wavelengths);

signals:
    void calculationComplete(const QVector<double>& wavelengths,
        const QVector<double>& transmission,
        const QVector<double>& reflection);

private:
    DatabaseManager& m_db;

    std::complex<double> getRefractiveIndex(const QString& material, double wavelength);
    std::pair<double, double> calculateSingleWavelength(double wavelength,
        const QString& substrate,
        const QVector<QString>& materials,
        const QVector<double>& thicknesses);
};