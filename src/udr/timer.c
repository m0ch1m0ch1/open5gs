/*
 * Copyright (C) 2019,2020 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "context.h"

static udr_timer_cfg_t g_udr_timer_cfg[MAX_NUM_OF_UDR_TIMER] = {
    [UDR_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL] =
        { .duration = ogs_time_from_sec(3) },
};

udr_timer_cfg_t *udr_timer_cfg(udr_timer_e id)
{
    ogs_assert(id < MAX_NUM_OF_UDR_TIMER);
    return &g_udr_timer_cfg[id];
}

const char *udr_timer_get_name(udr_timer_e id)
{
    switch (id) {
    case UDR_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL:
        return "UDR_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL";
    case UDR_TIMER_NF_INSTANCE_HEARTBEAT_INTERVAL:
        return "UDR_TIMER_NF_INSTANCE_HEARTBEAT_INTERVAL";
    case UDR_TIMER_NF_INSTANCE_HEARTBEAT:
        return "UDR_TIMER_NF_INSTANCE_HEARTBEAT";
    case UDR_TIMER_NF_INSTANCE_VALIDITY:
        return "UDR_TIMER_NF_INSTANCE_VALIDITY";
    case UDR_TIMER_SUBSCRIPTION_VALIDITY:
        return "UDR_TIMER_SUBSCRIPTION_VALIDITY";
    default: 
       break;
    }

    return "UNKNOWN_TIMER";
}

static void sbi_timer_send_event(int timer_id, void *data)
{
    int rv;
    udr_event_t *e = NULL;
    ogs_assert(data);

    switch (timer_id) {
    case UDR_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL:
    case UDR_TIMER_NF_INSTANCE_HEARTBEAT_INTERVAL:
    case UDR_TIMER_NF_INSTANCE_HEARTBEAT:
    case UDR_TIMER_NF_INSTANCE_VALIDITY:
    case UDR_TIMER_SUBSCRIPTION_VALIDITY:
        e = udr_event_new(UDR_EVT_SBI_TIMER);
        ogs_assert(e);
        e->timer_id = timer_id;
        e->sbi.data = data;
        break;
    default:
        ogs_fatal("Unknown timer id[%d]", timer_id);
        ogs_assert_if_reached();
        break;
    }

    rv = ogs_queue_push(udr_self()->queue, e);
    if (rv != OGS_OK) {
        ogs_warn("ogs_queue_push() failed [%d] in %s",
                (int)rv, udr_timer_get_name(e->timer_id));
        udr_event_free(e);
    }
}

void udr_timer_nf_instance_registration_interval(void *data)
{
    sbi_timer_send_event(UDR_TIMER_NF_INSTANCE_REGISTRATION_INTERVAL, data);
}

void udr_timer_nf_instance_heartbeat_interval(void *data)
{
    sbi_timer_send_event(UDR_TIMER_NF_INSTANCE_HEARTBEAT_INTERVAL, data);
}

void udr_timer_nf_instance_heartbeat(void *data)
{
    sbi_timer_send_event(UDR_TIMER_NF_INSTANCE_HEARTBEAT, data);
}

void udr_timer_nf_instance_validity(void *data)
{
    sbi_timer_send_event(UDR_TIMER_NF_INSTANCE_VALIDITY, data);
}

void udr_timer_subscription_validity(void *data)
{
    sbi_timer_send_event(UDR_TIMER_SUBSCRIPTION_VALIDITY, data);
}
