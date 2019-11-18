// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/time/time.h"
#include "google/fhir/json_format.h"
#include "google/fhir/r4/profiles.h"
#include "proto/r4/core/resources/patient.pb.h"
#include "proto/myprofile/myprofile.pb.h"
#include "cc/example_utils.h"

using std::string;

using ::google::fhir::PrintFhirToJsonStringForAnalytics;
using ::fhirexamples::myprofile::DemoPatient;

// This is a comprehensive example that is meant to be run last of the C++
// examples!
// This example involves the full process of generating a custom profile with
// custom extensions, and ultimately uploading them to BigQuery.
// For instructions on setting up your workspace, see the top-level README.md
//
// To generate the myprofile.proto, run
// generate_definitions_and_protos.sh //proto/myprofile:myprofile
//
// To run:
// bazel build //cc:ProfilePatientsToCustomProfile
// bazel-bin/cc/ProfilePatientsToCustomProfile $WORKSPACE
//
// This generates $WORKSPACE/analytic/DemoPatient.analytic.ndjson
//
// Next, generate the analytic schema for demo patient:
// bazel build //java:GenerateBigQuerySchema.java $WORKSPACE
//
// Then, generate
int main(int argc, char** argv) {
  const std::string workspace = argv[1];

  absl::TimeZone time_zone;
  CHECK(absl::LoadTimeZone("America/Los_Angeles", &time_zone));

  const std::vector<DemoPatient>& patients =
      fhir_examples::ReadNdJsonFile<DemoPatient>(
          time_zone, absl::StrCat(workspace, "/ndjson/Patient.fhir.ndjson"));

  std::ofstream write_stream;
  write_stream.open(absl::StrCat(workspace, "/analytic/DemoPatient.analytic.ndjson"));

  for (const DemoPatient& patient : patients) {
    write_stream << PrintFhirToJsonStringForAnalytics(patient).ValueOrDie();
    write_stream << "\n";
  }
  write_stream.close();
}
