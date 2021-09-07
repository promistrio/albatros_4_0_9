#include "Plane.h"

/*
   call parachute library update
*/
void Plane::parachute_check()
{
#if PARACHUTE == ENABLED

    parachute.update();

    if (parachute.released() && parachute_enabled) {
        set_mode(mode_manual, ModeReason::GCS_COMMAND); // MODE_REASON_GCS_COMMAND is equil 2. Not exist in 4.0.9
        parachute_enabled = false;
        if (!release_msg_sended) {
            gcs().send_text(MAV_SEVERITY_CRITICAL, "Parachute: Released");
            release_msg_sended = true;
        }
    }


    // check deviation angles and relative altitude
    if (parachute.auto_enabled() 
        && !parachute.released() 
        // && is_flying() 
       ) {


        //check if plane below AUTO_ALT
        if (relative_altitude < parachute.auto_release_alt()
            && parachute.auto_release_alt_reached()
        ) {
            parachute_release();
            if (!release_reason_msg_sended) {
                gcs().send_text(MAV_SEVERITY_ALERT, "Parachute released: below \"AUTO_ALT\"");
                release_reason_msg_sended = true;
            }
        }

        if (arming.is_armed()) {

            // check if the plane is sinking too fast for more than a second and release parachute
            uint32_t time = AP_HAL::millis();
            if ((parachute.critical_sink() > 0) 
                && (parachute.sink_rate() > parachute.critical_sink())
               ) {

                if (parachute.sink_time() == 0) {
                    parachute.set_sink_time(AP_HAL::millis());
                }

                if ((time - parachute.sink_time()) >= 1000) {
                    parachute_release();
                    if (!release_reason_msg_sended) {
                        gcs().send_text(MAV_SEVERITY_ALERT, "Parachute released: critical sink reached");
                        release_reason_msg_sended = true;
                    }
                }

            } else {
                parachute.set_sink_time(0);
            }

            // check angles, only if we are higher then takeoff alt
            float baro_alt = barometer.get_altitude(); const float bmargin = 2;
            if (baro_alt > auto_state.baro_takeoff_alt + bmargin) {
                // check critical pitch
                if (abs(ahrs.pitch_sensor) >= parachute.critical_pitch()) {
                    parachute_release();
                    if (!release_reason_msg_sended) {
                        gcs().send_text(MAV_SEVERITY_ALERT, "Parachute released: Reached critical pitch angle");
                        release_reason_msg_sended = true;
                    }
                }

                // check critical roll
                if (abs(ahrs.roll_sensor) >= parachute.critical_roll()) {
                    parachute_release();
                    if (!release_reason_msg_sended) {
                        gcs().send_text(MAV_SEVERITY_ALERT, "Parachute released: Reached critical roll angle");
                        release_reason_msg_sended = true;
                    }
                }
            }
        }
    }

    // polling relative alt, to update release_alt_reached flag
    if (parachute.release_alt_reached(relative_altitude) && !chute_auto_ready_msg_sended) {
        gcs().send_text(MAV_SEVERITY_INFO, "Parachute: AUTO READY");
        chute_auto_ready_msg_sended = true;
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

    if (parachute.release_in_progress()) {
        return;
    }
    if (parachute.released()) {
        gcs().send_text(MAV_SEVERITY_DEBUG, "Parachute: Released again");
    } else {
        if (elevon_override_msg_sended == false) {
            gcs().send_text(MAV_SEVERITY_INFO,"Parachute: Disarmed, Elevon override");
            elevon_override_msg_sended = true;
        }
    }

    // release parachute
    parachute.release();

    /*
        #if LANDING_GEAR_ENABLED == ENABLED
            // deploy landing gear
            g2.landing_gear.set_position(AP_LandingGear::LandingGear_Deploy);
        #endif
    */
}

/*
  parachute_manual_release - trigger the release of the parachute,
  after performing some checks for pilot error checks if the vehicle
  is landed
*/
bool Plane::parachute_manual_release()
{
    // exit immediately if parachute is not enabled
    if (!parachute.enabled()) {
        return false;
    }
    /*
        if (parachute.alt_min() > 0 && relative_ground_altitude(false) < parachute.alt_min() &&
            auto_state.last_flying_ms > 0) {
            // Allow manual ground tests by only checking if flying too low if we've taken off
            gcs().send_text(MAV_SEVERITY_WARNING, "Parachute: Too low");
            return false;
        }
    */
    gcs().send_text(MAV_SEVERITY_WARNING, "Parachute: manual release");
    // if we get this far release parachute
    parachute_release();

    /*
        #if LANDING_GEAR_ENABLED == ENABLED
            // deploy landing gear
            g2.landing_gear.set_position(AP_LandingGear::LandingGear_Deploy);
        #endif
    */

    return true;
}
#endif
