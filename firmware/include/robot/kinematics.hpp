#pragma once

// Pure, idealized serial-leg mathematics. This is NOT verified RoboDog V2
// geometry and is not a servo command or transmission/calibration model.
namespace robot::kinematics {

// All dimensions are caller supplied in metres. No hardware defaults exist.
// The lateral offset is along local +y, before the two pitch links.
struct Geometry {
    double lateral_offset_m;
    double upper_length_m;
    double lower_length_m;
};

struct JointAngles {
    double hip_roll_rad;
    double hip_pitch_rad;
    double knee_pitch_rad;
};

struct FootPosition {
    double x_m;
    double y_m;
    double z_m;
};

enum class Status {
    Ok,
    InvalidGeometry,
    NonFiniteInput,
    InvalidBranch,
    Unreachable,
    Singular,
    NumericalFailure
};

// Sign of knee pitch in the returned solution (strictly between -pi and pi).
enum class KneeBranch { Positive, Negative };

// Sign of downward extension c BEFORE hip roll: Down selects c > 0;
// Up selects c < 0. This is not a global/world z-direction constraint.
enum class VerticalBranch { Down, Up };

struct ForwardResult {
    Status status;
    FootPosition position;
};

struct InverseResult {
    Status status;
    JointAngles angles;
};

// Right-handed frame: +x forward, +y lateral, +z up. Roll is about +x;
// both pitches are about their local -y. Zero links point down (-z).
// FK is defined at singular configurations and reports Ok there if finite.
// In every result, output data is usable ONLY when status == Status::Ok.
[[nodiscard]] ForwardResult forward(const Geometry& geometry,
                                    const JointAngles& angles) noexcept;

// Rejects unreachable goals; never projects a goal into the workspace.
// Rejects singular/near-singular inverse solutions. A dimensionless 1e-10
// threshold is applied to |c|/(upper+lower) and |sin(knee)|.
// Returned angles use [-pi, pi). No limits, collisions, gears, servo IDs,
// pulses, calibration, trajectories, or real-hardware validity are implied.
[[nodiscard]] InverseResult inverse(
    const Geometry& geometry, const FootPosition& position,
    KneeBranch knee = KneeBranch::Positive,
    VerticalBranch vertical = VerticalBranch::Down) noexcept;

}  // namespace robot::kinematics
