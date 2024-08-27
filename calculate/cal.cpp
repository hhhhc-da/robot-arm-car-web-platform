#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <utility>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define X_0 54
#define Y_0 85
#define LEN 80

using namespace std;

typedef std::tuple<double, double, double> pos3d_t;

std::pair<int, int> _arm_parameters_compute(pos3d_t pos)
{
    std::cout << std::endl;
    std::cout << "input: (" << std::get<0>(pos) << ", " << std::get<1>(pos) << ", " << std::get<2>(pos) << ")" << std::endl;

    /* 这里我们只计算左右臂参数, 依据 r、ψ, 不包括 θ */
    std::pair<int, int> ret = {74, 85};
    // gamma = arccos(1 - r^2/ (2 * l^2))
    double gamma = (180 / M_PI) * acos(1 - pow(std::get<0>(pos), 2) / (2 * pow(LEN, 2)));
    // fai = (y - y0) + (180 - gamma) / 2
    ret.second = std::get<2>(pos) + Y_0 - (180 - gamma) / 2;
    // gamma = (y - y0) + (x - x0)
    ret.first = gamma - ret.second + Y_0 + X_0;

    std::cout << "data: (" << ret.first << ", " << ret.second << ")" << std::endl;

    return ret;
}

int main(void)
{
    _arm_parameters_compute({80, 0, 120});
    _arm_parameters_compute({80, 0, 90});
    _arm_parameters_compute({138.56, 0, 120});

    system("pause");
    return 0;
}