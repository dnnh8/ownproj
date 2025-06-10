#pragma once
#include "DatabaseManager.h"
#include "RCWACalculator.h"
#include "StructureLoader.h"
#include <vector>
#include <string>
#include <memory>

/**
 * @brief Класс для представления оптической структуры
 */
struct OpticalStructure {
    std::string name;
    std::string substrate;
    std::vector<std::string> materials;
    std::vector<double> thicknesses;
    bool considerBackside = true;
    double angleDegrees = 0.0;
};

/**
 * @brief Основной класс приложения для анализа оптических покрытий
 */
class OpticalCoatingAnalyzer {
public:
    /**
     * @brief Конструктор с инициализацией пути к базе данных
     * @param db_path Путь к файлу SQLite базы данных
     */
    explicit OpticalCoatingAnalyzer(const std::string& db_path);

    /**
     * @brief Расчет спектра для указанной структуры
     * @param structure_name Название структуры из базы данных
     * @param wavelengths Вектор длин волн для расчета (в нанометрах)
     * @return Вектор пар {transmission, reflection} для каждой длины волны
     */
    std::vector<std::pair<double, double>> calculateSpectrum(
        const std::string& structure_name,
        const std::vector<double>& wavelengths);

    /**
     * @brief Оптимизация толщин слоев под целевой спектр
     * @param initial_structure Название начальной структуры
     * @param target_wavelengths Длины волн целевого спектра
     * @param target_values Целевые значения (отражательной способности)
     * @param max_iterations Максимальное число итераций оптимизации
     * @return Оптимизированная структура
     */
    OpticalStructure optimizeStructure(
        const std::string& initial_structure,
        const std::vector<double>& target_wavelengths,
        const std::vector<double>& target_values,
        int max_iterations = 100);

    /**
     * @brief Получение списка доступных структур из БД
     * @return Вектор названий структур
     */
    std::vector<std::string> getAvailableStructures() const;

    /**
     * @brief Экспорт результатов в CSV файл
     * @param filename Имя файла для экспорта
     * @param wavelengths Вектор длин волн
     * @param spectrum Вектор результатов {T, R}
     */
    static void exportToCSV(
        const std::string& filename,
        const std::vector<double>& wavelengths,
        const std::vector<std::pair<double, double>>& spectrum);

    /**
     * @brief Генерация равномерного диапазона длин волн
     * @param start Начальная длина волны (нм)
     * @param end Конечная длина волны (нм)
     * @param step Шаг (нм)
     * @return Вектор длин волн
     */
    static std::vector<double> generateWavelengthRange(
        double start, double end, double step = 1.0);

private:
    DatabaseManager db_;
    std::unique_ptr<RCWACalculator> calculator_;
    std::unique_ptr<StructureLoader> loader_;

    /**
     * @brief Внутренняя функция для расчета одной длины волны
     */
    std::pair<double, double> calculateSingleWavelength(
        const OpticalStructure& structure,
        double wavelength);
};