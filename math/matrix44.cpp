#include <cmath>
#include "utility.h"
#include "matrix44.h"
#include "vector4.h"

namespace math {

    matrix44::matrix44() : row_column{ { {1.0, 0.0, 0.0, 0.0}, {0.0, 1.0, 0.0, 0.0}, {0.0, 0.0, 1.0, 0.0}, {0.0, 0.0, 0.0, 1.0} } } {
    }

    matrix44::matrix44(const matrix_array& values) {
        row_column = values;
    }

    void matrix44::zero(matrix_array& target) {
        for (auto i = 0; i < column_max; ++i) {
            target[i].fill(0.0);
        }
    }

    void matrix44::unit() {
        zero(row_column);

        for (auto i = 0; i < column_max; ++i) {
            row_column[i][i] = 1.0;
        }
    }

    double matrix44::determinant() const {
        // ��O�p�s����쐬���čs�񎮂����߂�
        matrix_array upper_triangular = row_column;

        for (auto i = 0; i < column_max; ++i) {
            for (auto j = 0; j < row_max; ++j) {
                if (i >= j) {
                    continue;
                }

                const auto coefficient = upper_triangular[j][i] / upper_triangular[i][i];

                for (auto k = 0; k < row_max; ++k) {
                    upper_triangular[j][k] -= upper_triangular[i][k] * coefficient;
                }
            }
        }

        auto determinant = 1.0;

        for (auto i = 0; i < column_max; ++i) {
            determinant *= upper_triangular[i][i];
        }

        return determinant;
    }

    matrix44 matrix44::get_inverse() const {
        // �|���o���@�ɂ��t�s��̌v�Z
        matrix_array inverse{ { {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1} } };
        matrix_array temporary = row_column;

        for (auto i = 0; i < column_max; ++i) {
            auto coefficient = 1.0 / temporary[i][i];

            for (auto j = 0; j < row_max; ++j) {
                temporary[i][j] *= coefficient;
                inverse[i][j] *= coefficient;
            }

            for (auto j = 0; j < row_max; ++j) {
                if (i == j) {
                    continue;
                }

                coefficient = temporary[j][i];

                for (auto k = 0; k < row_max; ++k) {
                    temporary[j][k] -= temporary[i][k] * coefficient;
                    inverse[j][k] -= inverse[i][k] * coefficient;
                }
            }
        }

        return matrix44{inverse};
    }

    // �{���̓J�����ɑ����鏈�������֋X���������
    void matrix44::look_at(const vector4& position, const vector4& target, const vector4& up) {
        auto axis_z = target - position;
        auto axis_x = up.cross(axis_z);
        auto axis_y = axis_z.cross(axis_x);

        axis_x.normalized();
        axis_y.normalized();
        axis_z.normalized();

        row_column[0][0] = axis_x.get_x();
        row_column[0][1] = axis_y.get_x();
        row_column[0][2] = axis_z.get_x();
        row_column[0][3] = 0.0;

        row_column[1][0] = axis_x.get_y();
        row_column[1][1] = axis_y.get_y();
        row_column[1][2] = axis_z.get_y();
        row_column[1][3] = 0.0;

        row_column[2][0] = axis_x.get_z();
        row_column[2][1] = axis_y.get_z();
        row_column[2][2] = axis_z.get_z();
        row_column[2][3] = 0.0;

        row_column[3][0] = -axis_x.dot(position);
        row_column[3][1] = -axis_y.dot(position);
        row_column[3][2] = -axis_z.dot(position);
        row_column[3][3] = 1.0;
    }

    void matrix44::perspective(const double fov_y, const double aspect, const double near_z, const double far_z) {
        const auto cot = 1.0 / tan(fov_y * 0.5);
        const auto range = far_z - near_z;
        const auto temp = far_z / range;

        zero(row_column);

        row_column[0][0] = cot * aspect;
        row_column[1][1] = cot;
        row_column[2][2] = temp;
        row_column[2][3] = 1.0;
        row_column[3][2] = -near_z * temp;
    }

