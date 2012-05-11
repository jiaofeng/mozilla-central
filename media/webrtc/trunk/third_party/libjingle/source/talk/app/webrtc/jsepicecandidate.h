/*
 * libjingle
 * Copyright 2012, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Implements the IceCandidateInterface.

#ifndef TALK_APP_WEBRTC_JSEPICECANDIDATE_H_
#define TALK_APP_WEBRTC_JSEPICECANDIDATE_H_

#include <string>

#include "talk/app/webrtc/jsep.h"
#include "talk/base/constructormagic.h"
#include "talk/p2p/base/candidate.h"

namespace webrtc {

class JsepIceCandidate : public IceCandidateInterface {
 public:
  explicit JsepIceCandidate(const std::string& label);
  JsepIceCandidate(const std::string& label,
                   const cricket::Candidate& candidate);
  ~JsepIceCandidate();
  bool Initialize(const std::string& sdp);
  void SetCandidate(const cricket::Candidate& candidate) {
    candidate_ = candidate;
  }

  virtual std::string label() const { return label_;}
  virtual const cricket::Candidate& candidate() const {
    return candidate_;
  }

  virtual bool ToString(std::string* out) const;

 private:
  std::string label_;
  cricket::Candidate candidate_;

  DISALLOW_COPY_AND_ASSIGN(JsepIceCandidate);
};

// Implementation of IceCandidateColletion.
// This implementation stores JsepIceCandidates.
class JsepCandidateColletion : public IceCandidateColletion {
 public:
  ~JsepCandidateColletion();
  virtual size_t count() const {
    return candidates_.size();
  }
  virtual bool HasCandidate(const IceCandidateInterface* candidate) const;
  // Adds and takes ownership of the JsepIceCandidate.
  virtual void add(JsepIceCandidate* candidate) {
    candidates_.push_back(candidate);
  }
  virtual const IceCandidateInterface* at(size_t index) const {
    return candidates_[index];
  }

 private:
  std::vector<JsepIceCandidate*> candidates_;
};

}  // namespace webrtc

#endif  // TALK_APP_WEBRTC_JSEPICECANDIDATE_H_
