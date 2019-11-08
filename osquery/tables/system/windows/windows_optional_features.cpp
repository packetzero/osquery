/**
 *  Copyright (c) 2014-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under both the Apache 2.0 license (found in the
 *  LICENSE file in the root directory of this source tree) and the GPLv2 (found
 *  in the COPYING file in the root directory of this source tree).
 *  You may select, at your option, one of the above-listed licenses.
 */

#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include <osquery/logger.h>
#include <osquery/tables.h>

#include "osquery/core/windows/wmi.h"

namespace osquery {
namespace tables {

std::string getDismPackageFeatureStateName(uint32_t state);

/**
 * return collection of Windows Features installation states.
 * On Window 10 Pro, returns about 140 rows.
 */
QueryData genWinOptionalFeatures(QueryContext& context) {
  QueryData results;

  const WmiRequest wmiReq(
      "SELECT Caption,Name,InstallState FROM Win32_OptionalFeature");
  const std::vector<WmiResultItem>& wmiResults = wmiReq.results();

  if (wmiResults.empty()) {
    return results;
  }

  for (unsigned int i = 0; i < wmiResults.size(); ++i) {
    Row r;
    uint32_t state = 99;

    wmiResults[i].GetString("Name", r["name"]);
    wmiResults[i].GetString("Caption", r["caption"]);

    // wbemtest.exe shows Column as UINT32, but comes in as I4.
    // For whatever reason, I4 is accessed using GetLong().

    if (wmiResults[i].GetUnsignedInt32("InstallState", state).ok() == false) {
      long i4;
      if (wmiResults[i].GetLong("InstallState", i4).ok()) {
        static_cast<uint32_t>(i4);
      }
    }
    r["state"] = INTEGER(state);

    r["statename"] = getDismPackageFeatureStateName(atoi(r["state"].c_str()));
    results.push_back(r);
  }
  return results;
}

/*
https://docs.microsoft.com/en-us/windows/desktop/CIMWin32Prov/win32-optionalfeature
Enabled (1)
Disabled (2)
Absent (3)
Unknown (4)
*/
std::string getDismPackageFeatureStateName(uint32_t state) {
  static std::vector<std::string> stateNames = {
      "Unknown", "Enabled", "Disabled", "Absent"};

  if (state >= stateNames.size()) {
    return "Unknown";
  }

  return stateNames[state];
}

} // namespace tables
} // namespace osquery
