#include "Plane.h"

/* 
   call parachute library update
*/
void Plane::parachute_check()
{
#if PARACHUTE == ENABLED
    parachute.update();
    if (parachute.released())
    {
        if (parachute_enabled == true)
        {
            if (release_msg_sended == false){
                gcs().send_text(MAV_SEVERITY_CRITICAL, "Parachute: Released");
                release_msg_sended = true;
            }
            set_mode(mode_manual, ModeReason::GCS_COMMAND); // MODE_REASON_GCS_COMMAND is equil 2. Not exist in 4.0.9
            parachute_enabled = false;
        }
    }

    if (parachute.auto_enabled() &&
        parachute.auto_release_alt_reached() &&
        parachute.auto_release_alt() > relative_altitude)
    {
        parachute_release();
    }

    // check deviation angles and relative altitude
    // check deviation angles
    float baro_alt = barometer.get_altitude();
    const float blimit = 2;
    if (parachute.auto_enabled() &&
        arming.is_armed() &&
        is_flying() &&
        (baro_alt > auto_state.baro_takeoff_alt + blimit) && 
        ((abs(ahrs.pitch_sensor) >= PARACHUTE_CRITICAL_ANGLE_DEVIATION_PITCH)
        || (abs(ahrs.roll_sensor) >= PARACHUTE_CRITICAL_ANGLE_DEVIATION_ROLL)))
    {
        parachute_release();
        if (critical_angle_msg_sended == false)
        {
            gcs().send_text(MAV_SEVERITY_CRITICAL, "Parachute released: Reached critical angle");
            critical_angle_msg_sended = true;
        }
        
    }

    if (parachute.auto_enabled())
    {
        static bool chute_auto_ready = false;
        bool alt_reached = parachute.update_alt(relative_altitude);
        if (alt_reached && chute_auto_ready == false)
        {
            gcs().send_text(MAV_SEVERITY_CRITICAL, "Parachute: AUTO READY");
            chute_auto_ready = true;
        }
        chute_auto_ready = alt_reached;
    }
#endif
}

#if PARACHUTE == ENABLED

/*
  parachute_release - trigger the release of the parachute
*/
void Plane::parachute_release()
{
    arming.disarm();
    set_mode(mode_stabilize, ModeReason::GCS_COMMAND); 
    parachute_enabled = true;

    if (parachute.release_in_progress())
    {
        return;
    }
    if (parachute.released())
    {
        gcs().send_text(MAV_SEVERITY_CRITICAL, "Parachute: Released again");
    }
    else
    {
        if (elevon_override_msg_sended == false){
            gcs().send_text(MAV_SEVERITY_CRITICAL,"Parachute: Disarmed, Elevon override");
            elevon_override_msg_sended = true;
        }
    }

    // release parachute
    parachute.release();

/*#if LANDING_GEAR_ENABLED == ENABLED
    // deploy landing gear
    g2.landing_gear.set_position(AP_LandingGear::LandingGear_Deploy);
#endif*/
}

/*
  parachute_manual_release - trigger the release of the parachute,
  after performing some checks for pilot error checks if the vehicle
  is landed
*/
bool Plane::parachute_manual_release()
{
    // exit immediately if parachute is not enabled
    if (!parachute.enabled() || parachute.released())
    {
        return false;
    }

    if (parachute.alt_min() > 0 && relative_ground_altitude(false) < parachute.alt_min() &&
        auto_state.last_flying_ms > 0)
    {
        // Allow manual ground tests by only checking if flying too low if we've taken off
        gcs().send_text(MAV_SEVERITY_WARNING, "Parachute: Too low");
        return false;
    }

    // if we get this far release parachute
    parachute_release();

/*#if LANDING_GEAR_ENABLED == ENABLED
    // deploy landing gear
    g2.landing_gear.set_position(AP_LandingGear::LandingGear_Deploy);
#endif*/
    return true;
}
#endif