    void matrix44::viewport(const double width, const double height) {
        auto w = width  * 0.5;
        auto h = height * 0.5;

        unit();

        row_column[0][0] =  w;
        row_column[1][1] = -h;
        row_column[3][0] =  w;
        row_column[3][1] =  h;
    }

    matrix44 matrix44::operator *(const matrix44& rhs) const {
        matrix_array result{ { {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} } };

        for (auto i = 0; i < column_max; ++i) {
            for (auto j = 0; j < row_max; ++j) {
                auto m = 0.0;

                for (auto k = 0; k < column_max; ++k) {
                    m += row_column[i][k] * rhs.get_value(k, j);
                }

                result[i][j] = m;
            }
        }

        return matrix44{result};
    }

    matrix44 matrix44::operator +(const vector4& rhs) const
    {
        matrix_array result = row_column;

        result[3][0] += rhs.get_x();
        result[3][1] += rhs.get_y();
        result[3][2] += rhs.get_z();

        return matrix44{result};
    }

    void matrix44::transfer(const double x, const double y, const double z, const bool make) {
        if (make) {
            unit();
        }

        row_column[3][0] += x;
        row_column[3][1] += y;
        row_column[3][2] += z;
    }

    void matrix44::scale(const double x, const double y, const double z, const bool make) {
        if (make) {
            unit();

            row_column[0][0] *= x;
            row_column[1][1] *= y;
            row_column[2][2] *= z;
        }
        else {
            matrix44 scale;

            scale.row_column[0][0] = x;
            scale.row_column[1][1] = y;
            scale.row_column[2][2] = z;

            *this = *this * scale;
        }
    }

    void matrix44::rotate_x(const double degree, const bool make) {
        const auto sin_cos = get_sin_cos(degree);
        const auto sin = std::get<0>(sin_cos);
        const auto cos = std::get<1>(sin_cos);

        if (make) {
            unit();

            row_column[1][1] =  cos;
            row_column[1][2] =  sin;
            row_column[2][1] = -sin;
            row_column[2][2] =  cos;
        }
        else {
            const matrix_array array{ { {1, 0, 0, 0}, {0, cos, sin, 0}, {0, -sin, cos, 0}, {0, 0, 0, 1} } };
            const matrix44 rot_x(array);

            *this = *this * rot_x;
        }
    }

    void matrix44::rotate_y(const double degree, const bool make) {
        const auto sin_cos = get_sin_cos(degree);
        const auto sin = std::get<0>(sin_cos);
        const auto cos = std::get<1>(sin_cos);

        if (make) {
            unit();

            row_column[0][0] =  cos;
            row_column[0][2] = -sin;
            row_column[2][0] =  sin;
            row_column[2][2] =  cos;
        }
        else {
            const matrix_array array{ { {cos, 0, -sin, 0}, {0, 1, 0, 0}, {sin, 0, cos, 0}, {0, 0, 0, 1} } };
            const matrix44 rot_y(array);

            *this = *this * rot_y;
        }
    }

    void matrix44::rotate_z(const double degree, const bool make) {
        const auto sin_cos = get_sin_cos(degree);
        const auto sin = std::get<0>(sin_cos);
        const auto cos = std::get<1>(sin_cos);

        if (make) {
            unit();

            row_column[0][0] =  cos;
            row_column[0][1] =  sin;
            row_column[1][0] = -sin;
            row_column[1][1] =  cos;
        }
        else {
            const matrix_array array{ { {cos, sin, 0, 0}, {-sin, cos, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1} } };
            const matrix44 rot_z(array);

            *this = *this * rot_z;
        }
    }

    matrix44 matrix44::get_rotate() const
    {
        matrix_array none_transfer = row_column;

        none_transfer[3][0] = 0.0;
        none_transfer[3][1] = 0.0;
        none_transfer[3][2] = 0.0;

        return matrix44{none_transfer};
    }

    std::tuple<double, double> matrix44::get_sin_cos(const double degree) {
        const auto radian = utility::degree_to_radian(degree);

        return std::make_tuple(std::sin(radian), std::cos(radian));
    }

}
