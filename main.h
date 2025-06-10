#pragma once
#include "DatabaseManager.h"
#include "RCWACalculator.h"
#include "StructureLoader.h"
#include <vector>
#include <string>
#include <memory>

/**
 * @brief ����� ��� ������������� ���������� ���������
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
 * @brief �������� ����� ���������� ��� ������� ���������� ��������
 */
class OpticalCoatingAnalyzer {
public:
    /**
     * @brief ����������� � �������������� ���� � ���� ������
     * @param db_path ���� � ����� SQLite ���� ������
     */
    explicit OpticalCoatingAnalyzer(const std::string& db_path);

    /**
     * @brief ������ ������� ��� ��������� ���������
     * @param structure_name �������� ��������� �� ���� ������
     * @param wavelengths ������ ���� ���� ��� ������� (� ����������)
     * @return ������ ��� {transmission, reflection} ��� ������ ����� �����
     */
    std::vector<std::pair<double, double>> calculateSpectrum(
        const std::string& structure_name,
        const std::vector<double>& wavelengths);

    /**
     * @brief ����������� ������ ����� ��� ������� ������
     * @param initial_structure �������� ��������� ���������
     * @param target_wavelengths ����� ���� �������� �������
     * @param target_values ������� �������� (������������� �����������)
     * @param max_iterations ������������ ����� �������� �����������
     * @return ���������������� ���������
     */
    OpticalStructure optimizeStructure(
        const std::string& initial_structure,
        const std::vector<double>& target_wavelengths,
        const std::vector<double>& target_values,
        int max_iterations = 100);

    /**
     * @brief ��������� ������ ��������� �������� �� ��
     * @return ������ �������� ��������
     */
    std::vector<std::string> getAvailableStructures() const;

    /**
     * @brief ������� ����������� � CSV ����
     * @param filename ��� ����� ��� ��������
     * @param wavelengths ������ ���� ����
     * @param spectrum ������ ����������� {T, R}
     */
    static void exportToCSV(
        const std::string& filename,
        const std::vector<double>& wavelengths,
        const std::vector<std::pair<double, double>>& spectrum);

    /**
     * @brief ��������� ������������ ��������� ���� ����
     * @param start ��������� ����� ����� (��)
     * @param end �������� ����� ����� (��)
     * @param step ��� (��)
     * @return ������ ���� ����
     */
    static std::vector<double> generateWavelengthRange(
        double start, double end, double step = 1.0);

private:
    DatabaseManager db_;
    std::unique_ptr<RCWACalculator> calculator_;
    std::unique_ptr<StructureLoader> loader_;

    /**
     * @brief ���������� ������� ��� ������� ����� ����� �����
     */
    std::pair<double, double> calculateSingleWavelength(
        const OpticalStructure& structure,
        double wavelength);
};