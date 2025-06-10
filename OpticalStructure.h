#pragma once
#include <vector>
#include <string>

struct Layer {
    std::string material;
    double thickness; // в нанометрах
};

class OpticalStructure {
public:
    std::string substrate;
    std::vector<Layer> layers;
    bool considerBackside = true;
    double angleDegrees = 0.0;

    void addLayer(const std::string& material, double thickness) {
        layers.push_back({ material, thickness });
    }
};