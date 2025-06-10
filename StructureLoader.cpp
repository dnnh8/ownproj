#include "StructureLoader.h"

OpticalStructure StructureLoader::load(const std::string& structureName) {
    OpticalStructure structure;

    // Загрузка подложки
    std::string sql = "SELECT substrate_id FROM Structures WHERE name = ?";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db.getHandle(), sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, structureName.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        throw std::runtime_error("Structure not found: " + structureName);
    }

    int substrateId = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    // Получаем название подложки
    sql = "SELECT name FROM Substrates WHERE id = ?";
    // ... выполнение запроса и установка structure.substrate

    // Загрузка слоёв
    sql = R"(
        SELECT m.name, sl.physical_thickness
        FROM StructureLayers sl
        JOIN Materials m ON m.id = sl.material_id
        WHERE sl.structure_id = (SELECT id FROM Structures WHERE name = ?)
        ORDER BY sl.layer_number
    )";
    // ... выполнение запроса и заполнение structure.layers

    return structure;
}