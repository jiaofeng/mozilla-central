/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
 
#ifndef _PEER_CONNECTION_IMPL_H_
#define _PEER_CONNECTION_IMPL_H_

#include <string>
#include <vector>
#include <map>

#include "mozilla/RefPtr.h"
#include "nricectx.h"
#include "nricemediastream.h"

#include "prlock.h"
#include "PeerConnection.h"
#include "CallControlManager.h"
#include "CC_Device.h"
#include "CC_Call.h"
#include "CC_Observer.h"

namespace sipcc {

class LocalSourceStreamInfo : public mozilla::MediaStreamListener {
public:
   LocalSourceStreamInfo(nsRefPtr<nsDOMMediaStream>& aMediaStream);
  ~LocalSourceStreamInfo();
  
  /**
   * Notify that changes to one of the stream tracks have been queued.
   * aTrackEvents can be any combination of TRACK_EVENT_CREATED and
   * TRACK_EVENT_ENDED. aQueuedMedia is the data being added to the track
   * at aTrackOffset (relative to the start of the stream).
   */
  virtual void NotifyQueuedTrackChanges(
    mozilla::MediaStreamGraph* aGraph, 
    mozilla::TrackID aID,
    mozilla::TrackRate aTrackRate,
    mozilla::TrackTicks aTrackOffset,
    PRUint32 aTrackEvents,
    const mozilla::MediaSegment& aQueuedMedia);

  nsRefPtr<nsDOMMediaStream> GetMediaStream();
  void ExpectAudio();
  void ExpectVideo();
  unsigned AudioTrackCount();
  unsigned VideoTrackCount();
  
private:
  nsRefPtr<nsDOMMediaStream> mMediaStream;
  nsTArray<mozilla::TrackID> mAudioTracks;
  nsTArray<mozilla::TrackID> mVideoTracks;
};
  
class PeerConnectionImpl : public PeerConnectionInterface,
                           public sigslot::has_slots<> {
public:
  PeerConnectionImpl();
  ~PeerConnectionImpl();
    
  virtual StatusCode Initialize(PeerConnectionObserver* observer);
 
  // JSEP Calls
  virtual StatusCode CreateOffer(const std::string& hints);
  virtual StatusCode CreateAnswer(const std::string& hints, const  std::string& offer);
  virtual StatusCode SetLocalDescription(Action action, const  std::string& sdp);
  virtual StatusCode SetRemoteDescription(Action action, const std::string& sdp);
  virtual const std::string& localDescription() const;
  virtual const std::string& remoteDescription() const;
  
  virtual void AddStream(nsRefPtr<nsDOMMediaStream>& aMediaStream);
  virtual void RemoveStream(nsRefPtr<nsDOMMediaStream>& aMediaStream);
  virtual void CloseStreams();

  virtual void AddIceCandidate(const std::string& strCandidate); 

  virtual ReadyState ready_state();
  virtual SipccState sipcc_state();
  virtual IceState ice_state();
  
  virtual void Shutdown();
  
  // Implementation of the only observer we need
  virtual void onCallEvent(ccapi_call_event_e callEvent, CSF::CC_CallPtr call, CSF::CC_CallInfoPtr info);

  // Handle system to allow weak references to be passed through C code
  static PeerConnectionImpl *AcquireInstance(const std::string& handle);
  virtual void ReleaseInstance();
  virtual const std::string& GetHandle();

  // ICE events
  void IceGatheringCompleted(NrIceCtx *ctx);
  void IceCompleted(NrIceCtx *ctx);
  void IceStreamReady(NrIceMediaStream *stream);

  mozilla::RefPtr<NrIceCtx> ice_ctx() const { return mIceCtx; }
  mozilla::RefPtr<NrIceMediaStream> ice_media_stream(size_t i) const {
    // TODO(ekr@rtfm.com): If someone asks for a value that doesn't exist,
    // make one.
    if (i >= mIceStreams.size())
      return NULL;
             
    return mIceStreams[i];
  }


  
private:
  void ChangeReadyState(PeerConnectionInterface::ReadyState ready_state);

  PeerConnectionImpl(const PeerConnectionImpl&rhs);  
  PeerConnectionImpl& operator=(PeerConnectionImpl);   
  CSF::CC_CallPtr mCall;  
  PeerConnectionObserver* mPCObserver;
  ReadyState mReadyState;

  // The SDP sent in from JS - here for debugging.
  std::string mLocalRequestedSDP;
  std::string mRemoteRequestedSDP;
  // The SDP we are using.
  std::string mLocalSDP;
  std::string mRemoteSDP;
  
  // A list of streams returned from GetUserMedia
  PRLock *mLocalSourceStreamsLock;
  nsTArray<nsRefPtr<LocalSourceStreamInfo> > mLocalSourceStreams;

  // A handle to refer to this PC with
  std::string mHandle;

  // ICE objects
  mozilla::RefPtr<NrIceCtx> mIceCtx;
  std::vector<mozilla::RefPtr<NrIceMediaStream> > mIceStreams;
  IceState mIceState;

  // Singleton list of all the PeerConnections
  static std::map<const std::string, PeerConnectionImpl *> peerconnections;
};
 
}  // end sipcc namespace

#endif  // _PEER_CONNECTION_IMPL_H_
