#include "arrayProduct.h"

void arrayProduct(std::vector<double>& inMatrix, double multiplier) {

    for (auto& elem : inMatrix) {
        elem *= multiplier;
    }

}
