#pragma once
#include "DatabaseManager.h"
#include "OpticalStructure.h"

class StructureLoader {
public:
    StructureLoader(DatabaseManager& db) : db(db) {}

    OpticalStructure load(const std::string& structureName);

private:
    DatabaseManager& db;
};
