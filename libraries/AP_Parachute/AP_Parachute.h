/// @file	AP_Parachute.h
/// @brief	Parachute release library
#pragma once

#include <AP_Param/AP_Param.h>
#include <AP_Common/AP_Common.h>
#include <AP_Relay/AP_Relay.h>

#define AP_PARACHUTE_TRIGGER_TYPE_RELAY_0          0
#define AP_PARACHUTE_TRIGGER_TYPE_RELAY_1          1
#define AP_PARACHUTE_TRIGGER_TYPE_RELAY_2          2
#define AP_PARACHUTE_TRIGGER_TYPE_RELAY_3          3
#define AP_PARACHUTE_TRIGGER_TYPE_SERVO            10
#define AP_PARACHUTE_RELEASE_DELAY_MS              500    // delay in milliseconds between call to release() and when servo or relay actually moves.  Allows for warning to user
#define AP_PARACHUTE_RELEASE_DURATION_MS          5000    // when parachute is released, servo or relay stay at their released position/value for 5000ms (5seconds)
#define AP_PARACHUTE_SERVO_ON_PWM_DEFAULT         1300    // default PWM value to move servo to when shutter is activated
#define AP_PARACHUTE_SERVO_OFF_PWM_DEFAULT        1100    // default PWM value to move servo to when shutter is deactivated
#define AP_PARACHUTE_CRITICAL_ANGLE_PITCH         5000    // absolute critical pitch angle 50 degrees to detect
#define AP_PARACHUTE_CRITICAL_ANGLE_ROLL          5500    // absolute critical roll angle 55 degrees to detect
#define AP_PARACHUTE_ALT_MIN_DEFAULT              10     // default min altitude the vehicle should have before parachute is released
#define AP_PARACHUTE_CRITICAL_SINK_DEFAULT        4    // default critical sink speed in m/s to trigger emergency parachute
#define AP_PARACHUTE_AUTO_RELEASE_ALT             20

/// @class	AP_Parachute
/// @brief	Class managing the release of a parachute
class AP_Parachute
{

public:
    /// Constructor
    AP_Parachute(AP_Relay &relay)
        : _relay(relay)
          // , _release_time(0)
          // , _release_initiated(false)
          // , _release_in_progress(false)
          // , _released(false)
          // , _release_alt_reached(false)
    {
        // setup parameter defaults
#if CONFIG_HAL_BOARD == HAL_BOARD_SITL
        if (_singleton != nullptr) {
            AP_HAL::panic("Rally must be singleton");
        }
#endif
        _singleton = this;
        AP_Param::setup_object_defaults(this, var_info);
    }

    /* Do not allow copies */
    AP_Parachute(const AP_Parachute &other) = delete;
    AP_Parachute &operator=(const AP_Parachute&) = delete;

    /// enabled - enable or disable parachute release
    void enabled(bool on_off);

    /// enabled - returns true if parachute release is enabled
    bool enabled() const
    {
        return _enabled;
    }

    /// release - release parachute
    void release();

    /// released - true if the parachute has been released (or release is in progress)
    bool released() const
    {
        return _released;
    }

    /// release_initiated - true if the parachute release sequence has been initiated (may wait before actual release)
    bool release_initiated() const
    {
        return _release_initiated;
    }

    /// release_in_progress - true if the parachute release sequence is in progress
    bool release_in_progress() const
    {
        return _release_in_progress;
    }

    /// update - shuts off the trigger should be called at about 10hz
    void update();

    /// release_alt_reached - update alt_reached flag
    /// return whether alt_reached so chute ready if enabled
    bool release_alt_reached(int32_t relative_alt);

    /// critical_sink - returns the configured maximum sink rate to trigger emergency release
    float critical_sink() const
    {
        return _critical_sink;
    }

    /// alt_min - returns the min altitude above home the vehicle should have before parachute is released
    ///   0 = altitude check disabled
    int16_t alt_min() const
    {
        return _alt_min;
    }

    /// set_sink_rate - set vehicle sink rate
    void set_sink_rate(float sink_rate)
    {
        _sink_rate = sink_rate;
    }

    float sink_rate() const
    {
        return _sink_rate;
    }

    /// auto_release_alt- returns max altitude for automatic chute release, plane should be below
    int16_t auto_release_alt() const
    {
        return _auto_release_alt;
    }

    /// auto_enabled - returns true if parachute AUTO release is enabled
    bool auto_enabled() const
    {
        return _auto_enabled;
    }

    /// auto_release_alt_reached - return true if release alt was reached
    bool auto_release_alt_reached() const
    {
        return _release_alt_reached;
    }

    /// auto_enable_alt - return altitude above home
    /// the vehicel should have, for chute AUTO mode become enabled
    int16_t auto_enable_alt() const
    {
        if (_auto_enable_alt > _auto_release_alt) {
            return _auto_enable_alt;
        }

        // if _auto_enable_alt is less then _auto_releaze_alt
        // vehicle should have altitude margin, before AUTO mode become enabled.
        // or accidently chute can be releazed on takeoff.
        return (auto_release_alt() + 30);
    }

    /// pitch value to set before chute release
    int16_t pitch() const
    {
        return _pitch;
    }

    void set_sink_time(uint32_t time)
    {
        _sink_time = time;
    }

    int16_t critical_pitch() const
    {
        return _critical_pitch;
    }

    int16_t critical_roll() const
    {
        return _critical_roll;
    }

    uint32_t sink_time() const
    {
        return _sink_time;
    }

    static const struct AP_Param::GroupInfo        var_info[];

    // get singleton instance
    static AP_Parachute *get_singleton()
    {
        return _singleton;
    }

private:
    static AP_Parachute *_singleton;
    // Parameters
    AP_Int8     _enabled;               // 1 if parachute release is enabled
    AP_Int8     _release_type;          // 0:Servo,1:Relay
    AP_Int16    _servo_on_pwm;          // PWM value to move servo to when shutter is activated
    AP_Int16    _servo_off_pwm;         // PWM value to move servo to when shutter is deactivated
    AP_Int16    _alt_min;               // min altitude the vehicle should have before parachute is released
    AP_Int16    _delay_ms;              // delay before chute release for motors to stop
    AP_Float    _critical_sink;         // critical sink rate to trigger emergency parachute
    AP_Int8     _auto_enabled;          // 1 if parachute AUTO release is enabled
    AP_Int16    _auto_release_alt;      // max altitude for automatic chute release, plane chould be below
    AP_Int16    _auto_enable_alt;       // altitude after which the "AUTO release" mode will turn on
    AP_Int16    _pitch;                 // plane pitch in stabilization mode, which must be achieved before releasing chute
    AP_Int16    _critical_pitch;        //
    AP_Int16    _critical_roll;         //

    // internal variables
    AP_Relay    &_relay;                  // pointer to relay object from the base class Relay.
    uint32_t    _release_time;           // system time that parachute is ordered to be released (actual release will happen 0.5 seconds later)
    bool        _release_initiated:1;    // true if the parachute release initiated (may still be waiting for engine to be suppressed etc.)
    bool        _release_in_progress:1;  // true if the parachute release is in progress
    bool        _released:1;             // true if the parachute has been released
    bool        _release_alt_reached:1;
    float       _sink_rate;              // vehicle sink rate in m/s
    uint32_t    _sink_time;             // time that the vehicle exceeded critical sink rate
};

namespace AP
{
AP_Parachute *parachute();
};
