#include "RCWACalculator.h"
#include <QtConcurrent>
#include <algorithm>

RCWACalculator::RCWACalculator(DatabaseManager& db, QObject* parent)
    : QObject(parent), m_db(db) {
}

void RCWACalculator::calculateSpectrum(const QString& structureName,
    const QVector<double>& wavelengths) {
    QtConcurrent::run([=]() {
        // 1. Загрузка структуры из БД (оптимизированный запрос)
        auto [substrate, materials, thicknesses] = m_db.loadStructure(structureName);

        // 2. Параллельный расчет для каждой длины волны
        QVector<double> transmission(wavelengths.size());
        QVector<double> reflection(wavelengths.size());

        std::transform(std::execution::par, wavelengths.begin(), wavelengths.end(),
            transmission.begin(), reflection.begin(),
            [&](double wl) {
                auto [T, R] = calculateSingleWavelength(wl, substrate, materials, thicknesses);
                return std::make_pair(T, R);
            });

        // 3. Отправка результатов в GUI поток
        QMetaObject::invokeMethod(this, [=]() {
            emit calculationComplete(wavelengths, transmission, reflection);
            }, Qt::QueuedConnection);
        });
}

std::pair<double, double> RCWACalculator::calculateSingleWavelength(
    double wavelength, const QString& substrate,
    const QVector<QString>& materials,
    const QVector<double>& thicknesses)
{
    using Complex = std::complex<double>;
    const Complex I(0.0, 1.0);

    // Получение показателей преломления
    Complex ns = getRefractiveIndex(substrate, wavelength);
    Complex n0(1.0, 0.0); // воздух

    // Ваш RCWA алгоритм (адаптированный)
    double k0 = 2.0 * M_PI / wavelength;
    // ... остальная реализация метода ...

    return { T_avg, R_avg };
}