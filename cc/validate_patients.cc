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
#include <random>
#include <stdlib.h>

#include "absl/strings/str_cat.h"
#include "absl/time/time.h"
#include "google/fhir/json_format.h"
#include "google/fhir/status/status.h"
#include "google/fhir/resource_validation.h"
#include "proto/r4/core/resources/patient.pb.h"
#include "proto/r4/uscore.pb.h"
#include "cc/example_utils.h"

using std::string;

using ::google::fhir::r4::core::Patient;
using ::google::fhir::JsonFhirStringToProto;
using ::google::fhir::r4::uscore::USCorePatientProfile;

// Example code for running resource validation
//
// To run:
// bazel build //cc:ValidatePatients
// bazel-bin/cc/ValidatePatients $WORKSPACE
//
// where $WORKSPACE is the location of a synthea dataset.
// For instructions on setting up your workspace, see the top-level README.md
int main(int argc, char** argv) {
  absl::TimeZone time_zone;
  CHECK(absl::LoadTimeZone("America/Los_Angeles", &time_zone));

  // Read all the synthea patients directly into USCore Patient protos.
  std::vector<USCorePatientProfile> patients =
      fhir_examples::ReadNdJsonFile<USCorePatientProfile>(
          time_zone, absl::StrCat(argv[1], "/ndjson/Patient.fhir.ndjson"));

  // All of the synthea patients are valid USCore Patients.
  // We know this is true, because if any failed to meet the requirements of
  // that profile, they would have failed the "ValueOrDie" check in
  // ReadNdJsonFile.
  // To demonstrate validation, let's make some of them invalid.
  std::default_random_engine generator;
  std::uniform_real_distribution<> distribution(0, 1);
  for (USCorePatientProfile& patient : patients) {
    if (distribution(generator) < .05) {
      // Gender is required by US Core
      patient.clear_gender();
    } else if (distribution(generator) < .05) {
      // Set a managingOrganization with a practitioner id.
      // This is invalid, because FHIR requires managingOrganization references
      // to refer to organizations.
      patient.mutable_managing_organization()
          ->mutable_practitioner_id()
          ->set_value("1234");
    }
  }

  for (USCorePatientProfile& patient : patients) {
    google::fhir::Status status = google::fhir::ValidateResource(patient);
    if (!status.ok()) {
      std::cout << "Patient " << patient.identifier(0).value().value()
                << " is invalid: " << status.error_message() << std::endl;
    }
  }
}