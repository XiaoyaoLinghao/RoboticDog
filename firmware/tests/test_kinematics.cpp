#include "robot/kinematics.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>

namespace {

namespace kin = robot::kinematics;
constexpr double kPi = 3.141592653589793238462643383279502884;
int checks = 0;
int failures = 0;

// Deliberately independent of assert/NDEBUG: Release builds run every check.
#define CHECK(expression)                                                        \
    do {                                                                         \
        ++checks;                                                                \
        if (!(expression)) {                                                     \
            ++failures;                                                          \
            std::cerr << __FILE__ << ':' << __LINE__ << ": " #expression "\n";   \
        }                                                                        \
    } while (false)

bool near(double a, double b, double tolerance = 1e-11) {
    return std::isfinite(a) && std::isfinite(b) && std::abs(a - b) <= tolerance;
}

bool same_angle(double a, double b) {
    return near(std::remainder(a - b, 2.0 * kPi), 0.0);
}

void check_position(const kin::FootPosition& actual, const kin::FootPosition& expected,
                    double tolerance = 1e-11) {
    CHECK(near(actual.x_m, expected.x_m, tolerance));
    CHECK(near(actual.y_m, expected.y_m, tolerance));
    CHECK(near(actual.z_m, expected.z_m, tolerance));
}

void known_poses_and_signs() {
    // Synthetic, dimensionally convenient fixtures; not robot dimensions.
    const kin::Geometry g{1.0, 3.0, 2.0};
    auto result = kin::forward(g, {0.0, 0.0, 0.0});
    CHECK(result.status == kin::Status::Ok);  // FK exists at an IK singularity.
    check_position(result.position, {0.0, 1.0, -5.0});

    result = kin::forward(g, {0.0, kPi / 2.0, 0.0});
    CHECK(result.status == kin::Status::Ok);
    check_position(result.position, {5.0, 1.0, 0.0});

    result = kin::forward(g, {0.0, -kPi / 2.0, 0.0});
    CHECK(result.status == kin::Status::Ok);
    check_position(result.position, {-5.0, 1.0, 0.0});

    result = kin::forward(g, {kPi / 2.0, 0.0, 0.0});
    CHECK(result.status == kin::Status::Ok);
    check_position(result.position, {0.0, 5.0, 1.0});

    result = kin::forward(g, {-kPi / 2.0, 0.0, 0.0});
    CHECK(result.status == kin::Status::Ok);
    check_position(result.position, {0.0, -5.0, -1.0});

    result = kin::forward(g, {0.0, 0.0, kPi / 2.0});
    CHECK(result.status == kin::Status::Ok);
    check_position(result.position, {2.0, 1.0, -3.0});
    const auto inverse = kin::inverse(g, result.position);
    CHECK(inverse.status == kin::Status::Ok);
    CHECK(same_angle(inverse.angles.hip_roll_rad, 0.0));
    CHECK(same_angle(inverse.angles.hip_pitch_rad, 0.0));
    CHECK(same_angle(inverse.angles.knee_pitch_rad, kPi / 2.0));

    result = kin::forward(g, {0.0, 0.0, -kPi / 2.0});
    CHECK(result.status == kin::Status::Ok);
    check_position(result.position, {-2.0, 1.0, -3.0});
    const auto negative = kin::inverse(g, result.position, kin::KneeBranch::Negative);
    CHECK(negative.status == kin::Status::Ok);
    CHECK(same_angle(negative.angles.hip_roll_rad, 0.0));
    CHECK(same_angle(negative.angles.hip_pitch_rad, 0.0));
    CHECK(same_angle(negative.angles.knee_pitch_rad, -kPi / 2.0));

    result = kin::forward(g, {kPi / 2.0, 0.0, kPi / 2.0});
    CHECK(result.status == kin::Status::Ok);
    check_position(result.position, {2.0, 3.0, 1.0});

    result = kin::forward(g, {0.0, kPi, kPi / 2.0});
    CHECK(result.status == kin::Status::Ok);
    check_position(result.position, {-2.0, 1.0, 3.0});
    const auto up = kin::inverse(g, result.position, kin::KneeBranch::Positive,
                                 kin::VerticalBranch::Up);
    CHECK(up.status == kin::Status::Ok);
    CHECK(same_angle(up.angles.hip_roll_rad, 0.0));
    CHECK(same_angle(up.angles.hip_pitch_rad, kPi));
    CHECK(same_angle(up.angles.knee_pitch_rad, kPi / 2.0));
}

void invalid_inputs() {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();
    const kin::Geometry good{0.5, 2.0, 1.0};
    const kin::JointAngles q{0.2, -0.3, 1.0};
    const kin::FootPosition p{1.0, 0.5, -2.0};
    const kin::Geometry invalid[] = {
        {-0.1, 2.0, 1.0}, {0.5, -2.0, 1.0}, {0.5, 2.0, -1.0},
        {0.5, 0.0, 1.0}, {0.5, 2.0, 0.0}, {0.0, 0.0, 0.0},
        {nan, 2.0, 1.0}, {0.5, nan, 1.0}, {0.5, 2.0, nan},
        {infinity, 2.0, 1.0}, {0.5, infinity, 1.0}, {0.5, 2.0, infinity},
        {-infinity, 2.0, 1.0}, {0.5, -infinity, 1.0}, {0.5, 2.0, -infinity}};
    for (const auto& g : invalid) {
        CHECK(kin::forward(g, q).status == kin::Status::InvalidGeometry);
        CHECK(kin::inverse(g, p).status == kin::Status::InvalidGeometry);
    }
    for (double value : {nan, infinity, -infinity}) {
        CHECK(kin::forward(good, {value, 0.0, 1.0}).status == kin::Status::NonFiniteInput);
        CHECK(kin::forward(good, {0.0, value, 1.0}).status == kin::Status::NonFiniteInput);
        CHECK(kin::forward(good, {0.0, 0.0, value}).status == kin::Status::NonFiniteInput);
        CHECK(kin::inverse(good, {value, 0.5, -2.0}).status == kin::Status::NonFiniteInput);
        CHECK(kin::inverse(good, {1.0, value, -2.0}).status == kin::Status::NonFiniteInput);
        CHECK(kin::inverse(good, {1.0, 0.5, value}).status == kin::Status::NonFiniteInput);
    }
    CHECK(kin::inverse(good, p, static_cast<kin::KneeBranch>(99)).status ==
          kin::Status::InvalidBranch);
    CHECK(kin::inverse(good, p, kin::KneeBranch::Positive,
                       static_cast<kin::VerticalBranch>(99)).status == kin::Status::InvalidBranch);
}

void workspace_and_singularities() {
    const kin::Geometry no_offset{0.0, 2.0, 1.0};
    CHECK(kin::inverse(no_offset, {0.0, 0.0, -3.0}).status == kin::Status::Singular);
    CHECK(kin::inverse(no_offset, {0.0, 0.0, -1.0}).status == kin::Status::Singular);
    CHECK(kin::inverse(no_offset, {0.0, 0.0, -0.5}).status == kin::Status::Unreachable);
    CHECK(kin::inverse(no_offset, {0.0, 0.0, -3.5}).status == kin::Status::Unreachable);
    CHECK(kin::inverse(no_offset, {4.0, 0.0, 0.0}).status == kin::Status::Unreachable);
    // Adjacent representable points OUTSIDE either sphere must not be clamped.
    const double outside_outer = std::nextafter(3.0, 4.0);
    const double outside_inner = std::nextafter(1.0, 0.0);
    CHECK(kin::inverse(no_offset, {0.0, 0.0, -outside_outer}).status == kin::Status::Unreachable);
    CHECK(kin::inverse(no_offset, {0.0, 0.0, -outside_inner}).status == kin::Status::Unreachable);
    CHECK(kin::inverse(no_offset, {0.0, 0.0, -(3.0 - 1e-8)}).status == kin::Status::Ok);
    CHECK(kin::inverse(no_offset, {0.0, 0.0, -(1.0 + 1e-8)}).status == kin::Status::Ok);
    CHECK(kin::inverse(no_offset, {2.0, 0.0, 0.0}).status == kin::Status::Singular);
    CHECK(kin::inverse(no_offset, {2.0, 0.0, -1e-12}).status == kin::Status::Singular);

    const kin::Geometry offset{1.0, 3.0, 2.0};
    CHECK(kin::inverse(offset, {3.0, 0.5, 0.0}).status == kin::Status::Unreachable);
    CHECK(kin::inverse(offset, {4.0, 1.0, 0.0}).status == kin::Status::Singular);
    CHECK(kin::inverse(offset, {6.0, 1.0, 0.0}).status == kin::Status::Unreachable);
    CHECK(kin::inverse(offset, {0.0, 1.0, 0.0}).status == kin::Status::Unreachable);
    CHECK(kin::inverse({0.0, 1.0, 1.0}, {0.0, 0.0, 0.0}).status == kin::Status::Singular);

    // Positive lateral offset is allowed to be zero, and the two planes still
    // give distinct angle solutions away from singularities.
    for (const auto plane : {kin::VerticalBranch::Down, kin::VerticalBranch::Up}) {
        for (const auto knee : {kin::KneeBranch::Positive, kin::KneeBranch::Negative}) {
            const auto result = kin::inverse(no_offset, {0.3, 0.0, -2.0}, knee, plane);
            CHECK(result.status == kin::Status::Ok);
            const auto fk = kin::forward(no_offset, result.angles);
            CHECK(fk.status == kin::Status::Ok);
            check_position(fk.position, {0.3, 0.0, -2.0});
        }
    }
}

void finite_precision_and_scale() {
    const double huge = std::numeric_limits<double>::max();
    const double tiny = std::numeric_limits<double>::min();
    CHECK(kin::forward({0.0, huge, huge}, {0.0, 0.0, 0.0}).status ==
          kin::Status::NumericalFailure);
    CHECK(kin::forward({huge, tiny, tiny}, {0.0, 0.0, 1.0}).status ==
          kin::Status::NumericalFailure);
    CHECK(kin::inverse({huge, tiny, tiny}, {0.0, huge, 0.0}).status ==
          kin::Status::NumericalFailure);
    CHECK(kin::forward({tiny, huge, huge}, {0.0, 0.0, 1.0}).status ==
          kin::Status::NumericalFailure);
    CHECK(kin::inverse({tiny, huge, huge}, {0.0, tiny, -huge}).status ==
          kin::Status::NumericalFailure);
    CHECK(kin::inverse({0.0, tiny, tiny}, {huge, 0.0, 0.0}).status == kin::Status::Unreachable);
    CHECK(kin::forward({0.0, 1.0, 1.0}, {huge, huge, huge}).status == kin::Status::Ok);

    for (double scale : {tiny, 1e-100, 1e-6, 1.0, 1e6, 1e100, huge / 8.0}) {
        const kin::Geometry g{0.25 * scale, 2.0 * scale, 1.0 * scale};
        const kin::JointAngles original{0.4, -0.2, 1.0};
        const auto fk = kin::forward(g, original);
        CHECK(fk.status == kin::Status::Ok);
        const auto ik = kin::inverse(g, fk.position);
        CHECK(ik.status == kin::Status::Ok);
        CHECK(same_angle(ik.angles.hip_roll_rad, original.hip_roll_rad));
        CHECK(same_angle(ik.angles.hip_pitch_rad, original.hip_pitch_rad));
        CHECK(same_angle(ik.angles.knee_pitch_rad, original.knee_pitch_rad));
    }
}

void all_branch_roundtrips() {
    std::mt19937 random(0x35A17U);
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    for (int sample = 0; sample < 1000; ++sample) {
        const kin::Geometry g{0.5 * unit(random), 0.2 + unit(random), 0.2 + unit(random)};
        const kin::JointAngles original{
            (2.0 * unit(random) - 1.0) * kPi,
            (2.0 * unit(random) - 1.0) * kPi,
            (sample % 2 == 0 ? 1.0 : -1.0) * (0.1 + unit(random) * (kPi - 0.2))};
        const double extension = g.upper_length_m * std::cos(original.hip_pitch_rad) +
                                 g.lower_length_m * std::cos(original.hip_pitch_rad +
                                                              original.knee_pitch_rad);
        // Test strictly nonsingular configurations; boundary handling is above.
        if (std::abs(extension) < 1e-5) {
            --sample;
            continue;
        }
        const auto fk = kin::forward(g, original);
        CHECK(fk.status == kin::Status::Ok);
        for (const auto plane : {kin::VerticalBranch::Down, kin::VerticalBranch::Up}) {
            for (const auto knee : {kin::KneeBranch::Positive, kin::KneeBranch::Negative}) {
                const auto ik = kin::inverse(g, fk.position, knee, plane);
                CHECK(ik.status == kin::Status::Ok);
                if (ik.status != kin::Status::Ok) {
                    continue;
                }
                CHECK(ik.angles.hip_roll_rad >= -kPi && ik.angles.hip_roll_rad < kPi);
                CHECK(ik.angles.hip_pitch_rad >= -kPi && ik.angles.hip_pitch_rad < kPi);
                CHECK(ik.angles.knee_pitch_rad >= -kPi && ik.angles.knee_pitch_rad < kPi);
                CHECK((ik.angles.knee_pitch_rad > 0.0) == (knee == kin::KneeBranch::Positive));
                const double actual_extension = g.upper_length_m * std::cos(ik.angles.hip_pitch_rad) +
                    g.lower_length_m * std::cos(ik.angles.hip_pitch_rad + ik.angles.knee_pitch_rad);
                CHECK((actual_extension > 0.0) == (plane == kin::VerticalBranch::Down));
                const auto back = kin::forward(g, ik.angles);
                CHECK(back.status == kin::Status::Ok);
                check_position(back.position, fk.position);
                if (((extension > 0.0) == (plane == kin::VerticalBranch::Down)) &&
                    ((original.knee_pitch_rad > 0.0) == (knee == kin::KneeBranch::Positive))) {
                    CHECK(same_angle(ik.angles.hip_roll_rad, original.hip_roll_rad));
                    CHECK(same_angle(ik.angles.hip_pitch_rad, original.hip_pitch_rad));
                    CHECK(same_angle(ik.angles.knee_pitch_rad, original.knee_pitch_rad));
                }
            }
        }
    }
}

}  // namespace

int main() {
    known_poses_and_signs();
    invalid_inputs();
    workspace_and_singularities();
    finite_precision_and_scale();
    all_branch_roundtrips();
    if (failures != 0) {
        std::cerr << failures << " of " << checks << " kinematics checks failed\n";
        return 1;
    }
    std::cout << "All " << checks << " kinematics checks passed\n";
    return 0;
}
