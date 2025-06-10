#include "DatabaseManager.h"
#include <stdexcept>
#include <algorithm>
#include <sstream>

DatabaseManager::DatabaseManager(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
        throw std::runtime_error("Cannot open database: " + std::string(sqlite3_errmsg(db_)));
    }

    // Включение кэширования запросов
    sqlite3_exec(db_, "PRAGMA cache_size = -10000", nullptr, nullptr, nullptr);
}

DatabaseManager::~DatabaseManager() {
    if (db_) {
        sqlite3_close(db_);
    }
}

template<typename T>
std::vector<OpticalData> DatabaseManager::fetchOpticalData(
    const std::string& table_name,
    const std::string& join_table,
    const std::string& name_column,
    T name_value) {

    std::vector<OpticalData> result;
    std::stringstream sql;

    sql << "SELECT wavelength, n_value, k_value FROM " << table_name
        << " JOIN " << join_table << " ON " << join_table << ".id = " << table_name << ".material_id"
        << " WHERE " << join_table << "." << name_column << " = ?"
        << " ORDER BY wavelength";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("SQL error: " + std::string(sqlite3_errmsg(db_)));
    }

    if constexpr (std::is_same_v<T, int>) {
        sqlite3_bind_int(stmt, 1, name_value);
    }
    else {
        sqlite3_bind_text(stmt, 1, name_value.c_str(), -1, SQLITE_STATIC);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        OpticalData data;
        data.wavelength = sqlite3_column_double(stmt, 0);
        data.n_value = sqlite3_column_double(stmt, 1);
        data.k_value = sqlite3_column_double(stmt, 2);
        result.push_back(data);
    }

    sqlite3_finalize(stmt);
    return result;
}

std::vector<OpticalData> DatabaseManager::getMaterialData(const std::string& material_name) {
    return fetchOpticalData("MaterialOpticalData", "Materials", "name", material_name);
}

std::vector<OpticalData> DatabaseManager::getSubstrateData(const std::string& substrate_name) {
    return fetchOpticalData("SubstrateOpticalData", "Substrates", "name", substrate_name);
}

const std::vector<OpticalData>& DatabaseManager::getCachedMaterialData(const std::string& material_name) {
    auto it = material_cache_.find(material_name);
    if (it == material_cache_.end()) {
        auto data = getMaterialData(material_name);
        it = material_cache_.emplace(material_name, std::move(data)).first;
    }
    return it->second;
}

const std::vector<OpticalData>& DatabaseManager::getCachedSubstrateData(const std::string& substrate_name) {
    auto it = substrate_cache_.find(substrate_name);
    if (it == substrate_cache_.end()) {
        auto data = getSubstrateData(substrate_name);
        it = substrate_cache_.emplace(substrate_name, std::move(data)).first;
    }
    return it->second;
}

std::vector<std::string> DatabaseManager::getAllStructureNames() {
    std::vector<std::string> names;
    const char* sql = "SELECT name FROM Structures ORDER BY name";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db_));
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        names.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
    }

    sqlite3_finalize(stmt);
    return names;
}

StructureInfo DatabaseManager::loadStructure(const std::string& structure_name) {
    StructureInfo info;

    // Загрузка подложки
    const char* sql = R"(
        SELECT s.name, sub.name 
        FROM Structures s
        JOIN Substrates sub ON sub.id = s.substrate_id
        WHERE s.name = ?
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db_));
    }

    sqlite3_bind_text(stmt, 1, structure_name.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        info.substrate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    }
    else {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Structure not found: " + structure_name);
    }
    sqlite3_finalize(stmt);

    // Загрузка слоев
    sql = R"(
        SELECT m.name, sl.physical_thickness
        FROM StructureLayers sl
        JOIN Materials m ON m.id = sl.material_id
        WHERE sl.structure_id = (SELECT id FROM Structures WHERE name = ?)
        ORDER BY sl.layer_number
    )";

    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(sqlite3_errmsg(db_));
    }

    sqlite3_bind_text(stmt, 1, structure_name.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        info.materials.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        info.thicknesses.push_back(sqlite3_column_double(stmt, 1));
    }

    sqlite3_finalize(stmt);
    return info;
}

std::complex<double> DatabaseManager::interpolateOpticalData(
    const std::vector<OpticalData>& data,
    double wavelength) {

    if (data.empty()) {
        throw std::runtime_error("Empty optical data for interpolation");
    }

    auto it = std::lower_bound(data.begin(), data.end(), OpticalData{ wavelength, 0, 0 });

    if (it == data.begin()) return { it->n_value, it->k_value };
    if (it == data.end()) return { data.back().n_value, data.back().k_value };

    auto prev = it - 1;
    double t = (wavelength - prev->wavelength) / (it->wavelength - prev->wavelength);

    double n = prev->n_value + t * (it->n_value - prev->n_value);
    double k = prev->k_value + t * (it->k_value - prev->k_value);

    return { n, k };
}