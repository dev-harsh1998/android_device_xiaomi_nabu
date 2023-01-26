/*
 * Copyright (C) 2020 LineageOS Project
 * Copyright (C) 2023 Yogesh Nimangre
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <vector>
#include <string>
#include <fstream>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include <android-base/properties.h>
#include <android-base/logging.h>

#include "property_service.h"
#include "vendor_init.h"


void property_override(char const prop[], char const value[], bool add = true) {
  prop_info *pi;
  pi = (prop_info *)__system_property_find(prop);
  if (pi)
    __system_property_update(pi, value, strlen(value));
  else if (add)
    __system_property_add(prop, strlen(prop), value, strlen(value));
}

void set_device_properties(const std::string model, const std::string name,
                           const std::string marketname) {
  const std::array<std::string, 7> props_order = {
      std::string(""), "bootimage.",  "odm.",   "product.",
      "system.",       "system_ext.", "vendor."};

  const auto set_ro_build_prop = [](
      const std::string &source, const std::string &prop,
      const std::string &value, bool product = true) {
    property_override(
        std::string((product) ? std::string("ro.product." + source + prop)
                              : std::string("ro." + source + prop))
            .c_str(),
        value.c_str());
  };
  for (auto &props : props_order) {
    set_ro_build_prop(props, "model", model);
    set_ro_build_prop(props, "name", name);
    set_ro_build_prop(props, "marketname", marketname);
  }
  property_override("bluetooth.device.default_name", model.c_str());
  property_override("vendor.usb.product_string", model.c_str());
}

void witch_nabu() {
  if ((::android::base::GetProperty("ro.boot.hwc", std::string("GLOBAL")) ==
       "GLOBAL")) {
    set_device_properties("21051182G", "nabu_global", "Xiaomi Pad 5");
  }
  else {
    set_device_properties("21051182C", "nabu_cn", "Xiaomi Pad 5");
  }
  property_override("ro.boot.hardware.revision", ::android::base::GetProperty("ro.boot.hwversion", "").c_str());
}

void vendor_load_properties() { witch_nabu(); }
