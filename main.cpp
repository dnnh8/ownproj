#include "main.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

OpticalCoatingAnalyzer::OpticalCoatingAnalyzer(const std::string& db_path)
    : db_(db_path),
    calculator_(std::make_unique<RCWACalculator>(db_)),
    loader_(std::make_unique<StructureLoader>(db_)) {
}

std::vector<std::pair<double, double>> OpticalCoatingAnalyzer::calculateSpectrum(
    const std::string& structure_name,
    const std::vector<double>& wavelengths) {

    // Загрузка структуры из БД
    OpticalStructure structure = loader_->load(structure_name);
    structure.name = structure_name;

    // Подготовка результатов
    std::vector<std::pair<double, double>> results;
    results.reserve(wavelengths.size());

    // Расчет для каждой длины волны
    for (double wl : wavelengths) {
        results.push_back(calculateSingleWavelength(structure, wl));
    }

    return results;
}

OpticalStructure OpticalCoatingAnalyzer::optimizeStructure(
    const std::string& initial_structure,
    const std::vector<double>& target_wavelengths,
    const std::vector<double>& target_values,
    int max_iterations) {

    // Загрузка начальной структуры
    OpticalStructure structure = loader_->load(initial_structure);

    // Проверка входных данных
    if (target_wavelengths.size() != target_values.size()) {
        throw std::invalid_argument("Target wavelengths and values must have same size");
    }

    // Простая реализация градиентного спуска для демонстрации
    const double learning_rate = 0.1;
    const double tolerance = 1e-4;

    for (int iter = 0; iter < max_iterations; ++iter) {
        // Расчет текущего спектра
        auto current_spectrum = calculateSpectrum(initial_structure, target_wavelengths);

        // Расчет градиента
        std::vector<double> gradients(structure.thicknesses.size(), 0.0);
        double total_error = 0.0;

        for (size_t i = 0; i < target_wavelengths.size(); ++i) {
            double error = current_spectrum[i].second - target_values[i];
            total_error += error * error;

            // Конечные разницы для вычисления градиента
            for (size_t j = 0; j < structure.thicknesses.size(); ++j) {
                double original_thickness = structure.thicknesses[j];

                // Прямая разница
                structure.thicknesses[j] += 1.0;
                auto perturbed_spectrum = calculateSpectrum(initial_structure, { target_wavelengths[i] });
                double perturbed_value = perturbed_spectrum[0].second;
                structure.thicknesses[j] = original_thickness;

                gradients[j] += 2 * error * (perturbed_value - current_spectrum[i].second);
            }
        }

        // Обновление толщин
        for (size_t j = 0; j < structure.thicknesses.size(); ++j) {
            structure.thicknesses[j] -= learning_rate * gradients[j];
            // Ограничение минимальной толщины
            structure.thicknesses[j] = std::max(1.0, structure.thicknesses[j]);
        }

        // Проверка сходимости
        if (std::sqrt(total_error) < tolerance) {
            break;
        }
    }

    return structure;
}

std::vector<std::string> OpticalCoatingAnalyzer::getAvailableStructures() const {
    return db_.getAllStructureNames();
}

void OpticalCoatingAnalyzer::exportToCSV(
    const std::string& filename,
    const std::vector<double>& wavelengths,
    const std::vector<std::pair<double, double>>& spectrum) {

    if (wavelengths.size() != spectrum.size()) {
        throw std::invalid_argument("Wavelengths and spectrum sizes mismatch");
    }

    std::ofstream out(filename);
    if (!out.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    out << "Wavelength(nm),Transmission,Reflection\n";
    for (size_t i = 0; i < wavelengths.size(); ++i) {
        out << wavelengths[i] << ","
            << spectrum[i].first << ","
            << spectrum[i].second << "\n";
    }
}

std::vector<double> OpticalCoatingAnalyzer::generateWavelengthRange(
    double start, double end, double step) {

    if (start >= end || step <= 0) {
        throw std::invalid_argument("Invalid range parameters");
    }

    std::vector<double> result;
    size_t count = static_cast<size_t>((end - start) / step) + 1;
    result.reserve(count);

    for (double wl = start; wl <= end + 1e-9; wl += step) {
        result.push_back(wl);
    }

    return result;
}

std::pair<double, double> OpticalCoatingAnalyzer::calculateSingleWavelength(
    const OpticalStructure& structure,
    double wavelength) {

    return calculator_->RCWAMethod(
        wavelength,
        structure.substrate,
        structure.materials,
        structure.thicknesses,
        structure.considerBackside,
        structure.angleDegrees);
}