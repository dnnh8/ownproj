#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <complex>
#include <sqlite3.h>

struct OpticalData {
    double wavelength;
    double n_value;
    double k_value;

    bool operator<(const OpticalData& other) const {
        return wavelength < other.wavelength;
    }
};

struct StructureInfo {
    std::string substrate;
    std::vector<std::string> materials;
    std::vector<double> thicknesses;
};

class DatabaseManager {
public:
    explicit DatabaseManager(const std::string& db_path);
    ~DatabaseManager();

    // Запрет копирования
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // Основные методы
    std::vector<OpticalData> getMaterialData(const std::string& material_name);
    std::vector<OpticalData> getSubstrateData(const std::string& substrate_name);

    // Методы для работы со структурами
    std::vector<std::string> getAllStructureNames();
    StructureInfo loadStructure(const std::string& structure_name);

    // Кэшированные версии
    const std::vector<OpticalData>& getCachedMaterialData(const std::string& material_name);
    const std::vector<OpticalData>& getCachedSubstrateData(const std::string& substrate_name);

    // Утилиты
    static std::complex<double> interpolateOpticalData(
        const std::vector<OpticalData>& data,
        double wavelength);

private:
    sqlite3* db_;
    std::unordered_map<std::string, std::vector<OpticalData>> material_cache_;
    std::unordered_map<std::string, std::vector<OpticalData>> substrate_cache_;

    template<typename T>
    std::vector<OpticalData> fetchOpticalData(
        const std::string& table_name,
        const std::string& join_table,
        const std::string& name_column,
        T name_value);
};