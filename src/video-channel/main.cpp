#include <iostream>

#include "api/create_peerconnection_factory.h"

#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"

#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"

#include "media/base/video_adapter.h"
#include "media/base/video_broadcaster.h"
#include "pc/video_track_source.h"

static auto g_signal_thread = rtc::Thread::CreateWithSocketServer();

static auto g_peer_connection_factory = webrtc::CreatePeerConnectionFactory(
              nullptr /* network_thread */, nullptr /* worker_thread */,
              g_signal_thread.get()/* signaling_thread */, nullptr /* default_adm */,
              webrtc::CreateBuiltinAudioEncoderFactory(),
              webrtc::CreateBuiltinAudioDecoderFactory(),
              webrtc::CreateBuiltinVideoEncoderFactory(),
              webrtc::CreateBuiltinVideoDecoderFactory(),
              nullptr /* audio_mixer */, nullptr /* audio_processing */);

rtc::scoped_refptr<webrtc::PeerConnectionInterface> get_default_peer_connection(
        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory,
        rtc::Thread* signaling_thread, webrtc::PeerConnectionObserver* observer)
{
    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    config.enable_dtls_srtp = true;
    //webrtc::PeerConnectionInterface::IceServer server;
    //server.uri = GetPeerConnectionString();
    //config.servers.push_back(server);
    auto peer_connection = factory->CreatePeerConnection(
        config, nullptr, nullptr, observer);
    assert(peer_connection);
    return peer_connection;
}

class VideoSourceMock : public rtc::VideoSourceInterface<webrtc::VideoFrame> {
public:
    void start();
    void stop();
private:
    void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink,
                                 const rtc::VideoSinkWants& wants) override {
        broadcaster_.AddOrUpdateSink(sink, wants);
        (void) video_adapter_; //we willn't use adapter at this demo
    }
    void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override {
        broadcaster_.RemoveSink(sink);
        (void) video_adapter_; //we willn't use adapter at this demo
    }
private:
    rtc::VideoBroadcaster broadcaster_;
    cricket::VideoAdapter video_adapter_;
};

class VideoTrack : public webrtc::VideoTrackSource {
public:
protected:
    rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
        return my_source_.get();
    }
private:
    std::unique_ptr<VideoSourceMock> my_source_;
};


class DummySetSessionDescriptionObserver
    : public webrtc::SetSessionDescriptionObserver {
 public:
  static DummySetSessionDescriptionObserver* Create() {
    return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
  }
  virtual void OnSuccess() { }
  virtual void OnFailure(webrtc::RTCError error) { assert(false);}
};


class SimpleClient : public webrtc::DataChannelObserver,
    public webrtc::PeerConnectionObserver,
    public webrtc::CreateSessionDescriptionObserver{
public:
    SimpleClient()
        : peer_connection_(nullptr)
    {
    }
    ~SimpleClient()
    {
    }
    void start()
    {
        g_signal_thread->Invoke<void>(RTC_FROM_HERE, [this]()
        {
            peer_connection_->CreateOffer(
                this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
        });
    }
    void on_ice_candidate(const webrtc::IceCandidateInterface* candidate) {
        peer_connection_->AddIceCandidate(candidate);
    }
    void on_sdp(webrtc::SessionDescriptionInterface* desc) {
        peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(), desc);
        if(desc->GetType() == webrtc::SdpType::kOffer) {
            peer_connection_->CreateAnswer(
                    this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
        }
    }
protected:
    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override
    {
        peer_connection_->AddIceCandidate(candidate);
        std::string candidate_str;
        candidate->ToString(&candidate_str);
        /* sending ice to remote */
    }
    //unused
    void OnSignalingChange(
          webrtc::PeerConnectionInterface::SignalingState new_state) override {}
    void OnRenegotiationNeeded() override {}
    void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override {}
protected:
    virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc) override {
        peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create(), desc);

        std::string sdp_str;
        desc->ToString(&sdp_str);
        /* sending sdp to remote */
    }
    virtual void OnFailure(webrtc::RTCError error) {
        std::cout<<"[error] err:"<<error.message()<<std::endl;
        assert(false);
    }

private:
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
};

int main()
{
    auto video_souce = new rtc::RefCountedObject<VideoTrack>();

    auto video_track = g_peer_connection_factory->CreateVideoTrack("video", video_souce);

    auto peer_connection = get_default_peer_connection(g_peer_connection_factory, g_signal_thread.get(), nullptr);

    auto err = peer_connection->AddTrack(video_track, {"stream1"});

    assert(err.ok());

    return 0;
}
