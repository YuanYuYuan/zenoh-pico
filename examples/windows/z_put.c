//
// Copyright (c) 2022 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
//

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <zenoh-pico.h>

int main(int argc, char **argv) {
    (void)(argc);
    (void)(argv);
    const char *keyexpr = "demo/example/zenoh-pico-put";
    const char *value = "Pub from Pico!";
    const char *mode = "client";
    char *locator = NULL;

    z_owned_config_t config = z_config_default();
    zp_config_insert(z_loan(config), Z_CONFIG_MODE_KEY, z_string_make(mode));
    if (locator != NULL) {
        zp_config_insert(z_loan(config), Z_CONFIG_CONNECT_KEY, z_string_make(locator));
    }

    printf("Opening session...\n");
    z_owned_session_t s = z_open(z_move(config));
    if (!z_check(s)) {
        printf("Unable to open session!\n");
        return -1;
    }

    // Start read and lease tasks for zenoh-pico
    if (zp_start_read_task(z_loan(s), NULL) < 0 || zp_start_lease_task(z_loan(s), NULL) < 0) {
        printf("Unable to start read and lease tasks");
        return -1;
    }

    printf("Declaring key expression '%s'...\n", keyexpr);
    z_owned_keyexpr_t ke = z_declare_keyexpr(z_loan(s), z_keyexpr(keyexpr));
    if (!z_check(ke)) {
        printf("Unable to declare key expression!\n");
        return -1;
    }

    printf("Putting Data ('%s': '%s')...\n", keyexpr, value);
    z_put_options_t options = z_put_options_default();
    options.encoding = z_encoding(Z_ENCODING_PREFIX_TEXT_PLAIN, NULL);
    if (z_put(z_loan(s), z_loan(ke), (const uint8_t *)value, strlen(value), &options) < 0) {
        printf("Oh no! Put has failed...\n");
    }

    z_undeclare_keyexpr(z_loan(s), z_move(ke));

    // Stop read and lease tasks for zenoh-pico
    zp_stop_read_task(z_loan(s));
    zp_stop_lease_task(z_loan(s));

    z_close(z_move(s));
    return 0;
}
