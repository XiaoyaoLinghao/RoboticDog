#include "robot/kinematics.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace robot::kinematics {
namespace {

constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr double kTwoPi = 2.0 * kPi;
constexpr double kSingularityTolerance = 1e-10;
constexpr double kResidualTolerance = 256.0 * std::numeric_limits<double>::epsilon();

bool valid_geometry(const Geometry& g) noexcept {
    return std::isfinite(g.lateral_offset_m) &&
           std::isfinite(g.upper_length_m) && std::isfinite(g.lower_length_m) &&
           g.lateral_offset_m >= 0.0 && g.upper_length_m > 0.0 &&
           g.lower_length_m > 0.0;
}

bool finite(const JointAngles& q) noexcept {
    return std::isfinite(q.hip_roll_rad) && std::isfinite(q.hip_pitch_rad) &&
           std::isfinite(q.knee_pitch_rad);
}

bool finite(const FootPosition& p) noexcept {
    return std::isfinite(p.x_m) && std::isfinite(p.y_m) && std::isfinite(p.z_m);
}

double scale_of(const Geometry& g) noexcept {
    return std::max(g.lateral_offset_m,
                    std::max(g.upper_length_m, g.lower_length_m));
}

Geometry normalized(const Geometry& g, double scale) noexcept {
    return {g.lateral_offset_m / scale, g.upper_length_m / scale,
            g.lower_length_m / scale};
}

FootPosition evaluate(const Geometry& g, const JointAngles& q) noexcept {
    const double sr = std::sin(q.hip_roll_rad);
    const double cr = std::cos(q.hip_roll_rad);
    const double sh = std::sin(q.hip_pitch_rad);
    const double ch = std::cos(q.hip_pitch_rad);
    const double sk = std::sin(q.knee_pitch_rad);
    const double ck = std::cos(q.knee_pitch_rad);
    // Angle addition avoids overflowing h+k for large finite input angles.
    const double s = g.upper_length_m * sh + g.lower_length_m * (sh * ck + ch * sk);
    const double c = g.upper_length_m * ch + g.lower_length_m * (ch * ck - sh * sk);
    return {s, g.lateral_offset_m * cr + c * sr,
            g.lateral_offset_m * sr - c * cr};
}

double wrap_angle(double angle) noexcept {
    double wrapped = std::remainder(angle, kTwoPi);
    if (wrapped >= kPi) {
        wrapped -= kTwoPi;
    }
    return wrapped;
}

ForwardResult forward_error(Status status) noexcept {
    return {status, {0.0, 0.0, 0.0}};
}

InverseResult inverse_error(Status status) noexcept {
    return {status, {0.0, 0.0, 0.0}};
}

}  // namespace

ForwardResult forward(const Geometry& geometry, const JointAngles& angles) noexcept {
    if (!valid_geometry(geometry)) {
        return forward_error(Status::InvalidGeometry);
    }
    if (!finite(angles)) {
        return forward_error(Status::NonFiniteInput);
    }
    const double scale = scale_of(geometry);
    const Geometry g = normalized(geometry, scale);
    if (g.upper_length_m == 0.0 || g.lower_length_m == 0.0 ||
        (geometry.lateral_offset_m > 0.0 && g.lateral_offset_m == 0.0)) {
        return forward_error(Status::NumericalFailure);
    }
    FootPosition position = evaluate(g, angles);
    position.x_m *= scale;
    position.y_m *= scale;
    position.z_m *= scale;
    if (!finite(position)) {
        return forward_error(Status::NumericalFailure);
    }
    return {Status::Ok, position};
}

InverseResult inverse(const Geometry& geometry, const FootPosition& position,
                      KneeBranch knee, VerticalBranch vertical) noexcept {
    if (!valid_geometry(geometry)) {
        return inverse_error(Status::InvalidGeometry);
    }
    if (!finite(position)) {
        return inverse_error(Status::NonFiniteInput);
    }
    if ((knee != KneeBranch::Positive && knee != KneeBranch::Negative) ||
        (vertical != VerticalBranch::Down && vertical != VerticalBranch::Up)) {
        return inverse_error(Status::InvalidBranch);
    }

    const double scale = scale_of(geometry);
    const Geometry g = normalized(geometry, scale);
    const double a = g.upper_length_m;
    const double b = g.lower_length_m;
    const double d = g.lateral_offset_m;
    const double denominator = 2.0 * a * b;
    if (a == 0.0 || b == 0.0 || denominator == 0.0 ||
        (geometry.lateral_offset_m > 0.0 && d == 0.0)) {
        return inverse_error(Status::NumericalFailure);
    }
    const FootPosition p{position.x_m / scale, position.y_m / scale,
                         position.z_m / scale};
    if (!finite(p)) {
        // A finite target whose normalized magnitude overflows is necessarily
        // farther away than these normalized links (each at most one).
        return inverse_error(Status::Unreachable);
    }
    const double outer = a + b;
    const double inner = std::abs(a - b);
    const double rho = std::hypot(p.y_m, p.z_m);
    if (!std::isfinite(rho) || std::abs(p.x_m) > outer || rho > d + outer || rho < d) {
        return inverse_error(Status::Unreachable);
    }
    // Product form loses less precision than rho*rho-d*d near rho == d.
    const double c_abs = std::sqrt((rho - d) * (rho + d));
    const double radius = std::hypot(p.x_m, c_abs);
    if (radius < inner || radius > outer) {
        return inverse_error(Status::Unreachable);
    }
    if (radius == inner || radius == outer || c_abs / outer <= kSingularityTolerance) {
        return inverse_error(Status::Singular);
    }

    const double cos_knee = (radius * radius - a * a - b * b) / denominator;
    // Deliberately do not clamp. Geometric bounds above reject impossible
    // goals, and unrepresentable arithmetic is reported rather than projected.
    if (!std::isfinite(cos_knee) || cos_knee < -1.0 || cos_knee > 1.0) {
        return inverse_error(Status::NumericalFailure);
    }
    double sin_knee = std::sqrt((1.0 - cos_knee) * (1.0 + cos_knee));
    if (sin_knee <= kSingularityTolerance) {
        return inverse_error(Status::Singular);
    }
    if (knee == KneeBranch::Negative) {
        sin_knee = -sin_knee;
    }
    const double c = vertical == VerticalBranch::Down ? c_abs : -c_abs;
    const JointAngles angles{
        wrap_angle(std::atan2(p.z_m, p.y_m) + std::atan2(c, d)),
        wrap_angle(std::atan2(p.x_m, c) - std::atan2(b * sin_knee, a + b * cos_knee)),
        wrap_angle(std::atan2(sin_knee, cos_knee))};
    if (!finite(angles)) {
        return inverse_error(Status::NumericalFailure);
    }
    const FootPosition check = evaluate(g, angles);
    const double residual = std::hypot(std::hypot(check.x_m - p.x_m, check.y_m - p.y_m),
                                       check.z_m - p.z_m);
    if (!std::isfinite(residual) || residual > kResidualTolerance * (d + outer)) {
        return inverse_error(Status::NumericalFailure);
    }
    return {Status::Ok, angles};
}

}  // namespace robot::kinematics
