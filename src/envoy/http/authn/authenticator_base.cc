/* Copyright 2018 Istio Authors. All Rights Reserved.
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

#include "src/envoy/http/authn/authenticator_base.h"
#include "common/common/assert.h"
#include "common/config/metadata.h"
#include "src/envoy/http/authn/authn_utils.h"
#include "src/envoy/utils/filter_names.h"
#include "src/envoy/utils/utils.h"

using istio::authn::Payload;

namespace iaapi = istio::authentication::v1alpha1;

namespace Envoy {
namespace Http {
namespace Istio {
namespace AuthN {

namespace {
// The default header name for an exchanged token
static const std::string kExchangedTokenHeaderName = "ingress-authorization";

// Returns whether the header for an exchanged token is found
bool FindHeaderOfExchangedToken(const iaapi::Jwt& jwt) {
  return (jwt.jwt_headers_size() == 1 &&
          LowerCaseString(kExchangedTokenHeaderName) ==
              LowerCaseString(jwt.jwt_headers(0)));
}

}  // namespace

AuthenticatorBase::AuthenticatorBase(FilterContext* filter_context)
    : filter_context_(*filter_context) {}

AuthenticatorBase::~AuthenticatorBase() {}

bool AuthenticatorBase::validateX509(const iaapi::MutualTls& mtls,
                                     Payload* payload) const {
  const Network::Connection* connection = filter_context_.connection();
  if (connection == nullptr) {
    // It's wrong if connection does not exist.
    return false;
  }
  // Always try to get principal and set to output if available.
  const bool has_user =
      connection->ssl() != nullptr &&
      connection->ssl()->peerCertificatePresented() &&
      Utils::GetPrincipal(connection, true,
                          payload->mutable_x509()->mutable_user());

  ENVOY_LOG(debug, "validateX509 mode {}: ssl={}, has_user={}",
            iaapi::MutualTls::Mode_Name(mtls.mode()),
            connection->ssl() != nullptr, has_user);
  // Return value depend on mode:
  // - PERMISSIVE: plaintext connection is acceptable, thus return true
  // regardless.
  // - STRICT: must be TLS with valid certificate.
  switch (mtls.mode()) {
    case iaapi::MutualTls::PERMISSIVE:
      return true;
    case iaapi::MutualTls::STRICT:
      return has_user;
    default:
      NOT_REACHED_GCOVR_EXCL_LINE;
  }
}

bool AuthenticatorBase::validateJwt(const iaapi::Jwt& jwt, Payload* payload) {
  std::string jwt_payload;
  if (filter_context()->getJwtPayload(jwt.issuer(), &jwt_payload)) {
    std::string payload_to_process = jwt_payload;
    std::string original_payload;
    if (FindHeaderOfExchangedToken(jwt)) {
      if (AuthnUtils::ExtractOriginalPayload(jwt_payload, &original_payload)) {
        // When the header of an exchanged token is found and the token
        // contains the claim of the original payload, the original payload
        // is extracted and used as the token payload.
        payload_to_process = original_payload;
      } else {
        // When the header of an exchanged token is found but the token
        // does not contain the claim of the original payload, it
        // is regarded as an invalid exchanged token.
        ENVOY_LOG(
            error,
            "Expect exchanged-token with original payload claim. Received: {}",
            jwt_payload);
        return false;
      }
    }
    return AuthnUtils::ProcessJwtPayload(payload_to_process,
                                         payload->mutable_jwt());
  }
  return false;
}

}  // namespace AuthN
}  // namespace Istio
}  // namespace Http
}  // namespace Envoy
