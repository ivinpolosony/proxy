/* Copyright 2017 Istio Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MIXERCONTROL_TCP_ATTRIBUTES_BUILDER_H
#define MIXERCONTROL_TCP_ATTRIBUTES_BUILDER_H

#include "control/include/tcp/check_data.h"
#include "control/include/tcp/report_data.h"
#include "control/src/request_context.h"

namespace istio {
namespace mixer_control {
namespace tcp {

// The builder class to add TCP attributes.
class AttributesBuilder {
 public:
  AttributesBuilder(RequestContext* request) : request_(request) {}

  // Extract attributes for Check.
  void ExtractCheckAttributes(CheckData* check_data);
  // Extract attributes for Report.
  void ExtractReportAttributes(ReportData* report_data);

 private:
  // The request context object.
  RequestContext* request_;
};

}  // namespace tcp
}  // namespace mixer_control
}  // namespace istio

#endif  // MIXERCONTROL_TCP_ATTRIBUTES_BUILDER_H
