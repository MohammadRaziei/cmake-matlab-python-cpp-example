#include "arrayProduct.h"

void arrayProduct(double* inMatrix, size_t size, double multiplier) {

    for (auto it=inMatrix; it < inMatrix + size; ++it) {
        *it *= multiplier;
    }

}


void arrayProduct(std::vector<double>& inMatrix, double multiplier) {

    arrayProduct(inMatrix.data(), inMatrix.size(), multiplier);
}
